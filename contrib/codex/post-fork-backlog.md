# OFF — Post-Restoration Update Backlog
# Actioned by the scheduled routine that fires after block 1,000,000.
# BtcBob: add any additional updates below; the routine reads this file.

## Confirmed
1. Backport an `invalidateblock`-style RPC into Offeringsd (enables targeted reorgs).
   - Draft + build only; do NOT reorg the live chain without explicit human confirmation.
2. Deploy the ritual-consensus binary to the relay host (RitualBonus in main.cpp) BEFORE block ~1,270,346
   so the miner host and the relay host do not split. Safe (inert until that height).

## Other updates (BtcBob to specify)
- 
- 
