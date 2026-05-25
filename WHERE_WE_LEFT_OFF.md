# OFF — Where We Left Off  (snapshot 2026-05-25)

> Fresh session? Read this, then for LIVE state run on **chaos**:  `bash ~/status.sh`
> (chaos is the miner box, tailscale 100.87.114.52, reachable as `ssh btcbob@chaos`)
> ⚠ If you are ALREADY ON chaos (hostname=chaos), do NOT `ssh chaos` — it has no
> loopback key and fails with "Permission denied", which looks like a dead mesh but isn't.
> Just run `bash ~/status.sh` / `Offerings-cli ...` directly. ssh to vps1/2/3/nas is fine.

## What this is
Community revival of **Offerings to Cthulhu (OFF)** as the **Restoration Hardfork v2.0.0**,
forked from the recovered block-966,413 chainstate (Wayback 2015 snapshot, verified backup on
vps1 at ~/Offering-chainstate-backup-2026-05-21, sha256 12033fa5…). Activates at block 1,000,000.

## Source tree — ONE canonical, vps3 only (NEW POLICY 2026-05-25)
- **vps3** `~/claude/offerings-master/` — **THE CANONICAL TREE.** Edit here. Commit here. Push here.
  Git origin: `https://github.com/SubGeniusFinance/Offerings-to-Cthulhu.git`. Branch: `main`.
- **chaos** `~/offerings-master.STALE-do-not-edit-2026-05-25/` — STALE. Renamed today. Do NOT edit.
  Preserved only for running-binary provenance and history.
- Older trees on chaos (`~/off-canonical/`, etc.) are also stale, ignore.
- See `[[feedback-off-canonical-source-vps3]]` in auto-memory for full rationale.

## DONE (in the running binary on chaos unless noted)
- Consensus v2.0.0: 1.5 OFF/block lock, 7/8 miner + 1/8 Conclave Treasury, 150k Tithe at fork,
  8 BCT-#699 attacker addresses banned.
- **Renewed Ritual** (HUNK 10 in canonical patch, code in `src/main.cpp::RitualBonus`):
  recurring ~6-month rite, 10,000 OFF finale to the miner at xxx,666 heights;
  first finale 1,141,666 (autumnal equinox 2026-09-22). INERT until ~block 1,101,346.
- **Chain Codex** (`src/miner.cpp` lines 378-540): public-domain Lovecraft corpus + Descent verses
  + Phase-B "Dreaming" R'lyehian generator. Canon starts at block 1,000,001.
  As of 2026-05-25, the Codex code is in the CANONICAL tree (vps3) via commit `9062e1d` — port
  of the chaos STALE tree's miner.cpp. Building on vps3 still TBD (Linux build hit C++17 issues).
- **The Descent** (10 ceremonial verses at heights 999,991-1,000,000): revived 2026-05-25 via
  `CODEX_DESCENT_START=999991` constant and guard fix `if (nHeight < CODEX_DESCENT_START) return egg;`
  Was previously dead code (early-return at CODEX_ANCHOR=1000001 blocked the Descent range).
- **OFFSIG signed-mining window** (heights 1,000,000-1,057,329, only Conclave keys can mine):
  validation in main.cpp + chainparams (already on vps3); mining-side (placeholder OP_RETURN +
  `SignBlockIfNeeded`) ported into vps3's miner.cpp in commit `9062e1d`.
- Miner crash fixes (chainActive data race TRY_LOCK, dead-chain IBD gate removed):
  also in vps3's miner.cpp post-`9062e1d`.
- **Web** (vps3, cron): https://23skidoo.info/codex/ (paginated Library e-reader) +
  /awakening/ (live countdown). Note: site currently reveals all 24 book bodies despite 0%
  inscription; user flagged 2026-05-25 that books 1-23 should be sealed pre-inscription,
  Proem stays readable. UX change pending.
- **Wallet GUI**: Qt5 port that compiles+launches on Debian 13 on chaos. Codex tab in
  `src/qt/codexpage.{h,cpp}`. Source ported to vps3 alongside miner.cpp work. NOT visually
  verified yet — run on chaos's desktop AFTER `sudo systemctl stop offeringsd` (datadir lock).

## RUNNING UNATTENDED (survives terminals/reboots)
- `offeringsd` systemd service on chaos (miner) + vps3 (relay), both enabled-on-boot.
- chaos cron `~/reset-finalize.sh` (*/30): self-deleted once height passed 967,250.
- vps3 crons: e-reader (every 2m), countdown (every 1m).
- vps1 pool: `pool.23skidoo.info:3040` (Miningcore), currently mining most blocks
  (vps3+chaos's daemons are relays/validators, not solo-miners — see Miningcore tags in
  recent coinbase scriptSigs).
- Cloud routine 2026-06-18 (claude.ai/code/routines/trig_01H43m3GD6M5h1saCmABPSBS): post-fork
  batch — backport invalidateblock RPC, deploy ritual binary to vps3, read ~/codex/post-fork-backlog.md.

## OPEN / NEXT
- Build the canonical binary on vps3 (Linux build is hitting C++17 issues unrelated to
  recent changes — `leveldbwrapper.h` dynamic exception specs, `net.cpp` miniupnp API,
  `rpcserver.cpp` filesystem ambiguity). Need to modernize source or pin compiler.
- Once vps3 builds clean: distribute new binary to chaos + vps1 (pool) so the Codex
  inscription + Descent verses + OFFSIG signing actually fire at the fork.
- Push the bundle to GitHub once `gh` auth exists. After today's port, the GitHub push
  will include the Codex (which was missing from the older bundle).
- Re-cut the bundle to include the Codex GUI tab + the ported miner.cpp.
- Visually verify the GUI Codex tab on chaos's desktop.
- Decide whether the site should seal the 23 Lovecraft books behind inscription progress
  (per user 2026-05-25); Proem stays open as Conclave invocation.

## Milestones
- fork: 1,000,000  | codex starts: 1,000,001  | Descent: 999,991-1,000,000  | 1st Ritual finale: 1,141,666 (2026-09-22)  | OFFSIG window ends: 1,057,329

## Notes
- Auto-memory is per-box/per-cwd — it does NOT sync across chaos/vps3. This file is the
  cross-machine source of truth. Multi-Claude coordination historically via handoff files at
  https://23skidoo.info/handoff-<uuid>.md.
- **Concurrent Claude on chaos** is working on `.github/workflows/windows-build-depends.yml` +
  `depends/**` (Windows Qt5 cross-compile via GitHub Actions). Stay out of those paths.
- **The OFF Reclamation portal** is live at https://23skidoo.info/bridge/ (Phase 1, FastAPI on
  vps3, indexes at /var/lib/bridge-portal/). See `[[project-off-bridge-portal-live]]` memory.
