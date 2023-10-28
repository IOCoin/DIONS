#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#include "BigNum.h"
#include "Sync.h"
#include "Net.h"
#include "Script.h"
#include "crypto/HashBlock.h"
#include "ccoin/Block.h"
#include "ptrie/FixedHash.h"
#include "ptrie/TrieDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "TxMemPool.h"
#include "DiskBlockLocator.h"

#include <list>


class __wx__;
class CBlock;
class CBlockIndex;
class CKeyItem;
class CReserveKey;
class COutPoint;
class CAddress;
class CInv;
class CRequestTracker;
class CNode;
class CTxMemPool;
#include "Constants.h"
#include "ConfigurationState.h"
extern bool fReindex;

#define LR_R 0x32


inline bool MoneyRange(int64_t nValue)
{
  return (nValue >= 0 && nValue <= ConfigurationState::MAX_MONEY);
}


inline bool IsProtocolV2(int nHeight)
{
  return nHeight > 65000;
}
inline bool V3(int nHeight)
{
  return fTestNet ? nHeight >= 0x0a : nHeight >= 1625000;
}
inline bool V4(int nHeight)
{
  return fTestNet ? nHeight >= 0x10 : nHeight >= ConfigurationState::SHADE_FEATURE_UPDATE;
}
inline bool V5(int nHeight)
{
  return fTestNet ? nHeight >= 0x20 : nHeight >= ConfigurationState::BLOCK_REWARD_HALVING;
}

inline int64_t PastDrift(int64_t nTime, int nHeight)
{
  return IsProtocolV2(nHeight) ? nTime : nTime - 10 * 60;
}
inline int64_t FutureDrift(int64_t nTime, int nHeight)
{
  return IsProtocolV2(nHeight) ? nTime + 15 : nTime + 10 * 60;
}

inline unsigned int GetTargetSpacing(int nHeight)
{
  return IsProtocolV2(nHeight) ? 64 : 60;
}

extern unsigned int nCoinCacheSize;
extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern std::map<uint256, CBlockIndex*> mapBlockIndex;
extern std::set<std::pair<COutPoint, unsigned int> > setStakeSeen;
extern CBlockIndex* pindexGenesisBlock;
extern unsigned int nStakeMinAge;
extern unsigned int nStakeMaxAge;
extern unsigned int nNodeLifespan;
extern int nCoinbaseMaturity;
extern int nBestHeight;
extern uint256 nBestChainTrust;
extern uint256 nBestInvalidTrust;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockSize;
extern int64_t nLastCoinStakeSearchInterval;
extern const std::string strMessageMagic;
extern int64_t nTimeBestReceived;
extern CCriticalSection cs_setpwalletRegistered;
extern std::set<__wx__*> setpwalletRegistered;
extern unsigned char pchMessageStart[4];
extern std::map<uint256, CBlock*> mapOrphanBlocks;


extern int64_t nReserveBalance;
extern int64_t nMinimumInputValue;
extern bool fUseFastIndex;
extern unsigned int nDerivationMethodIndex;

extern bool fEnforceCanonical;

static const uint64_t nMinDiskSpace = 52428800;

class CReserveKey;
class CTxIndex;

void RegisterWallet(__wx__* pwalletIn);
void UnregisterWallet(__wx__* pwalletIn);
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock = NULL, bool fUpdate = false, bool fConnect = true);
bool ProcessBlock(CNode* pfrom, CBlock* pblock);
bool CheckDiskSpace(uint64_t nAdditionalBytes=0);
FILE* AppendBlockFile(unsigned int& nFileRet);
bool LoadBlockIndex(bool fAllowNew=true);
void PrintBlockTree();
CBlockIndex* FindBlockByHeight(int nHeight);
bool ProcessMessages(CNode* pfrom);
bool SendMessages(CNode* pto, bool fSendTrickle);
bool LoadExternalBlockFile(FILE* fileIn);

int GetPowHeight(const CBlockIndex* pindex);
bool CheckProofOfWork(uint256 hash, unsigned int nBits);
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake, int64_t nFees);
int64_t GetProofOfWorkReward(int64_t nPowHeight, int64_t nFees);
int64_t GetProofOfStakeInterest(int nHeight);
int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees, int nHeight);
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime);
unsigned int ComputeMinStake(unsigned int nBase, int64_t nTime, unsigned int nBlockTime);
bool minBase(const CTxIndex& txindex, const CBlockIndex* pindexFrom, int nMaxDepth, int& nActualDepth);
int GetNumBlocksOfPeers();
bool IsInitialBlockDownload();
std::string GetWarnings(std::string strFor);
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock, bool s=false);
uint256 WantedByOrphan(const CBlock* pblockOrphan);
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
void StakeMiner(__wx__ *pwallet);
void ResendWalletTransactions(bool fForce = false);



bool AcceptToMemoryPool(CTxMemPool& pool, CTransaction &tx,
                        bool* pfMissingInputs);
bool GetWalletFile(__wx__* pwallet, std::string &strWalletFileOut);

bool IsStandardTx(const CTransaction& tx);

bool IsFinalTx(const CTransaction &tx, int nBlockHeight = 0, int64_t nBlockTime = 0);


extern CTxMemPool mempool;


#endif
