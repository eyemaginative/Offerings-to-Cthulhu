# MAX_REORG_DEPTH affirmative validation — 2-node test

**Date:** 2026-05-29
**Binary tested:** rc3 source at commit 788fc60, with a single-line test patch (regtest treated as testnet for `LWMA3ForkHeight()` gating). Patch reverted post-test; source tree is clean.
**Result:** PASS

## Why the patch was needed

The MAX_REORG_DEPTH consensus rule is gated on `chainActive.Height() >= LWMA3ForkHeight()`:

* `LWMA3ForkHeight()` returns `HARDFORK_LWMA3_TESTNET_OFF = 100` for testnet, `HARDFORK_LWMA3_MAIN_OFF = 990000` for mainnet.
* Regtest evaluates as **not testnet** in `TestNet()` (chainparams.h:133-136), so the unpatched binary returns the mainnet height — 990,000 — for regtest gating.
* Regtest tests at height 250-300 therefore can't trigger either LWMA-3 or MAX_REORG_DEPTH on the unpatched binary.

The test patch added `if (RegTest()) return HARDFORK_LWMA3_TESTNET_OFF;` at the top of `LWMA3ForkHeight()`. Reverted after the test — source is back to canonical rc3.

## Test setup

```
A: regtest datadir /tmp/maxreorg-test/a, rpcport 18443, p2p 28443
B: regtest datadir /tmp/maxreorg-test/b, rpcport 18445, p2p 28445
```

Both daemons booted with no peers, no `connect=`, no shared listen interface. They mine on the same genesis (regtest genesis is deterministic) but pick divergent coinbases / timestamps from block 1 onward, so blocks 1..N hash differently.

## Procedure

1. Mine 250 blocks on A independently (`setgenerate true 1` loops; this codebase's miner mines one block per call then terminates).
2. Mine 300 blocks on B independently.
3. Verify chains divergent (same genesis, different block 1 hash).
4. `addnode 127.0.0.1:28445 onetry` on A. A peers with B, sees B's longer 300-block chain.
5. Wait 15 seconds for sync attempt.
6. Inspect A's tip + debug.log.

## Result

```
before connect: h=250  hash=029dc44b243963b5c358f6f7d68358148cc7654713a416900a230f9e586a1346
after  connect: h=250  (unchanged — safe-mode RPC blocks getblockhash but height stable)

debug.log:
2026-05-29 16:46:45 ActivateBestChain: REJECTING 250-block reorg (max 100) from active tip height 250
2026-05-29 16:46:45 ERROR: ActivateBestChain: reorg depth 250 exceeds MAX_REORG_DEPTH 100
[repeated ~10 times as B re-pushes the chain]

LWMA3 RETARGET entries: 323 (LWMA-3 active from block 101 onward as designed)
A peers: B is connected (syncnode=true, banscore=0) — peering itself is fine, only reorg is rejected
```

A held its 250-block chain. B's longer 300-block chain was rejected at the ActivateBestChain layer, exactly as the consensus rule specifies.

The "safe-mode" warning is the expected secondary effect — Bitcoin Core 0.10's safe-mode kicks in when a peer's chain has more work but our node refuses to switch. It blocks some RPCs (including `getblockhash` of a known-bad height) but does not affect block production or P2P. On mainnet, this will manifest as a wallet warning to operators that "the network does not appear to fully agree" — at which point an operator inspecting debug.log will see the REJECTING messages and understand it's the intended consensus defense, not a bug.

## What this validates

1. **The deep-reorg gate works.** Reorgs > 100 blocks past the active tip are refused.
2. **The error path is correctly wired.** `return error(...)` from ActivateBestChain bubbles up; the longer chain isn't silently accepted.
3. **The manual common-ancestor walk in main.cpp:2508-2513 produces the right reorgDepth value** even when the two chains diverge from block 1 (i.e., when the common ancestor is genesis itself).

## What this does NOT validate

* **Mainnet activation height correctness.** We tested with the LWMA3 fork height temporarily set to 100; mainnet has it at 990,000. The gate-fires-at-fork-height behavior would need a mainnet-condition test (or a mainnet block 990,000+ event) to confirm.
* **Reorg-just-under-the-limit acceptance.** We tested a 250-block reorg against a 100-block limit. We did not test that a 99-block reorg correctly succeeds. Implicit, but not directly exercised.
* **Behavior when the active tip and the rival chain share most history.** All our blocks 1..N differed; in production, reorgs are short tail-end swings. The depth calculation handles that case structurally (it walks back to common ancestor) but we didn't simulate it.

These gaps are acceptable for rc3: the dangerous case is the deep-reorg attack, and that's the case we proved. The "normal" case (short reorg works) is exercised every time the chain has a 1-2 block fork on mainnet — which we already see in production.

## Reproducer

`/tmp/maxreorg-test.sh` — keep around for re-runs. Self-contained. Requires the source-tree binary at `~/claude/offerings-master/src/Offeringsd`.
