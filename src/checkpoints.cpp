// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
		(0, uint256("0x000009d430e3047d749faf37b95b748d8c481a5e09d43f072af32bf490503e69"))
		(1, uint256("0x000000a0548f81e13270ba50855b129d408ccecc06d381a0eb880d0d8fa4157d"))
		(2, uint256("0x000005a603970e25230bee8b4f5b74b8b7e4ecfca5f64aed5f8b068bb74a839c"))
		(496, uint256("0x0000002368a1c64d34bc88ecbc467b42bca5a9b3dc1b1b0ad1094d71267e3df6"))
		(1284, uint256("0x0000013abdb2e1e50446f26f6d888de29b6b475c9e46747cd9f98c95453a65f6"))
		(1749, uint256("0x000000d9b27b45d3a9afcabf5cc7d36d190aa121facd14da721aca4c3dd4d59a"))
		(2153, uint256("0x000002feae50d69acd0e32a67000419e46d77174af0bd433412438b43101fbfe"))
		(2623, uint256("0x000004142b9af60df82c2c83d1801a78e543e5332ea76762a329bbba2ac07e8f"))
		(3111, uint256("0x000000a27356f9c9403668c29f44ca77bc917566bca089046bf92923f70494aa"))
		(3521, uint256("0x00000242c10fc0bf0628a8b04485ca7f70acd9a28c9a92f8d25eee722e202bb4"))
		(3890, uint256("0x0000000358ec2634a95304db3727f5279544e12e543bb1f1c46eb067d9c2b4a2"))
		(4154, uint256("0x000001e33a9ea39b838e6f752bbbaf1c098e6bff1d2f96f4afdb99a07a82dfd9"))
		
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1502353406, // * UNIX timestamp of last checkpoint block
        120,          // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        8000.0      // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet = 
        boost::assign::map_list_of
        (0, uint256("0x000009d430e3047d749faf37b95b748d8c481a5e09d43f072af32bf490503e69"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1404645149,
        0,
        300.0
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (fTestNet) return NULL; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    uint256 GetLastAvailableCheckpoint()
    {
        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints) {

            const uint256& hash = i.second;
            if(mapBlockIndex.count(hash) && mapBlockIndex[hash]->IsInMainChain())
              return(hash);
        }

        return(hashGenesisBlock);
    }

    uint256 GetLatestHardenedCheckpoint()
    {
        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
        return (checkpoints.rbegin()->second);
    }
}
