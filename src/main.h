#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"
#include "hashblock.h"
#include "block.h"
#include "ptrie/FixedHash.h"
#include "ptrie/TrieDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"

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

#include "constants.h"


extern bool fReindex;

const int LAST_POW_BLOCK = 12815;


#define LR_R 0x32


const unsigned int MAX_BLOCK_SIZE = 4.0 * 1024 * 1024;



const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE;


static const unsigned int MAX_STANDARD_TX_SIZE = 0.6 * MAX_BLOCK_SIZE_GEN;

const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;

static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;

static const unsigned int MAX_INV_SZ = 50000;

static const int64_t MIN_TX_FEE = 10000;

static const int64_t MIN_RELAY_TX_FEE = MIN_TX_FEE;
static const int64_t S_MIN_TX_FEE = 100000;

static const int64_t MAX_MONEY = std::numeric_limits<int64_t>::max();
inline bool MoneyRange(int64_t nValue)
{
  return (nValue >= 0 && nValue <= MAX_MONEY);
}

static const unsigned int LOCKTIME_THRESHOLD = 500000000;
static const unsigned int MAX_TX_INFO_LEN = 140;
static const int64_t MIN_COIN_YEAR_REWARD = 1 * CENT;
static const int64_t MAX_COIN_YEAR_REWARD = 3 * CENT;



const int SHADE_FEATURE_UPDATE = 75 * 500 + 1860837;
const int BASELINE_LOCK = 0x00ff0;
const int BLOCK_REWARD_HALVING = 0x2dc6c0;

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
  return fTestNet ? nHeight >= 0x10 : nHeight >= SHADE_FEATURE_UPDATE;
}
inline bool V5(int nHeight)
{
  return fTestNet ? nHeight >= 0x20 : nHeight >= BLOCK_REWARD_HALVING;
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
extern unsigned int nTransactionsUpdated;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockSize;
extern int64_t nLastCoinStakeSearchInterval;
extern const std::string strMessageMagic;
extern int64_t nTimeBestReceived;
extern CCriticalSection cs_setpwalletRegistered;
extern std::set<__wx__*> setpwalletRegistered;
extern unsigned char pchMessageStart[4];
extern std::map<uint256, CBlock*> mapOrphanBlocks;


extern int64_t nTransactionFee;
extern int64_t nReserveBalance;
extern int64_t nMinimumInputValue;
extern bool fUseFastIndex;
extern unsigned int nDerivationMethodIndex;

extern bool fEnforceCanonical;
extern int nStakeMinConfirmations;

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
class CInPoint
{
public:
  CTransaction* ptx;
  unsigned int n;

  CInPoint()
  {
    SetNull();
  }
  CInPoint(CTransaction* ptxIn, unsigned int nIn)
  {
    ptx = ptxIn;
    n = nIn;
  }
  void SetNull()
  {
    ptx = NULL;
    n = (unsigned int) -1;
  }
  bool IsNull() const
  {
    return (ptx == NULL && n == (unsigned int) -1);
  }
};

bool IsStandardTx(const CTransaction& tx);

bool IsFinalTx(const CTransaction &tx, int nBlockHeight = 0, int64_t nBlockTime = 0);

class CTxMemPool
{
public:
  mutable CCriticalSection cs;
  std::map<uint256, CTransaction> mapTx;
  std::map<COutPoint, CInPoint> mapNextTx;

  bool addUnchecked(const uint256& hash, CTransaction &tx);
  bool remove(const CTransaction &tx, bool fRecursive = false);
  bool removeConflicts(const CTransaction &tx);
  void clear();
  void queryHashes(std::vector<uint256>& vtxid);

  unsigned long size() const
  {
    LOCK(cs);
    return mapTx.size();
  }

  bool exists(uint256 hash) const
  {
    LOCK(cs);
    return (mapTx.count(hash) != 0);
  }

  bool lookup(uint256 hash, CTransaction& result) const
  {
    LOCK(cs);
    std::map<uint256, CTransaction>::const_iterator i = mapTx.find(hash);

    if (i == mapTx.end())
    {
      return false;
    }

    result = i->second;
    return true;
  }
};

extern CTxMemPool mempool;

#endif
