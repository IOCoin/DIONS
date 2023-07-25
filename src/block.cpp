#include "block.h"
#include "kernel.h"
#include "db.h"
#include "checkpoints.h"

#include "ui_interface.h"

extern const unsigned int MAX_BLOCK_SIZE; 
extern const unsigned int MAX_BLOCK_SIZE_GEN;
extern const unsigned int MAX_BLOCK_SIGOPS;
extern const int LAST_POW_BLOCK ;
extern inline bool V4(int nHeight);
extern int GetPowHeight(const CBlockIndex* pindex);
extern inline int64_t FutureDrift(int64_t nTime, int nHeight);
extern enum Checkpoints::CPMode CheckpointsMode;
extern CCriticalSection cs_main;
static boost::filesystem::path BlockFilePath(unsigned int nFile)
{
  string strBlockFn = strprintf("blk%04u.dat", nFile);
  return GetDataDir() / strBlockFn;
}
FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode)
{
  if ((nFile < 1) || (nFile == (unsigned int) -1))
  {
    return NULL;
  }

  FILE* file = fopen(BlockFilePath(nFile).string().c_str(), pszMode);

  if (!file)
  {
    return NULL;
  }

  if (nBlockPos != 0 && !strchr(pszMode, 'a') && !strchr(pszMode, 'w'))
  {
    if (fseek(file, nBlockPos, SEEK_SET) != 0)
    {
      fclose(file);
      return NULL;
    }
  }

  return file;
}
bool CBlock::CheckBlock(bool fCheckPOW, bool fCheckMerkleRoot, bool fCheckSig) const
{
  if (vtx.empty() || vtx.size() > MAX_BLOCK_SIZE || ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
  {
    return DoS(100, error("CheckBlock() : size limits failed"));
  }

  if (fCheckPOW && IsProofOfWork() && !CheckProofOfWork(GetPoWHash(), nBits))
  {
    return DoS(50, error("CheckBlock() : proof of work failed"));
  }

  if (vtx.empty() || !vtx[0].IsCoinBase())
  {
    return DoS(100, error("CheckBlock() : first tx is not coinbase"));
  }

  for (unsigned int i = 1; i < vtx.size(); i++)
    if (vtx[i].IsCoinBase())
    {
      return DoS(100, error("CheckBlock() : more than one coinbase"));
    }

  if (IsProofOfStake())
  {
    if (vtx[0].vout.size() != 1 || !vtx[0].vout[0].IsEmpty())
    {
      return DoS(100, error("CheckBlock() : coinbase output not empty for proof-of-stake block"));
    }

    if (vtx.empty() || !vtx[1].IsCoinStake())
    {
      return DoS(100, error("CheckBlock() : second tx is not coinstake"));
    }

    for (unsigned int i = 2; i < vtx.size(); i++)
      if (vtx[i].IsCoinStake())
      {
        return DoS(100, error("CheckBlock() : more than one coinstake"));
      }

    if (fCheckSig && !CheckBlockSignature())
    {
      return DoS(100, error("CheckBlock() : bad proof-of-stake block signature"));
    }
  }

  BOOST_FOREACH(const CTransaction& tx, vtx)
  {
    if (!tx.CheckTransaction())
    {
      return DoS(tx.nDoS, error("CheckBlock() : CheckTransaction failed"));
    }

    if (GetBlockTime() < (int64_t)tx.nTime)
    {
      return DoS(50, error("CheckBlock() : block timestamp earlier than transaction timestamp"));
    }
  }
  set<uint256> uniqueTx;
  BOOST_FOREACH(const CTransaction& tx, vtx)
  {
    uniqueTx.insert(tx.GetHash());
  }

  if (uniqueTx.size() != vtx.size())
  {
    return DoS(100, error("CheckBlock() : duplicate transaction"));
  }

  unsigned int nSigOps = 0;
  BOOST_FOREACH(const CTransaction& tx, vtx)
  {
    nSigOps += tx.GetLegacySigOpCount();
  }

  if (nSigOps > MAX_BLOCK_SIGOPS)
  {
    return DoS(100, error("CheckBlock() : out-of-bounds SigOpCount"));
  }

  if (fCheckMerkleRoot && hashMerkleRoot != BuildMerkleTree())
  {
    return DoS(100, error("CheckBlock() : hashMerkleRoot mismatch"));
  }

  return true;
}
bool CBlock::AcceptBlock()
{
  AssertLockHeld(cs_main);

  if(!V4(nBestHeight))
  {
    if (nVersion > CURRENT_VERSION)
    {
      return DoS(100, error("AcceptBlock() : reject unknown block version %d", nVersion));
    }
  }

  uint256 hash = GetHash();

  if (mapBlockIndex.count(hash))
  {
    return error("AcceptBlock() : block already in mapBlockIndex");
  }

  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);

  if (mi == mapBlockIndex.end())
  {
    return DoS(10, error("AcceptBlock() : prev block not found"));
  }

  CBlockIndex* pindexPrev = (*mi).second;
  int nHeight = pindexPrev->nHeight+1;
  int nPowHeight = GetPowHeight(pindexPrev)+1;

  if (IsProtocolV2(nHeight) && nVersion < 7)
  {
    return DoS(100, error("AcceptBlock() : reject too old nVersion = %d", nVersion));
  }
  else if (!fTestNet && !IsProtocolV2(nHeight) && nVersion > 6)
  {
    return DoS(100, error("AcceptBlock() : reject too new nVersion = %d", nVersion));
  }

  if (IsProofOfWork() && nPowHeight > LAST_POW_BLOCK)
  {
    return DoS(100, error("AcceptBlock() : reject proof-of-work at height %d", nHeight));
  }

  if (GetBlockTime() > FutureDrift(GetAdjustedTime(), nHeight))
  {
    return error("AcceptBlock() : block timestamp too far in the future");
  }

  if (GetBlockTime() > FutureDrift((int64_t)vtx[0].nTime, nHeight))
  {
    return DoS(50, error("AcceptBlock() : coinbase timestamp is too early"));
  }

  if (IsProofOfStake() && !CheckCoinStakeTimestamp(nHeight, GetBlockTime(), (int64_t)vtx[1].nTime))
  {
    return DoS(50, error("AcceptBlock() : coinstake timestamp violation nTimeBlock=%" PRId64 " nTimeTx=%u", GetBlockTime(), vtx[1].nTime));
  }

  int64_t nFees = 0;
  CTxDB txdb("r");
  map<uint256, CTxIndex> mapQueuedChanges;
  BOOST_FOREACH(CTransaction& tx, vtx)
  {
    MapPrevTx mapInputs;

    if (!tx.IsCoinBase())
    {
      bool fInvalid;

      if (tx.FetchInputs(txdb, mapQueuedChanges, true, false, mapInputs, fInvalid))
      {
        int64_t nTxValueIn = tx.GetValueIn(mapInputs);
        int64_t nTxValueOut = tx.GetValueOut();

        if (!tx.IsCoinStake())
        {
          nFees += nTxValueIn - nTxValueOut;
        }
      }
    }
  }

  if (nBits != GetNextTargetRequired(pindexPrev, IsProofOfStake(), nFees))
  {
    return DoS(100, error("AcceptBlock() : incorrect %s", IsProofOfWork() ? "proof-of-work" : "proof-of-stake"));
  }

  if (GetBlockTime() <= pindexPrev->GetPastTimeLimit() || FutureDrift(GetBlockTime(), nHeight) < pindexPrev->GetBlockTime())
  {
    return error("AcceptBlock() : block's timestamp is too early");
  }

  BOOST_FOREACH(const CTransaction& tx, vtx)

  if (!IsFinalTx(tx, nHeight, GetBlockTime()))
  {
    return DoS(10, error("AcceptBlock() : contains a non-final transaction"));
  }

  if (!Checkpoints::CheckHardened(nHeight, hash))
  {
    return DoS(100, error("AcceptBlock() : rejected by hardened checkpoint lock-in at %d", nHeight));
  }

  uint256 hashProof;

  if (IsProofOfStake())
  {
    uint256 targetProofOfStake;

    if (!CheckProofOfStake(pindexPrev, vtx[1], nBits, hashProof, targetProofOfStake))
    {
      printf("WARNING: AcceptBlock(): check proof-of-stake failed for block %s\n", hash.ToString().c_str());
      return false;
    }
  }

  if (IsProofOfWork())
  {
    hashProof = GetPoWHash();
  }

  bool cpSatisfies = Checkpoints::CheckSync(hash, pindexPrev);

  if (CheckpointsMode == Checkpoints::STRICT && !cpSatisfies)
  {
    return error("AcceptBlock() : rejected by synchronized checkpoint");
  }

  if (CheckpointsMode == Checkpoints::ADVISORY && !cpSatisfies)
  {
    strMiscWarning = _("WARNING: syncronized checkpoint violation detected, but skipped!");
  }

  CScript expect = CScript() << nHeight;

  if (vtx[0].vin[0].scriptSig.size() < expect.size() ||
      !std::equal(expect.begin(), expect.end(), vtx[0].vin[0].scriptSig.begin()))
  {
    return DoS(100, error("AcceptBlock() : block height mismatch in coinbase"));
  }

  if (!CheckDiskSpace(::GetSerializeSize(*this, SER_DISK, CLIENT_VERSION)))
  {
    return error("AcceptBlock() : out of disk space");
  }

  unsigned int nFile = -1;
  unsigned int nBlockPos = 0;

  if (!WriteToDisk(nFile, nBlockPos))
  {
    return error("AcceptBlock() : WriteToDisk failed");
  }

  if (!AddToBlockIndex(nFile, nBlockPos, hashProof))
  {
    return error("add : failed");
  }

  int nBlockEstimate = Checkpoints::GetTotalBlocksEstimate();

  if (hashBestChain == hash)
  {
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)

    if (nBestHeight > (pnode->nStartingHeight != -1 ? pnode->nStartingHeight - 2000 : nBlockEstimate))
    {
      pnode->PushInventory(CInv(MSG_BLOCK, hash));
    }
  }

  Checkpoints::AcceptPendingSyncCheckpoint();
  return true;
}
