
#include <boost/assign/list_of.hpp>
#include "kernel.h"
#include "txdb.h"
using namespace std;
typedef std::map<int, unsigned int> MapModifierCheckpoints;
static std::map<int, unsigned int> mapStakeModifierCheckpoints =
  boost::assign::map_list_of
  ( 0, 0xfd11f4e7 )
  ;
static std::map<int, unsigned int> mapStakeModifierCheckpointsTestNet;
int64_t GetWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd)
{
  return nIntervalEnd - nIntervalBeginning - nStakeMinAge;
}
static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64_t& nStakeModifier, int64_t& nModifierTime)
{
  if (!pindex)
  {
    return error("GetLastStakeModifier: null pindex");
  }

  while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
  {
    pindex = pindex->pprev;
  }

  if (!pindex->GeneratedStakeModifier())
  {
    return error("GetLastStakeModifier: no generation at genesis block");
  }

  nStakeModifier = pindex->nStakeModifier;
  nModifierTime = pindex->GetBlockTime();
  return true;
}
static int64_t GetStakeModifierSelectionIntervalSection(int nSection)
{
  assert (nSection >= 0 && nSection < 64);
  return (nModifierInterval * 63 / (63 + ((63 - nSection) * (MODIFIER_INTERVAL_RATIO - 1))));
}
static int64_t GetStakeModifierSelectionInterval()
{
  int64_t nSelectionInterval = 0;

  for (int nSection=0; nSection<64; nSection++)
  {
    nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
  }

  return nSelectionInterval;
}
static bool SelectBlockFromCandidates(vector<pair<int64_t, uint256> >& vSortedByTimestamp, map<uint256, const CBlockIndex*>& mapSelectedBlocks,
                                      int64_t nSelectionIntervalStop, uint64_t nStakeModifierPrev, const CBlockIndex** pindexSelected)
{
  bool fSelected = false;
  uint256 hashBest = 0;
  *pindexSelected = (const CBlockIndex*) 0;
  BOOST_FOREACH(const PAIRTYPE(int64_t, uint256)& item, vSortedByTimestamp)
  {
    if (!mapBlockIndex.count(item.second))
    {
      return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString().c_str());
    }

    const CBlockIndex* pindex = mapBlockIndex[item.second];

    if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop)
    {
      break;
    }

    if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
    {
      continue;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << pindex->hashProof << nStakeModifierPrev;
    uint256 hashSelection = Hash(ss.begin(), ss.end());

    if (pindex->IsProofOfStake())
    {
      hashSelection >>= 32;
    }

    if (fSelected && hashSelection < hashBest)
    {
      hashBest = hashSelection;
      *pindexSelected = (const CBlockIndex*) pindex;
    }
    else if (!fSelected)
    {
      fSelected = true;
      hashBest = hashSelection;
      *pindexSelected = (const CBlockIndex*) pindex;
    }
  }

  if (fDebug && GetBoolArg("-printstakemodifier"))
  {
    printf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString().c_str());
  }

  return fSelected;
}
# 140 "kernel.cpp"
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier)
{
  nStakeModifier = 0;
  fGeneratedStakeModifier = false;

  if (!pindexPrev)
  {
    fGeneratedStakeModifier = true;
    return true;
  }

  int64_t nModifierTime = 0;

  if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
  {
    return error("ComputeNextStakeModifier: unable to get last modifier");
  }

  if (fDebug)
  {
    printf("ComputeNextStakeModifier: prev modifier=0x%016" PRIx64 " time=%s\n", nStakeModifier, DateTimeStrFormat(nModifierTime).c_str());
  }

  if (nModifierTime / nModifierInterval >= pindexPrev->GetBlockTime() / nModifierInterval)
  {
    return true;
  }

  vector<pair<int64_t, uint256> > vSortedByTimestamp;
  vSortedByTimestamp.reserve(64 * nModifierInterval / GetTargetSpacing(pindexPrev->nHeight));
  int64_t nSelectionInterval = GetStakeModifierSelectionInterval();
  int64_t nSelectionIntervalStart = (pindexPrev->GetBlockTime() / nModifierInterval) * nModifierInterval - nSelectionInterval;
  const CBlockIndex* pindex = pindexPrev;

  while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart)
  {
    vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
    pindex = pindex->pprev;
  }

  int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;
  reverse(vSortedByTimestamp.begin(), vSortedByTimestamp.end());
  sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end());
  uint64_t nStakeModifierNew = 0;
  int64_t nSelectionIntervalStop = nSelectionIntervalStart;
  map<uint256, const CBlockIndex*> mapSelectedBlocks;

  for (int nRound=0; nRound<min(64, (int)vSortedByTimestamp.size()); nRound++)
  {
    nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);

    if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex))
    {
      return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);
    }

    nStakeModifierNew |= (((uint64_t)pindex->GetStakeEntropyBit()) << nRound);
    mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));

    if (fDebug && GetBoolArg("-printstakemodifier"))
    {
      printf("ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n", nRound, DateTimeStrFormat(nSelectionIntervalStop).c_str(), pindex->nHeight, pindex->GetStakeEntropyBit());
    }
  }

  if (fDebug && GetBoolArg("-printstakemodifier"))
  {
    string strSelectionMap = "";
    strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
    pindex = pindexPrev;

    while (pindex && pindex->nHeight >= nHeightFirstCandidate)
    {
      if (pindex->IsProofOfStake())
      {
        strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
      }

      pindex = pindex->pprev;
    }

    BOOST_FOREACH(const PAIRTYPE(uint256, const CBlockIndex*)& item, mapSelectedBlocks)
    {
      strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake()? "S" : "W");
    }
    printf("ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap.c_str());
  }

  if (fDebug)
  {
    printf("ComputeNextStakeModifier: new modifier=0x%016" PRIx64 " time=%s\n", nStakeModifierNew, DateTimeStrFormat(pindexPrev->GetBlockTime()).c_str());
  }

  nStakeModifier = nStakeModifierNew;
  fGeneratedStakeModifier = true;
  return true;
}
static bool GetKernelStakeModifier(uint256 hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake)
{
  nStakeModifier = 0;

  if (!mapBlockIndex.count(hashBlockFrom))
  {
    return error("GetKernelStakeModifier() : block not indexed");
  }

  const CBlockIndex* pindexFrom = mapBlockIndex[hashBlockFrom];
  nStakeModifierHeight = pindexFrom->nHeight;
  nStakeModifierTime = pindexFrom->GetBlockTime();
  int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();
  const CBlockIndex* pindex = pindexFrom;

  while (nStakeModifierTime < pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval)
  {
    if (!pindex->pnext)
    {
      if (fPrintProofOfStake || (pindex->GetBlockTime() + nStakeMinAge - nStakeModifierSelectionInterval > GetAdjustedTime()))
        return error("GetKernelStakeModifier() : reached best block %s at height %d from block %s",
                     pindex->GetBlockHash().ToString().c_str(), pindex->nHeight, hashBlockFrom.ToString().c_str());
      else
      {
        return false;
      }
    }

    pindex = pindex->pnext;

    if (pindex->GeneratedStakeModifier())
    {
      nStakeModifierHeight = pindex->nHeight;
      nStakeModifierTime = pindex->GetBlockTime();
    }
  }

  nStakeModifier = pindex->nStakeModifier;
  return true;
}
# 297 "kernel.cpp"
static bool CheckStakeKernelHashV1(unsigned int nBits, const CBlock& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, uint256& targetProofOfStake, bool fPrintProofOfStake)
{
  if (nTimeTx < txPrev.nTime)
  {
    return error("CheckStakeKernelHash() : nTime violation");
  }

  unsigned int nTimeBlockFrom = blockFrom.GetBlockTime();
  CBigNum bnTargetPerCoinDay;
  bnTargetPerCoinDay.SetCompact(nBits);
  int64_t nValueIn = txPrev.vout[prevout.n].nValue;
  uint256 hashBlockFrom = blockFrom.GetHash();
  CBigNum bnCoinDayWeight = CBigNum(nValueIn) * GetWeight((int64_t)txPrev.nTime, (int64_t)nTimeTx) / COIN / (60);
  targetProofOfStake = (bnCoinDayWeight * bnTargetPerCoinDay).getuint256();
  CDataStream ss(SER_GETHASH, 0);
  uint64_t nStakeModifier = 0;
  int nStakeModifierHeight = 0;
  int64_t nStakeModifierTime = 0;

  if (!GetKernelStakeModifier(hashBlockFrom, nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake))
  {
    return false;
  }

  ss << nStakeModifier;
  ss << nTimeBlockFrom << nTxPrevOffset << txPrev.nTime << prevout.n << nTimeTx;
  hashProofOfStake = Hash(ss.begin(), ss.end());

  if (fPrintProofOfStake)
  {
    printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
           nStakeModifier, nStakeModifierHeight,
           DateTimeStrFormat(nStakeModifierTime).c_str(),
           mapBlockIndex[hashBlockFrom]->nHeight,
           DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
    printf("CheckStakeKernelHash() : check modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
           nStakeModifier,
           nTimeBlockFrom, nTxPrevOffset, txPrev.nTime, prevout.n, nTimeTx,
           hashProofOfStake.ToString().c_str());
  }

  if (CBigNum(hashProofOfStake) > bnCoinDayWeight * bnTargetPerCoinDay)
  {
    return false;
  }

  if (fDebug && !fPrintProofOfStake)
  {
    printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
           nStakeModifier, nStakeModifierHeight,
           DateTimeStrFormat(nStakeModifierTime).c_str(),
           mapBlockIndex[hashBlockFrom]->nHeight,
           DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
    printf("CheckStakeKernelHash() : pass modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
           nStakeModifier,
           nTimeBlockFrom, nTxPrevOffset, txPrev.nTime, prevout.n, nTimeTx,
           hashProofOfStake.ToString().c_str());
  }

  return true;
}
# 386 "kernel.cpp"
static bool CheckStakeKernelHashV2(CBlockIndex* pindexPrev, unsigned int nBits, unsigned int nTimeBlockFrom, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, uint256& targetProofOfStake, bool fPrintProofOfStake)
{
  if (nTimeTx < txPrev.nTime)
  {
    return error("CheckStakeKernelHash() : nTime violation");
  }

  CBigNum bnTarget;
  bnTarget.SetCompact(nBits);
  int64_t nValueIn = txPrev.vout[prevout.n].nValue;
  CBigNum bnWeight = CBigNum(nValueIn);
  bnTarget *= bnWeight;
  targetProofOfStake = bnTarget.getuint256();
  uint64_t nStakeModifier = pindexPrev->nStakeModifier;
  int nStakeModifierHeight = pindexPrev->nHeight;
  int64_t nStakeModifierTime = pindexPrev->nTime;
  CDataStream ss(SER_GETHASH, 0);
  ss << nStakeModifier << nTimeBlockFrom << txPrev.nTime << prevout.hash << prevout.n << nTimeTx;
  hashProofOfStake = Hash(ss.begin(), ss.end());

  if (fPrintProofOfStake)
  {
    printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from timestamp=%s\n",
           nStakeModifier, nStakeModifierHeight,
           DateTimeStrFormat(nStakeModifierTime).c_str(),
           DateTimeStrFormat(nTimeBlockFrom).c_str());
    printf("CheckStakeKernelHash() : check modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
           nStakeModifier,
           nTimeBlockFrom, txPrev.nTime, prevout.n, nTimeTx,
           hashProofOfStake.ToString().c_str());
  }

  if (CBigNum(hashProofOfStake) > bnTarget)
  {
    return false;
  }

  if (fDebug && !fPrintProofOfStake)
  {
    printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from timestamp=%s\n",
           nStakeModifier, nStakeModifierHeight,
           DateTimeStrFormat(nStakeModifierTime).c_str(),
           DateTimeStrFormat(nTimeBlockFrom).c_str());
    printf("CheckStakeKernelHash() : pass modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
           nStakeModifier,
           nTimeBlockFrom, txPrev.nTime, prevout.n, nTimeTx,
           hashProofOfStake.ToString().c_str());
  }

  return true;
}
bool CheckStakeKernelHash(CBlockIndex* pindexPrev, unsigned int nBits, const CBlock& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, uint256& targetProofOfStake, bool fPrintProofOfStake)
{
  if (V3(nBestHeight) || IsProtocolV2(pindexPrev->nHeight+1))
  {
    return CheckStakeKernelHashV2(pindexPrev, nBits, blockFrom.GetBlockTime(), txPrev, prevout, nTimeTx, hashProofOfStake, targetProofOfStake, fPrintProofOfStake);
  }
  else
  {
    return CheckStakeKernelHashV1(nBits, blockFrom, nTxPrevOffset, txPrev, prevout, nTimeTx, hashProofOfStake, targetProofOfStake, fPrintProofOfStake);
  }
}
bool CheckKernel(CBlockIndex* pindexPrev, unsigned int nBits, int64_t nTime, const COutPoint& prevout, int64_t* pBlockTime)
{
  uint256 hashProofOfStake;
  uint256 targetProofOfStake;
  CTxDB txdb("r");
  CTransaction txPrev;
  CTxIndex txindex;

  if(!txPrev.ReadFromDisk(txdb, prevout, txindex))
  {
    return false;
  }

  CBlock block;

  if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
  {
    return fDebug? error("CheckProofOfStake() : read block failed") : false;
  }

  if(V3(nBestHeight))
  {
    int base;

    if(!minBase(txindex, pindexPrev, ConfigurationState::nStakeMinConfirmations - 1, base))
    {
      return false;
    }
  }
  else
  {
    unsigned int nTimeBlockFrom = block.GetBlockTime();

    if (nTimeBlockFrom + nStakeMinAge > nTime)
    {
      return true;
    }
  }

  if (!CheckStakeKernelHash(pindexPrev, nBits, block, txindex.pos.nTxPos - txindex.pos.nBlockPos, txPrev, prevout, nTime, hashProofOfStake, targetProofOfStake, fDebug))
  {
    return false;
  }

  return true;
}
bool CheckProofOfStake(CBlockIndex* pindexPrev, const CTransaction& tx, unsigned int nBits, uint256& hashProofOfStake, uint256& targetProofOfStake)
{
  if(!tx.IsCoinStake())
  {
    return error("CheckProofOfStake() : called on non-coinstake %s", tx.GetHash().ToString().c_str());
  }

  const CTxIn& txin = tx.vin[0];
  CTxDB txdb("r");
  CTransaction txPrev;
  CTxIndex txindex;

  if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
  {
    return tx.DoS(1, error("CheckProofOfStake() : INFO: read txPrev failed"));
  }

  if (!VerifySignature(txPrev, tx, 0, SCRIPT_VERIFY_NONE, 0))
  {
    return tx.DoS(100, error("CheckProofOfStake() : VerifySignature failed on coinstake %s", tx.GetHash().ToString().c_str()));
  }

  CBlock block;

  if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
  {
    return fDebug? error("CheckProofOfStake() : read block failed") : false;
  }

  if(V3(nBestHeight))
  {
    int base;

    if(!minBase(txindex, pindexPrev, ConfigurationState::nStakeMinConfirmations - 1, base))
    {
      return tx.DoS(100, error("CheckProofOfStake() : insufficient depth %d", base + 1));
    }
  }
  else
  {
    unsigned int nTimeBlockFrom = block.GetBlockTime();

    if (nTimeBlockFrom + nStakeMinAge > tx.nTime)
    {
      return error("CheckProofOfStake() : min age violation");
    }
  }

  if (!CheckStakeKernelHash(pindexPrev, nBits, block, txindex.pos.nTxPos - txindex.pos.nBlockPos, txPrev, txin.prevout, tx.nTime, hashProofOfStake, targetProofOfStake, fDebug))
  {
    return tx.DoS(1, error("CheckProofOfStake() : INFO: check kernel failed on coinstake %s, hashProof=%s", tx.GetHash().ToString().c_str(), hashProofOfStake.ToString().c_str()));
  }

  return true;
}
bool CheckCoinStakeTimestamp(int nHeight, int64_t nTimeBlock, int64_t nTimeTx)
{
  if (IsProtocolV2(nHeight))
  {
    return (nTimeBlock == nTimeTx) && ((nTimeTx & STAKE_TIMESTAMP_MASK) == 0);
  }
  else
  {
    return (nTimeBlock == nTimeTx);
  }
}
unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex)
{
  assert (pindex->pprev || pindex->GetBlockHash() == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
  CDataStream ss(SER_GETHASH, 0);

  if (pindex->pprev)
  {
    ss << pindex->pprev->nStakeModifierChecksum;
  }

  ss << pindex->nFlags << (pindex->IsProofOfStake() ? pindex->hashProof : 0) << pindex->nStakeModifier;
  uint256 hashChecksum = Hash(ss.begin(), ss.end());
  hashChecksum >>= (256 - 32);
  return hashChecksum.Get64();
}
bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum)
{
  MapModifierCheckpoints& checkpoints = (fTestNet ? mapStakeModifierCheckpointsTestNet : mapStakeModifierCheckpoints);

  if (checkpoints.count(nHeight))
  {
    return nStakeModifierChecksum == checkpoints[nHeight];
  }

  return true;
}
