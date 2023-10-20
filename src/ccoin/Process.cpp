#include "Alert.h"
#include "Process.h"
#include "CheckPoints.h"
#include "db.h"
#include "TxDB.h"
#include "Net.h"
#include "wallet/Wallet.h"
#include "UIInterface.h"
#include "Kernel.h"
#include "rpc/Client.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "dions/dions.h"
#include "ConfigurationState.h"
using namespace json_spirit;
using namespace std;
using namespace boost;
extern ConfigurationState globalState;
int STAKE_INTEREST_V3 = 150000;
CCriticalSection cs_setpwalletRegistered;
set<__wx__*> setpwalletRegistered;
CCriticalSection cs_main;
CTxMemPool mempool;
bool fReindex = false;
map<uint256, CBlockIndex*> mapBlockIndex;
set<pair<COutPoint, unsigned int> > setStakeSeen;
CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);
CBigNum bnProofOfStakeLimit(~uint256(0) >> 20);
CBigNum bnProofOfStakeLimitV2(~uint256(0) >> 48);
CBigNum bnProofOfWorkLimitTestNet(~uint256(0) >> 1);
unsigned int nStakeMinAge = 8 * 60 * 60;
unsigned int nStakeMaxAge = -1;
unsigned int nModifierInterval = 10 * 60;
int POS_v3_DIFFICULTY_HEIGHT = 100000;
bool FEATURE_SET_SHADE_ACTIVE = false;
int nCoinbaseMaturity = 100;
CBlockIndex* pindexGenesisBlock = NULL;
int nBestHeight = -1;
unsigned long EAX0_SHIFT_REGISTER__ = 0x989680;
extern __wx__* pwalletMainId;
void StartShutdown();
unsigned int LR_SHIFT__[] =
{
  0x1f, 0x29, 0x3b, 0x1a, 0x35, 0x3a, 0x61, 0x5d, 0x17, 0x54,
  0x23, 0xd4, 0xc3, 0x16, 0x02, 0xce, 0x58, 0x4a, 0x3c, 0x76,
  0x17, 0xc4, 0x51, 0x86, 0xbc, 0xaf, 0x39, 0x21, 0x58, 0x74,
  0x52, 0xaf, 0x58, 0xcd, 0x42, 0xff, 0xad, 0x32, 0x89, 0x21,
  0x51, 0xd3, 0xfe, 0xae, 0x32, 0x41, 0x29, 0x57, 0xcb, 0xa4,
  0x38, 0x26, 0xd2, 0x89, 0x25, 0xae, 0xdf, 0xc6, 0xd3, 0xc1,
  0x0a, 0x58, 0xcc, 0x41, 0xff, 0x88, 0x80, 0xed, 0xfa, 0xc4,
  0x85, 0xc7, 0xbc, 0x47, 0x19, 0x29, 0x62, 0x71, 0xff, 0x5c
};
const char scan_tab[201] =
{
  ')', '}', '&', '@', '^',
  '{', ';', '\'', '?', '@',
  '(', '\'', '@', ',', '(',
  '^', '{', '?', '}', '#',
  '$', '\'', '{', '+', '^',
  '\'', ';', '#', ':', '^',
  ',', '(', '\'', '@', '?',
  '}', '#', '\'', ':', '^',
  ';', '[', '^', '\'', '=',
  ')', '{', '*', '\'', '!',
  '}', '#', '@', '\'', '{',
  '%', '^', '.', '[', ':',
  '^', ';', ';', ')', '@',
  ')', '{', '{', '#', '+',
  '!', '^', ':', ';', '?',
  '}', '#', '=', '{', '}',
  ',', ';', '}', '+', '^',
  '@', '(', ')', '{', '*',
  '\'', '!', '}', '#', '@',
  ')', '@', '!', '#', '@',
  ',', '(', '^', '{', '?',
  '}', '#', '$', '\'', '{',
  '{', '}', '@', '+', '^',
  '\'', ';', '#', ':', '^',
  ')', '@', ',', '(', '^',
  '{', '?', '}', '#', '$',
  '\'', '{', '{', '}', '@',
  '^', '.', '[', ':', '^',
  ';', ';', ')', '@', ')',
  '{', '{', '#', '+', '!',
  '^', ':', ';', '?', '}',
  '#', ':', '=', '{', '}',
  ',', '_', '^', '%', '*',
  '^', ')', ';', '}', '&',
  '\'', '+', '^', '\'', '*',
  ':', '^', '\'', '{', '%',
  '#', '{', ';', '\'', '@',
  ')', ';', '&', '\'', '$',
  '@', '}', ':', '?', '=',
  ')', '{', '%', '.', '.',
  '\0'
};
int INTERN_REF0__ = 0x3520;
int EXTERN_REF0__ = 0x0040;
uint256 nBestChainTrust = 0;
uint256 nBestInvalidTrust = 0;
int DUAL__ = INTERN_REF0__;
uint256 hashBestChain = 0;
CBlockIndex* pindexBest = NULL;
int64_t nTimeBestReceived = 0;
CMedianFilter<int> cPeerBlockCounts(5, 0);
map<uint256, CBlock*> mapOrphanBlocks;
multimap<uint256, CBlock*> mapOrphanBlocksByPrev;
set<pair<COutPoint, unsigned int> > setStakeSeenOrphan;
map<uint256, CTransaction> mapOrphanTransactions;
map<uint256, set<uint256> > mapOrphanTransactionsByPrev;
CScript COINBASE_FLAGS;
const string strMessageMagic = "I/OCoin Signed Message:\n";
extern LocatorNodeDB* ln1Db;
int64_t nReserveBalance = 0;
int64_t nMinimumInputValue = 0;
unsigned int nCoinCacheSize = 5000;
static const int NUM_OF_POW_CHECKPOINT = 1;
static const int checkpointPoWHeight[NUM_OF_POW_CHECKPOINT][2] =
{
  { 12000, 6390}
};
extern enum Checkpoints::CPMode CheckpointsMode;
void RegisterWallet(__wx__* pwalletIn)
{
  {
    LOCK(cs_setpwalletRegistered);
    setpwalletRegistered.insert(pwalletIn);
  }
}
void UnregisterWallet(__wx__* pwalletIn)
{
  {
    LOCK(cs_setpwalletRegistered);
    setpwalletRegistered.erase(pwalletIn);
  }
}
bool static GetTransaction(const uint256& hashTx, __wx__Tx& wtx)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)

  if (pwallet->GetTransaction(hashTx,wtx))
  {
    return true;
  }

  return false;
}
bool minBase(const CTxIndex& txindex, const CBlockIndex* pindexFrom, int nMaxDepth, int& nActualDepth)
{
  for (const CBlockIndex* pindex = pindexFrom; pindex && pindexFrom->nHeight - pindex->nHeight < nMaxDepth; pindex = pindex->pprev)
  {
    if (pindex->nBlockPos == txindex.pos.nBlockPos && pindex->nFile == txindex.pos.nFile)
    {
      nActualDepth = pindexFrom->nHeight - pindex->nHeight;
      return false;
    }
  }

  return true;
}
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock, bool fUpdate, bool fConnect)
{
  if (!fConnect)
  {
    if (tx.IsCoinStake())
    {
      BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)

      if (pwallet->IsFromMe(tx))
      {
        pwallet->DisableTransaction(tx);
      }
    }

    return;
  }

  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)
  {
    pwallet->AddToWalletIfInvolvingMe(tx, pblock, fUpdate);
  }
}
void SetBestChain(const CBlockLocator& loc)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)
  pwallet->SetBestChain(loc);
}
void static PrintWallets(const CBlock& block)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)
  pwallet->PrintWallet(block);
}
void static Inventory(const uint256& hash)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)
  pwallet->Inventory(hash);
}
void ResendWalletTransactions(bool fForce)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)
  pwallet->ResendWalletTransactions(fForce);
}
bool AddOrphanTx(const CTransaction& tx)
{
  uint256 hash = tx.GetHash();

  if (mapOrphanTransactions.count(hash))
  {
    return false;
  }

  size_t nSize = tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);

  if (nSize > 5000)
  {
    printf("ignoring large orphan tx (size: %" PRIszu ", hash: %s)\n", nSize, hash.ToString().substr(0,10).c_str());
    return false;
  }

  mapOrphanTransactions[hash] = tx;
  BOOST_FOREACH(const CTxIn& txin, tx.vin)
  mapOrphanTransactionsByPrev[txin.prevout.hash].insert(hash);
  printf("stored orphan tx %s (mapsz %" PRIszu ")\n", hash.ToString().substr(0,10).c_str(),
         mapOrphanTransactions.size());
  return true;
}
void static EraseOrphanTx(uint256 hash)
{
  if (!mapOrphanTransactions.count(hash))
  {
    return;
  }

  const CTransaction& tx = mapOrphanTransactions[hash];
  BOOST_FOREACH(const CTxIn& txin, tx.vin)
  {
    mapOrphanTransactionsByPrev[txin.prevout.hash].erase(hash);

    if (mapOrphanTransactionsByPrev[txin.prevout.hash].empty())
    {
      mapOrphanTransactionsByPrev.erase(txin.prevout.hash);
    }
  }
  mapOrphanTransactions.erase(hash);
}
unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans)
{
  unsigned int nEvicted = 0;

  while (mapOrphanTransactions.size() > nMaxOrphans)
  {
    uint256 randomhash = GetRandHash();
    map<uint256, CTransaction>::iterator it = mapOrphanTransactions.lower_bound(randomhash);

    if (it == mapOrphanTransactions.end())
    {
      it = mapOrphanTransactions.begin();
    }

    EraseOrphanTx(it->first);
    ++nEvicted;
  }

  return nEvicted;
}
bool IsStandardTx(const CTransaction& tx)
{
  if (tx.nVersion > CTransaction::CURRENT_VERSION)
  {
    return false;
  }


  if (!IsFinalTx(tx, nBestHeight + 1))
  {
    return false;
  }

  if (tx.nTime > FutureDrift(GetAdjustedTime(), nBestHeight + 1))
  {
    return false;
  }

  unsigned int sz = tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);

  if (sz >= ConfigurationState::MAX_STANDARD_TX_SIZE)
  {
    return false;
  }

  BOOST_FOREACH(const CTxIn& txin, tx.vin)
  {
    if (txin.scriptSig.size() > 500)
    {
      return false;
    }

    if (!txin.scriptSig.IsPushOnly())
    {
      return false;
    }

    if (fEnforceCanonical && !txin.scriptSig.HasCanonicalPushes())
    {
      return false;
    }
  }
  unsigned int nDataOut = 0;
  txnouttype whichType;
  BOOST_FOREACH(const CTxOut& txout, tx.vout)
  {
    if (!::IsStandard(txout.scriptPubKey, whichType))
    {
      return false;
    }

    if (whichType == TX_NULL_DATA)
    {
      nDataOut++;
    }
    else if(txout.nValue == 0)
    {
      return false;
    }

    if (fEnforceCanonical && !txout.scriptPubKey.HasCanonicalPushes())
    {
      return false;
    }
  }

  if (nDataOut > 1)
  {
    return false;
  }

  return true;
}
bool IsFinalTx(const CTransaction &tx, int nBlockHeight, int64_t nBlockTime)
{
  AssertLockHeld(cs_main);

  if (tx.nLockTime == 0)
  {
    return true;
  }

  if (nBlockHeight == 0)
  {
    nBlockHeight = nBestHeight;
  }

  if (nBlockTime == 0)
  {
    nBlockTime = GetAdjustedTime();
  }

  if ((int64_t)tx.nLockTime < ((int64_t)tx.nLockTime < ConfigurationState::LOCKTIME_THRESHOLD ? (int64_t)nBlockHeight : nBlockTime))
  {
    return true;
  }

  BOOST_FOREACH(const CTxIn& txin, tx.vin)

  if (!txin.IsFinal())
  {
    return false;
  }

  return true;
}
bool __wx__Tx::AcceptWalletTransaction(CTxDB& txdb)
{
  {
    BOOST_FOREACH(CMerkleTx& tx, vtxPrev)
    {
      if (!(tx.IsCoinBase() || tx.IsCoinStake()))
      {
        uint256 hash = tx.GetHash();

        if (!mempool.exists(hash) && !txdb.ContainsTx(hash))
        {
          mempool.accept(tx, NULL);
        }
      }
    }
    return mempool.accept(*this, NULL);
  }
  return false;
}
bool __wx__Tx::AcceptWalletTransaction()
{
  CTxDB txdb("r");
  return AcceptWalletTransaction(txdb);
}
int CTxIndex::GetDepthInMainChain() const
{
  CBlock block;

  if (!block.ReadFromDisk(pos.nFile, pos.nBlockPos, false))
  {
    return 0;
  }

  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(block.GetHash());

  if (mi == mapBlockIndex.end())
  {
    return 0;
  }

  CBlockIndex* pindex = (*mi).second;

  if (!pindex || !pindex->IsInMainChain())
  {
    return 0;
  }

  return 1 + nBestHeight - pindex->nHeight;
}
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock,
                    bool s)
{
  {
    if(s)
    {
      LOCK(cs_main);
      {
        if (mempool.lookup(hash, tx))
        {
          return true;
        }
      }
    }

    CTxDB txdb("r");
    CTxIndex txindex;

    if (tx.ReadFromDisk(txdb, COutPoint(hash, 0), txindex))
    {
      CBlock block;

      if (block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
      {
        hashBlock = block.GetHash();
      }

      return true;
    }
  }
  return false;
}
CBlockIndex* pblockindexFBBHLast;
CBlockIndex* FindBlockByHeight(int nHeight)
{
  CBlockIndex *pblockindex;

  if (nHeight < nBestHeight / 2)
  {
    pblockindex = pindexGenesisBlock;
  }
  else
  {
    pblockindex = pindexBest;
  }

  if (pblockindexFBBHLast && abs(nHeight - pblockindex->nHeight) > abs(nHeight - pblockindexFBBHLast->nHeight))
  {
    pblockindex = pblockindexFBBHLast;
  }

  while (pblockindex->nHeight > nHeight)
  {
    pblockindex = pblockindex->pprev;
  }

  while (pblockindex->nHeight < nHeight)
  {
    pblockindex = pblockindex->pnext;
  }

  pblockindexFBBHLast = pblockindex;
  return pblockindex;
}
uint256 static GetOrphanRoot(const CBlock* pblock)
{
  while (mapOrphanBlocks.count(pblock->hashPrevBlock))
  {
    pblock = mapOrphanBlocks[pblock->hashPrevBlock];
  }

  return pblock->GetHash();
}
uint256 WantedByOrphan(const CBlock* pblockOrphan)
{
  while (mapOrphanBlocks.count(pblockOrphan->hashPrevBlock))
  {
    pblockOrphan = mapOrphanBlocks[pblockOrphan->hashPrevBlock];
  }

  return pblockOrphan->hashPrevBlock;
}
int GetPowHeight(const CBlockIndex* pindex)
{
  int count = 0;
  int index = -1;
  int height = pindex->nHeight;
  int maxCheck = height;

  if(NUM_OF_POW_CHECKPOINT != 0)
  {
    for(int i = 1; i <= NUM_OF_POW_CHECKPOINT; i++)
    {
      if(height > checkpointPoWHeight[NUM_OF_POW_CHECKPOINT - i][0])
      {
        index = NUM_OF_POW_CHECKPOINT - i;
        break;
      }
    }
  }

  if(index != -1)
  {
    maxCheck = height - checkpointPoWHeight[index][0];
  }

  for (int j = 0; j < maxCheck; j++)
  {
    if(!pindex->IsProofOfStake())
    {
      ++count;
    }

    pindex = pindex->pprev;
  }

  if(index != -1)
  {
    count += checkpointPoWHeight[index][1];
  }

  return count;
}
int GetPosHeight(const CBlockIndex* pindex)
{
  int posH = pindex->nHeight - GetPowHeight(pindex);
  return posH;
}
static CBigNum GetProofOfStakeLimit(int nHeight)
{
  if (IsProtocolV2(nHeight))
  {
    return bnProofOfStakeLimitV2;
  }
  else
  {
    return bnProofOfStakeLimit;
  }
}
int64_t GetProofOfWorkReward(int64_t nPowHeight, int64_t nFees)
{
  int64_t nBaseSubsidy = 1250 * COIN;
  int64_t nSubsidy = 0 * COIN;

  if (nPowHeight <= 30)
  {
    nSubsidy = nBaseSubsidy / 2;
  }
  else
  {
    nSubsidy = nBaseSubsidy;
  }

  if (fDebug && GetBoolArg("-printcreation"))
  {
    printf("GetProofOfWorkReward() : create=%s nSubsidy=%" PRId64 "\n", FormatMoney(nSubsidy).c_str(), nSubsidy);
  }

  return nSubsidy + nFees;
}
double GetMinDifficulty(bool proofOfStake, int nHeight)
{
  if (proofOfStake)
  {
    return nBitsToDifficulty(GetProofOfStakeLimit(nHeight).GetCompact());
  }
  else
  {
    return nBitsToDifficulty(bnProofOfWorkLimit.GetCompact());
  }
}
int64_t GetProofOfStakeInterest(int nHeight)
{
  if (IsProtocolV2(nHeight))
  {
    return ConfigurationState::MAX_COIN_YEAR_REWARD / 2;
  }
  else
  {
    double maxDiff = 4.2;
    double minDiff = GetMinDifficulty(true, nHeight);
    double diffPoS = GetDifficulty(GetLastBlockIndex(pindexBest, true));

    if (diffPoS > maxDiff)
    {
      return ConfigurationState::MAX_COIN_YEAR_REWARD;
    }
    else if (diffPoS < minDiff)
    {
      return ConfigurationState::MIN_COIN_YEAR_REWARD;
    }
    else
    {
      return ConfigurationState::MIN_COIN_YEAR_REWARD +
             ConfigurationState::MAX_COIN_YEAR_REWARD *
             ((diffPoS - minDiff) / (maxDiff - minDiff));
    }
  }
}
int64_t GetProofOfStakeInterestV2(int nHeight)
{
  double weight = GetPoSKernelPS(nHeight);
  uint64_t rate = ConfigurationState::MIN_COIN_YEAR_REWARD;

  if (weight > 16384)
  {
    rate = std::max(ConfigurationState::MIN_COIN_YEAR_REWARD,
                    std::min(static_cast<int64_t>(ConfigurationState::MIN_COIN_YEAR_REWARD * std::log(weight / 16384.0)),
                             ConfigurationState::MAX_COIN_YEAR_REWARD));
  }

  return rate;
}
int64_t GetProofOfStakeInterestV3(int nHeight)
{
  uint64_t rate = ConfigurationState::MIN_COIN_YEAR_REWARD;
  return rate;
}
int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees, int nHeight)
{
  int64_t nSubsidy;

  if(V3(nHeight))
  {
    nSubsidy = COIN * 1.5;
  }
  else
  {
    int64_t interest = (nHeight>STAKE_INTEREST_V3)? GetProofOfStakeInterestV3(nHeight) : GetProofOfStakeInterestV2(nHeight);
    nSubsidy = nCoinAge * interest * 33 / (365 * 33 + 8);
  }

  if (fDebug && GetBoolArg("-printcreation"))
  {
    printf("GetProofOfStakeReward(): create=%s nCoinAge=%" PRId64 "\n", FormatMoney(nSubsidy).c_str(), nCoinAge);
  }

  return nSubsidy + nFees;
}
static const int64_t nTargetTimespan = 16 * 60;
unsigned int ComputeMaxBits(CBigNum bnTargetLimit, unsigned int nBase, int64_t nTime)
{
  CBigNum bnResult;
  bnResult.SetCompact(nBase);
  bnResult *= 2;

  while (nTime > 0 && bnResult < bnTargetLimit)
  {
    bnResult *= 2;
    nTime -= 24 * 60 * 60;
  }

  if (bnResult > bnTargetLimit)
  {
    bnResult = bnTargetLimit;
  }

  return bnResult.GetCompact();
}
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime)
{
  return ComputeMaxBits(bnProofOfWorkLimit, nBase, nTime);
}
unsigned int ComputeMinStake(unsigned int nBase, int64_t nTime, unsigned int nBlockTime)
{
  return ComputeMaxBits(bnProofOfStakeLimit, nBase, nTime);
}
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
  while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
  {
    pindex = pindex->pprev;
  }

  return pindex;
}
static unsigned int GetNextTargetRequiredV1(const CBlockIndex* pindexLast, bool fProofOfStake)
{
  CBigNum bnTargetLimit = fProofOfStake ? bnProofOfStakeLimit : bnProofOfWorkLimit;

  if (pindexLast == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);

  if (pindexPrev->pprev == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

  if (pindexPrevPrev->pprev == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  int64_t nTargetSpacing = GetTargetSpacing(pindexLast->nHeight);
  int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
  CBigNum bnNew;
  bnNew.SetCompact(pindexPrev->nBits);
  int64_t nInterval = nTargetTimespan / nTargetSpacing;
  bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
  bnNew /= ((nInterval + 1) * nTargetSpacing);

  if (bnNew > bnTargetLimit)
  {
    bnNew = bnTargetLimit;
  }

  return bnNew.GetCompact();
}
static unsigned int GetNextTargetRequiredV2(const CBlockIndex* pindexLast, bool fProofOfStake)
{
  CBigNum bnTargetLimit = fProofOfStake ? GetProofOfStakeLimit(pindexLast->nHeight) : bnProofOfWorkLimit;

  if (pindexLast == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);

  if (pindexPrev->pprev == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

  if (pindexPrevPrev->pprev == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  int64_t nTargetSpacing = GetTargetSpacing(pindexLast->nHeight);
  int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

  if (nActualSpacing < 0)
  {
    nActualSpacing = nTargetSpacing;
  }

  CBigNum bnNew;
  bnNew.SetCompact(pindexPrev->nBits);
  int64_t nInterval = nTargetTimespan / nTargetSpacing;
  bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
  bnNew /= ((nInterval + 1) * nTargetSpacing);

  if (bnNew <= 0 || bnNew > bnTargetLimit)
  {
    bnNew = bnTargetLimit;
  }

  return bnNew.GetCompact();
}
static unsigned int GetNextTargetRequiredV3(const CBlockIndex* pindexLast, bool fProofOfStake, int64_t nFees)
{
  CBigNum bnTargetLimit = fProofOfStake ? GetProofOfStakeLimit(pindexLast->nHeight) : bnProofOfWorkLimit;

  if (pindexLast == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);

  if (pindexPrev->pprev == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

  if (pindexPrevPrev->pprev == NULL)
  {
    return bnTargetLimit.GetCompact();
  }

  int64_t nTargetSpacing = GetTargetSpacing(pindexLast->nHeight);
  int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

  if (nActualSpacing < 0)
  {
    nActualSpacing = nTargetSpacing;
  }

  if(V3(pindexLast->nHeight))
  {
    if(nActualSpacing > nTargetSpacing * 10)
    {
      nActualSpacing = nTargetSpacing;
    }
  }

  CBigNum bnNew, bnMit;
  bnNew.SetCompact(pindexPrev->nBits);
  int64_t nFeesMitigation = nFees / ( CTransaction::MIN_TX_FEE * 10) + 1;
  int64_t nInterval = nTargetTimespan / nTargetSpacing;
  bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
  bnNew /= ((nInterval + 1) * nTargetSpacing);

  if (bnNew <= 0 || bnNew > bnTargetLimit)
  {
    bnNew = bnTargetLimit;
  }
  else if (bnNew < bnTargetLimit/4)
  {
    bnMit = bnNew*nFeesMitigation;

    if (bnMit > bnTargetLimit/4)
    {
      bnNew=bnTargetLimit/4;
    }
    else
    {
      bnNew=bnMit;
    }
  }

  return bnNew.GetCompact();
}
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake, int64_t nFees)
{
  if(pindexLast->nHeight < 24376)
  {
    return GetNextTargetRequiredV1(pindexLast, fProofOfStake);
  }
  else
  {
    if (pindexLast->nHeight < POS_v3_DIFFICULTY_HEIGHT)
    {
      return GetNextTargetRequiredV2(pindexLast, fProofOfStake);
    }
    else
    {
      return GetNextTargetRequiredV3(pindexLast, fProofOfStake, nFees);
    }
  }
}
bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
  CBigNum bnTarget;
  bnTarget.SetCompact(nBits);

  if (bnTarget <= 0 || bnTarget > bnProofOfWorkLimit)
  {
    return error("CheckProofOfWork() : nBits below minimum work");
  }

  if (hash > bnTarget.getuint256())
  {
    return error("CheckProofOfWork() : hash doesn't match nBits");
  }

  return true;
}
int GetNumBlocksOfPeers()
{
  return std::max(cPeerBlockCounts.median(), Checkpoints::GetTotalBlocksEstimate());
}
bool IsInitialBlockDownload()
{
  LOCK(cs_main);

  if (pindexBest == NULL || nBestHeight < Checkpoints::GetTotalBlocksEstimate())
  {
    return true;
  }

  if(fTestNet)
  {
    return false;
  }

  static int64_t nLastUpdate;
  static CBlockIndex* pindexLastBest;

  if (pindexBest != pindexLastBest)
  {
    pindexLastBest = pindexBest;
    nLastUpdate = GetTime();
  }

  return GetTime() - nLastUpdate < 15 &&
         pindexBest->GetBlockTime() < GetTime() - 8 * 60 * 60;
}
void InvalidChainFound(CBlockIndex* pindexNew)
{
  if (pindexNew->nChainTrust > nBestInvalidTrust)
  {
    nBestInvalidTrust = pindexNew->nChainTrust;
    CTxDB().WriteBestInvalidTrust(CBigNum(nBestInvalidTrust));
    uiInterface.NotifyBlocksChanged();
  }

  uint256 nBestInvalidBlockTrust = pindexNew->nChainTrust - pindexNew->pprev->nChainTrust;
  uint256 nBestBlockTrust = pindexBest->nHeight != 0 ? (pindexBest->nChainTrust - pindexBest->pprev->nChainTrust) : pindexBest->nChainTrust;
  printf("InvalidChainFound: invalid block=%s  height=%d  trust=%s  blocktrust=%" PRId64 "  date=%s\n",
         pindexNew->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->nHeight,
         CBigNum(pindexNew->nChainTrust).ToString().c_str(), nBestInvalidBlockTrust.Get64(),
         DateTimeStrFormat("%x %H:%M:%S", pindexNew->GetBlockTime()).c_str());
  printf("InvalidChainFound:  current best=%s  height=%d  trust=%s  blocktrust=%" PRId64 "  date=%s\n",
         hashBestChain.ToString().substr(0,20).c_str(), nBestHeight,
         CBigNum(pindexBest->nChainTrust).ToString().c_str(),
         nBestBlockTrust.Get64(),
         DateTimeStrFormat("%x %H:%M:%S", pindexBest->GetBlockTime()).c_str());
}
bool CTransaction::DisconnectInputs(CTxDB& txdb)
{
  if (!IsCoinBase())
  {
    BOOST_FOREACH(const CTxIn& txin, vin)
    {
      COutPoint prevout = txin.prevout;
      CTxIndex txindex;

      if (!txdb.ReadTxIndex(prevout.hash, txindex))
      {
        return error("DisconnectInputs() : ReadTxIndex failed");
      }

      if (prevout.n >= txindex.vSpent.size())
      {
        return error("DisconnectInputs() : prevout.n out of range");
      }

      txindex.vSpent[prevout.n].SetNull();

      if (!txdb.UpdateTxIndex(prevout.hash, txindex))
      {
        return error("DisconnectInputs() : UpdateTxIndex failed");
      }
    }
  }

  txdb.EraseTxIndex(*this);
  return true;
}
bool CTransaction::FetchInputs(CTxDB& txdb, const map<uint256, CTxIndex>& mapTestPool,
                               bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid)
{
  fInvalid = false;

  if (IsCoinBase())
  {
    return true;
  }

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    COutPoint prevout = vin[i].prevout;

    if (inputsRet.count(prevout.hash))
    {
      continue;
    }

    CTxIndex& txindex = inputsRet[prevout.hash].first;
    bool fFound = true;

    if ((fBlock || fMiner) && mapTestPool.count(prevout.hash))
    {
      txindex = mapTestPool.find(prevout.hash)->second;
    }
    else
    {
      fFound = txdb.ReadTxIndex(prevout.hash, txindex);
    }

    if (!fFound && (fBlock || fMiner))
    {
      return fMiner ? false : error("FetchInputs() : %s prev tx %s index entry not found", GetHash().ToString().substr(0,10).c_str(), prevout.hash.ToString().substr(0,10).c_str());
    }

    CTransaction& txPrev = inputsRet[prevout.hash].second;

    if (!fFound || txindex.pos == CDiskTxPos(1,1,1))
    {
      if (!mempool.lookup(prevout.hash, txPrev))
      {
        return error("FetchInputs() : %s mempool Tx prev not found %s", GetHash().ToString().substr(0,10).c_str(), prevout.hash.ToString().substr(0,10).c_str());
      }

      if (!fFound)
      {
        txindex.vSpent.resize(txPrev.vout.size());
      }
    }
    else
    {
      if (!txPrev.ReadFromDisk(txindex.pos))
      {
        return error("FetchInputs() : %s ReadFromDisk prev tx %s failed", GetHash().ToString().substr(0,10).c_str(), prevout.hash.ToString().substr(0,10).c_str());
      }
    }
  }

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    const COutPoint prevout = vin[i].prevout;
    assert(inputsRet.count(prevout.hash) != 0);
    const CTxIndex& txindex = inputsRet[prevout.hash].first;
    const CTransaction& txPrev = inputsRet[prevout.hash].second;

    if (prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
    {
      fInvalid = true;
      return DoS(100, error("FetchInputs() : %s prevout.n out of range %d %" PRIszu " %" PRIszu " prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str()));
    }
  }

  return true;
}
const CTxOut& CTransaction::GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const
{
  MapPrevTx::const_iterator mi = inputs.find(input.prevout.hash);

  if (mi == inputs.end())
  {
    throw std::runtime_error("CTransaction::GetOutputFor() : prevout.hash not found");
  }

  const CTransaction& txPrev = (mi->second).second;

  if (input.prevout.n >= txPrev.vout.size())
  {
    throw std::runtime_error("CTransaction::GetOutputFor() : prevout.n out of range");
  }

  return txPrev.vout[input.prevout.n];
}
int64_t CTransaction::GetValueIn(const MapPrevTx& inputs) const
{
  if (IsCoinBase())
  {
    return 0;
  }

  int64_t nResult = 0;

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    nResult += GetOutputFor(vin[i], inputs).nValue;
  }

  return nResult;
}
unsigned int CTransaction::GetP2SHSigOpCount(const MapPrevTx& inputs) const
{
  if (IsCoinBase())
  {
    return 0;
  }

  unsigned int nSigOps = 0;

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    const CTxOut& prevout = GetOutputFor(vin[i], inputs);

    if (prevout.scriptPubKey.IsPayToScriptHash())
    {
      nSigOps += prevout.scriptPubKey.GetSigOpCount(vin[i].scriptSig);
    }
  }

  return nSigOps;
}
bool Reorganize(CTxDB& txdb, CBlockIndex* pindexNew)
{
  printf("REORGANIZE\n");
  CBlockIndex* pfork = pindexBest;
  CBlockIndex* plonger = pindexNew;

  while (pfork != plonger)
  {
    while (plonger->nHeight > pfork->nHeight)
      if (!(plonger = plonger->pprev))
      {
        return error("Reorganize() : plonger->pprev is null");
      }

    if (pfork == plonger)
    {
      break;
    }

    if (!(pfork = pfork->pprev))
    {
      return error("Reorganize() : pfork->pprev is null");
    }
  }

  vector<CBlockIndex*> vDisconnect;

  for (CBlockIndex* pindex = pindexBest; pindex != pfork; pindex = pindex->pprev)
  {
    vDisconnect.push_back(pindex);
  }

  vector<CBlockIndex*> vConnect;

  for (CBlockIndex* pindex = pindexNew; pindex != pfork; pindex = pindex->pprev)
  {
    vConnect.push_back(pindex);
  }

  reverse(vConnect.begin(), vConnect.end());
  printf("REORGANIZE: Disconnect %" PRIszu " blocks; %s..%s\n", vDisconnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexBest->GetBlockHash().ToString().substr(0,20).c_str());
  printf("REORGANIZE: Connect %" PRIszu " blocks; %s..%s\n", vConnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->GetBlockHash().ToString().substr(0,20).c_str());
  list<CTransaction> vResurrect;
  BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
  {
    CBlock block;

    if (!block.ReadFromDisk(pindex))
    {
      return error("Reorganize() : ReadFromDisk for disconnect failed");
    }

    if (!block.DisconnectBlock(txdb, pindex))
    {
      return error("Reorganize() : DisconnectBlock %s failed", pindex->GetBlockHash().ToString().substr(0,20).c_str());
    }

    BOOST_REVERSE_FOREACH(const CTransaction& tx, block.vtx)
    {
      if (!(tx.IsCoinBase() || tx.IsCoinStake()) && pindex->nHeight > Checkpoints::GetTotalBlocksEstimate())
      {
        vResurrect.push_front(tx);
      }
    }
  }
  vector<CTransaction> vDelete;

  for (unsigned int i = 0; i < vConnect.size(); i++)
  {
    CBlockIndex* pindex = vConnect[i];
    CBlock block;

    if (!block.ReadFromDisk(pindex))
    {
      return error("Reorganize() : ReadFromDisk for connect failed");
    }

    if (!block.ConnectBlock(txdb, pindex))
    {
      return error("Reorganize() : ConnectBlock %s failed", pindex->GetBlockHash().ToString().substr(0,20).c_str());
    }

    BOOST_FOREACH(const CTransaction& tx, block.vtx)
    vDelete.push_back(tx);
  }

  if (!txdb.WriteHashBestChain(pindexNew->GetBlockHash()))
  {
    return error("Reorganize() : WriteHashBestChain failed");
  }

  if (!txdb.TxnCommit())
  {
    return error("Reorganize() : TxnCommit failed");
  }

  BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
  {
    if (pindex->pprev)
    {
      pindex->pprev->pnext = NULL;
    }
  }
  BOOST_FOREACH(CBlockIndex* pindex, vConnect)
  {
    if (pindex->pprev)
    {
      pindex->pprev->pnext = pindex;
    }
  }
  BOOST_FOREACH(CTransaction& tx, vResurrect)
  {
    mempool.accept(tx, NULL);
  }
  BOOST_FOREACH(CTransaction& tx, vDelete)
  {
    mempool.remove(tx);
    mempool.removeConflicts(tx);
  }
  printf("REORGANIZE: done\n");
  return true;
}
bool CTransaction::GetCoinAge(CTxDB& txdb, uint64_t& nCoinAge) const
{
  CBigNum bnCentSecond = 0;
  nCoinAge = 0;

  if (IsCoinBase())
  {
    return true;
  }

  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    CTransaction txPrev;
    CTxIndex txindex;

    if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
    {
      continue;
    }

    if (nTime < txPrev.nTime)
    {
      return false;
    }

    CBlock block;

    if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
    {
      return false;
    }

    if (block.GetBlockTime() + nStakeMinAge > nTime)
    {
      continue;
    }

    int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
    bnCentSecond += CBigNum(nValueIn) * (nTime-txPrev.nTime) / CENT;

    if (fDebug && GetBoolArg("-printcoinage"))
    {
      printf("coin age nValueIn=%" PRId64 " nTimeDiff=%d bnCentSecond=%s\n", nValueIn, nTime - txPrev.nTime, bnCentSecond.ToString().c_str());
    }
  }
  CBigNum bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);

  if (fDebug && GetBoolArg("-printcoinage"))
  {
    printf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());
  }

  nCoinAge = bnCoinDay.getuint64();
  return true;
}
bool CTransaction::GetCoinAge(CTxDB& txdb, const CBlockIndex* pindexPrev, uint64_t& nCoinAge) const
{
  CBigNum bnCentSecond = 0;
  nCoinAge = 0;

  if (IsCoinBase())
  {
    return true;
  }

  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    CTransaction txPrev;
    CTxIndex txindex;

    if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
    {
      continue;
    }

    if (nTime < txPrev.nTime)
    {
      return false;
    }

    if(V3(pindexPrev->nHeight))
    {
      int nSpendDepth;

      if(!minBase(txindex, pindexPrev, ConfigurationState::nStakeMinConfirmations - 1, nSpendDepth))
      {
        continue;
      }
    }
    else
    {
      CBlock block;

      if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
      {
        return false;
      }

      if (block.GetBlockTime() + nStakeMinAge > nTime)
      {
        continue;
      }

      int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
      bnCentSecond += CBigNum(nValueIn) * (nTime-txPrev.nTime) / CENT;

      if (fDebug && GetBoolArg("-printcoinage"))
      {
        printf("coin age nValueIn=%" PRId64 " nTimeDiff=%d bnCentSecond=%s\n", nValueIn, nTime - txPrev.nTime, bnCentSecond.ToString().c_str());
      }
    }
  }
  CBigNum bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);

  if (fDebug && GetBoolArg("-printcoinage"))
  {
    printf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());
  }

  nCoinAge = bnCoinDay.getuint64();
  return true;
}
bool ProcessBlock(CNode* pfrom, CBlock* pblock)
{
  AssertLockHeld(cs_main);
  uint256 hash = pblock->GetHash();

  if (mapBlockIndex.count(hash))
  {
    return error("ProcessBlock() : already have block %d %s", mapBlockIndex[hash]->nHeight, hash.ToString().substr(0,20).c_str());
  }

  if (mapOrphanBlocks.count(hash))
  {
    return error("ProcessBlock() : already have block (orphan) %s", hash.ToString().substr(0,20).c_str());
  }

  if (pblock->IsProofOfStake() && setStakeSeen.count(pblock->GetProofOfStake()) && !mapOrphanBlocksByPrev.count(hash) && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
  {
    return error("ProcessBlock() : duplicate proof-of-stake (%s, %d) for block %s", pblock->GetProofOfStake().first.ToString().c_str(), pblock->GetProofOfStake().second, hash.ToString().c_str());
  }

  CBlockIndex* pcheckpoint = Checkpoints::GetLastSyncCheckpoint();

  if (pcheckpoint && pblock->hashPrevBlock != hashBestChain && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
  {
    if (pblock->GetBlockTime() - pcheckpoint->nTime < -600)
    {
      if (pfrom)
      {
        pfrom->Misbehaving(1);
      }

      return error("ProcessBlock() : block with timestamp before last checkpoint");
    }

  }

  if (!pblock->CheckBlock())
  {
    return error("ProcessBlock() : CheckBlock FAILED");
  }

  if (!IsInitialBlockDownload())
  {
    Checkpoints::AskForPendingSyncCheckpoint(pfrom);
  }

  if (!mapBlockIndex.count(pblock->hashPrevBlock))
  {
    printf("ProcessBlock: ORPHAN BLOCK, prev=%s\n", pblock->hashPrevBlock.ToString().substr(0,20).c_str());

    if (pblock->IsProofOfStake())
    {
      if (setStakeSeenOrphan.count(pblock->GetProofOfStake()) && !mapOrphanBlocksByPrev.count(hash) && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
      {
        return error("ProcessBlock() : duplicate proof-of-stake (%s, %d) for orphan block %s", pblock->GetProofOfStake().first.ToString().c_str(), pblock->GetProofOfStake().second, hash.ToString().c_str());
      }
      else
      {
        setStakeSeenOrphan.insert(pblock->GetProofOfStake());
      }
    }

    CBlock* pblock2 = new CBlock(*pblock);
    mapOrphanBlocks.insert(make_pair(hash, pblock2));
    mapOrphanBlocksByPrev.insert(make_pair(pblock2->hashPrevBlock, pblock2));

    if (pfrom)
    {
      pfrom->PushGetBlocks(pindexBest, GetOrphanRoot(pblock2));

      if (!IsInitialBlockDownload())
      {
        pfrom->AskFor(CInv(MSG_BLOCK, WantedByOrphan(pblock2)));
      }
    }

    return true;
  }

  if (!pblock->AcceptBlock())
  {
    return error("ProcessBlock() : AcceptBlock FAILED");
  }

  vector<uint256> vWorkQueue;
  vWorkQueue.push_back(hash);

  for (unsigned int i = 0; i < vWorkQueue.size(); i++)
  {
    uint256 hashPrev = vWorkQueue[i];

    for (multimap<uint256, CBlock*>::iterator mi = mapOrphanBlocksByPrev.lower_bound(hashPrev);
         mi != mapOrphanBlocksByPrev.upper_bound(hashPrev);
         ++mi)
    {
      CBlock* pblockOrphan = (*mi).second;

      if (pblockOrphan->AcceptBlock())
      {
        vWorkQueue.push_back(pblockOrphan->GetHash());
      }

      mapOrphanBlocks.erase(pblockOrphan->GetHash());
      setStakeSeenOrphan.erase(pblockOrphan->GetProofOfStake());
      delete pblockOrphan;
    }

    mapOrphanBlocksByPrev.erase(hashPrev);
  }

  printf("ProcessBlock: ACCEPTED\n");

  if (pfrom && !CSyncCheckpoint::strMasterPrivKey.empty())
  {
    Checkpoints::SendSyncCheckpoint(Checkpoints::AutoSelectSyncCheckpoint());
  }

  return true;
}
bool CheckDiskSpace(uint64_t nAdditionalBytes)
{
  uint64_t nFreeBytesAvailable = boost::filesystem::space(GetDataDir()).available;

  if (nFreeBytesAvailable < nMinDiskSpace + nAdditionalBytes)
  {
    fShutdown = true;
    string strMessage = _("Warning: Disk space is low!");
    strMiscWarning = strMessage;
    printf("*** %s\n", strMessage.c_str());
    uiInterface.ThreadSafeMessageBox(strMessage, "I/OCoin", CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
    StartShutdown();
    return false;
  }

  return true;
}
static unsigned int nCurrentBlockFile = 1;
FILE* AppendBlockFile(unsigned int& nFileRet)
{
  nFileRet = 0;

  while (true)
  {
    FILE* file = OpenBlockFile(nCurrentBlockFile, 0, "ab");

    if (!file)
    {
      return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
      return NULL;
    }

    if (ftell(file) < (long)(0x7F000000 - MAX_SIZE))
    {
      nFileRet = nCurrentBlockFile;
      return file;
    }

    fclose(file);
    nCurrentBlockFile++;
  }
}
bool LoadBlockIndex(bool fAllowNew)
{
  LOCK(cs_main);
  CBigNum bnTrustedModulus;

  if (fTestNet)
  {
    pchMessageStart[0] = 0xff;
    pchMessageStart[1] = 0xc4;
    pchMessageStart[2] = 0xbb;
    pchMessageStart[3] = 0xdf;
    bnTrustedModulus.SetHex("f0d14cf72623dacfe738d0892b599be0f31052239cddd95a3f25101c801dc990453b38c9434efe3f372db39a32c2bb44cbaea72d62c8931fa785b0ec44531308df3e46069be5573e49bb29f4d479bfc3d162f57a5965db03810be7636da265bfced9c01a6b0296c77910ebdc8016f70174f0f18a57b3b971ac43a934c6aedbc5c866764a3622b5b7e3f9832b8b3f133c849dbcc0396588abcd1e41048555746e4823fb8aba5b3d23692c6857fccce733d6bb6ec1d5ea0afafecea14a0f6f798b6b27f77dc989c557795cc39a0940ef6bb29a7fc84135193a55bcfc2f01dd73efad1b69f45a55198bd0e6bef4d338e452f6a420f1ae2b1167b923f76633ab6e55");
    bnProofOfWorkLimit = bnProofOfWorkLimitTestNet;
    nStakeMinAge = 1 * 60 * 60;
    nCoinbaseMaturity = 10;
    ConfigurationState::nStakeMinConfirmations = 10;
  }
  else
  {
    bnTrustedModulus.SetHex("d01f952e1090a5a72a3eda261083256596ccc192935ae1454c2bafd03b09e6ed11811be9f3a69f5783bbbced8c6a0c56621f42c2d19087416facf2f13cc7ed7159d1c5253119612b8449f0c7f54248e382d30ecab1928dbf075c5425dcaee1a819aa13550e0f3227b8c685b14e0eae094d65d8a610a6f49fff8145259d1187e4c6a472fa5868b2b67f957cb74b787f4311dbc13c97a2ca13acdb876ff506ebecbb904548c267d68868e07a32cd9ed461fbc2f920e9940e7788fed2e4817f274df5839c2196c80abe5c486df39795186d7bc86314ae1e8342f3c884b158b4b05b4302754bf351477d35370bad6639b2195d30006b77bf3dbb28b848fd9ecff5662bf39dde0c974e83af51b0d3d642d43834827b8c3b189065514636b8f2a59c42ba9b4fc4975d4827a5d89617a3873e4b377b4d559ad165748632bd928439cfbc5a8ef49bc2220e0b15fb0aa302367d5e99e379a961c1bc8cf89825da5525e3c8f14d7d8acca2fa9c133a2176ae69874d8b1d38b26b9c694e211018005a97b40848681b9dd38feb2de141626fb82591aad20dc629b2b6421cef1227809551a0e4e943ab99841939877f18f2d9c0addc93cf672e26b02ed94da3e6d329e8ac8f3736eebbf37bb1a21e5aadf04ee8e3b542f876aa88b2adf2608bd86329b7f7a56fd0dc1c40b48188731d11082aea360c62a0840c2db3dad7178fd7e359317ae081");
  }

  CTxDB txdb("cr+");

  if (!txdb.LoadBlockIndex())
  {
    return false;
  }

  if (mapBlockIndex.empty())
  {
    if (!fAllowNew)
    {
      return false;
    }

    const char* pszTimestamp = "23 July 2014 BitPay Releases Copay Beta - A New Multi-signature Wallet";
    CTransaction txNew;
    txNew.nTime = 1406153471;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 0 << CBigNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].SetEmpty();
    CBlock block;
    block.vtx.push_back(txNew);
    block.hashPrevBlock = 0;
    block.hashMerkleRoot = block.BuildMerkleTree();
    block.nVersion = 1;
    block.nTime = 1406153471;
    block.nBits = bnProofOfWorkLimit.GetCompact();
    block.nNonce = !fTestNet ? 306504 : 0;
    assert(block.hashMerkleRoot == hashGenesisMerkleRoot);
    block.print();
    assert(block.GetHash() == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
    assert(block.CheckBlock());
    unsigned int nFile;
    unsigned int nBlockPos;

    if (!block.WriteToDisk(nFile, nBlockPos))
    {
      return error("LoadBlockIndex() : writing genesis block to disk failed");
    }

    if (!block.AddToBlockIndex(nFile, nBlockPos, hashGenesisBlock))
    {
      return error("LoadBlockIndex() : genesis block not accepted");
    }

    if (!Checkpoints::WriteSyncCheckpoint((!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet)))
    {
      return error("LoadBlockIndex() : failed to init sync checkpoint");
    }
  }

  string strPubKey = "";

  if (!txdb.ReadCheckpointPubKey(strPubKey) || strPubKey != CSyncCheckpoint::strMasterPubKey)
  {
    txdb.TxnBegin();

    if (!txdb.WriteCheckpointPubKey(CSyncCheckpoint::strMasterPubKey))
    {
      return error("LoadBlockIndex() : failed to write new checkpoint master key to db");
    }

    if (!txdb.TxnCommit())
    {
      return error("LoadBlockIndex() : failed to commit new checkpoint master key to db");
    }

    if ((!fTestNet) && !Checkpoints::ResetSyncCheckpoint())
    {
      return error("LoadBlockIndex() : failed to reset sync-checkpoint");
    }
  }

  return true;
}
void PrintBlockTree()
{
  AssertLockHeld(cs_main);
  map<CBlockIndex*, vector<CBlockIndex*> > mapNext;

  for (map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
  {
    CBlockIndex* pindex = (*mi).second;
    mapNext[pindex->pprev].push_back(pindex);
  }

  vector<pair<int, CBlockIndex*> > vStack;
  vStack.push_back(make_pair(0, pindexGenesisBlock));
  int nPrevCol = 0;

  while (!vStack.empty())
  {
    int nCol = vStack.back().first;
    CBlockIndex* pindex = vStack.back().second;
    vStack.pop_back();

    if (nCol > nPrevCol)
    {
      for (int i = 0; i < nCol-1; i++)
      {
        printf("| ");
      }

      printf("|\\\n");
    }
    else if (nCol < nPrevCol)
    {
      for (int i = 0; i < nCol; i++)
      {
        printf("| ");
      }

      printf("|\n");
    }

    nPrevCol = nCol;

    for (int i = 0; i < nCol; i++)
    {
      printf("| ");
    }

    CBlock block;
    block.ReadFromDisk(pindex);
    printf("%d (%u,%u) %s  %08x  %s  mint %7s  tx %" PRIszu "",
           pindex->nHeight,
           pindex->nFile,
           pindex->nBlockPos,
           block.GetHash().ToString().c_str(),
           block.nBits,
           DateTimeStrFormat("%x %H:%M:%S", block.GetBlockTime()).c_str(),
           FormatMoney(pindex->nMint).c_str(),
           block.vtx.size());
    PrintWallets(block);
    vector<CBlockIndex*>& vNext = mapNext[pindex];

    for (unsigned int i = 0; i < vNext.size(); i++)
    {
      if (vNext[i]->pnext)
      {
        swap(vNext[0], vNext[i]);
        break;
      }
    }

    for (unsigned int i = 0; i < vNext.size(); i++)
    {
      vStack.push_back(make_pair(nCol+i, vNext[i]));
    }
  }
}
bool LoadExternalBlockFile(FILE* fileIn)
{
  int64_t nStart = GetTimeMillis();
  int nLoaded = 0;
  {
    LOCK(cs_main);

    try
    {
      CAutoFile blkdat(fileIn, SER_DISK, CLIENT_VERSION);
      unsigned int nPos = 0;

      while (nPos != (unsigned int)-1 && blkdat.good() && !fRequestShutdown)
      {
        unsigned char pchData[65536];

        do
        {
          fseek(blkdat, nPos, SEEK_SET);
          int nRead = fread(pchData, 1, sizeof(pchData), blkdat);

          if (nRead <= 8)
          {
            nPos = (unsigned int)-1;
            break;
          }

          void* nFind = memchr(pchData, pchMessageStart[0], nRead+1-sizeof(pchMessageStart));

          if (nFind)
          {
            if (memcmp(nFind, pchMessageStart, sizeof(pchMessageStart))==0)
            {
              nPos += ((unsigned char*)nFind - pchData) + sizeof(pchMessageStart);
              break;
            }

            nPos += ((unsigned char*)nFind - pchData) + 1;
          }
          else
          {
            nPos += sizeof(pchData) - sizeof(pchMessageStart) + 1;
          }
        }
        while(!fRequestShutdown);

        if (nPos == (unsigned int)-1)
        {
          break;
        }

        fseek(blkdat, nPos, SEEK_SET);
        unsigned int nSize;
        blkdat >> nSize;

        if (nSize > 0 && nSize <= CBlock::MAX_BLOCK_SIZE)
        {
          CBlock block;
          blkdat >> block;

          if (ProcessBlock(NULL,&block))
          {
            nLoaded++;
            nPos += 4 + nSize;
          }
        }
      }
    }
    catch (std::exception &e)
    {
      printf("%s() : Deserialize or I/O error caught during load\n",
             __PRETTY_FUNCTION__);
    }
  }
  printf("Loaded %i blocks from external file in %" PRId64 "ms\n", nLoaded, GetTimeMillis() - nStart);
  return nLoaded > 0;
}
extern map<uint256, CAlert> mapAlerts;
extern CCriticalSection cs_mapAlerts;
string GetWarnings(string strFor)
{
  int nPriority = 0;
  string strStatusBar;
  string strRPC;

  if (GetBoolArg("-testsafemode"))
  {
    strRPC = "test";
  }

  if (strMiscWarning != "")
  {
    nPriority = 1000;
    strStatusBar = strMiscWarning;
  }

  if (Checkpoints::hashInvalidCheckpoint != 0)
  {
    nPriority = 3000;
    strStatusBar = strRPC = _("WARNING: Invalid checkpoint found! Displayed transactions may not be correct! You may need to upgrade, or notify developers.");
  }

  {
    LOCK(cs_mapAlerts);
    BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
    {
      const CAlert& alert = item.second;

      if (alert.AppliesToMe() && alert.nPriority > nPriority)
      {
        nPriority = alert.nPriority;
        strStatusBar = alert.strStatusBar;

        if (nPriority > 1000)
        {
          strRPC = strStatusBar;
        }
      }
    }
  }

  if (strFor == "statusbar")
  {
    return strStatusBar;
  }
  else if (strFor == "rpc")
  {
    return strRPC;
  }

  assert(!"GetWarnings() : invalid parameter");
  return "error";
}
bool static AlreadyHave(CTxDB& txdb, const CInv& inv)
{
  switch (inv.type)
  {
  case MSG_TX:
  {
    bool txInMap = false;
    txInMap = mempool.exists(inv.hash);
    return txInMap ||
           mapOrphanTransactions.count(inv.hash) ||
           txdb.ContainsTx(inv.hash);
  }

  case MSG_BLOCK:
    return mapBlockIndex.count(inv.hash) ||
           mapOrphanBlocks.count(inv.hash);
  }

  return true;
}
unsigned char pchMessageStart[4] = { 0xfe, 0xc3, 0xba, 0xde };
bool static ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv)
{
  static map<CService, CPubKey> mapReuseKey;
  RandAddSeedPerfmon();

  if (fDebug)
  {
    printf("received: %s (%" PRIszu " bytes)\n", strCommand.c_str(), vRecv.size());
  }

  if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
  {
    printf("dropmessagestest DROPPING RECV MESSAGE\n");
    return true;
  }

  if (strCommand == "version")
  {
    if (pfrom->nVersion != 0)
    {
      pfrom->Misbehaving(1);
      return false;
    }

    int64_t nTime;
    CAddress addrMe;
    CAddress addrFrom;
    uint64_t nNonce = 1;
    vRecv >> pfrom->nVersion >> pfrom->nServices >> nTime >> addrMe;

    if (pfrom->nVersion < MIN_PEER_PROTO_VERSION)
    {
      printf("partner %s using obsolete version %i; disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
      pfrom->fDisconnect = true;
      return false;
    }

    if (pfrom->nVersion == 10300)
    {
      pfrom->nVersion = 300;
    }

    if (!vRecv.empty())
    {
      vRecv >> addrFrom >> nNonce;
    }

    if (!vRecv.empty())
    {
      vRecv >> pfrom->strSubVer;
    }

    if (!vRecv.empty())
    {
      vRecv >> pfrom->nStartingHeight;
    }

    if (pfrom->fInbound && addrMe.IsRoutable())
    {
      pfrom->addrLocal = addrMe;
      SeenLocal(addrMe);
    }

    if (nNonce == nLocalHostNonce && nNonce > 1)
    {
      printf("connected to self at %s, disconnecting\n", pfrom->addr.ToString().c_str());
      pfrom->fDisconnect = true;
      return true;
    }

    if (addrFrom.IsRoutable() && addrMe.IsRoutable())
    {
      addrSeenByPeer = addrMe;
    }

    if (pfrom->fInbound)
    {
      pfrom->PushVersion();
    }

    pfrom->fClient = !(pfrom->nServices & NODE_NETWORK);

    if (GetBoolArg("-synctime", true))
    {
      AddTimeData(pfrom->addr, nTime);
    }

    pfrom->PushMessage("verack");
    pfrom->ssSend.SetVersion(min(pfrom->nVersion, PROTOCOL_VERSION));

    if (!pfrom->fInbound)
    {
      if (!fNoListen && !IsInitialBlockDownload())
      {
        CAddress addr = GetLocalAddress(&pfrom->addr);

        if (addr.IsRoutable())
        {
          pfrom->PushAddress(addr);
        }
      }

      if (pfrom->fOneShot || pfrom->nVersion >= CADDR_TIME_VERSION || addrman.size() < 1000)
      {
        pfrom->PushMessage("getaddr");
        pfrom->fGetAddr = true;
      }

      addrman.Good(pfrom->addr);
    }
    else
    {
      if (((CNetAddr)pfrom->addr) == (CNetAddr)addrFrom)
      {
        addrman.Add(addrFrom, addrFrom);
        addrman.Good(addrFrom);
      }
    }

    static int nAskedForBlocks = 0;

    if (!pfrom->fClient && !pfrom->fOneShot &&
        (pfrom->nStartingHeight > (nBestHeight - 144)) &&
        (pfrom->nVersion < NOBLKS_VERSION_START ||
         pfrom->nVersion >= NOBLKS_VERSION_END) &&
        (nAskedForBlocks < 1 || vNodes.size() <= 1))
    {
      nAskedForBlocks++;
      pfrom->PushGetBlocks(pindexBest, uint256(0));
    }

    {
      LOCK(cs_mapAlerts);
      BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
      item.second.RelayTo(pfrom);
    }

    {
      LOCK(Checkpoints::cs_hashSyncCheckpoint);

      if (!Checkpoints::checkpointMessage.IsNull())
      {
        Checkpoints::checkpointMessage.RelayTo(pfrom);
      }
    }

    pfrom->fSuccessfullyConnected = true;
    printf("receive version message: version %d, blocks=%d, us=%s, them=%s, peer=%s\n", pfrom->nVersion, pfrom->nStartingHeight, addrMe.ToString().c_str(), addrFrom.ToString().c_str(), pfrom->addr.ToString().c_str());
    cPeerBlockCounts.input(pfrom->nStartingHeight);

    if (!IsInitialBlockDownload())
    {
      Checkpoints::AskForPendingSyncCheckpoint(pfrom);
    }
  }
  else if (pfrom->nVersion == 0)
  {
    pfrom->Misbehaving(1);
    return false;
  }
  else if (strCommand == "verack")
  {
    pfrom->SetRecvVersion(min(pfrom->nVersion, PROTOCOL_VERSION));
  }
  else if (strCommand == "addr")
  {
    vector<CAddress> vAddr;
    vRecv >> vAddr;

    if (pfrom->nVersion < CADDR_TIME_VERSION && addrman.size() > 1000)
    {
      return true;
    }

    if (vAddr.size() > 1000)
    {
      pfrom->Misbehaving(20);
      return error("message addr size() = %" PRIszu "", vAddr.size());
    }

    vector<CAddress> vAddrOk;
    int64_t nNow = GetAdjustedTime();
    int64_t nSince = nNow - 10 * 60;
    BOOST_FOREACH(CAddress& addr, vAddr)
    {
      if (fShutdown)
      {
        return true;
      }

      if (addr.nTime <= 100000000 || addr.nTime > nNow + 10 * 60)
      {
        addr.nTime = nNow - 5 * 24 * 60 * 60;
      }

      pfrom->AddAddressKnown(addr);
      bool fReachable = IsReachable(addr);

      if (addr.nTime > nSince && !pfrom->fGetAddr && vAddr.size() <= 10 && addr.IsRoutable())
      {
        {
          LOCK(cs_vNodes);
          static uint256 hashSalt;

          if (hashSalt == 0)
          {
            hashSalt = GetRandHash();
          }

          uint64_t hashAddr = addr.GetHash();
          uint256 hashRand = hashSalt ^ (hashAddr<<32) ^ ((GetTime()+hashAddr)/(24*60*60));
          hashRand = Hash(BEGIN(hashRand), END(hashRand));
          multimap<uint256, CNode*> mapMix;
          BOOST_FOREACH(CNode* pnode, vNodes)
          {
            if (pnode->nVersion < CADDR_TIME_VERSION)
            {
              continue;
            }

            unsigned int nPointer;
            memcpy(&nPointer, &pnode, sizeof(nPointer));
            uint256 hashKey = hashRand ^ nPointer;
            hashKey = Hash(BEGIN(hashKey), END(hashKey));
            mapMix.insert(make_pair(hashKey, pnode));
          }
          int nRelayNodes = fReachable ? 2 : 1;

          for (multimap<uint256, CNode*>::iterator mi = mapMix.begin(); mi != mapMix.end() && nRelayNodes-- > 0; ++mi)
          {
            ((*mi).second)->PushAddress(addr);
          }
        }
      }

      if (fReachable)
      {
        vAddrOk.push_back(addr);
      }
    }
    addrman.Add(vAddrOk, pfrom->addr, 2 * 60 * 60);

    if (vAddr.size() < 1000)
    {
      pfrom->fGetAddr = false;
    }

    if (pfrom->fOneShot)
    {
      pfrom->fDisconnect = true;
    }
  }
  else if (strCommand == "inv")
  {
    vector<CInv> vInv;
    vRecv >> vInv;

    if (vInv.size() > ConfigurationState::MAX_INV_SZ)
    {
      pfrom->Misbehaving(20);
      return error("message inv size() = %" PRIszu "", vInv.size());
    }

    unsigned int nLastBlock = (unsigned int)(-1);

    for (unsigned int nInv = 0; nInv < vInv.size(); nInv++)
    {
      if (vInv[vInv.size() - 1 - nInv].type == MSG_BLOCK)
      {
        nLastBlock = vInv.size() - 1 - nInv;
        break;
      }
    }

    CTxDB txdb("r");

    for (unsigned int nInv = 0; nInv < vInv.size(); nInv++)
    {
      const CInv &inv = vInv[nInv];

      if (fShutdown)
      {
        return true;
      }

      pfrom->AddInventoryKnown(inv);
      bool fAlreadyHave = AlreadyHave(txdb, inv);

      if (fDebug)
      {
        printf("  got inventory: %s  %s\n", inv.ToString().c_str(), fAlreadyHave ? "have" : "new");
      }

      if (!fAlreadyHave)
      {
        pfrom->AskFor(inv);
      }
      else if (inv.type == MSG_BLOCK && mapOrphanBlocks.count(inv.hash))
      {
        pfrom->PushGetBlocks(pindexBest, GetOrphanRoot(mapOrphanBlocks[inv.hash]));
      }
      else if (nInv == nLastBlock)
      {
        pfrom->PushGetBlocks(mapBlockIndex[inv.hash], uint256(0));

        if (fDebug)
        {
          printf("force request: %s\n", inv.ToString().c_str());
        }
      }

      Inventory(inv.hash);
    }
  }
  else if (strCommand == "getdata")
  {
    vector<CInv> vInv;
    vRecv >> vInv;

    if (vInv.size() > ConfigurationState::MAX_INV_SZ)
    {
      pfrom->Misbehaving(20);
      return error("message getdata size() = %" PRIszu "", vInv.size());
    }

    if (fDebugNet || (vInv.size() != 1))
    {
      printf("received getdata (%" PRIszu " invsz)\n", vInv.size());
    }

    BOOST_FOREACH(const CInv& inv, vInv)
    {
      if (fShutdown)
      {
        return true;
      }

      if (fDebugNet || (vInv.size() == 1))
      {
        printf("received getdata for: %s\n", inv.ToString().c_str());
      }

      if (inv.type == MSG_BLOCK)
      {
        map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(inv.hash);

        if (mi != mapBlockIndex.end())
        {
          CBlock block;
          block.ReadFromDisk((*mi).second);
          pfrom->PushMessage("block", block);

          if (inv.hash == pfrom->hashContinue)
          {
            vector<CInv> vInv;
            vInv.push_back(CInv(MSG_BLOCK, GetLastBlockIndex(pindexBest, false)->GetBlockHash()));
            pfrom->PushMessage("inv", vInv);
            pfrom->hashContinue = 0;
          }
        }
      }
      else if (inv.IsKnownType())
      {
        bool pushed = false;
        {
          LOCK(cs_mapRelay);
          map<CInv, CDataStream>::iterator mi = mapRelay.find(inv);

          if (mi != mapRelay.end())
          {
            pfrom->PushMessage(inv.GetCommand(), (*mi).second);
            pushed = true;
          }
        }

        if (!pushed && inv.type == MSG_TX)
        {
          CTransaction tx;

          if (mempool.lookup(inv.hash, tx))
          {
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss.reserve(1000);
            ss << tx;
            pfrom->PushMessage("tx", ss);
          }
        }
      }

      Inventory(inv.hash);
    }
  }
  else if (strCommand == "getblocks")
  {
    CBlockLocator locator;
    uint256 hashStop;
    vRecv >> locator >> hashStop;
    CBlockIndex* pindex = locator.GetBlockIndex();

    if (pindex)
    {
      pindex = pindex->pnext;
    }

    int nLimit = 500;
    printf("getblocks %d to %s limit %d\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str(), nLimit);

    for (; pindex; pindex = pindex->pnext)
    {
      if (pindex->GetBlockHash() == hashStop)
      {
        printf("  getblocks stopping at %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str());

        if (hashStop != hashBestChain && pindex->GetBlockTime() + nStakeMinAge > pindexBest->GetBlockTime())
        {
          pfrom->PushInventory(CInv(MSG_BLOCK, hashBestChain));
        }

        break;
      }

      pfrom->PushInventory(CInv(MSG_BLOCK, pindex->GetBlockHash()));

      if (--nLimit <= 0)
      {
        printf("  getblocks stopping at limit %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str());
        pfrom->hashContinue = pindex->GetBlockHash();
        break;
      }
    }
  }
  else if (strCommand == "checkpoint")
  {
    CSyncCheckpoint checkpoint;
    vRecv >> checkpoint;

    if (checkpoint.ProcessSyncCheckpoint(pfrom))
    {
      pfrom->hashCheckpointKnown = checkpoint.hashCheckpoint;
      LOCK(cs_vNodes);
      BOOST_FOREACH(CNode* pnode, vNodes)
      checkpoint.RelayTo(pnode);
    }
  }
  else if (strCommand == "getheaders")
  {
    CBlockLocator locator;
    uint256 hashStop;
    vRecv >> locator >> hashStop;
    CBlockIndex* pindex = NULL;

    if (locator.IsNull())
    {
      map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashStop);

      if (mi == mapBlockIndex.end())
      {
        return true;
      }

      pindex = (*mi).second;
    }
    else
    {
      pindex = locator.GetBlockIndex();

      if (pindex)
      {
        pindex = pindex->pnext;
      }
    }

    vector<CBlock> vHeaders;
    int nLimit = 2000;
    printf("getheaders %d to %s\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str());

    for (; pindex; pindex = pindex->pnext)
    {
      vHeaders.push_back(pindex->GetBlockHeader());

      if (--nLimit <= 0 || pindex->GetBlockHash() == hashStop)
      {
        break;
      }
    }

    pfrom->PushMessage("headers", vHeaders);
  }
  else if (strCommand == "tx")
  {
    vector<uint256> vWorkQueue;
    vector<uint256> vEraseQueue;
    CTransaction tx;
    vRecv >> tx;
    CInv inv(MSG_TX, tx.GetHash());
    pfrom->AddInventoryKnown(inv);
    bool fMissingInputs = false;

    if (mempool.accept(tx, &fMissingInputs))
    {
      SyncWithWallets(tx, NULL, true);
      RelayTransaction(tx, inv.hash);
      mapAlreadyAskedFor.erase(inv);
      vWorkQueue.push_back(inv.hash);
      vEraseQueue.push_back(inv.hash);

      for (unsigned int i = 0; i < vWorkQueue.size(); i++)
      {
        uint256 hashPrev = vWorkQueue[i];

        for (set<uint256>::iterator mi = mapOrphanTransactionsByPrev[hashPrev].begin();
             mi != mapOrphanTransactionsByPrev[hashPrev].end();
             ++mi)
        {
          const uint256& orphanTxHash = *mi;
          CTransaction& orphanTx = mapOrphanTransactions[orphanTxHash];
          bool fMissingInputs2 = false;

          if (mempool.accept(orphanTx, &fMissingInputs2))
          {
            printf("   accepted orphan tx %s\n", orphanTxHash.ToString().substr(0,10).c_str());
            SyncWithWallets(tx, NULL, true);
            RelayTransaction(orphanTx, orphanTxHash);
            mapAlreadyAskedFor.erase(CInv(MSG_TX, orphanTxHash));
            vWorkQueue.push_back(orphanTxHash);
            vEraseQueue.push_back(orphanTxHash);
          }
          else if (!fMissingInputs2)
          {
            vEraseQueue.push_back(orphanTxHash);
            printf("   removed invalid orphan tx %s\n", orphanTxHash.ToString().substr(0,10).c_str());
          }
        }
      }

      BOOST_FOREACH(uint256 hash, vEraseQueue)
      EraseOrphanTx(hash);
    }
    else if (fMissingInputs)
    {
      AddOrphanTx(tx);
      unsigned int nEvicted = LimitOrphanTxSize(ConfigurationState::MAX_ORPHAN_TRANSACTIONS);

      if (nEvicted > 0)
      {
        printf("mapOrphan overflow, removed %u tx\n", nEvicted);
      }
    }

    if (tx.nDoS)
    {
      pfrom->Misbehaving(tx.nDoS);
    }
  }
  else if (strCommand == "block")
  {
    CBlock block;
    vRecv >> block;
    uint256 hashBlock = block.GetHash();
    printf("received block %s\n", hashBlock.ToString().substr(0,20).c_str());
    CInv inv(MSG_BLOCK, hashBlock);
    pfrom->AddInventoryKnown(inv);

    if (ProcessBlock(pfrom, &block))
    {
      mapAlreadyAskedFor.erase(inv);
    }

    if (block.nDoS)
    {
      pfrom->Misbehaving(block.nDoS);
    }
  }
  else if (strCommand == "getaddr")
  {
    int64_t nCutOff = GetTime() - (nNodeLifespan * 24 * 60 * 60);
    pfrom->vAddrToSend.clear();
    vector<CAddress> vAddr = addrman.GetAddr();
    BOOST_FOREACH(const CAddress &addr, vAddr)

    if(addr.nTime > nCutOff)
    {
      pfrom->PushAddress(addr);
    }
  }
  else if (strCommand == "mempool")
  {
    std::vector<uint256> vtxid;
    mempool.queryHashes(vtxid);
    vector<CInv> vInv;

    for (unsigned int i = 0; i < vtxid.size(); i++)
    {
      CInv inv(MSG_TX, vtxid[i]);
      vInv.push_back(inv);

      if (i == (ConfigurationState::MAX_INV_SZ - 1))
      {
        break;
      }
    }

    if (vInv.size() > 0)
    {
      pfrom->PushMessage("inv", vInv);
    }
  }
  else if (strCommand == "checkorder")
  {
    uint256 hashReply;
    vRecv >> hashReply;

    if (!GetBoolArg("-allowreceivebyip"))
    {
      pfrom->PushMessage("reply", hashReply, (int)2, string(""));
      return true;
    }

    __wx__Tx order;
    vRecv >> order;

    if (!mapReuseKey.count(pfrom->addr))
    {
      pwalletMainId->GetKeyFromPool(mapReuseKey[pfrom->addr], true);
    }

    CScript scriptPubKey;
    scriptPubKey << mapReuseKey[pfrom->addr] << OP_CHECKSIG;
    pfrom->PushMessage("reply", hashReply, (int)0, scriptPubKey);
  }
  else if (strCommand == "reply")
  {
    uint256 hashReply;
    vRecv >> hashReply;
    CRequestTracker tracker;
    {
      LOCK(pfrom->cs_mapRequests);
      map<uint256, CRequestTracker>::iterator mi = pfrom->mapRequests.find(hashReply);

      if (mi != pfrom->mapRequests.end())
      {
        tracker = (*mi).second;
        pfrom->mapRequests.erase(mi);
      }
    }

    if (!tracker.IsNull())
    {
      tracker.fn(tracker.param1, vRecv);
    }
  }
  else if (strCommand == "ping")
  {
    if (pfrom->nVersion > BIP0031_VERSION)
    {
      uint64_t nonce = 0;
      vRecv >> nonce;
      pfrom->PushMessage("pong", nonce);
    }
  }
  else if (strCommand == "alert")
  {
    CAlert alert;
    vRecv >> alert;
    uint256 alertHash = alert.GetHash();

    if (pfrom->setKnown.count(alertHash) == 0)
    {
      if (alert.ProcessAlert())
      {
        pfrom->setKnown.insert(alertHash);
        {
          LOCK(cs_vNodes);
          BOOST_FOREACH(CNode* pnode, vNodes)
          alert.RelayTo(pnode);
        }
      }
      else
      {
        pfrom->Misbehaving(10);
      }
    }
  }
  else
  {
  }

  if (pfrom->fNetworkNode)
    if (strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
    {
      AddressCurrentlyConnected(pfrom->addr);
    }

  return true;
}
bool ProcessMessages(CNode* pfrom)
{
  bool fOk = true;
  std::deque<CNetMessage>::iterator it = pfrom->vRecvMsg.begin();

  while (!pfrom->fDisconnect && it != pfrom->vRecvMsg.end())
  {
    if (pfrom->nSendSize >= SendBufferSize())
    {
      break;
    }

    CNetMessage& msg = *it;

    if (!msg.complete())
    {
      break;
    }

    it++;

    if (memcmp(msg.hdr.pchMessageStart, pchMessageStart, sizeof(pchMessageStart)) != 0)
    {
      printf("\n\nPROCESSMESSAGE: INVALID MESSAGESTART\n\n");
      fOk = false;
      break;
    }

    CMessageHeader& hdr = msg.hdr;

    if (!hdr.IsValid())
    {
      printf("\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand().c_str());
      continue;
    }

    string strCommand = hdr.GetCommand();
    unsigned int nMessageSize = hdr.nMessageSize;
    CDataStream& vRecv = msg.vRecv;
    uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
    unsigned int nChecksum = 0;
    memcpy(&nChecksum, &hash, sizeof(nChecksum));

    if (nChecksum != hdr.nChecksum)
    {
      printf("ProcessMessages(%s, %u bytes) : CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
             strCommand.c_str(), nMessageSize, nChecksum, hdr.nChecksum);
      continue;
    }

    bool fRet = false;

    try
    {
      {
        LOCK(cs_main);
        fRet = ProcessMessage(pfrom, strCommand, vRecv);
      }

      if (fShutdown)
      {
        break;
      }
    }
    catch (std::ios_base::failure& e)
    {
      if (strstr(e.what(), "end of data"))
      {
        printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught, normally caused by a message being shorter than its stated length\n", strCommand.c_str(), nMessageSize, e.what());
      }
      else if (strstr(e.what(), "size too large"))
      {
        printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught\n", strCommand.c_str(), nMessageSize, e.what());
      }
      else
      {
        PrintExceptionContinue(&e, "ProcessMessages()");
      }
    }
    catch (std::exception& e)
    {
      PrintExceptionContinue(&e, "ProcessMessages()");
    }
    catch (...)
    {
      PrintExceptionContinue(NULL, "ProcessMessages()");
    }

    if (!fRet)
    {
      printf("ProcessMessage(%s, %u bytes) FAILED\n", strCommand.c_str(), nMessageSize);
    }
  }

  if (!pfrom->fDisconnect)
  {
    pfrom->vRecvMsg.erase(pfrom->vRecvMsg.begin(), it);
  }

  return fOk;
}
bool SendMessages(CNode* pto, bool fSendTrickle)
{
  TRY_LOCK(cs_main, lockMain);

  if (lockMain)
  {
    if (pto->nVersion == 0)
    {
      return true;
    }

    if (pto->nLastSend && GetTime() - pto->nLastSend > 30 * 60 && pto->vSendMsg.empty())
    {
      uint64_t nonce = 0;

      if (pto->nVersion > BIP0031_VERSION)
      {
        pto->PushMessage("ping", nonce);
      }
      else
      {
        pto->PushMessage("ping");
      }
    }

    ResendWalletTransactions();
    static int64_t nLastRebroadcast;

    if (!IsInitialBlockDownload() && (GetTime() - nLastRebroadcast > 24 * 60 * 60))
    {
      {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
        {
          if (nLastRebroadcast)
          {
            pnode->setAddrKnown.clear();
          }

          if (!fNoListen)
          {
            CAddress addr = GetLocalAddress(&pnode->addr);

            if (addr.IsRoutable())
            {
              pnode->PushAddress(addr);
            }
          }
        }
      }
      nLastRebroadcast = GetTime();
    }

    if (fSendTrickle)
    {
      vector<CAddress> vAddr;
      vAddr.reserve(pto->vAddrToSend.size());
      BOOST_FOREACH(const CAddress& addr, pto->vAddrToSend)
      {
        if (pto->setAddrKnown.insert(addr).second)
        {
          vAddr.push_back(addr);

          if (vAddr.size() >= 1000)
          {
            pto->PushMessage("addr", vAddr);
            vAddr.clear();
          }
        }
      }
      pto->vAddrToSend.clear();

      if (!vAddr.empty())
      {
        pto->PushMessage("addr", vAddr);
      }
    }

    vector<CInv> vInv;
    vector<CInv> vInvWait;
    {
      LOCK(pto->cs_inventory);
      vInv.reserve(pto->vInventoryToSend.size());
      vInvWait.reserve(pto->vInventoryToSend.size());
      BOOST_FOREACH(const CInv& inv, pto->vInventoryToSend)
      {
        if (pto->setInventoryKnown.count(inv))
        {
          continue;
        }

        if (inv.type == MSG_TX && !fSendTrickle)
        {
          static uint256 hashSalt;

          if (hashSalt == 0)
          {
            hashSalt = GetRandHash();
          }

          uint256 hashRand = inv.hash ^ hashSalt;
          hashRand = Hash(BEGIN(hashRand), END(hashRand));
          bool fTrickleWait = ((hashRand & 3) != 0);

          if (!fTrickleWait)
          {
            __wx__Tx wtx;

            if (GetTransaction(inv.hash, wtx))
              if (wtx.fFromMe)
              {
                fTrickleWait = true;
              }
          }

          if (fTrickleWait)
          {
            vInvWait.push_back(inv);
            continue;
          }
        }

        if (pto->setInventoryKnown.insert(inv).second)
        {
          vInv.push_back(inv);

          if (vInv.size() >= 1000)
          {
            pto->PushMessage("inv", vInv);
            vInv.clear();
          }
        }
      }
      pto->vInventoryToSend = vInvWait;
    }

    if (!vInv.empty())
    {
      pto->PushMessage("inv", vInv);
    }

    vector<CInv> vGetData;
    int64_t nNow = GetTime() * 1000000;
    CTxDB txdb("r");

    while (!pto->mapAskFor.empty() && (*pto->mapAskFor.begin()).first <= nNow)
    {
      const CInv& inv = (*pto->mapAskFor.begin()).second;

      if (!AlreadyHave(txdb, inv))
      {
        if (fDebugNet)
        {
          printf("sending getdata: %s\n", inv.ToString().c_str());
        }

        vGetData.push_back(inv);

        if (vGetData.size() >= 1000)
        {
          pto->PushMessage("getdata", vGetData);
          vGetData.clear();
        }

        mapAlreadyAskedFor[inv] = nNow;
      }

      pto->mapAskFor.erase(pto->mapAskFor.begin());
    }

    if (!vGetData.empty())
    {
      pto->PushMessage("getdata", vGetData);
    }
  }

  return true;
}
Value getnumblocksofpeers(const Array& params, bool fHelp)
{
  if(fHelp || params.size() != 0)
  {
    throw runtime_error("getnumblocksofpeers return maximum number of blocks that other nodes claim to have");
  }

  return (int)std::max(cPeerBlockCounts.median(), Checkpoints::GetTotalBlocksEstimate());
}
