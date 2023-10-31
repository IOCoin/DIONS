#pragma once

#include "ccoin/Process.h"
#include "wallet/Wallet.h"


CBlock* CreateNewBlock(__wx__* pwallet, bool fProofOfStake=false, int64_t* pFees = 0);


void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);


void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1);


bool CheckWork(CBlock* pblock, __wx__& wallet, CReserveKey& reservekey);


bool CheckStake(CBlock* pblock, __wx__& wallet);


void SHA256Transform(void* pstate, void* pinput, const void* pinit);

