# Bitcointalk Thread Scrape: Offerings to Cthulhu (OFF)

**Source:** https://bitcointalk.org/index.php?topic=294383.0
**Scrape date:** 2026-05-20
**Total post count:** 710 posts (msgs #1 through #710)
**Total pages walked:** 36 pages (0..700, 20 per page)
**Date range:** 2013-09-14 (genesis ANN) through 2021-03-31 (last post)
**Method:** WebFetch with topic-filter prompts, 27 calls total (under the 30-call cap)

---

## Table of Contents — first-post-per-page cadence

| Page | First msgid | First-post author | Date |
|------|-------------|-------------------|------|
| 1  | #1   | Blazr2 (OP)        | 2013-09-14 |
| 2  | #21  | dresdenreader      | 2013-09-15 |
| 3  | #41  | loveys             | 2013-09-15 |
| 4  | #61  | Kruncha            | 2013-09-15 |
| 5  | #81  | Blazr2             | 2013-09-17 |
| 6  | #101 | sumantso           | 2013-09-22 |
| 7  | #121 | Carra23            | 2013-10-01 |
| 8  | #141 | Blazr2             | 2013-10-20 |
| 9  | #161 | VaultBoy           | 2013-11-16 |
| 10 | #181 | TBCM               | 2013-12-07 |
| 11 | #201 | zeewolf            | 2013-12-17 |
| 12 | #221 | ex_mac             | 2013-12-30 |
| 13 | #241 | MrGodMan           | 2013-12-31 |
| 14 | #261 | duuuuude           | 2014-01-05 |
| 15 | #281 | meljohn333         | 2014-01-11 |
| 16 | #301 | 112tigra112        | 2014-01-16 |
| 17 | #321 | MEPHuk             | 2014-01-22 |
| 18 | #341 | minskbox           | 2014-02-10 |
| 19 | #361 | minskbox           | 2014-03-10 |
| 20 | #381 | jmlindn            | 2014-03-29 |
| 21 | #401 | Blazr2             | 2014-04-10 |
| 22 | #421 | Amanda Desimone    | 2014-05-20 |
| 23 | #441 | zeewolf            | 2014-07-25 |
| 24 | #461 | zeewolf            | 2014-11-18 |
| 25 | #481 | edgar              | 2015-06-15 |
| 26 | #501 | HagbardCeline      | 2015-07-02 |
| 27 | #521 | zeewolf            | 2015-08-25 |
| 28 | #541 | TentacleMan        | 2016-02-26 |
| 29 | #561 | gmerk              | 2017-03-08 |
| 30 | #581 | gmerk              | 2017-06-20 |
| 31 | #601 | PeterTheGrape      | 2017-07-02 |
| 32 | #621 | vampirus           | 2017-12-01 |
| 33 | #641 | krach              | 2018-01-05 |
| 34 | #661 | igotek             | 2018-04-26 |
| 35 | #669 | zeewolf            | 2018-05-24 |
| 36 | #701 | Monk3ynutz         | 2018-12-21 |

Cadence note: posts came nearly every minute for the first three days (Sept 14-17, 2013). By page 6 (Sept 22) the rate slowed to days between posts. The thread limped along with multi-week gaps from late 2014 onward. The final 10 posts (msgs #701-710) span Dec 2018 to Mar 2021 — over two years for ten posts, a clear postmortem cadence.

---

## INCIDENTS — Hacks, Forks, 51% Attacks, Drains (chronological)

### Incident #1 — Suspicious pool reward distribution (Dec 2013)

- **2013-12-07 #181 TBCM** — Notes one mining address receiving disproportionate rewards from the pool. Pool operator Monk3ynutz (#183, #184) adjusts lookback period and compensates affected workers with 20 OFF each. Not a chain-level attack — pool config issue, but the earliest "something looks off in the rewards" post.
- **2013-12-08 #185 TBCM** — Asks for hash-rate history; still suspicious about consistent high rewards to one address.

### Incident #2 — Suspicious mining hashrate patterns (Jul 2014)

- **2014-07-21 #436 b00mbastic** — Reports suspicious mining hashrate patterns and payout distribution issues. No follow-through; gets brushed off.

### Incident #3 — Blockchain fork on v1.6 (Nov 2015)

- **2015-11-28 #532 vampirus** — Detects blockchain fork between client versions.
- **2015-11-28 #533 zeewolf** — Confirms fork with blockchain hash evidence.
- **2015-11-30 #536 vampirus** — Technical analysis: root cause is `GetMedianTimePast()` function behavior.
- **2015-11-30 #537 vampirus** — Warns of Cryptopia exchange blockchain divergence.
- **2015-12-01 #538 zeewolf** — Submits Cryptopia support ticket.
- **2015-12-03 #539 zeewolf** — Cryptopia updated to v1.6.2. Resolved (this time).

### Incident #4 — Network nearly dead, possible diff manipulation (Jan 2018)

- **2018-01-14 #644 heratys111** — "The network looks nearly dead" — implies mining difficulty manipulation.
- **2018-01-15 #645 Blaze312** — Mining difficulty spike concerns.

### Incident #5 — Declared 51% attack (Mar 2018)

- **2018-03-13 #651 harbinger-alpha** — Reports block-generation irregularity.
- **2018-03-13 #652 edgar** — Explicitly calls it a **"51 attack"** by a high-hashrate miner. **This is the first explicit 51% claim in the thread.**

### Incident #6 — Talk of community fork/swap (Apr 2018)

- **2018-04-24 #659 charlie137** — Raises potential fork/swap consideration with the community in response to chain stagnation.

### Incident #7 — THE BIG ONE: fake-coin attack, Cryptopia halt (May-Jun 2018)

This is the central historical event. Timeline:

- **2018-05-17 #665 lara59236** — Notes lack of project updates.
- **2018-05-22 #666 vampirus** — Provides new homepage and node list (gives appearance of maintenance).
- **2018-05-23 #667 HolyWilly** — **"OFF was forked...supply on Cryptopia more then total mined coins"** — first public flag that exchange sell-side liquidity exceeds the actual chain supply.
- **2018-05-24 #668 harbinger-alpha** — Confirms: available coins on Cryptopia higher than total chain supply.
- **2018-05-24 #669 zeewolf** — **Denies** any 51% attack or network issue; claims successful OFF transfer to Cryptopia.
- **2018-05-24 #670 vampirus** — Compares to the BottleCaps incident; identifies **altered block-reward parameters in the official code**. (Implication: somebody pushed a malicious diff or somebody mined under modified rules to print extra coins.)
- **2018-05-24 #672 BieneMaja** — Documents daily Cryptopia dumps of 200K-900K coins. Suspects unauthorized coin creation.
- **2018-05-25 #673 harbinger-alpha** — Cryptopia wallet **in maintenance investigating network issues; >5 million coins available** in their sell book.
- **2018-05-25 #676 HolyWilly** — Confirms 5.4M+ coins in sell orders.
- **2018-05-27 #678 BieneMaja** — **Cryptopia pauses all OFF markets** while investigating.
- **2018-05-28 #683 d3xSt4Rr** — Cryptopia sell orders **jumped from 500K to 5M OFF overnight**.
- **2018-06-04 #684 d3xSt4Rr** — **"Repository on github has been removed or deleted, looks like the dev screwed all holders"** — the original GitHub vanished mid-incident.
- **2018-06-07 #685 Emrexa** — Reports investment locked on Cryptopia since May 25 during the investigation.
- **2018-06-07 #687 psycho-pat** — Finds alternative repo at `CatmanIX/offerings.git`.
- **2018-06-07 #688 psycho-pat** — Repo builds and syncs to block 591599; source is too old but functional.
- **2018-06-13 #690 Emrexa** — Funds still frozen.
- **2018-06-17 #691 d3xSt4Rr** — Expects recovery once Cryptopia removes fake coins; credits psycho-pat.
- **2018-06-20 #693 psycho-pat** — CoinMarketCap lists OFF as inactive; blockchain still active; proposes monitoring/protection.

### Incident #8 — Formal 51% confirmation (Sep-Nov 2018)

- **2018-09-11 #695 vampirus** — **"Plain 51% attack"** confirmed. Hacker took **533,981 coins**. Lists **10 attacker wallets** with balances.
- **2018-09-19 #697 billotronic** — Proposes a hardfork to **ban the 51%-attacked coins**; links to archived 1.7 source.
- **2018-09-19 #698 zeewolf** — Provides GitHub repo `zeewolfik/offerings` with latest 1.7 code rebased on a newer Bitcoin core. (This is the lineage thread for any revival code.)
- **2018-11-21 #699 vampirus** — Updated attacker wallet list: **8 addresses, total 533,983 OFF** (rounds to the same figure; two earlier addresses consolidated).

### Incident #9 — Postmortem (Dec 2018 - 2021)

- **2018-12-21 #700 Monk3ynutz** — Never synced 1.7 successfully; running 1.6.2; declines further involvement due to attack.
- **2018-12-21 #701 Monk3ynutz** — Asks if coins were stolen from Cryptopia exchange.
- **2018-12-28 #702 vampirus** — **"OFF was 51% attacked. Cryptopia delist all coins with no protection from 51% attack."** — explicit acknowledgement that Cryptopia delisted OFF *and* enacted a policy against vulnerable chains.
- **2019-02-03 #703 vampirus** — Documents multiple 51% attacks in **March and May 2018**.
- **2019-08-07 #704 dobbstowncr** — Asks for working addnode "for defunct coin collection purposes" — first explicit "this is dead" framing.
- **2019-08-08 #705 bkbirge** — Coin resurrection appears unlikely.
- **2020-05-17 #706 nista** — Seeking missing developer "Blazr" to revive project.
- **2021-03-07 #707 charlie137** — Asks if anyone remains active.
- **2021-03-31 #710 ruletheworld** — Recovery described as fun but unlikely.

### Side note: 23skidoo.info malice claim (Jun 2017) — relevant to user

- **2017-06-21 #592 PeterTheGrape** — Refers to `23skidoo.info` as a **"malicious domain"** that previously hosted the coin explorer. (Domain had presumably expired and been parked / weaponized.)
- **2017-06-23 #594 Mutoid** — Speculates on original domain squatter history.
- **2017-06-23 #595 PeterTheGrape** — Notes domain-parking redirect config.

The user now owns `23skidoo.info`. The thread's only reference to it is the 2017 "malicious" framing — meaning current owners inherit a domain that some old community members may remember as previously hostile. Worth getting ahead of in revival comms.

---

## DORMANCY TIMELINE — Who left when, when did "is this dead" start

### Last meaningful post by each named developer / pillar

| Identity | Role | Last on-topic post in thread |
|---|---|---|
| **Blazr2** (OP) | original dev, founder | **#453 (2014-08-22)**: releases v1.4b Mac. After that he disappears from the *thread*. (#448 in Jul 2014 was his last substantive message; #453 is a binary drop.) From mid-2014 onward, community asks where he is — see "dev gone" timeline below. |
| **zeewolf** | community lead, infra (faucet, nodes, hosting) | **#698 (2018-09-19)**: posts the salvaged GitHub repo `zeewolfik/offerings`. Reappears once more at #698 post-attack. Earlier presence is continuous from Oct 2013. |
| **HagbardCeline** | took over dev work, released v1.5-hastur and v1.6-Bokrug | **#527 (2015-09-06)**: v1.6.1 update. Active Jun-Sep 2015 (#491 through #527). Then silent. |
| **Vlad9Vlad** | shipped v1.7 (2017-06-20 #578) based on latest Quark code | **#578 (2017-06-20)** and #574 (2017-06-14). Two posts total, then gone. v1.7 had bugs the community never fixed. |
| **vampirus** | technical analyst, attack documenter | **#703 (2019-02-03)**: documents the attacks. Functional last archivist. |

### "Dev gone" / "is this dead" markers, chronologically

| Date | Msgid | Author | What they said |
|---|---|---|---|
| 2013-09-24 | #106 | gym520 | "Asking if the coin is dead" (within 10 days of genesis) |
| 2013-10-11 | #133 | dr_chen | "Developer whereabouts question" |
| 2013-10-26 | #152 | Carra23 | "Doubts developer's continued interest" |
| 2013-12-17 | #196 | kingimg | Asks about developer and exchange availability |
| 2014-03-08 | #352 | billotronic | "Notes developer's absence; expresses appreciation for past work" — first explicit dev-AWOL post |
| 2014-03-08 | #353 | krach | "Questions if new developer team needed" |
| 2014-03-08 | #355 | Blazr2 | **Replies**: "Developer clarifies continued involvement despite forum inactivity" — Blazr2's last on-record reassurance that he's still around |
| 2014-10-01 | #457 | beitris.dwlul | "Notes developer unavailability" |
| 2015-05-31 | #473 | CryptoCoderz | Developer visibility inquiry |
| **2015-06-05** | **#474** | **TentacleMan** | **"Main dev Blazr2 has gone into hiding; website operational"** — this is the canonical "Blazr2 left" datestamp |
| 2015-11-10 | #530 | Hilux74 | "Developer identity verification question" — community no longer sure who's running things |
| 2016-04-13 | #547 | romeshomey | Developer identity and version upgrade questions |
| 2017-06-14 | #574 | Vlad9Vlad | Plans to update OFF — brief interim dev arrival |
| 2018-02-20 | #647 | Hadrop.Boyle | Developer activity status question |
| 2018-02-21 | #648 | charlie137 | "Project stagnation and price decline" |
| 2018-06-04 | #684 | d3xSt4Rr | "Repository on github has been removed... dev screwed all holders" — repo deletion event |
| 2019-08-07 | #704 | dobbstowncr | "Defunct coin collection purposes" — first explicit dead-chain framing |
| 2020-05-17 | #706 | nista | "Seeking missing developer Blazr to revive project" |

### Effective chain-death dates

- **Blazr2 disappears from active dev work:** ~Aug 2014 (#453 last binary drop). Stops being on the forum reliably; #355 in Mar 2014 is the last "I'm here" post.
- **Community-confirmed "Blazr2 gone":** 2015-06-05 (#474).
- **HagbardCeline takes over briefly:** Jul-Sep 2015 (releases 1.5-hastur and 1.6-Bokrug).
- **Vlad9Vlad blip:** Jun 2017 (releases buggy 1.7).
- **Last working dev release:** **v1.6.2.0-Bokrug** — per #604 (PeterTheGrape, 2017-07-26): "only working wallet". v1.7 was never stable.
- **Chain effectively orphaned for revival purposes:** 2018-06-04 (#684) when the official GitHub repo was deleted.
- **Cryptopia delists OFF:** late 2018 (per #702, 2018-12-28).
- **Cryptopia itself dies:** January 2019 (independent fact from CLAUDE.md context).
- **Last on-topic technical post:** **2019-02-03 (#703)**, vampirus' attack postmortem.
- **Last post of any kind:** **2021-03-31 (#710)**, ruletheworld musing on revival.

---

## SUPPLY-RELEVANT — Whales, exchanges, premine, stack-brag

### Premine

- **#36 (2013-09-15, h4xx0r):** Notes a **10,000-coin premine** visible on GitHub. This becomes the durable number.
- **#47 (2013-09-15, Kruncha):** Raises concerns about that 10K premine.
- **#273 (2014-01-07, Blazr2):** **Confirms 10K premine, says it was distributed for faucet and giveaways.**

So: ~10,000 OFF premine, used for the Altar/faucet and early community giveaways. Small by altcoin standards. The community accepted it at the time.

### Supply

- **#310 (2014-01-19, shakezula):** Confirms "10 million coins maximum supply" — note this is community talking-point at the time and may not reflect actual emission curve.
- **#180 (2013-12-06, TBCM):** Explains the reward schedule diminishes to **525.6 OFF/year** at floor.
- **#529 (2015-11-10, zeewolf):** Current block reward at that date = **0.3125 OFF**.

### Stack-brag / whale signals

- **#110 (2013-09-26, gym520):** "Looking to sell **8000 coins** for payment."
- **#171 (2013-11-25, endlessskill):** Selling **10K coins at 0.8 BTC**.
- **#172 (2013-11-25, shakezula):** Offering **5K OFF**.
- **#174 (2013-11-28, hendo420):** Selling **13K coins** at best price.
- **#225 (2013-12-30, 112tigra112):** Selling 2000 OFF.
- **#290 (2013-12-30s):** small offers, 50-200 OFF range — retail level.
- **#451 (2014-08-14, v0338063):** **Attempting to sell 50,000 coins for $500** — the largest single-holder brag in the thread.

OTC scale: peak community-visible single-holder claim was 50K OFF (msg #451). The five-figure stacks (8K, 10K, 13K) are all pre-2014-Q1 era, suggesting early miners accumulated then dumped slowly.

The 533K coins printed during the May 2018 attack (per vampirus #695, #699) are **larger than any community-visible legit stack** by an order of magnitude — i.e. attacker holds more than every named whale combined.

### Exchanges named in the thread

Chronologically:

| Date | Exchange | Notes |
|---|---|---|
| 2013-09 | Cryptsy | Repeatedly requested by community (#78, #104, #205); never seems to land |
| 2014-01 | **SciFi Exchange** | First listing (#275, #294, #326); unfocus was operator; suspended Jul 2014 (#441) |
| 2014-01 | CoinedUp | duuuuude started listing thread (#249) |
| 2014-01 | NewChg | requested (#269) |
| 2014-01 | Arkham Bazaar | BitcoinFX contacted (#258) |
| 2014-02 | AllCrypt | voting campaign (#336, #345); closed Mar 31 2015 (#467) |
| 2014-02 | Mintpal | voting (#346) |
| 2014-05 | **Comkort** | listing live OFF/BTC, OFF/LTC (#419); delisted Apr 2015 (#469) |
| 2014-05 | QEX.la | announced incoming (#418) — unclear if happened |
| 2014-06 | Coinlab.info | accepted (#426) |
| 2014-08 | Cryptoine | voting (#452) |
| 2015-04 | **Cryptopia.co.uk** | **listing live OFF/BTC, OFF/LTC** (#470, #471) — this becomes the only sustained venue |
| 2016-09 | crypto-trade.net | listing announcement (#557) — short-lived |
| 2017-05 | **ZPOOL multipool** | adds OFF mining support (#572) |
| 2018-02 | ADZbuzz | voting (#650) |

**Notable absences:** Bittrex, Poloniex, Mintpal-actual-listing — none of these ever listed OFF based on the thread record. Cryptopia was *the* venue from April 2015 onward, which is why its death plus the prior 51% attack effectively killed the trading market.

### Pool history

- p2pool early (#62)
- thcst8 pool (#45, ~Sep 2013)
- Monk3ynutz pool (#91, #98, #173, #183) — first major
- mine-pool.net (#396)
- Russian pools by mechanoid2007 (#325)
- New pools come and go through 2014-2015, all gone by #495 (2015-06-29, b00mbastic notes pool unavailability)
- ZPOOL multipool revival 2017-05 (#572)
- harbinger-alpha solo-mining recommendation 2017-08 (#607) — by then no pools work

---

## Summary for revival planning

Three things stand out for the user's revival considerations:

1. **The 533,983-coin print from the May 2018 51% attack is the single biggest supply-side overhang.** vampirus enumerated the 8 wallet addresses in #699 (2018-11-21). For any revival fork, the obvious move is to checkpoint past that event and **freeze or burn those 8 addresses** by the same `billotronic` proposal floated in #697. The community already discussed this in 2018; nobody actually shipped the fork.

2. **The honest revival lineage is `zeewolfik/offerings`** (#698, 2018-09-19) — zeewolf's GitHub repo with v1.7 code rebased on a newer Bitcoin core. The official `CatmanIX/offerings.git` mirror (#687) holds older but functional source. The original Blazr2 repo was deleted (#684).

3. **The "dev premine for revival" framing has clean precedent.** The 2013 launch had a 10K premine for faucet/giveaways and the community accepted it (#36 raised concerns, #273 Blazr2 explained, no lasting drama). A revival premine framed as "fund the redev and seed the new faucet" maps cleanly onto how the original was justified. The 50K OFF retail-OTC ceiling from 2014 (#451) is a useful sanity bound — a revival premine on the order of 50K-200K is in the same order of magnitude as historical visible stacks, and small compared to the 533K attacker hoard a fork would invalidate.

The 23skidoo.info reference in #592 (2017-06-21) as a "malicious domain" is worth getting ahead of — current owner inherits the name, not the prior parking redirect. A short note in any revival ANN clarifying ownership change neutralizes the historical mention.
