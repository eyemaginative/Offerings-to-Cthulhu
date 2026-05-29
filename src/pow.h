// Copyright (c) 2026 The Offerings developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include <stdint.h>

class CBlockIndex;
class CBlockHeader;

// HARD FORK ACTIVATION HEIGHTS — LWMA-3 + MAX_REORG_DEPTH defense
// vs rented-hash drive-by attacks. Companion to the h=976000 hardcoded
// checkpoint (mapCheckpoints) shipped in v2.0.1-Bokrug-checkpoint
// (non-consensus, 2026-05-29) — the checkpoint locks old history, this
// fork hardens the chain going forward.
//
// HARDFORK_LWMA3_MAIN_OFF: TBD — set ~7-10 days from release after
// testnet validation completes. Block-height activation (not wall-time)
// so the fork lands deterministically regardless of attack-compressed
// solvetimes.
static const int64_t HARDFORK_LWMA3_MAIN_OFF    = 990000;  // TBD post-testnet
static const int64_t HARDFORK_LWMA3_TESTNET_OFF = 100;

// MAX_REORG_DEPTH: chains attempting to reorganize past this many buried
// blocks of the active tip are rejected at consensus. 100 blocks ≈ 100
// minutes at OFF's 60s target — economically infeasible for an attacker
// to mine in secret on a small chain at any plausible hashrate.
static const int MAX_REORG_DEPTH = 100;

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock);
unsigned int GetNextWorkRequired_Legacy(const CBlockIndex* pindexLast, const CBlockHeader *pblock);
unsigned int GetNextWorkRequired_LWMA3(const CBlockIndex* pindexLast, const CBlockHeader *pblock);
int64_t LWMA3ForkHeight();

#endif // BITCOIN_POW_H
