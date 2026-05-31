#!/usr/bin/env python3
"""
Build the three static JSON indexes the Bridge portal needs.

Walks ~/.Offering/blocks/blk*.dat directly (no daemon RPC dependency, no
-txindex requirement). Reconstructs the chain by following prev-block
pointers from genesis. Filters out anything past block 966,413 — that's
the canonical pre-Restoration chainstate tip.

Outputs to ./indexes/:
  miners_index.json           addr -> [{height, txid}, ...]   coinbase recipients
  chainstate_utxo_index.json  addr -> sats balance at block 966,413
  banlist.json                {banned: [...], banned_one_hop: [...]}

Usage:
  python3 build_portal_indexes.py
  python3 build_portal_indexes.py --blocks-dir /custom/path --out /custom/dir
"""

import argparse, hashlib, json, os, struct, sys, time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

try:
    import requests
except ImportError:
    print("ERROR: needs `requests`. Install via `apt install python3-requests`.", file=sys.stderr)
    sys.exit(2)

# OpenSSL 3 removed RIPEMD-160 from default providers; load the legacy provider
# at startup so hashlib.new('ripemd160') works for P2PK address derivation.
def _enable_ripemd160():
    try:
        hashlib.new('ripemd160')
        return  # already works
    except Exception:
        pass
    try:
        import ctypes, ctypes.util
        lib = ctypes.CDLL(ctypes.util.find_library('crypto'))
        if lib.OSSL_PROVIDER_try_load(None, b'legacy', 1):
            hashlib.new('ripemd160')  # verify
            return
    except Exception:
        pass
    print("WARNING: RIPEMD-160 unavailable; P2PK addresses will be skipped", file=sys.stderr)
_enable_ripemd160()

# OFF chain constants (verified from chainparams.cpp via offerings-facts.md)
MAGIC = b'\x03\xa5\xfe\xdd'
ADDR_VERSION_P2PKH = 58   # Q-prefix
ADDR_VERSION_P2SH  = 9    # 4-prefix
GENESIS_HASH_HEX   = '000006829ac5ad04fb30abfcbf6d927c67c30fc2f198fb0bdce5a0c914b091b5'
CHAINSTATE_TIP     = 966413

# #699 attacker banlist (vampirus, BCT 2018-11-21)
BANNED_ADDRS = [
    'QTLUPH9b4dRQdz9uKB7GreMvHPA8iyDoQY',
    'QeHkx6jFvStkzaVaSTtfPrSAwwrqMgauP8',
    'QgynW4zGXyjhG3DQHn9vBuHwNp4c4xqtgM',
    'QjfP4o7o2TszP5Ph4TmNVmktzDCjYkq2xj',
    'QM8ZeuBDwrhya9BHQfNKifEzfwUhyh7Tji',
    'Qb6jxfUmfWHh7XTTRWKBoiZ43sSNTJrw8J',
    'QireWv3upmhVuRMcE6u7h81gmhWfiGEyTt',
    'QSJU4tDNsZiaNcUuBWYcvjqKWoB8EHDVsT',
]

# ---------- base58 + address helpers ----------

B58_ALPHA = '123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz'

def b58encode(b: bytes) -> str:
    n = int.from_bytes(b, 'big')
    out = ''
    while n > 0:
        n, r = divmod(n, 58)
        out = B58_ALPHA[r] + out
    # leading zeros -> leading '1's
    pad = 0
    for c in b:
        if c == 0: pad += 1
        else: break
    return '1' * pad + out

def b58check(version: int, payload20: bytes) -> str:
    body = bytes([version]) + payload20
    chk = hashlib.sha256(hashlib.sha256(body).digest()).digest()[:4]
    return b58encode(body + chk)

def hash160(b: bytes) -> bytes:
    return hashlib.new('ripemd160', hashlib.sha256(b).digest()).digest()

def addr_from_script(script: bytes):
    """Extract an OFF address from a standard scriptPubKey. Returns str or None."""
    n = len(script)
    # P2PKH: OP_DUP OP_HASH160 0x14 <20> OP_EQUALVERIFY OP_CHECKSIG  (25 bytes)
    if n == 25 and script[0] == 0x76 and script[1] == 0xa9 and script[2] == 0x14 \
            and script[23] == 0x88 and script[24] == 0xac:
        return b58check(ADDR_VERSION_P2PKH, script[3:23])
    # P2SH: OP_HASH160 0x14 <20> OP_EQUAL  (23 bytes)
    if n == 23 and script[0] == 0xa9 and script[1] == 0x14 and script[22] == 0x87:
        return b58check(ADDR_VERSION_P2SH, script[2:22])
    # P2PK uncompressed: 0x41 <65> OP_CHECKSIG  (67 bytes)
    if n == 67 and script[0] == 0x41 and script[66] == 0xac:
        return b58check(ADDR_VERSION_P2PKH, hash160(script[1:66]))
    # P2PK compressed: 0x21 <33> OP_CHECKSIG  (35 bytes)
    if n == 35 and script[0] == 0x21 and script[34] == 0xac:
        return b58check(ADDR_VERSION_P2PKH, hash160(script[1:34]))
    # Bare multisig, OP_RETURN, non-standard, premine scriptPubKey -> ignore
    return None

# ---------- block / tx parsing ----------

class ParseError(Exception): pass

class Cursor:
    __slots__ = ('buf', 'pos', 'end')
    def __init__(self, buf, pos=0, end=None):
        self.buf = buf
        self.pos = pos
        self.end = len(buf) if end is None else end
    def _check(self, n):
        if self.pos + n > self.end:
            raise ParseError(f"read past end (pos={self.pos} need={n} end={self.end})")
    def read(self, n):
        self._check(n)
        r = self.buf[self.pos:self.pos+n]
        self.pos += n
        return r
    def u8(self):  self._check(1); v=self.buf[self.pos]; self.pos+=1; return v
    def u16(self): self._check(2); v=struct.unpack_from('<H', self.buf, self.pos)[0]; self.pos+=2; return v
    def u32(self): self._check(4); v=struct.unpack_from('<I', self.buf, self.pos)[0]; self.pos+=4; return v
    def u64(self): self._check(8); v=struct.unpack_from('<Q', self.buf, self.pos)[0]; self.pos+=8; return v
    def varint(self, max_value=None):
        n = self.u8()
        if n == 0xfd:   v = self.u16()
        elif n == 0xfe: v = self.u32()
        elif n == 0xff: v = self.u64()
        else: v = n
        if max_value is not None and v > max_value:
            raise ParseError(f"varint {v} exceeds max {max_value}")
        return v

MAX_INPUTS  = 100_000
MAX_OUTPUTS = 100_000
MAX_SCRIPT  = 1_000_000  # Bitcoin caps script at 10K but be lenient

def parse_tx(cur: Cursor):
    """Parse one tx (Bitcoin v1, no segwit). Returns (txid_bytes, [vouts], is_coinbase, spent_inputs)."""
    tx_start = cur.pos
    version = cur.u32()
    input_count = cur.varint(max_value=MAX_INPUTS)
    is_coinbase = False
    spent = []
    for i in range(input_count):
        prev_txid = cur.read(32)
        prev_vout = cur.u32()
        script_len = cur.varint(max_value=MAX_SCRIPT)
        cur.read(script_len)
        seq = cur.u32()
        if prev_vout == 0xFFFFFFFF and prev_txid == b'\x00' * 32:
            is_coinbase = True
        else:
            # NOTE: prev_txid is LE (file-order); utxo dict keys txids as BE.
            # Reverse here so utxo.pop() matches.
            spent.append((prev_txid[::-1], prev_vout))
    output_count = cur.varint(max_value=MAX_OUTPUTS)
    vouts = []
    for n in range(output_count):
        value = cur.u64()
        spk_len = cur.varint(max_value=MAX_SCRIPT)
        spk = bytes(cur.read(spk_len))
        vouts.append((value, spk))
    cur.u32()  # locktime
    # OFF v2+ tx has strTxComment after locktime (non-standard field — see core.h:201)
    if version >= 2:
        comment_len = cur.varint(max_value=MAX_SCRIPT)
        cur.read(comment_len)
    tx_raw = bytes(cur.buf[tx_start:cur.pos])
    txid = hashlib.sha256(hashlib.sha256(tx_raw).digest()).digest()[::-1]  # internal byte order -> txid
    return txid, vouts, is_coinbase, spent

def parse_block_from_buf(buf, pos, slice_end=None):
    """Parse one block starting at pos (after magic+size). Returns (hash, prev_hash, [txs], end_pos)."""
    cur = Cursor(buf, pos, end=slice_end)
    header_start = cur.pos
    cur.u32()                 # version
    prev_hash = cur.read(32)  # internal LE order
    cur.read(32)              # merkle root
    cur.u32()                 # time
    cur.u32()                 # bits
    cur.u32()                 # nonce
    header = bytes(buf[header_start:cur.pos])
    # NOTE: OFF block hash is HashScrypt(header), NOT double-SHA256.
    # For chain reconstruction we don't need the PoW hash —
    # we only need a hash that uniquely identifies the block.
    # Use double-SHA256 of header as an INTERNAL identity hash for the dict;
    # we cross-reference with daemon RPC for the few we need by real hash.
    # Actually, OFF still uses SHA256(SHA256(header)) for the BLOCK IDENTITY hash;
    # only the PoW check uses scrypt. So this works as the actual block hash.
    block_hash = hashlib.sha256(hashlib.sha256(header).digest()).digest()
    tx_count = cur.varint(max_value=100_000)
    txs = []
    for i in range(tx_count):
        txs.append(parse_tx(cur))
    return block_hash, bytes(prev_hash), txs, cur.pos

MAX_BLOCK_SIZE = 4_000_000  # OFF blocks are tiny in practice

def index_block_positions(path, by_prev):
    """Pass 1: scan + FULL parse-then-discard for validation. Records only
    (filename, offset, size, prev_hash) per block — no tx data retained.
    Full parse ensures phase 3 will never see an unparseable 'block' from a
    false-magic match."""
    data = path.read_bytes()
    n = len(data)
    pos = 0
    bad = 0
    found = 0
    fname = str(path)
    while pos + 8 <= n:
        mpos = data.find(MAGIC, pos)
        if mpos < 0:
            break
        pos = mpos
        if pos + 8 > n:
            break
        block_size = struct.unpack_from('<I', data, pos+4)[0]
        if block_size == 0 or block_size > MAX_BLOCK_SIZE:
            pos += 4
            continue
        if pos + 8 + block_size > n:
            break
        try:
            # Full parse to validate; discard txs immediately
            _bh, prev_hash, _txs, _e = parse_block_from_buf(
                data, pos + 8, slice_end=pos + 8 + block_size)
            by_prev.setdefault(prev_hash, []).append((fname, pos + 8, block_size))
            found += 1
            pos += 8 + block_size
        except (ParseError, IndexError, struct.error):
            bad += 1
            pos += 4
            continue
    if bad:
        print(f"   (skipped {bad} false-magic candidates in {path.name})", flush=True)
    return found

def parse_block_at(fname, offset, size):
    """Pass 3: open file, parse one block at the recorded offset."""
    with open(fname, 'rb') as f:
        f.seek(offset)
        buf = f.read(size)
    return parse_block_from_buf(buf, 0, slice_end=size)

# ---------- RPC helpers ----------

def _load_rpc_creds():
    conf_paths = [
        Path.home() / '.Offering' / 'Offerings.conf',
        Path.home() / '.Offering' / 'Offering.conf',
    ]
    conf = None
    for p in conf_paths:
        if p.exists():
            conf = p
            break
    if not conf:
        raise RuntimeError(f"Offerings.conf not found in {conf_paths}")
    settings = {}
    for line in conf.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith('#') or '=' not in line:
            continue
        k, v = line.split('=', 1)
        settings[k.strip()] = v.strip()
    user = settings.get('rpcuser') or 'rpcuser'
    pw   = settings.get('rpcpassword') or ''
    port = settings.get('rpcport') or '11928'
    host = settings.get('rpcbind') or '127.0.0.1'
    url  = f'http://{host}:{port}/'
    return url, (user, pw)

def _rpc_one(session, url, auth, method, params):
    r = session.post(url, json={'jsonrpc': '1.0', 'id': 'idx', 'method': method, 'params': params},
                     auth=auth, timeout=30)
    r.raise_for_status()
    j = r.json()
    if j.get('error'):
        raise RuntimeError(f"RPC error: {j['error']}")
    return j['result']

def _fetch_heights_to_hashes(url, auth, start, end_inclusive, workers=4, chunk=10_000):
    """Threaded getblockhash for heights start..end_inclusive. Chunks submissions
    to keep the in-flight Future count bounded (avoids OOM at large N)."""
    result = {}
    session = requests.Session()
    session.headers.update({'content-type': 'application/json'})

    def fetch(h):
        return h, _rpc_one(session, url, auth, 'getblockhash', [h])

    total = end_inclusive + 1 - start
    t0 = time.time()
    last_report = t0
    done = 0
    for chunk_start in range(start, end_inclusive + 1, chunk):
        chunk_end = min(chunk_start + chunk, end_inclusive + 1)
        heights = list(range(chunk_start, chunk_end))
        with ThreadPoolExecutor(max_workers=workers) as ex:
            futures = [ex.submit(fetch, h) for h in heights]
            for fut in as_completed(futures):
                h, hh = fut.result()
                result[h] = hh
                done += 1
                if time.time() - last_report > 3.0:
                    rate = done / (time.time() - t0)
                    eta_s = (total - done) / rate if rate > 0 else 0
                    print(f"   {done:,}/{total:,} ({rate:.0f}/s, ETA {eta_s/60:.1f} min)", flush=True)
                    last_report = time.time()
    print(f"   completed in {time.time()-t0:.1f}s ({done/(time.time()-t0):.0f}/s)", flush=True)
    return result

# ---------- main walker ----------

def build_indexes(blocks_dir: Path, out_dir: Path, cutoff_height: int):
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"[1/4] Indexing block positions in {blocks_dir} (no tx parse) ...", flush=True)
    t0 = time.time()
    by_prev = {}  # prev_hash_bytes -> [(filename, offset, size), ...]
    total = 0
    for blk_file in sorted(blocks_dir.glob('blk*.dat')):
        t1 = time.time()
        n = index_block_positions(blk_file, by_prev)
        total += n
        print(f"   {blk_file.name}: +{n} block positions  ({time.time()-t1:.1f}s)", flush=True)
    print(f"   total: {total} block positions indexed in {time.time()-t0:.1f}s", flush=True)

    print(f"[2/4] Fetching height->quark_hash map from daemon via RPC ...", flush=True)
    # OFF uses Quark Hash9 for block-identity hashing (not SHA256d), so we can't
    # compute block hashes locally. Daemon already knows them — fetch in parallel.
    hash_cache_path = out_dir / f'_height_to_hash_cache_{cutoff_height}.json'
    if hash_cache_path.exists():
        print(f"   using cached {hash_cache_path}", flush=True)
        height_to_hash = {int(k): v for k, v in json.loads(hash_cache_path.read_text()).items()}
    else:
        rpc_url, rpc_auth = _load_rpc_creds()
        height_to_hash = _fetch_heights_to_hashes(rpc_url, rpc_auth, 0, cutoff_height)
        hash_cache_path.write_text(json.dumps({str(k): v for k, v in height_to_hash.items()}))
        print(f"   cached to {hash_cache_path}", flush=True)
    print(f"   got {len(height_to_hash):,} (height, quark_hash) pairs", flush=True)

    # Match each height to a block position via prev_hash linkage.
    # chain_order is just a sequence of (height, file_position_tuple).
    print(f"   matching heights to block positions ...", flush=True)
    chain_order = []  # list of (height, (fname, offset, size))
    # Genesis (height 0): its prev_hash field is zeros
    zero_match = by_prev.get(b'\x00' * 32, [])
    if not zero_match:
        print("   ERROR: no block with prev_hash == zeros (genesis)", file=sys.stderr)
        sys.exit(1)
    chain_order.append((0, zero_match[0]))
    missing = 0
    multi = 0
    for h in range(1, cutoff_height + 1):
        parent_hash_le = bytes.fromhex(height_to_hash[h - 1])[::-1]
        matches = by_prev.get(parent_hash_le, [])
        if not matches:
            missing += 1
            continue
        if len(matches) > 1:
            multi += 1  # orphan competition; pick first
        chain_order.append((h, matches[0]))
    print(f"   chain reconstructed: {len(chain_order):,} blocks matched, {missing:,} missing, {multi:,} orphan-competition heights", flush=True)
    if len(chain_order) < (cutoff_height + 1) * 0.99:
        print(f"   WARNING: matched {len(chain_order)}/{cutoff_height+1} — chain has significant gaps", flush=True)
    # Free the prev->positions index now that chain is ordered (positions retained in chain_order)
    by_prev.clear()

    print(f"[3/4] Walking txs to build miners + UTXO + address-first-seen indexes (cutoff {cutoff_height}) ...", flush=True)
    miners = {}        # addr -> [(height, txid_hex)]
    utxo = {}          # (txid_bytes, vout_idx) -> (addr, value_sats)
    # Per-address: [first_seen_height, n_appearances_as_recipient]
    addr_meta = {}
    t2 = time.time()
    last_report = t2
    tx_count_total = 0
    # Open each blk file once, cache the buffer to amortize disk I/O
    file_cache = {}
    def get_buf(fname):
        if fname not in file_cache:
            with open(fname, 'rb') as f:
                file_cache[fname] = f.read()
            if len(file_cache) > 1:
                # Drop oldest cached file to bound memory (each .dat is ~134MB)
                oldest = next(iter(file_cache))
                if oldest != fname:
                    del file_cache[oldest]
        return file_cache[fname]
    skipped_bad = 0
    for height, (fname, offset, size) in chain_order:
        if height > cutoff_height:
            break
        buf = get_buf(fname)
        try:
            _bhash, _prev, txs, _end = parse_block_from_buf(buf, offset, slice_end=offset + size)
        except (ParseError, IndexError, struct.error):
            skipped_bad += 1
            continue
        for txid_be, vouts, is_coinbase, spent in txs:
            tx_count_total += 1
            txid_hex = txid_be.hex()
            for n, (value, spk) in enumerate(vouts):
                addr = addr_from_script(spk)
                if addr is None:
                    continue
                utxo[(txid_be, n)] = (addr, value)
                # Track first_seen + appearance count for the recognition formula
                meta = addr_meta.get(addr)
                if meta is None:
                    addr_meta[addr] = [height, 1]
                else:
                    meta[1] += 1
                if is_coinbase:
                    miners.setdefault(addr, []).append({'height': height, 'txid': txid_hex})
            # Mark spent UTXOs gone
            for stxid, svout in spent:
                utxo.pop((stxid, svout), None)
        if time.time() - last_report > 5.0:
            print(f"   height {height}/{cutoff_height}  utxos={len(utxo):,}  miners_uniq={len(miners):,}  txs={tx_count_total:,}", flush=True)
            last_report = time.time()
    file_cache.clear()
    print(f"   walk complete in {time.time()-t2:.1f}s  txs={tx_count_total:,}  utxos@tip={len(utxo):,}  skipped_bad={skipped_bad}", flush=True)

    print(f"[4/4] Aggregating balances + computing 1-hop downstream banlist ...", flush=True)
    # Per-address balance from UTXO set
    balances = {}
    for (txid, n), (addr, value) in utxo.items():
        balances[addr] = balances.get(addr, 0) + value
    utxo.clear()  # free ~600MB before two more chain walks
    # 1-hop downstream from banlist: any address that received from any banned addr.
    # Two passes over chain via disk re-read: build tx_out_addr, then find spends-from-banned.
    banned_set = set(BANNED_ADDRS)
    one_hop = set()
    tx_out_addr = {}  # (txid_bytes, vout) -> addr — only for outputs that paid a banned addr
    # Pass A: find which (txid, vout) outputs ever paid a banned addr
    file_cache_a = {}
    def get_buf_a(fname):
        if fname not in file_cache_a:
            if len(file_cache_a) > 0:
                file_cache_a.pop(next(iter(file_cache_a)))
            with open(fname, 'rb') as f:
                file_cache_a[fname] = f.read()
        return file_cache_a[fname]
    for height, (fname, offset, size) in chain_order:
        if height > cutoff_height: break
        buf = get_buf_a(fname)
        try:
            _bh, _prev, txs, _e = parse_block_from_buf(buf, offset, slice_end=offset + size)
        except (ParseError, IndexError, struct.error):
            continue
        for txid_be, vouts, is_coinbase, spent in txs:
            for n, (value, spk) in enumerate(vouts):
                addr = addr_from_script(spk)
                if addr in banned_set:
                    tx_out_addr[(txid_be, n)] = addr
    file_cache_a.clear()
    # Pass B: find txs that SPEND a tx_out_addr-tagged (banned-paid) output
    file_cache_b = {}
    def get_buf_b(fname):
        if fname not in file_cache_b:
            if len(file_cache_b) > 0:
                file_cache_b.pop(next(iter(file_cache_b)))
            with open(fname, 'rb') as f:
                file_cache_b[fname] = f.read()
        return file_cache_b[fname]
    for height, (fname, offset, size) in chain_order:
        if height > cutoff_height: break
        buf = get_buf_b(fname)
        try:
            _bh, _prev, txs, _e = parse_block_from_buf(buf, offset, slice_end=offset + size)
        except (ParseError, IndexError, struct.error):
            continue
        for txid_be, vouts, is_coinbase, spent in txs:
            if is_coinbase: continue
            spent_from_banned = any((stxid, svout) in tx_out_addr for stxid, svout in spent)
            if spent_from_banned:
                for n, (value, spk) in enumerate(vouts):
                    addr = addr_from_script(spk)
                    if addr and addr not in banned_set:
                        one_hop.add(addr)
    file_cache_b.clear()
    print(f"   balances: {len(balances):,} unique addrs   1-hop downstream: {len(one_hop):,}", flush=True)

    # Write outputs — stream per-key to avoid building giant intermediate strings.
    # Free large intermediate state before writes start.
    height_to_hash.clear()
    chain_order.clear()
    tx_out_addr.clear()

    def stream_write_dict(path, d, value_dumper):
        """Stream-write a flat dict to JSON: {key1:val1,key2:val2,...}"""
        with open(path, 'w') as f:
            f.write('{')
            first = True
            for k, v in d.items():
                if not first: f.write(',')
                first = False
                f.write(json.dumps(k))
                f.write(':')
                f.write(value_dumper(v))
            f.write('}')

    miners_path = out_dir / 'miners_index.json'
    utxo_path   = out_dir / 'chainstate_utxo_index.json'
    ban_path    = out_dir / 'banlist.json'
    addr_path   = out_dir / 'address_index.json'
    n_miners = len(miners)
    n_balances = len(balances)
    n_addrs = len(addr_meta)
    print(f"Writing {miners_path}", flush=True)
    stream_write_dict(miners_path, miners, lambda v: json.dumps(v, separators=(',', ':')))
    miners.clear()
    print(f"Writing {utxo_path}", flush=True)
    stream_write_dict(utxo_path, balances, lambda v: str(v))
    print(f"Writing {addr_path}", flush=True)
    # Per-address: [first_seen, n_appearances, balance_sats]
    with open(addr_path, 'w') as f:
        f.write('{')
        first = True
        for addr, meta in addr_meta.items():
            if not first: f.write(',')
            first = False
            bal = balances.get(addr, 0)
            f.write(json.dumps(addr))
            f.write(f':[{meta[0]},{meta[1]},{bal}]')
        f.write('}')
    addr_meta.clear()
    balances.clear()
    print(f"Writing {ban_path}", flush=True)
    ban_path.write_text(json.dumps({
        'banned': sorted(banned_set),
        'banned_one_hop': sorted(one_hop),
        'source': 'BCT msg #699 (vampirus, 2018-11-21)',
        'computed_at_height': cutoff_height,
    }, indent=2))

    # Quick stats
    print()
    print(f"=== INDEX BUILD COMPLETE ===")
    print(f"  miners_index.json:           {miners_path.stat().st_size/1024:.1f} KB  ({n_miners:,} unique miner addrs)")
    print(f"  chainstate_utxo_index.json:  {utxo_path.stat().st_size/1024:.1f} KB  ({n_balances:,} addrs with balance)")
    print(f"  address_index.json:          {addr_path.stat().st_size/1024:.1f} KB  ({n_addrs:,} addrs with first-seen tracking)")
    print(f"  banlist.json:                {ban_path.stat().st_size/1024:.1f} KB  ({len(one_hop):,} one-hop downstream)")
    print(f"  TOTAL TIME: {time.time()-t0:.1f}s")

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--blocks-dir', default=os.path.expanduser('~/.Offering/blocks'))
    p.add_argument('--out', default=os.path.join(os.path.dirname(__file__), 'indexes'))
    p.add_argument('--cutoff-height', type=int, default=CHAINSTATE_TIP)
    args = p.parse_args()
    build_indexes(Path(args.blocks_dir), Path(args.out), args.cutoff_height)

if __name__ == '__main__':
    main()
