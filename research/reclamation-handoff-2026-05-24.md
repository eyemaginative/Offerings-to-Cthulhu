# Reclamation Portal + Ritual-Continuation Handoff — 2026-05-24

**For:** Next Claude session resuming OFF work, once the v2.0.0-Restoration wallet builds cleanly again.
**From:** Claude session on the production host, working with **BtcBob** / @dobbscoin.
**Date:** 2026-05-24, ~16:30 UTC.
**Status of OFF chain:** still pre-fork. Tip ~block 969,000-ish. Fork activates at 1,000,000 (~31K blocks away). Restoration patch lives at `~/claude/offerings-master/research/restoration-hardfork.patch`. Daemon running locally at `~/.Offering/`.

---

## 1) What you're walking into

Three threads of work braided together in this session. All three are now in coherent states — but the THIRD has a drafted but un-committed patch hunk you need to land.

### Thread A — The Reclamation (renamed from "Honest Holder Bridge")

Program got a substantive policy + branding overhaul today:

- **Renamed** "Honest Holder Bridge" → **"The Reclamation"**. URL slugs kept at `/bridge/` and `/bridge/claim/` for SEO continuity. Tier codes WR-A / WR-B (Worshipper Recognition) are sub-mechanisms under the Reclamation umbrella.
- **Calendar window REMOVED.** No more 12-month T0+365 closure. The Reclamation stays open until the 1,500,000 OFF ceiling is exhausted.
- **Worshipper Recognition (WR) tier replaces flat-50 HMR.** Now scaled by earliness × depth:

  ```
  recognition_OFF = 100 × earliness × depth

  earliness:  block 0–999 → ×5.0  (genesis)
              1K–50K     → ×3.0
              50K–241K   → ×2.0
              241K–483K  → ×1.0
              483K–724K  → ×0.6
              724K–966K  → ×0.3

  depth = min(1 + log10(n_appearances), 5.0)   # 1→1.0, 10→2.0, 100→3.0, 1K→4.0, 10K+→5.0 capped
  ```

  Range: **30 OFF (late one-shot) → 2,500 OFF (genesis heavy miner cap)**. **BASE was 10 originally; bumped to 100** after user gut-checked that 150K Tithe vs 100-OFF-max was a 1500x mismatch.
- **WR-B (gap-era) flat 250 OFF** until a post-2015 chainstate surfaces (then re-processed at upgraded formula amount).
- **Post-2015 chainstate recovery hook** documented on the explainer page + in BCT post draft: if anyone surfaces v1.7+ chainstate fragments from 2015–2018, Conclave mounts on air-gapped wallet, re-processes WR-B/Class-B claims at formula amount.
- **Challenge string prefix:** `Conclave-Reclamation-<YYYYMMDDw>-<addr>-<amount>` (was `Conclave-Restoration-Bridge-...`).

### Thread B — Phase-1 portal LIVE

Status: **deployed and operational**. See [[project-off-bridge-portal-live]] memory for full ops details.

| Surface | URL / Path |
|---|---|
| Explainer | `https://23skidoo.info/bridge/` |
| Claim wizard | `https://23skidoo.info/bridge/claim/` |
| API | `https://23skidoo.info/bridge/api/{health,challenge,lookup,verify,submit,state}` |
| systemd unit | `bridge-portal.service` (1 uvicorn worker, ~1.2GB warm) |
| App | `/opt/bridge-portal/app.py` (FastAPI + coincurve pure-Python ECDSA) |
| Indexes | `/var/lib/bridge-portal/indexes/{miners,chainstate_utxo,address,banlist}.json` (~290MB total) |
| Indexer source | `~/claude/offerings-master/scripts/build_portal_indexes.py` |

End-to-end test passed via public nginx earlier today: fresh daemon address → portal-issued challenge → daemon signmessage → portal verify (ok:true) → submit logged. Pure-Python coincurve produces identical results to the daemon's verifymessage.

**Live verification examples (sanity-check against API after wallet rebuild):**
```
QXXVh… (biggest fish, 64,914 blocks at 230K)  →  WR-A 1,000 OFF + ALREADY-WHOLE 217.15 OFF
Qhay…  (heavy early, 69 blocks at 24K)        →  WR-A 852 OFF
QVcy…  (genesis 1-block, block 0)             →  WR-A 500 OFF
QPCg…  (mid-chain holder, 148 OFF balance)    →  WR-A 71 OFF
QR5G…  (late one-shot, block 933K)            →  WR-A 30 OFF
QTLU…  (banlisted #699 addr)                  →  DISQUALIFIED, 0 OFF
```

### Thread C — Restoration-era Ritual continuation (DRAFTED, NOT BUILT)

User confirmed direction (response "3" then "exactly" framing) but explicitly said **"draft it up but don't build it"** when I was about to write the patch. **No changes to `restoration-hardfork.patch` were made.** The draft lives only in chat scrollback as of this writing.

**What's needed:** add a new HUNK 10 to `~/claude/offerings-master/research/restoration-hardfork.patch` that:

1. Adds `GetRestorationRitualSubsidy(int nHeight, int64_t* out_subsidy)` — computes Ritual special-block subsidy for post-fork blocks. Cadence: 260,000 blocks between cycles. First post-fork finale at **block 1,142,666** (≈ autumnal equinox 2026). 32 escalating special blocks (1 per day, spaced 1440 blocks = 24h) + 1 finale at xxx,666 = **~14,777 OFF per cycle**. Twice yearly perpetually.

2. Modifies HUNK 4's post-fork override in `GetBlockValue()`:
   ```cpp
   // BEFORE (current HUNK 4):
   if (IsAfterRestorationFork(nHeight)) {
       nSubsidy = Params().PostForkSubsidy();   // 1.5 OFF
   }

   // AFTER:
   if (IsAfterRestorationFork(nHeight)) {
       int64_t ritual_subsidy;
       if (GetRestorationRitualSubsidy(nHeight, &ritual_subsidy)) {
           nSubsidy = ritual_subsidy;   // Ritual block — lucky worshipper gets the mint
       } else {
           nSubsidy = Params().PostForkSubsidy();   // 1.5 OFF normal
       }
   }
   ```

3. Modifies `GetMinerSubsidy()` and `GetTreasurySubsidy()` so **Ritual blocks are miner-only** — full Ritual mint to the lucky worshipper, Treasury sits the block out (Treasury gets its 1/8 cut from all other 259,968 normal blocks between cycles).

4. Bypasses the Treasury-output requirement in `ConnectBlock` for Ritual blocks (coinbase has ONE output on Ritual blocks, not two).

5. Bypasses the Treasury output in `miner.cpp::CreateNewBlock` for Ritual blocks (same reason).

Full draft (the C++ code for the helper, every modification site, the schedule preview table, and emission impact analysis) was written inline in the 2026-05-24 chat session. Recover it from `~/.claude/projects/-home-btcbob-claude-offerings/sessions/` if needed. Look for the message that opens with "Draft only. Here's what I'd add to `restoration-hardfork.patch` (new HUNK 10)".

**Why it matters:** without this hunk, the Restoration patch silently kills Cycle 5 of the Ritual (and all future cycles). The 1.5-OFF-flat lock currently overrides everything post-fork, including the v1.6.2-coded Ritual special blocks. After this hunk, the Ritual continues twice-yearly forever, anchored to the equinoxes, with each finale block ending in 666.

**WAIT FOR USER GO** before writing to the patch file. They said "draft it up but don't build it."

---

## 2) Why we're paused — wallet build issues

User is currently patching the v2.0.0-Restoration wallet source for unrelated build issues (separate session, separate context). The next Claude session resumes once the wallet builds cleanly again. Until then:

- **Don't touch the patch file** — they might be regenerating it from a clean base
- **Don't try to compile** anything
- **DO** verify the portal is still serving correctly via `sudo systemctl status bridge-portal` and `curl https://23skidoo.info/bridge/api/state`
- **DO** verify the indexer outputs are intact at `/var/lib/bridge-portal/indexes/`

---

## 3) Page-design state (visible to user; don't revert)

User has been iterating on visual design. Recent state (some may have been edited again after my view of it):

- **Logo orange:** `#f0a838` is the chosen accent for headings/taglines/section sub-text on the homepage. Defined as `--orange` in `index.html`'s `:root`.
- **"SubGenius.Finance — The Conclave" eyebrow** is colored `#f0a838` (orange) on `/bridge/`.
- **Homepage** had its `.claim` blocks, `h2` headers, `.section-sub`, `.codex-foot`, and footer's `:last-child` (NFA line) all set to `--orange`. Footer copyright lines set to `--accent` (green, matching links).
- **Homepage** also got a new "Mine OFF / pool.23skidoo.info" pill near the top — user-added since my last view.
- **Bridge page** has a prominent **claim CTA** near the top — green-bordered rectangle with live remainder bar pulled from `/bridge/api/state`. Replaced the dishonest "quarterly accounting" promise with "tracked live at the top of this page."
- **SMF font sizes:** user noted max is 7 and **7 is too large IMHO**. Standardize on `[size=5]` for hero/main and `[size=4]` for sub-headers in any BCT post drafts.

---

## 4) BCT announcement file — UNFIXED

`~/claude/offerings-master/research/bct-revival-announcement.txt` was reverted by the user mid-session back to its original state (still has `[size=12/14/15/20]` over-large tags, `[tt]…[/tt]` deprecated tags, "12-MONTH CLAIM WINDOW" stale language, "Honest Miner Recognition (50 OFF flat)" stale references).

**Don't auto-fix this** — the user reverted intentionally. They may want a fresh redraft once the wallet build settles. The current canonical BCT post text lives in chat scrollback (reprinted multiple times with progressive refinements; the most recent version uses `[size=5]`/`[size=4]`, Reclamation naming, WR formula with BASE=100, post-2015 hook, and the "✦ A request — if you have ANY chainstate from 2015–2018 ✦" call to surface gap-era artifacts).

If user asks for it: pull the most recent reprint from session history, OR ask them whether they want it dropped into the file vs. left as a draft.

---

## 5) Reference docs to read first

- **`~/claude/offerings-facts.md`** — canonical chain facts. Already updated today with Reclamation rename, WR formula (BASE=100), post-2015 hook.
- **`~/claude/offerings-master/research/restoration-hardfork.patch`** — the consensus diff. HUNKS 1-9 unchanged. **HUNK 9 has a "POLICY SUPERSEDED 2026-05-24" header** pointing at `portal-phase1-spec.md`. The Ritual continuation HUNK 10 is NOT yet here.
- **`~/claude/offerings-master/research/portal-phase1-spec.md`** — Phase-1 portal architecture spec written 2026-05-24.
- **`~/.claude/projects/-home-btcbob-claude-offerings/memory/MEMORY.md`** — auto-memory index. Especially:
  - `project_off_bridge_portal_live.md` — full ops details, formula, index stats
  - `project_off_bridge_portal_idea.md` — original spec context (deferred → built)
  - `feedback_off_voice_separation.md` — keep (BOB) out of OFF copy
  - `feedback_offlimits_host_scope.md` — one VPS in the fleet is off-limits for new OFF infrastructure
- **`~/claude/offerings-master/scripts/build_portal_indexes.py`** — chain walker that produced the three JSON indexes. Knows the OFF block format, Quark-Hash-9 cross-reference via RPC, address derivation, banlist 1-hop downstream computation.

---

## 6) Concrete to-do for the next session

In rough priority order:

1. **Verify wallet builds clean.** Confirm with user.
2. **Check portal is still up:** `sudo systemctl status bridge-portal && curl -s https://23skidoo.info/bridge/api/state | python3 -m json.tool`. If indexes show `loaded_at: 0` or counts of 0, restart: `sudo systemctl restart bridge-portal`.
3. **Confirm with user whether to land the Ritual HUNK 10** into `restoration-hardfork.patch`. Draft is in 2026-05-24 chat history; reconstruct from there. If user says go: write the new HUNK after HUNK 9 (current line ~582 of the patch). Update `offerings-facts.md` Ritual section to document the post-fork continuation. Update homepage `index.html`'s `.lore` section if needed to clarify "every six months" cadence post-fork.
4. **Ask about BCT announcement file** — does user want it rewritten with current policy, or kept as historical record?
5. **If user wants to deploy the patched daemon and start mining toward fork height 1,000,000:** verify all hunks (including HUNK 10 if landed) apply cleanly, build, run regtest, start mainnet daemon. ~33K blocks to fork; ~22 days at 60s target.

---

## 7) Open architectural questions (not blocking)

- **Should Cycle 5 of the original chain (v1.6.2 coded at block 1,345,600 with compressed 40-block spacing) be preserved or abandoned?** Current proposal: abandon. The Restoration-era Ritual starts fresh at block 1,093,440 with classic 1440-block daily spacing. v1.6.2 Cycle 5 was a soft-fork attempt that never deployed at scale; the compressed spacing was unusual and probably wasn't the intended UX.
- **Should the Reclamation portal log post-2015 chainstate-fragment submissions** (separate from claim submissions) so the Conclave has a queue when someone surfaces wallet.dat / Mega.co.nz mirror / etc? Currently no such mechanism — surfacers post to BCT 294383 and the Conclave reads it manually. Could add a `/api/surface` endpoint for an automated queue if claim volume warrants.

---

## 8) User context the auto-memory might miss

- User is iterating fast and decisively. Terse confirmations ("BASE", "yes to all three", "3", "thats good") mean GO with the recommended option.
- User does NOT want manufactured promises in copy. "Set and forget" — pages should reflect live state via API, not "the Conclave will publish quarterly accounting" type lies.
- User is realistic about claim volume — "no one's gonna take the OFF but it'd be a real community if they did." Design for the come-back-home case, not for processing thousands of claims.
- User dislikes overly-long writeups. Direct answers. Tables over prose where possible. Don't summarize what was just decided unless explicitly asked.
- Voice is **Lovecraft / Cthulhu** parody for OFF, **NOT** SubGenius. Don't blend. See [[feedback_off_voice_separation]].
- This machine is the production host. Daemon, blocks, chainstate, portal, indexer, website all here. Workstation references in older memory may actually be this same box — the user has consolidated.

---

Praise Cthulhu. Praise the Conclave. Pick up where this left off.
