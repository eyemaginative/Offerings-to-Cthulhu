# Honest Holder Bridge — Phase-1 Portal Spec

**Status:** draft, 2026-05-24
**Author:** session at workstation, working with btcbob
**Scope:** Phase 1 only — verification + BCT-post generation. Phase 2 (Conclave queue, multisig signing, broadcast) deferred.

---

## Policy changes that this spec assumes

These supersede the original `restoration-hardfork.patch` HUNK 9 text. The HUNK is research/announcement-layer, not consensus code, so editing it is free. To formalize, update HUNK 9 to match the policy stated here, and re-publish the BCT announcement draft.

### P1. The 12-month window is removed.
The Bridge stays open **indefinitely** as long as the 1.5M ceiling has remaining capacity. Realistic claim volume is low (most 2015–2018 holders lost keys, died, or disengaged); a calendar deadline punishes them for being out of the loop without serving any real auditability goal. Closure trigger is **ceiling exhaustion**, not date.

### P2. New tier — Honest Miner Recognition (HMR).
Anyone who can prove they ever received a coinbase reward on the OFF chain gets a flat **50 OFF per address**, regardless of whether they kept the OFF, regardless of class.

| Sub-tier | Proof | Verification effort |
|---|---|---|
| HMR-A | Address appears as coinbase vout in any block 0–966,413 | Zero — direct chainstate lookup. Auto-approve. |
| HMR-B | Wayback-archived 2015–2018 pool dashboard / explorer page showing the address as a coinbase or pool-payout recipient, plus signmessage | Conclave reviews the URL + archive snapshot |

HMR is **additive** to balance restoration: a Class A holder who still controls UTXOs at 966,413 keeps those UTXOs *and* claims HMR-A. The 50K-per-address absolute ceiling still binds, as does the 1.5M program ceiling and the #699 banlist + 1-hop downstream check.

### P3. Reframe — community gesture, not audit window.
The Bridge isn't a regulatory-style reparations program with a closing date. It is an open invitation: if you mined OFF honestly, here is something. If you held OFF honestly, here is restitution. The chain came back; come back with it. This framing should drive the public copy on `/bridge/`.

---

## Portal goals

Three concrete asks:

1. **Cut Conclave verification toil** — automate the cryptographic and chainstate parts so the Conclave only handles the judgment-call parts.
2. **Cut claimant friction** — wizard-style flow that figures out a claimant's class for them and pre-formats the BCT post.
3. **Be honest** — surface what the portal can and cannot prove. No false trustlessness theatre.

**Non-goals:** the portal does NOT handle key material, NOT sign transactions, NOT broadcast disbursements. All custody stays with the 2-of-3 Conclave multisig. BCT thread remains the authoritative audit log.

---

## URL layout

```
https://23skidoo.info/bridge/          → existing explainer page
https://23skidoo.info/bridge/claim/    → portal entry / wizard
https://23skidoo.info/bridge/api/...   → JSON endpoints (read-only)
```

Co-locating with the existing /bridge/ explainer keeps the mental model simple; users land on the explainer, scroll to "Begin claim", flow into the wizard.

---

## Wizard flow

### Step 0 — Entry (`/bridge/claim/`)
- Restate the prerequisites: old wallet that can `signmessage`, an OFF address, ~5 minutes
- Display current week's challenge string (rotates Mondays 00:00 UTC; see "Challenge string" below)
- Big "Begin" button → Step 1

### Step 1 — Identify (`/bridge/claim/identify`)
- Input: claimant's historical OFF address (Q-prefix)
- Backend `GET /api/lookup?addr=Q…` returns:
  - `banned: bool` — in #699 list
  - `banned_one_hop: bool` — direct recipient from a banned address
  - `chainstate_balance: int` (sats) — UTXOs at 966,413, if any
  - `is_chainstate_miner: bool` — appears as coinbase vout in chain history
  - `recommended_class: "A" | "B" | "HMR-A" | "HMR-B" | "DISQUALIFIED"`
  - `notes: string[]` — human-readable explanations
- UI branches:
  - **Banned / 1-hop banned** → red, explain, dead-end
  - **Class A holder (UTXO balance)** → "You're already whole — see explainer; do you also want HMR-A?"
  - **HMR-A only (mined but spent/transferred)** → "You qualify for 50 OFF recognition payment"
  - **Class B (no chainstate presence)** → "Discretionary claim — proceed if you have a story"
- "Continue" → Step 2

### Step 2 — Sign (`/bridge/claim/sign`)
- Render the exact CLI command, pre-filled with address + challenge:
  ```
  Offerings-cli signmessage Q…xY "Conclave-Restoration-Bridge-20260525w-Q…xY-50.0"
  ```
- (Or: link to a future browser-side signing helper using a JS library — Phase 1.5)
- Textarea for the user to paste the base64 signature back
- "Verify" → calls `POST /api/verify` with `{addr, sig, msg}` → green/red
- On green, proceeds to Step 3

### Step 3 — Compose claim (`/bridge/claim/compose`)
- Inputs the wizard collects:
  - Claim amount (pre-filled from class: 50 for HMR, balance for Class A, 1000 default for Class B with cap warning)
  - New-chain destination address (Q-prefix, validated)
  - Story textarea (required for Class B, optional for HMR-B with Wayback URL field, hidden for HMR-A and Class A)
- "Generate BCT post" → Step 4

### Step 4 — Submit (`/bridge/claim/submit`)
- Renders the canonical BCT post block in a copy-friendly `<pre>`:
  ```
  [code]
  Tier: HMR-A
  Claim: Q…historical-address
  Amount: 50.0 OFF
  Signature: <base64>
  Challenge: Conclave-Restoration-Bridge-20260525w-…
  Destination: Q…new-address
  Story: (optional, Class B / HMR-B only)
  Evidence URL: (HMR-B only)
  [/code]
  ```
- Big "Copy to clipboard" button
- "Now post this to BCT thread 294383 →" with deep link
- Optional: "Paste your BCT post URL back here when you've posted, so the Conclave can see it sooner" → logs to the Conclave review queue

---

## Backend — read-only verification service

### Where it runs
- **Phase 1:** lightweight Python service on the production host, talking to the Offerings daemon over an SSH-tunnelled RPC from the workstation. Read-only.
- **Phase 2:** dedicated Offerings daemon co-located with the portal once the restored chain is live and stable. Cuts the SSH-tunnel dependency.

### Stack
- Python 3.11 + FastAPI (or Flask if we want fewer deps)
- gunicorn + systemd unit
- nginx reverse-proxy at `https://23skidoo.info/bridge/api/`
- No database — see "State" below

### Endpoints

```
GET  /api/challenge                           → current week's string + ISO-week tag
GET  /api/lookup?addr=Q…                       → lookup result (see Step 1)
POST /api/verify                              → {addr, sig, msg, claim_amount} → {valid, reason, recommended_tier}
GET  /api/state                               → {claims_logged, ceiling_remaining, hmr_count, classB_count}
POST /api/log_submission (optional)           → {bct_post_url, addr, tier, amount} → ack
```

### Static indexes built once
To avoid walking the chain on every lookup:

- `miners_index.json` — map of `address → list of {height, txid}` for every coinbase recipient in blocks 0–966,413. Built once from a chainstate walk, ~few MB. Refresh nightly after fork to include new restored-chain blocks.
- `chainstate_utxo_index.json` — map of `address → {balance_sats, utxo_count}` at block 966,413. Built once from `gettxoutsetinfo` walk.
- `banlist.json` — the 8 #699 addresses + (computed) all 1-hop downstream addresses from chainstate walk.

These are flat files served from disk; lookup is O(1) hashmap.

### State / persistence
- Portal **does not** store user secrets or key material
- Optional: `claims_log.json` (append-only) of `{timestamp, addr, tier, amount, bct_url}` for Conclave review queue — purely a convenience, BCT remains authoritative
- No claim is ever marked "approved" by the portal; the portal only marks it "verified-and-submittable"

---

## Challenge string

Format and rotation kept from the original spec:
```
Conclave-Restoration-Bridge-<YYYYMMDDw>-<address>-<claim-amount-in-OFF>
```
- `YYYYMMDDw` = ISO-week start date (Monday) in `YYYYMMDDw` form, e.g. `20260525w`
- Rotates Monday 00:00 UTC
- Backend `/api/challenge` returns the current week's string; portal renders it everywhere needed
- Verification accepts signatures against the current week OR the previous week (1-week grace for users who started mid-week)

---

## Anti-abuse

The portal inherits the existing program guards:

1. **#699 banlist check** — auto-reject at `/api/lookup`
2. **1-hop downstream check** — auto-reject; computed once at index build
3. **Per-address ceiling: 50K OFF** — enforced at `/api/verify`
4. **Program ceiling: 1.5M OFF** — enforced at `/api/verify`; if `ceiling_remaining < claim_amount`, reject with explanation
5. **Sybil note**: a single human could in theory claim from many addresses they control. With HMR at 50 OFF flat, that's 200 addresses to gross 10K. The 50K per-address ceiling and the 1.5M program ceiling bound this. No further sybil mitigation.

The portal **does not** attempt rate-limiting per IP / browser. Verification is cheap; abuse would be Conclave-visible at BCT post time anyway.

---

## What this portal explicitly does NOT do

- ❌ Hold private keys
- ❌ Sign transactions
- ❌ Broadcast disbursements
- ❌ Auto-approve Class B / HMR-B claims (those require Conclave eyes on the story / evidence)
- ❌ Replace the BCT thread as audit log
- ❌ Custody anything

If we ever want any of these, that's Phase 2 and needs separate design + security review.

---

## Build estimate

| Component | Effort |
|---|---|
| Backend service + endpoints | 1 day |
| Static indexes (one-time chain walk) | 0.5 day |
| Wizard UI (5 pages) | 1 day |
| nginx integration + systemd unit | 0.5 day |
| End-to-end test with workstation daemon | 0.5 day |
| **Total Phase 1** | **~3.5 days of focused work** |

Phase 2 (Conclave queue dashboard, multisig signing helper, broadcast UX) is at least 2× that and needs threat-modelling on the custody side.

---

## Open questions for next session

1. **HMR-B evidence standard.** What Wayback URL is good enough? Single cthulhu.tk explorer page showing one block coinbase to the address? A pool dashboard showing repeated payouts? Codify before publishing.
2. **Workstation→portal-host RPC tunnel resilience.** SSH tunnel can drop. Use `autossh` or build local read-replica indexes on the portal host and refresh nightly from a workstation dump.
3. **Should HMR claimants be required to provide a destination address?** Or could the portal optionally credit the *signing* address (if they still control it, the Restoration chain can pay there directly)? Simpler UX, but means HMR is paid into the historical address rather than a fresh one.
4. **Update the patch.** HUNK 9 still says 12-month window. Should the patch file be updated to reflect the new policy, with a note that the change post-dates the original draft? Or kept as historical record with a sibling `restoration-hardfork-policy-v2.md` documenting the revision?

---

## Sibling pages to update once policy is confirmed

- `/var/www/23skidoo.info/bridge/index.html` — remove 12-month window language, add HMR tier section, reframe as community gesture
- `/home/btcbob/codex/build_countdown.py` — update the Bridge bullet on `/awakening/` to drop "12-month" and the misleading "vanished in the chainstate gap" framing (since HMR covers people whose balances *didn't* vanish but who mined honestly)
- `~/claude/offerings-master/research/restoration-hardfork.patch` — update HUNK 9 to reflect the new window + HMR tier, OR add a sibling policy doc
- `~/claude/offerings-master/research/bct-revival-announcement.txt` — update the Bridge section to match
