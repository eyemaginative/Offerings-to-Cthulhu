// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MINER_H
#define BITCOIN_MINER_H

#include <stdint.h>

class CBlock;
class CBlockIndex;
struct CBlockTemplate;
class CReserveKey;
class CScript;
class CWallet;

/** Run the miner threads */
void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads);
/** Generate a new block, without valid proof-of-work */
CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn);
CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey);
/** Modify the extranonce in a block */
void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);
/** Sign the block's OFFSIG output if in the Conclave signed-mining window. */
bool SignBlockIfNeeded(CBlock* pblock, int nHeight, CWallet* pwallet);
/** v2.0.0: Rebuild the coinbase Miningcore-shape for external pool consumption.
 *  Replaces vout[0] with pool address, keeps Treasury/OFFSIG outputs, writes a
 *  scriptSig with a fixed 8-byte extranonce placeholder (so Miningcore can
 *  reconstruct shares), preserves the Codex chunk, recomputes merkle root,
 *  signs OFFSIG. Returns false if poolAddrStr is empty/invalid or signing fails.
 *  On success: serializedCoinbase is the full tx bytes; extranonceOffset is the
 *  byte index where the 8-byte placeholder starts (caller splits there). */
#include <string>
#include <vector>
bool BuildPoolCoinbase(CBlock* pblock, int nHeight, CWallet* pwallet,
                       const std::string& poolAddrStr, const std::string& tag,
                       std::vector<unsigned char>& serializedCoinbase,
                       size_t& extranonceOffset);
/** Do mining precalculation */
void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1);
/** Check mined block */
bool CheckWork(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey);
/** Base sha256 mining transform */
void SHA256Transform(void* pstate, void* pinput, const void* pinit);

extern double dHashesPerSec;
extern int64_t nHPSTimerStart;

#endif // BITCOIN_MINER_H
