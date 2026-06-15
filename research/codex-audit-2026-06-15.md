# Codex Audit — 2026-06-15

First end-to-end audit of the on-chain Codex inscriptions vs. the canonical
corpus. Run from `contrib/codex/codex_audit.py`.

## Tooling

- Script: `contrib/codex/codex_audit.py` (vps3 local-RPC, no txindex required —
  pulls raw block hex via `getblock <hash> false` and parses the coinbase
  scriptSig directly).
- Corpus: `contrib/codex/codex_corpus.txt`,
  sha256 `1c808560eebab87f9e8c5032e5157821f1fbdae2bafc6ea68b6f4310a2d6d82f`,
  2,267,867 bytes → 47,248 chunks of 48B.
  Public mirror at `https://23skidoo.info/static/corpus_v1.bin` matches byte-for-byte.

## Snapshot (tip h=1,001,159)

```
Range audited:     999,991 .. 1,001,159  (1,169 blocks)
Descent verses OK: 1/10
Canon chunks OK:   1,159 / 1,159
Dreaming blocks:   0
Missing OFF1 egg:  9
Failures:          0
Rate:              ~133 blocks/sec via local RPC
```

## Findings

### Canon transcription is byte-clean

Every block from h=1,000,001 onward carries:
- The OFF1 magic prefix
- `chunk_idx == height - 1,000,001` (no skips, no drift)
- Text bytes identical to `corpus[chunk_idx*48 : (chunk_idx+1)*48]`

This is what we wanted: the chain is faithfully transcribing the Lovecraft
canon, one ~48-byte fragment per coinbase scriptSig, in strict height order.
At the current rate (~1 block/min), the canon completes in ~32 more days at
h≈1,047,249, after which `CodexFragment()` switches to `RlyehianVerse()` and
the chain begins to speak in tongues (Phase B — the Dreaming).

### Descent verses 999,991–999,999 are lost forever

Only the final Descent verse (h=1,000,000, `IA! IA! CTHULHU FHTAGN! Block
1000000: the Restoration is come.`) made it into a coinbase scriptSig. The
nine preceding ceremonial verses encoded in `src/miner.cpp::DescentLine()`
have no on-chain copy. The 9 missed verses are:

| Height | Verse |
|---|---|
| 999,991 | `The angles turn wrong in the deep. He stirs.` |
| 999,992 | `Pressure mounts; the black seas blacken further.` |
| 999,993 | `The Conclave gathers, candles guttering green.` |
| 999,994 | `R'lyeh's drowned spires breach the surface.` |
| 999,995 | `That is not dead which can eternal lie,` |
| 999,996 | `and with strange aeons even death may die.` |
| 999,997 | `The stars come right.` |
| 999,998 | `Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn.` |
| 999,999 | `He dreams no longer.` |

### Diagnosis: setgenerate-mined under an older binary

All 9 missing-Descent coinbases carry the `/P2SH/` tag, which is Bitcoin
Core 0.10's default for the internal `setgenerate=true` CPU-miner. The pool
(`/Miningcore/` tag) was not the source. So one of the Conclave-key
daemons (vps1 pool wallet or chaos) was running with `setgenerate=true`
**but did not yet have the v2.0.0 Codex code**.

The v2.0.0 binary with `CodexFragment(nHeight)` + `DescentLine(nHeight)`
came online somewhere between blocks 999,999 and 1,000,000. The Awakening
block itself (1,000,000) was found by the upgraded daemon — but the 9
ceremonial verses leading in were not.

### Cannot be retroactively inscribed

Those 9 heights are now buried 1,160+ deep, far past `MAX_REORG_DEPTH=100`.
The Descent verses survive only in `src/miner.cpp` and in this document.
On-chain readers (the `/codex/` site, future Phase-B walkers) will render
those heights as `(…)` ellipses.

## Re-running

```
python3 ~/claude/offerings-master/contrib/codex/codex_audit.py
```

Takes ~10s for ~1,200 blocks. As the canon transcription extends and the
chain enters Phase B, expect the script to start counting Dreaming blocks
instead of canon chunks past h≈1,047,249. Failures should remain zero
unless a non-Codex daemon ever lands a post-fork block — which is now
defended against by the cluster's pool-only OFFSIG window through
h=1,050,666 plus the pool's `BuildPoolCoinbase` path also writing the
Codex egg.
