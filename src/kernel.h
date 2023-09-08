#ifndef PPCOIN_KERNEL_H
#define PPCOIN_KERNEL_H 

#include "process.h"



static const int STAKE_TIMESTAMP_MASK = 15;


extern unsigned int nModifierInterval;



static const int MODIFIER_INTERVAL_RATIO = 3;


bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier);

bool CheckKernel(CBlockIndex* pindexPrev, unsigned int nBits, int64_t nTime, const COutPoint& prevout, int64_t* pBlockTime = NULL);



bool CheckStakeKernelHash(CBlockIndex* pindexPrev, unsigned int nBits, const CBlock& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, uint256& targetProofOfStake, bool fPrintProofOfStake=false);



bool CheckProofOfStake(CBlockIndex* pindexPrev, const CTransaction& tx, unsigned int nBits, uint256& hashProofOfStake, uint256& targetProofOfStake);


bool CheckCoinStakeTimestamp(int nHeight, int64_t nTimeBlock, int64_t nTimeTx);


unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex);


bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum);


int64_t GetWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd);

#endif
