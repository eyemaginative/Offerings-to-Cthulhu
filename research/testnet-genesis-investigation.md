# OFF Testnet Genesis Investigation

**Date:** 2026-05-29
**Status:** investigation; no code change committed
**Context:** rc3 validation could not use the existing OFF testnet because the genesis block doesn't validate under Quark Hash9.

## Root cause

`src/chainparams.cpp:220-221` hardcodes the testnet genesis as:

```cpp
genesis.nTime = 1373481000;
genesis.nNonce = 905523645;
```

These values are Bitcoin testnet3 nLockTime/nNonce from the SHA256-d era — they predate this codebase's swap to Quark. The CBlockHeader::GetHash() at `src/core.cpp:218` is `Hash9(...)`, which is Quark, so the genesis hash under OFF's PoW is whatever Quark9 produces from those header bytes — almost certainly above `bnMainProofOfWorkLimit = ~uint256(0) >> 20`. The genesis hash assertion at `chainparams.cpp:223` is commented out, so the daemon doesn't crash at startup — but downstream PoW checks reject the chain.

This is why all rc3 validation had to fall back to regtest mode (which uses its own pow limit `~uint256(0) >> 1` and its own genesis, both trivially valid).

## Two paths forward

### Path A: lower testnet pow limit (smallest diff, ~5 lines)

Add to `CTestNetParams::CTestNetParams()`:

```cpp
bnProofOfWorkLimit = CBigNum(~uint256(0) >> 8);   // easy testnet target
genesis.nBits = bnProofOfWorkLimit.GetCompact();
```

Then iterate nNonce until Quark(genesis) satisfies it. With target = `~uint256(0)>>8`, expected attempts ≈ 256, so any value of nNonce we pick will probably work in well under a millisecond. We can leave the existing 905523645 if it happens to satisfy, or just set `genesis.nNonce = 0` and let the first hash work.

**Tradeoff:** testnet difficulty floor is much looser than mainnet's. Not a problem for our use case (validating consensus rules, mining a few hundred blocks to test LWMA-3 + MAX_REORG_DEPTH).

### Path B: mine a real testnet genesis under mainnet pow limit (proper)

Keep `bnProofOfWorkLimit = bnMainProofOfWorkLimit` for testnet (`~uint256(0) >> 20`). Iterate nNonce until Quark satisfies it. Expected attempts ~2^20 = ~1M, which on a modern CPU is ~1-3 seconds with Quark.

Implementation: a temporary `-mineTestnetGenesis` startup flag that builds the testnet genesis like chainparams does, loops nNonce, prints the result, exits. Bake result back into chainparams.cpp:221.

Sketch (drop into `src/init.cpp` AppInit2 after Params() is set):

```cpp
if (mapArgs.count("-mineTestnetGenesis") && TestNet()) {
    CBlock genesis = Params().GenesisBlock();
    const CBigNum target = Params().ProofOfWorkLimit();
    while (CBigNum(genesis.GetHash()) > target) {
        ++genesis.nNonce;
        if ((genesis.nNonce & 0xFFFFF) == 0)
            LogPrintf("genesis mining: tried %u nonces\n", genesis.nNonce);
    }
    LogPrintf("MINED testnet genesis: nNonce=%u hash=%s merkle=%s\n",
              genesis.nNonce, genesis.GetHash().ToString().c_str(),
              genesis.hashMerkleRoot.ToString().c_str());
    exit(0);
}
```

## Recommendation

**Path A** for rc3-cycle validation work. It's a 5-line change, gives us a functional testnet quickly, and the looser-than-mainnet difficulty is fine for the kind of consensus testing we want to do (LWMA-3 stress test against real P2P timestamps, MAX_REORG_DEPTH affirmative test with multiple geographically distributed nodes).

**Path B** later as a polish item — at the same time as we add a `seed.testnet.23skidoo.info` DNS seed and stand up 2-3 reachable testnet peers on the VPS fleet. That's a separate work batch, not a release blocker.

## What we lost by not having testnet

* Could not validate LWMA-3 against real P2P-derived timestamps (regtest CPU-mines too fast and uses local clock only)
* Could not validate OFFSIG window with two independently-mining authorized peers
* Could not run a long soak (>24h) on rc3 against real network conditions before fork day

All of these are nice-to-haves. The consensus rules themselves have been validated on regtest and reviewed in source; the testnet gap is about coverage breadth, not correctness depth.

## Action items if we proceed with Path A

1. Edit `src/chainparams.cpp` CTestNetParams ctor — add the two lines above.
2. Bump testnet `strDataDir = "testnet3"` to `"testnet4"` so existing testnet datadirs don't get clobbered.
3. Recompile, run `./Offeringsd -testnet`, observe the new genesis hash (will print at startup).
4. Update any docs/scripts that reference the old testnet genesis hash (probably none — testnet has been broken for years).
5. Stand up 1-2 testnet peers on the fleet (one host has spare bandwidth) and add DNS seed.
6. Optional: tag v2.0.0-rc4 with the testnet fix.
