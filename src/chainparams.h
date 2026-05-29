// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAIN_PARAMS_H
#define BITCOIN_CHAIN_PARAMS_H

#include "bignum.h"
#include "script.h"
#include "uint256.h"

#include <set>

#include <vector>

using namespace std;

#define MESSAGE_START_SIZE 4
typedef unsigned char MessageStartChars[MESSAGE_START_SIZE];

class CAddress;
class CBlock;

struct CDNSSeedData {
    string name, host;
    CDNSSeedData(const string &strName, const string &strHost) : name(strName), host(strHost) {}
};

/**
 * CChainParams defines various tweakable parameters of a given instance of the
 * Bitcoin system. There are three: the main network on which people trade goods
 * and services, the public test network which gets reset from time to time and
 * a regression test mode which is intended for private networks only. It has
 * minimal difficulty to ensure that blocks can be found instantly.
 */
class CChainParams
{
public:
    enum Network {
        MAIN,
        TESTNET,
        REGTEST,

        MAX_NETWORK_TYPES
    };

    enum Base58Type {
        PUBKEY_ADDRESS,
        SCRIPT_ADDRESS,
        SECRET_KEY,
        EXT_PUBLIC_KEY,
        EXT_SECRET_KEY,

        MAX_BASE58_TYPES
    };

    const uint256& HashGenesisBlock() const { return hashGenesisBlock; }
    const MessageStartChars& MessageStart() const { return pchMessageStart; }
    const vector<unsigned char>& AlertKey() const { return vAlertPubKey; }
    int GetDefaultPort() const { return nDefaultPort; }
    const CBigNum& ProofOfWorkLimit() const { return bnProofOfWorkLimit; }
    int SubsidyHalvingInterval() const { return nSubsidyHalvingInterval; }
    virtual const CBlock& GenesisBlock() const = 0;
    virtual bool RequireRPCPassword() const { return true; }
    const string& DataDir() const { return strDataDir; }
    virtual Network NetworkID() const = 0;
    const vector<CDNSSeedData>& DNSSeeds() const { return vSeeds; }
    const std::vector<unsigned char> &Base58Prefix(Base58Type type) const { return base58Prefixes[type]; }
    virtual const vector<CAddress>& FixedSeeds() const = 0;
    int RPCPort() const { return nRPCPort; }

    // ---- Restoration Hardfork v2.0.0 (BtcBob/SubGenius.Finance, 2026) ----
    int64_t RestorationForkHeight()    const { return nRestorationForkHeight; }
    const CScript& TreasuryScript()    const { return scriptTreasury; }
    int64_t PostForkSubsidy()          const { return nPostForkSubsidy; }
    int     TreasuryShareNumerator()   const { return nTreasuryNum; }
    int     TreasuryShareDenominator() const { return nTreasuryDen; }
    int64_t RestorationTithe()         const { return nRestorationTithe; }
    const std::set<CScript>& BannedAttackerScripts() const { return setBannedAttackers; }
    // Conclave signed-mining window: only blocks signed by a Conclave key are
    // valid for heights in [SignedWindowStart() .. OpenMiningHeight()]. The window
    // opens 9 blocks BEFORE RestorationForkHeight so the Descent verses
    // (heights 999991..1000000) are consensus-protected from outsider mining
    // alongside the canon-reading itself.
    int64_t SignedWindowStart() const { return nSignedWindowStart; }
    int64_t OpenMiningHeight()  const { return nOpenMiningHeight; }
    const std::vector<std::vector<unsigned char> >& ConclaveKeys() const { return vConclaveKeys; }

protected:
    CChainParams() {}

    uint256 hashGenesisBlock;
    MessageStartChars pchMessageStart;
    // Raw pub key bytes for the broadcast alert signing key.
    vector<unsigned char> vAlertPubKey;
    int nDefaultPort;
    int nRPCPort;
    CBigNum bnProofOfWorkLimit;
    int nSubsidyHalvingInterval;
    string strDataDir;
    vector<CDNSSeedData> vSeeds;
    std::vector<unsigned char> base58Prefixes[MAX_BASE58_TYPES];

    // ---- Restoration Hardfork v2.0.0 fields ----
    int64_t nRestorationForkHeight;
    CScript scriptTreasury;
    int64_t nPostForkSubsidy;
    int     nTreasuryNum;
    int     nTreasuryDen;
    int64_t nRestorationTithe;
    std::set<CScript> setBannedAttackers;
    int64_t nSignedWindowStart;
    int64_t nOpenMiningHeight;
    std::vector<std::vector<unsigned char> > vConclaveKeys;
};

/**
 * Return the currently selected parameters. This won't change after app startup
 * outside of the unit tests.
 */
const CChainParams &Params();

/** Sets the params returned by Params() to those for the given network. */
void SelectParams(CChainParams::Network network);

/**
 * Looks for -regtest or -testnet and then calls SelectParams as appropriate.
 * Returns false if an invalid combination is given.
 */
bool SelectParamsFromCommandLine();

inline bool TestNet() {
    // Note: it's deliberate that this returns "false" for regression test mode.
    return Params().NetworkID() == CChainParams::TESTNET;
}

inline bool RegTest() {
    return Params().NetworkID() == CChainParams::REGTEST;
}

#endif
