# OFF — Where We Left Off  (snapshot 2026-06-02, post PR #1 merge)

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

## DONE (in source on vps3 / origin — chaos is currently NOT carrying rc3/rc4, the rc2 OFFSIG window, or the h=976000 checkpoint; see OPEN/NEXT)
- Consensus v2.0.0: 1.5 OFF/block lock, 7/8 miner + 1/8 Conclave Treasury, 150k Tithe at fork,
  8 BCT-#699 attacker addresses banned.
- **Renewed Ritual** (HUNK 10 in canonical patch, code in `src/main.cpp::RitualBonus`):
  recurring ~6-month rite, 10,000 OFF finale to the miner at xxx,666 heights;
  first finale 1,141,666 (autumnal equinox 2026-09-22). INERT until ~block 1,101,346.
- **Chain Codex** (`src/miner.cpp` lines 378-540): public-domain Lovecraft corpus + Descent verses
  + Phase-B "Dreaming" R'lyehian generator. Canon starts at block 1,000,001.
  As of 2026-05-25, the Codex code is in the CANONICAL tree (vps3) via commit `9062e1d` — port
  of the chaos STALE tree's miner.cpp. (vps3 Linux build is now green — see entry below.)
- **The Descent** (10 ceremonial verses at heights 999,991-1,000,000): revived 2026-05-25 via
  `CODEX_DESCENT_START=999991` constant and guard fix `if (nHeight < CODEX_DESCENT_START) return egg;`
  Was previously dead code (early-return at CODEX_ANCHOR=1000001 blocked the Descent range).
- **OFFSIG signed-mining window** (heights 999,991-1,050,666, unchanged since rc2 —
  `nSignedWindowStart`/`nOpenMiningHeight` in `src/chainparams.cpp:167-168`).
  Covers all 10 Descent verses + canon-reading (1,000,001-1,047,248) + ~2.4-day
  post-canon tail. Only Conclave keys can mine in this range. Validation in
  main.cpp + chainparams; mining-side (placeholder OP_RETURN + `SignBlockIfNeeded`)
  in miner.cpp. Conclave signing privkey lives in chaos btcbob + vps1 btcbob
  (the pool's backing daemon) wallets + paper backup.
- **rc3 — LWMA-3 + MAX_REORG_DEPTH + extranonce-invariant OFFSIG** (commit `788fc60`,
  tag `v2.0.0-rc3`, 2026-05-29). Three concurrent consensus changes:
  - **LWMA-3 retarget** (Zawy's linear-weighted moving average, N=60, T=60s).
    New `src/pow.{h,cpp}::GetNextWorkRequired_LWMA3`. Old retarget renamed to
    `GetNextWorkRequired_Legacy` in main.cpp; dispatcher in pow.cpp routes by
    height. Activation `HARDFORK_LWMA3_MAIN_OFF=990000` (later 980000 in rc4),
    `HARDFORK_LWMA3_TESTNET_OFF=100`.
  - **MAX_REORG_DEPTH=100** finality rule in `src/main.cpp:2498-2523`
    (`ActivateBestChain`). Rejects reorgs deeper than 100 blocks from active tip
    past LWMA-3 fork height. Walks both chains backward to common ancestor
    manually (OFF's CChain API takes CBlockLocator, not CBlockIndex*).
  - **Extranonce-invariant OFFSIG** (Option B-a). `main.cpp::OffSigningHash`
    now blanks the coinbase scriptSig in addition to the OFFSIG output's
    scriptPubKey before computing merkleSansSig. Pools can serve one signed
    template to many workers searching different extranonce ranges without
    invalidating the signature. `rpcmining.cpp::getblocktemplate` now calls
    `SignBlockIfNeeded` after `CreateNewBlock + UpdateTime`. Closes the rc2
    gap where pool-served templates submitted placeholder OFFSIGs and tripped
    `bad-conclave-sig` rejection.
  - ⚠ **rc3 wire break:** rc2 binaries REJECT rc3-signed blocks (different
    merkleSansSig). All cluster nodes must upgrade to rc4 before block 999,991.
- **rc4 — LWMA-3 activation pulled 990000 → 980000** (commit `5b1c8ba`,
  tag `v2.0.0-rc4`, 2026-05-29). Chain has been locked at ~20s/block for 300+
  blocks under the legacy retarget's +10%/cycle clamp (diff 2.56 → 10.85 over
  16 cycles, solvetimes flat ~20s) — classic drive-by rented-hash pattern.
  980000 gives ~22k blocks of LWMA-3 settling (~15 days at 60s) before Descent
  at 999,991, vs ~5 days under legacy with hash still ramping. `src/pow.h:24`.
- **Anti-reorg checkpoint at h=976000** (commit `0c751a5`, 2026-05-29): one hardcoded
  entry in `src/checkpoints.cpp::mapCheckpoints` locking block hash
  `000000cd…7225f95`. Defends against deep reorgs by the >80%-hash attacker camped on
  the chain. Non-consensus for new blocks. Binary SHA256 = `6bef0f0f…743a3d7`.
  Deployed: vps1 btcbob (pool backend), vps3 relay. Chaos miner still TBD —
  and now must include rc3/rc4 as well, see OPEN/NEXT.
- **vps3 Linux build: GREEN as of 2026-06-01 at ec14634 (v2.0.1-Bokrug-checkpoint).**
  Fresh stripped binaries shipped in the v2.0.1 GitHub release as
  `cthulhu-offerings-linux-x86_64-v2.0.1.tar.gz` (sha `cc5cbda2…`). Internal:
  Offeringsd `645b1de5…`, Offerings-cli `32b57a98…`, Offerings-qt `e054133f…`.
  Configure: `--with-gui=qt5 --without-miniupnpc --disable-tests --disable-hardening`
  + **`CPPFLAGS=-I/home/btcbob/openssl-1.0.2/include -I/home/btcbob/db4/include`,
  `CXXFLAGS=-fPIC ...`** (the GUI build needs `-fPIC` for Qt5 on this Debian, and
  configure MUST be re-given the openssl-1.0.2 path or it falls back to system
  OpenSSL 3 and breaks on bignum.h). See [[offerings-build-recipe]] § GUI build.
- **v2.0.1 release on GitHub is corrected as of 2026-06-01.** Win64 zip + 3 exes
  came from the depends/ CI (run `26718849256`, Qt 5.15.16 + posix-threads MinGW).
  Linux tarball is the vps3 build above. The original 2026-05-31 Win64 upload was
  a stale Qt 5.7.1 + win32-threads MinGW build whose toolbar buttons were dead
  on Windows — replaced. See [[feedback-release-win64-from-ci]].
- **PR #1 from 9019x merged to main** (merge commit `1979b59`, 2026-06-02):
  *"Modernize depends build system"* — first external Restoration contributor's
  work landed. Scope: depends/ tree (Qt **downgraded** 5.15.16 → 5.9.8 — "last
  easy Qt5" without source modernization; miniupnpc 1.9 → 2.2.2; fontconfig
  2.11.1 → 2.12.1; freetype 2.5.3 → 2.7.1; protobuf 2.5.0 → 2.6.1; fanquake-era
  Qt patches + his own gcc-10 `<limits>` patch broadened to 12 headers after
  review). Plus a 2-line `configure.ac` fix adding `-DMINIUPNP_STATICLIB`
  alongside `-DSTATICLIB` for static miniupnpc on Windows. Plus one src adapter
  for the new miniupnpc 2.x API. 19 atomic commits. Identity: **skifdni** (BCT)
  ≡ **9019x** (GitHub) via shared OFF tip address `QZ4wBASgvrHdodYt6CBDqDv3MitAQYnd75`.
  His NACK on switching to `-lwinpthread` was vindicated empirically — the green
  Win64 binaries link 100% static with only Win32 system DLLs, no
  `libwinpthread-1.dll` anywhere. See [[project-off-contributor-skifdni]].
- **CI build-hardening landed on main** (commits `d795cfd` + `b75b6b7` +
  `ecfc27a`, 2026-06-01/02): pinned Win64 cross-compile runner to
  `ubuntu-22.04` (Noble's mingw-w64-13 hits TOUCHINPUT redefinition with Qt
  5.9.8); added `-Wa,-mbig-obj` to CXXFLAGS (gcc-10+ template instantiation in
  main.cpp blows past COFF's 16-bit section count, 33,772 sections on this
  build); scoped depends/ cache key by distro (`-jammy-v1-`) so a Noble-built
  ccache binary doesn't poison a Jammy runner via glibc-2.38-vs-2.35
  mismatch. These are OUR infrastructure changes, not part of PR #1 —
  they exist in the workflow file only. Proper homes: `-Wa,-mbig-obj` belongs
  in `configure.ac` under `host=*mingw*`, TOUCHINPUT needs a Qt patch to
  unpin runner from Jammy. See [[feedback-off-win64-ci-quirks]].
- **Homepage at https://23skidoo.info/ download buttons updated** (2026-06-02):
  LINUX GUI → `Offerings-qt-v2.0.2-fnu-linux64.tar.gz`, WINDOWS GUI →
  `Offerings-qt-v2.0.2-fnu-win64.zip`. Lineage line bumped to "v2.0.2 (2026,
  the Conclave)". Backups at `/var/www/23skidoo.info/index.html.bak-20260601-*`
  (prior v2.0.1 cut) and `index.html.bak-20260602-005230-v202` (pre-v2.0.2 cut).
  URLs verified live: both 302 → 200 with correct content-length.
- **v2.0.2 released on GitHub** (2026-06-02 00:48 UTC,
  https://github.com/SubGeniusFinance/Offerings-to-Cthulhu/releases/tag/v2.0.2):
  - **Win64**: `Offerings-qt-v2.0.2-fnu-win64.zip` (sha256 `e8fe0c1e…`).
    Cross-compiled via CI run `26790951112` (Jammy + Qt 5.9.8 + posix mingw,
    cache hit on the new `-jammy-v1-` key). DLL deps: Win32 system libraries
    only — no `libwinpthread-1.dll`, no `Qt5*.dll`, no `libgcc_s_seh-1.dll`,
    no `libstdc++-6.dll`. Fully self-contained.
  - **Linux x86_64**: `Offerings-qt-v2.0.2-fnu-linux64.tar.gz` (sha256
    `9cb518b5…`). Built natively on vps3 per [[offerings-build-recipe]] GUI
    incantation (--with-gui=qt5 --without-miniupnpc --disable-tests
    --disable-hardening, openssl-1.0.2 + db4 CPPFLAGS, -fPIC, -j1).
  - Both archives use top-level `Offerings-v2.0.2-fnu/` dir per Bitcoin Core
    convention. Per-binary + per-archive SHA256s in release notes.
  - 9019x credited as **first external Restoration contributor**, his
    requested `fnu` handle in filenames, "fnu works for me" quoted, tip
    address `QZ4wBASgvrHdodYt6CBDqDv3MitAQYnd75` referenced.
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
- (was: cut v2.0.2 — DONE, see DONE section below)
- **Move CI workarounds into source** as build-hardening follow-up: port
  `-Wa,-mbig-obj` from CI's CXXFLAGS into `configure.ac` under `host=*mingw*`
  conditional; add a Qt patch fixing the TOUCHINPUT redefinition so we can
  unpin the runner from `ubuntu-22.04` back to `ubuntu-latest`. Not blocking
  v2.0.2 — purely about portability for outside contributors building on
  non-Jammy/non-Focal distros.
- **Deploy rc4 binary to the cluster** — now urgent because of the rc3 wire
  break. rc2 binaries reject rc3-signed blocks. Every node that will be online
  at block 999,991 must be on rc4 (or rc3 — same wire) before then. Affected:
  - **chaos miner** — still on `cd0bfd2-Bokrug` (pre-checkpoint, pre-rc3).
    Build on chaos itself: Debian 13 + Boost 1.83 ABI differs from vps3's
    Boost 1.74 dynamic links. After rebuild, snapshot the binary SHA into this
    doc.
  - **vps1 btcbob (pool backend)** — was on `6bef0f0f…` (HEAD `0c751a5`,
    pre-rc3). Pool block submissions will fail with `bad-conclave-sig`
    starting at height 999,991 unless upgraded — but the extranonce-invariant
    OFFSIG fix in rc3 is precisely the pool's path forward. Rebuild + restart
    `offeringsd` + bounce Miningcore.
  - **vps3 relay** — same `6bef0f0f…` situation. Rebuild from HEAD; record
    new SHA.
  - **vps1 endciv (Treasury Key #2, cold)** — sibling-session deploy track;
    confirm with that session that rc3/rc4 is on their list too.
- **LWMA-3 testnet validation** — rc3 commit message and the `pow.cpp`
  README warning both flag this: LWMA-3 is NOT testnet-validated yet. The
  `HARDFORK_LWMA3_TESTNET_OFF=100` activation makes it trivial to spin up a
  testnet daemon and watch retarget behavior. Do this before block 980000
  on mainnet.
- Re-cut the source bundle to include the Codex GUI tab + the ported miner.cpp
  (Codex GUI is in vps3 source but no recent release bundle includes it).
- Visually verify the GUI Codex tab on chaos's desktop.
- Decide whether the site should seal the 23 Lovecraft books behind inscription progress
  (per user 2026-05-25); Proem stays open as Conclave invocation.

## Milestones
- LWMA-3 activates: **980,000** (rc4)  | fork: 1,000,000  | codex starts: 1,000,001  | Descent: 999,991-1,000,000  | 1st Ritual finale: 1,141,666 (2026-09-22)  | OFFSIG window: 999,991-1,050,666

## Notes
- Auto-memory is per-box/per-cwd — it does NOT sync across chaos/vps3. This file is the
  cross-machine source of truth. Multi-Claude coordination historically via handoff files at
  https://23skidoo.info/handoff-<uuid>.md.
- Note: the prior `.github/workflows/windows-build-depends.yml` + `depends/**` work
  is complete as of 2026-06-02 (PR #1 merged, three CI-hardening commits landed).
  No active concurrent session on those paths. Future edits welcome.
- **The OFF Reclamation portal** is live at https://23skidoo.info/bridge/ (Phase 1, FastAPI on
  vps3, indexes at /var/lib/bridge-portal/). See `[[project-off-bridge-portal-live]]` memory.
