#!/usr/bin/env python3
"""
Phase-2 ACP regtest harness (issue #40).

Spins up two regtest daemons. Node 0 holds the test-fixture ACP privkey
via -checkpointkey=<WIF>. Mines blocks, then calls `sendcheckpoint` to
sign and broadcast a checkpoint. Node 1 receives + accepts. Verifies:

  PHASE 1 — peer the nodes, mine past coinbase maturity, confirm both
    nodes agree on the tip.

  PHASE 2 — broadcaster calls `sendcheckpoint <target_hash>`. Signature
    is generated using the test-fixture privkey; pubkey baked at
    src/chainparams.cpp (regtest vConclaveKeys, slot #0).

  PHASE 3 — peer (node 1) receives the checkpoint message, validates
    the signature against Params().ConclaveKeys()[0], and persists
    hashSyncCheckpoint. Confirmed by grepping the peer's debug.log for
    the ProcessSyncCheckpoint: sync-checkpoint at <hash> log line.

Test fixture privkey is derived deterministically from
sha256("OFFv2-test-checkpointkey-2026!!!"). It is NOT a real Conclave
key — it is embedded in this file and in chainparams.cpp solely for
regtest. Do not reuse it for testnet or mainnet.
"""

import os
import sys
import json
import time
import shutil
import atexit
import tempfile
import subprocess

SRCDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "src"))
DAEMON = os.path.join(SRCDIR, "Offeringsd")
CLI    = os.path.join(SRCDIR, "Offerings-cli")

# Test fixture (chainparams.cpp regtest/testnet vConclaveKeys, slot #0).
# Privkey: sha256("OFFv2-test-checkpointkey-2026!!!")
# Pubkey (uncompressed): 04 07bfe025... 5c2b3e3f...  (65 bytes)
# WIF is uncompressed too (no 0x01 flag) — the pre-#40 ppcoin-derived signing
# path stores uncompressed master keys, and the privkey loaded from WIF must
# match that form for CKey::Sign output to verify against CPubKey::Verify.
TEST_CHECKPOINT_WIF = "9HWyiqthmTjXgUJwrmEk7NJY9XF9ShXADQhBYYEA2sSmRVejV3f"

NODES = []  # popen handles


def cleanup():
    for p in NODES:
        try:
            p.terminate()
        except Exception:
            pass
    for p in NODES:
        try:
            p.wait(timeout=5)
        except Exception:
            try:
                p.kill()
            except Exception:
                pass


atexit.register(cleanup)


def cli(datadir, *args):
    return subprocess.check_output(
        [CLI, "-regtest", "-datadir=" + datadir,
         "-rpcuser=test", "-rpcpassword=test"] + list(args),
        text=True, stderr=subprocess.STDOUT).strip()


def cli_try(datadir, *args):
    try:
        return cli(datadir, *args), None
    except subprocess.CalledProcessError as e:
        return None, e.output


def make_datadir(tmp, idx):
    d = os.path.join(tmp, "node%d" % idx)
    os.makedirs(d)
    rpc = 18570 + idx
    p2p = 18560 + idx
    with open(os.path.join(d, "Offerings.conf"), "w") as f:
        f.write("regtest=1\n")
        f.write("rpcuser=test\n")
        f.write("rpcpassword=test\n")
        f.write("rpcport=%d\n" % rpc)
        f.write("port=%d\n" % p2p)
    return d


def start_node(idx, datadir, extra_args):
    p2p = 18560 + idx
    rpc = 18570 + idx
    args = [DAEMON, "-regtest", "-datadir=" + datadir,
            "-rpcuser=test", "-rpcpassword=test",
            "-rpcport=%d" % rpc, "-port=%d" % p2p,
            "-server=1", "-listen=1", "-printtoconsole",
            "-keypool=1"] + extra_args
    log_path = os.path.join(datadir, "stdout.log")
    log_file = open(log_path, "w")
    NODES.append(subprocess.Popen(args, stdout=log_file, stderr=subprocess.STDOUT))
    # Wait up to 30s for RPC to come up
    for _ in range(60):
        out, err = cli_try(datadir, "getblockcount")
        if out is not None:
            return p2p, rpc
        time.sleep(0.5)
    raise RuntimeError("Node %d failed to start RPC (see %s)" % (idx, log_path))


def sync_blocks(datadirs, timeout=30):
    end = time.time() + timeout
    heights = []
    while time.time() < end:
        heights = [int(cli(d, "getblockcount")) for d in datadirs]
        if len(set(heights)) == 1:
            return heights[0]
        time.sleep(0.5)
    raise RuntimeError("Block sync timed out: heights=%s" % heights)


def main():
    print("=== Phase-2 ACP broadcast checkpoints (#40) — regtest harness ===\n")

    tmp = tempfile.mkdtemp(prefix="off-phase2-acp-")
    print("tmpdir: %s" % tmp)

    d0 = make_datadir(tmp, 0)
    d1 = make_datadir(tmp, 1)

    print("Starting broadcaster (node 0) with -checkpointkey=<test WIF> ...")
    p2p0, rpc0 = start_node(0, d0, ["-checkpointkey=" + TEST_CHECKPOINT_WIF])
    print("  node 0 up on p2p:%d rpc:%d" % (p2p0, rpc0))

    print("Starting peer (node 1) connecting to broadcaster ...")
    p2p1, rpc1 = start_node(1, d1, ["-connect=127.0.0.1:%d" % p2p0])
    print("  node 1 up on p2p:%d rpc:%d" % (p2p1, rpc1))

    # Give the connection a moment to settle
    time.sleep(3)
    conns0 = int(cli(d0, "getconnectioncount"))
    conns1 = int(cli(d1, "getconnectioncount"))
    print("  connections: node0=%d, node1=%d" % (conns0, conns1))
    if conns0 == 0 or conns1 == 0:
        raise RuntimeError("nodes failed to peer")

    print("\nPHASE 1 — mine 110 blocks on node 0, sync to node 1")
    cli(d0, "setgenerate", "true", "110")
    h = sync_blocks([d0, d1])
    print("  PASS  tip = %d on both nodes" % h)

    print("\nPHASE 2 — broadcaster signs + sends a checkpoint at tip-50")
    tip = int(cli(d0, "getblockcount"))
    target_h = tip - 50
    target_hash = cli(d0, "getblockhash", str(target_h))
    print("  target: h=%d hash=%s..." % (target_h, target_hash[:16]))

    out, err = cli_try(d0, "sendcheckpoint", target_hash)
    if out is None:
        raise RuntimeError("sendcheckpoint failed: %s" % err)
    print("  broadcaster: sendcheckpoint returned: %s" % (out or "(empty)"))

    print("\nPHASE 3 — verify peer accepts the checkpoint")
    # With -printtoconsole the daemon writes its log to stdout (captured to
    # stdout.log by start_node), NOT to regtest/debug.log. Grep stdout.log.
    log_path = os.path.join(d1, "stdout.log")
    needle = "ProcessSyncCheckpoint: sync-checkpoint at " + target_hash
    end = time.time() + 15
    received = False
    while time.time() < end:
        try:
            with open(log_path) as f:
                if needle in f.read():
                    received = True
                    break
        except IOError:
            pass
        time.sleep(0.5)

    if not received:
        raise RuntimeError(
            "node 1 did not accept the sync-checkpoint within 15s; "
            "check %s for ACP log lines" % log_path
        )
    print("  PASS  node 1 accepted sync-checkpoint at %s..." % target_hash[:16])

    # Bonus check: confirm broadcaster sees it too (local ProcessSyncCheckpoint(NULL))
    log0 = os.path.join(d0, "stdout.log")
    with open(log0) as f:
        if needle not in f.read():
            raise RuntimeError("broadcaster's local ProcessSyncCheckpoint didn't log acceptance")
    print("  PASS  broadcaster also processed locally")

    print("\n=== ALL PHASE-2 ACP CHECKS PASSED ===")
    print("(Negative test — forks past the checkpoint must be rejected — is TODO; "
          "requires building a competing chain fixture.)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
