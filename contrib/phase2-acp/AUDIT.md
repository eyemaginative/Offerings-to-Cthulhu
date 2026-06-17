# Phase-2 ACP — audit of existing CSyncCheckpoint plumbing

> Issue #40 audit phase. Done 2026-06-17.
> Branch: `feat/issue-40-phase2-acp` (forked from `main`).
> Source paths cited at HEAD of this branch.

## Bottom line

The entire ppcoin/Peercoin-style ACP (Auto-Checkpoints) stack is **already implemented end-to-end** in this tree, dormant. Stale, but architecturally complete and wired into block validation, P2P, RPC, and init.

The delta to bring Phase 2 live is **a single pubkey constant change** + an operational scheduler for periodic `sendcheckpoint` invocations.

This is dramatically less work than #40's "Work remaining" list implied.

## What exists, and where

### Header — full ACP interface

`src/checkpoints.h:17-105`

- `CUnsignedSyncCheckpoint` — base class, holds `hashCheckpoint`, serialization
- `CSyncCheckpoint` (derived) — adds `vchMsg`, `vchSig`, methods:
  - `RelayTo(CNode*)` — pushes "checkpoint" P2P message to one peer
  - `CheckSignature()`
  - `ProcessSyncCheckpoint(CNode*)`
  - Static keys: `strMainPubKey`, `strTestPubKey`, `strMasterPrivKey`

`src/checkpoints.h:110-148` — `Checkpoints` namespace declares the full lifecycle:
`WriteSyncCheckpoint`, `IsSyncCheckpointEnforced`, `AcceptPendingSyncCheckpoint`,
`AutoSelectSyncCheckpoint`, `CheckSyncCheckpoint`, `WantedByPendingSyncCheckpoint`,
`ResetSyncCheckpoint`, `AskForPendingSyncCheckpoint`, `CheckCheckpointPubKey`,
`SetCheckpointPrivKey`, `SendSyncCheckpoint`, `IsMatureSyncCheckpoint`,
`IsSyncCheckpointTooOld`, `WantedByOrphan`.

### Implementation — `src/checkpoints.cpp`

- ACP signature path: `CSyncCheckpoint::CheckSignature` (line 530), `SetCheckpointPrivKey` (line 429), `SendSyncCheckpoint` (line 454)
- Validation: `ValidateSyncCheckpoint` (line 199), `CheckSyncCheckpoint` (line 326), `AcceptPendingSyncCheckpoint` (line 267), `IsMatureSyncCheckpoint` (line 493), `IsSyncCheckpointTooOld` (line 504)
- State: `hashSyncCheckpoint`, `hashPendingCheckpoint`, `checkpointMessage`, `checkpointMessagePending`, `hashInvalidCheckpoint`, `cs_hashSyncCheckpoint` (lines 190-196)
- Lifecycle: `WriteSyncCheckpoint` (251), `ResetSyncCheckpoint` (371), `CheckCheckpointPubKey` (414)
- Peer-receipt: `CSyncCheckpoint::ProcessSyncCheckpoint(CNode*)` (line 546) — full flow: signature check → orphan handling → ValidateSyncCheckpoint → ConnectTip if not in main chain → WriteSyncCheckpoint

### Wiring — already done

**P2P receive handler** — `src/main.cpp:4492`
```cpp
else if (strCommand == "checkpoint") // ppcoin synchronized checkpoint
{
    CSyncCheckpoint checkpoint;
    vRecv >> checkpoint;
    if (checkpoint.ProcessSyncCheckpoint(pfrom)) {
        // Relay
        pfrom->hashCheckpointKnown = checkpoint.hashCheckpoint;
        ...
    }
}
```

**Block validation hook** — `src/main.cpp:2825-2833` (inside `AcceptBlock`)
```cpp
// Check that the block chain matches the known block chain up to a checkpoint
if (!Checkpoints::CheckBlock(...))
    return state.DoS(100, error("AcceptBlock() : rejected by checkpoint lock-in at %d", nHeight),
                     REJECT_CHECKPOINT, "checkpoint mismatch");

// ppcoin: check that the block satisfies synchronized checkpoint
if (Checkpoints::IsSyncCheckpointEnforced() // checkpoint enforce mode
    && !Checkpoints::CheckSyncCheckpoint(...))
    return error("AcceptBlock() : rejected by synchronized checkpoint");
```

**Startup-key option** — `src/init.cpp:574-577`
```cpp
if (mapArgs.count("-checkpointkey")) // ppcoin: checkpoint master priv key
{
    if (!Checkpoints::SetCheckpointPrivKey(GetArg("-checkpointkey", "")))
        return InitError(_("Unable to sign checkpoint, wrong checkpointkey?"));
}
```

**Static-checkpoint enable flag** — `src/init.cpp:519`
```cpp
Checkpoints::fEnabled = GetBoolArg("-checkpoints", true);
```

**RPCs** — `src/rpcserver.cpp`
- `sendcheckpoint <blockhash>` — declared at line 245, registered at line 351
- `enforcecheckpoint <enforce>` — declared at line 278, registered at line 352

**Post-tip-advance broadcast** — `src/main.cpp:3033`
> `// ppcoin: if responsible for sync-checkpoint send it`

So a node holding `strMasterPrivKey` already auto-broadcasts when the tip advances; no cron needed unless we want a redundancy heartbeat.

## What's stale / needs to change

### The single load-bearing change

`src/checkpoints.cpp:525-526`
```cpp
const std::string CSyncCheckpoint::strMainPubKey = "0466aa7cf205be5c40f114c80d0d4087959508ace5642c9b849af1ba78d7c6b969f3e8d36b3d44e5a0ac1d2d8f3e6f7452055713943870700385544c2a04c5aa55";
const std::string CSyncCheckpoint::strTestPubKey = "041ba70a9e3afd1c0c13b7577e4f71ede2eee884df617fa28bfb0ee3fe993b9cc2835c16b794e46095bf425c4e2cdc2e628becdb196f0302840282d3d32d6c69bd";
```

Both are inherited ppcoin/Peercoin-era keys. Neither is a Conclave key.

**Replace mainnet with OFFSIG Slot #1:**
```cpp
const std::string CSyncCheckpoint::strMainPubKey = "0238efde05d567979485df6cd6dcf3af2606348a1e260eedf9a6464df57f46b111";
```

The 33-byte compressed pubkey matches what `vConclaveKeys[0]` is loaded with at `src/chainparams.cpp:188`. The Slot #1 privkey lives where the active canon miner runs (verified via `validateaddress … ismine` against the running mainnet wallet — owns address `Qcad56y76jCGgjkhGHPyCMY89uC9j9CBXC`).

Testnet: free to pick. Either reuse a known testnet wallet key, or generate fresh. (Open subquestion in #40.)

### A possibly-redundant follow-on simplification

Since `Params().ConclaveKeys()` already exposes the network's three Conclave pubkeys, `CheckSignature()` could read `Params().ConclaveKeys()[0]` directly instead of the hardcoded `strMainPubKey` constant. That would:
- Eliminate one source of pubkey drift (single source of truth)
- Make a future fresh-key migration a one-file chainparams change

Not required for first-light, but worth doing if we touch this code anyway.

## Activation recipe (minimal viable)

1. Replace `strMainPubKey` with OFFSIG Slot #1's pubkey (~1-line patch).
2. Decide `strTestPubKey` (reuse a testnet wallet key, or fresh).
3. Rebuild and deploy to the operator host that holds Slot #1's privkey.
4. Start the daemon with `-checkpointkey=<WIF>` (init.cpp:574 wires this through `SetCheckpointPrivKey`).
5. The first `sendcheckpoint <currenttip>` RPC call (or auto-broadcast at next tip advance) signs and gossips the checkpoint.
6. Peers running this build accept the checkpoint and start enforcing it via `CheckSyncCheckpoint` in `AcceptBlock`.

## Open subquestions (for #40 design)

- **Broadcaster cadence**: rely on the `main.cpp:3033` post-tip-advance broadcast, or add a periodic re-broadcast (every N minutes) for redundancy against missed gossip?
- **Checkpoint depth**: `AutoSelectSyncCheckpoint` (cpp:314) walks back from tip by `-checkpointdepth` (default `-1`, which the code treats as walk-the-tip-itself). Tuning needed. Suggest `MAX_REORG_DEPTH = 100` as the working default.
- **Testnet pubkey choice**: fresh or reuse?
- **Drop the hardcoded `strMainPubKey` constant** in favor of `Params().ConclaveKeys()[0]` — clean-up or skip?

## Test plan implications

The Python `qa/rpc-tests/` directory already has the infra for regtest harness tests. A new `phase2_acp.py` would need to:
- Spin up two regtest daemons, peer them
- Pre-load one with a privkey via `-checkpointkey`
- Call `sendcheckpoint` on the broadcaster
- Verify peer receives + accepts via `enforcecheckpoint`
- Then try to deliver a competing fork past the checkpoint; verify rejection

This is straightforward; the mechanics already exist.

## Summary

What looked like 6 work items in #40 collapses to:
- 1 source change (pubkey constant)
- Operator runbook for the `-checkpointkey` deployment
- Regtest harness test
- Maybe the `Params().ConclaveKeys()[0]` cleanup

Estimated effort: hours, not days. The Peercoin engineers shipped the heavy lifting in 2014; we just need to point it at our key.
