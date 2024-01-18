
#include "Block.h"
#include "Kernel.h"
#include "wallet/DB.h"
#include "CheckPoints.h"
#include "wallet/Wallet.h"
#include "TxMemPool.h"

#include "UIInterface.h"
#include <boost/algorithm/string/replace.hpp>

extern int STAKE_INTEREST_V3;
extern inline bool V4(int nHeight);
extern int GetPowHeight(const CBlockIndex* pindex);
extern inline int64_t FutureDrift(int64_t nTime, int nHeight);
extern void InvalidChainFound(CBlockIndex* pindexNew);
extern bool Reorganize(CTxDB& txdb, CBlockIndex* pindexNew);
extern void SetBestChain(const CBlockLocator& loc);
extern enum Checkpoints::CPMode CheckpointsMode;
extern LocatorNodeDB* ln1Db;
extern CBlockIndex* pblockindexFBBHLast;
extern CCriticalSection cs_main;
int CONSISTENCY_MARGIN = 100;
CBlockIndex* p__ = NULL;
static boost::filesystem::path BlockFilePath(unsigned int nFile)
{
  string strBlockFn = strprintf("blk%04u.dat", nFile);
  return GetDataDir() / strBlockFn;
}
void UpdatedTransaction(const uint256& hashTx)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)
  pwallet->UpdatedTransaction(hashTx);
}
//test validate state root on blk accept
bool forwardValidate(dev::h256 stateRoot)
{
  dvmc_message create_msg{};
  create_msg.kind = DVMC_CREATE;
  create_msg.recipient = create_address__;
  create_msg.track = std::numeric_limits<int64_t>::max();
  dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
  dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
  const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());

  if (create_result.status_code != DVMC_SUCCESS)
  {
    std::cout << "verification failed: " << create_result.status_code << "\n";
  }

  auto& created_account = host.accounts[create_address__];
  created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);
  msg.recipient = create_address__;
  dvmc::bytes exec_code = created_account.code;
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

  if (nSigOps > ConfigurationState::MAX_BLOCK_SIGOPS)
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

  if (IsProofOfWork() && nPowHeight > ConfigurationState::LAST_POW_BLOCK)
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
bool CBlock::ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions)
{
  if (!fReadTransactions)
  {
    *this = pindex->GetBlockHeader();
    return true;
  }

  if (!ReadFromDisk(pindex->nFile, pindex->nBlockPos, fReadTransactions))
  {
    return false;
  }

  if (GetHash() != pindex->GetBlockHash())
  {
    return error("CBlock::ReadFromDisk() : GetHash() doesn't match index");
  }

  return true;
}
void CBlock::UpdateTime(const CBlockIndex* pindexPrev)
{
  nTime = max(GetBlockTime(), GetAdjustedTime());
}
bool CBlock::DisconnectBlock(CTxDB& txdb, CBlockIndex* pindex)
{
  for (int i = vtx.size()-1; i >= 0; i--)
    if (!vtx[i].DisconnectInputs(txdb))
    {
      return false;
    }

  if (pindex->pprev)
  {
    CDiskBlockIndex blockindexPrev(pindex->pprev);
    blockindexPrev.hashNext = 0;

    if (!txdb.WriteBlockIndex(blockindexPrev))
    {
      return error("DisconnectBlock() : WriteBlockIndex failed");
    }
  }

  BOOST_FOREACH(CTransaction& tx, vtx)
  SyncWithWallets(tx, this, false, false);
  return true;
}
bool CBlock::ConnectBlock(CTxDB& txdb, CBlockIndex* pindex, bool fJustCheck)
{
  if (!CheckBlock(!fJustCheck, !fJustCheck, false))
  {
    return false;
  }

  unsigned int flags = SCRIPT_VERIFY_NOCACHE;

  if(V3(pindex->nHeight))
  {
    flags |= SCRIPT_VERIFY_NULLDUMMY |
             SCRIPT_VERIFY_STRICTENC |
             SCRIPT_VERIFY_ALLOW_EMPTY_SIG |
             SCRIPT_VERIFY_FIX_HASHTYPE |
             SCRIPT_VERIFY_CHECKLOCKTIMEVERIFY;
  }

  unsigned int nTxPos;

  if (fJustCheck)
  {
    nTxPos = 1;
  }
  else
  {
    nTxPos = pindex->nBlockPos + ::GetSerializeSize(CBlock(), SER_DISK, CLIENT_VERSION) - (2 * GetSizeOfCompactSize(0)) + GetSizeOfCompactSize(vtx.size());
  }

  map<uint256, CTxIndex> mapQueuedChanges;
  int64_t nFees = 0;
  int64_t nValueIn = 0;
  int64_t nValueOut = 0;
  int64_t nStakeReward = 0;
  unsigned int nSigOps = 0;
  BOOST_FOREACH(CTransaction& tx, vtx)
  {
    uint256 hashTx = tx.GetHash();
    CTxIndex txindexOld;

    if (txdb.ReadTxIndex(hashTx, txindexOld))
    {
      BOOST_FOREACH(CDiskTxPos &pos, txindexOld.vSpent)

      if (pos.IsNull())
      {
        return false;
      }
    }

    nSigOps += tx.GetLegacySigOpCount();

    if (nSigOps > ConfigurationState::MAX_BLOCK_SIGOPS)
    {
      return DoS(100, error("ConnectBlock() : too many sigops"));
    }

    CDiskTxPos posThisTx(pindex->nFile, pindex->nBlockPos, nTxPos);

    if (!fJustCheck)
    {
      nTxPos += ::GetSerializeSize(tx, SER_DISK, CLIENT_VERSION);
    }

    MapPrevTx mapInputs;

    if (tx.IsCoinBase())
    {
      nValueOut += tx.GetValueOut();
    }
    else
    {
      bool fInvalid;

      if (!tx.FetchInputs(txdb, mapQueuedChanges, true, false, mapInputs, fInvalid))
      {
        return false;
      }

      nSigOps += tx.GetP2SHSigOpCount(mapInputs);

      if (nSigOps > ConfigurationState::MAX_BLOCK_SIGOPS)
      {
        return DoS(100, error("ConnectBlock() : too many sigops"));
      }

      int64_t nTxValueIn = tx.GetValueIn(mapInputs);
      int64_t nTxValueOut = tx.GetValueOut();

      if (tx.IsCoinStake() and pindex->nHeight<=STAKE_INTEREST_V3)
      {
        double nNetworkDriftBuffer = nTxValueOut*.02;
        nTxValueOut = nTxValueOut - nNetworkDriftBuffer;
        nStakeReward = nTxValueOut - nTxValueIn;
      }

      nValueIn += nTxValueIn;
      nValueOut += nTxValueOut;

      if (!tx.IsCoinStake())
      {
        nFees += nTxValueIn - nTxValueOut;
      }

      if (tx.IsCoinStake())
      {
        nStakeReward = nTxValueOut - nTxValueIn;
      }

      bool pre = tx.ConnectInputs(txdb, mapInputs, mapQueuedChanges, posThisTx, pindex, true, false, flags);

      if(!pre && tx.nVersion == CTransaction::DION_TX_VERSION)
      {
        return DoS(100, error("pre count"));
      }
      else if(!pre)
      {
        return false;
      }
    }

    mapQueuedChanges[hashTx] = CTxIndex(posThisTx, tx.vout.size());
  }

  if (IsProofOfWork())
  {
    int64_t nReward = GetProofOfWorkReward(GetPowHeight(pindex), nFees);

    if (vtx[0].GetValueOut() > nReward)
      return DoS(50, error("ConnectBlock() : coinbase reward exceeded (actual=%" PRId64 " vs calculated=%" PRId64 ")",
                           vtx[0].GetValueOut(),
                           nReward));
  }

  if (IsProofOfStake())
  {
    uint64_t nCoinAge;

    if (!vtx[1].GetCoinAge(txdb, pindex->pprev, nCoinAge))
    {
      return error("ConnectBlock() : %s unable to get coin age for coinstake", vtx[1].GetHash().ToString().substr(0,10).c_str());
    }

    int64_t nCalculatedStakeReward = GetProofOfStakeReward(nCoinAge, nFees, pindex->nHeight);

    if (nStakeReward > nCalculatedStakeReward)
    {
      return DoS(100, error("ConnectBlock() : coinstake pays too much(actual=%" PRId64 " vs calculated=%" PRId64 ")", nStakeReward, nCalculatedStakeReward));
    }
  }

  pindex->nMint = nValueOut - nValueIn + nFees;
  pindex->nMoneySupply = (pindex->pprev? pindex->pprev->nMoneySupply : 0) + nValueOut - nValueIn;

  if (!txdb.WriteBlockIndex(CDiskBlockIndex(pindex)))
  {
    return error("Connect() : WriteBlockIndex for pindex failed");
  }

  if (fJustCheck)
  {
    return true;
  }

  for (map<uint256, CTxIndex>::iterator mi = mapQueuedChanges.begin(); mi != mapQueuedChanges.end(); ++mi)
  {
    if (!txdb.UpdateTxIndex((*mi).first, (*mi).second))
    {
      return error("ConnectBlock() : UpdateTxIndex failed");
    }
  }

  if (pindex->pprev)
  {
    CDiskBlockIndex blockindexPrev(pindex->pprev);
    blockindexPrev.hashNext = pindex->GetBlockHash();

    if (!txdb.WriteBlockIndex(blockindexPrev))
    {
      return error("ConnectBlock() : WriteBlockIndex failed");
    }
  }

  BOOST_FOREACH(CTransaction& tx, vtx)
  SyncWithWallets(tx, this, true);
  return true;
}
bool CBlock::SetBestChainInner(CTxDB& txdb, CBlockIndex *pindexNew)
{
  uint256 hash = GetHash();

  if (!ConnectBlock(txdb, pindexNew) || !txdb.WriteHashBestChain(hash))
  {
    txdb.TxnAbort();
    InvalidChainFound(pindexNew);
    return false;
  }

  if (!txdb.TxnCommit())
  {
    return error("SetBestChain() : TxnCommit failed");
  }

  pindexNew->pprev->pnext = pindexNew;
  BOOST_FOREACH(CTransaction& tx, vtx)
  mempool.remove(tx);
  std::cout << "ln1DbLOCAL" << std::endl;
  LocatorNodeDB ln1DbLOCAL("r+");
  ln1DbLOCAL.filter(pindexNew);
  return true;
}
bool CBlock::SetBestChain(CTxDB& txdb, CBlockIndex* pindexNew)
{
  uint256 hash = GetHash();

  if (!txdb.TxnBegin())
  {
    return error("SetBestChain() : TxnBegin failed");
  }

  if (pindexGenesisBlock == NULL && hash == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet))
  {
    txdb.WriteHashBestChain(hash);

    if (!txdb.TxnCommit())
    {
      return error("SetBestChain() : TxnCommit failed");
    }

    pindexGenesisBlock = pindexNew;
    p__ = pindexNew;
  }
  else if (hashPrevBlock == hashBestChain)
  {
    if (!SetBestChainInner(txdb, pindexNew))
    {
      return error("SetBestChain() : SetBestChainInner failed");
    }
  }
  else
  {
    CBlockIndex *pindexIntermediate = pindexNew;
    std::vector<CBlockIndex*> vpindexSecondary;

    while (pindexIntermediate->pprev && pindexIntermediate->pprev->nChainTrust > pindexBest->nChainTrust)
    {
      vpindexSecondary.push_back(pindexIntermediate);
      pindexIntermediate = pindexIntermediate->pprev;
    }

    if (!vpindexSecondary.empty())
    {
      printf("Postponing %" PRIszu " reconnects\n", vpindexSecondary.size());
    }

    if (!Reorganize(txdb, pindexIntermediate))
    {
      txdb.TxnAbort();
      InvalidChainFound(pindexNew);
      return error("SetBestChain() : Reorganize failed");
    }

    BOOST_REVERSE_FOREACH(CBlockIndex *pindex, vpindexSecondary)
    {
      CBlock block;

      if (!block.ReadFromDisk(pindex))
      {
        printf("SetBestChain() : ReadFromDisk failed\n");
        break;
      }

      if (!txdb.TxnBegin())
      {
        printf("SetBestChain() : TxnBegin 2 failed\n");
        break;
      }

      if (!block.SetBestChainInner(txdb, pindex))
      {
        break;
      }
    }
  }

  bool fIsInitialDownload = IsInitialBlockDownload();

  if (!fIsInitialDownload)
  {
    const CBlockLocator locator(pindexNew);
    ::SetBestChain(locator);
  }

  hashBestChain = hash;
  pindexBest = pindexNew;
  pblockindexFBBHLast = NULL;
  nBestHeight = pindexBest->nHeight;
  nBestChainTrust = pindexNew->nChainTrust;
  nTimeBestReceived = GetTime();
  CTxMemPool::nTransactionsUpdated++;
  uint256 nBestBlockTrust = pindexBest->nHeight != 0 ? (pindexBest->nChainTrust - pindexBest->pprev->nChainTrust) : pindexBest->nChainTrust;
  printf("SetBestChain: new best=%s  height=%d  trust=%s  blocktrust=%" PRId64 "  date=%s\n",
         hashBestChain.ToString().substr(0,20).c_str(), nBestHeight,
         CBigNum(nBestChainTrust).ToString().c_str(),
         nBestBlockTrust.Get64(),
         DateTimeStrFormat("%x %H:%M:%S", pindexBest->GetBlockTime()).c_str());

  if (!fIsInitialDownload)
  {
    int nUpgraded = 0;
    const CBlockIndex* pindex = pindexBest;

    for (int i = 0; i < CONSISTENCY_MARGIN && pindex != NULL; i++)
    {
      if (pindex->nVersion > CBlock::CURRENT_VERSION)
      {
        ++nUpgraded;
      }

      pindex = pindex->pprev;
    }

    if (nUpgraded > 0)
    {
      printf("SetBestChain: %d of last 100 blocks above version %d\n", nUpgraded, CBlock::CURRENT_VERSION);
    }

    if (nUpgraded > 100/2)
    {
      strMiscWarning = _("Warning: This version is obsolete, upgrade required!");
    }
  }

  std::string strCmd = GetArg("-blocknotify", "");

  if (!fIsInitialDownload && !strCmd.empty())
  {
    boost::replace_all(strCmd, "%s", hashBestChain.GetHex());
    boost::thread t(runCommand, strCmd);
  }

  return true;
}
bool CBlock::AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos, const uint256& hashProof)
{
  uint256 hash = GetHash();

  if (mapBlockIndex.count(hash))
  {
    return error("AddToBlockIndex() : %s already exists", hash.ToString().substr(0,20).c_str());
  }

  CBlockIndex* pindexNew = new CBlockIndex(nFile, nBlockPos, *this);

  if (!pindexNew)
  {
    return error("AddToBlockIndex() : new CBlockIndex failed");
  }

  pindexNew->phashBlock = &hash;
  map<uint256, CBlockIndex*>::iterator miPrev = mapBlockIndex.find(hashPrevBlock);

  if (miPrev != mapBlockIndex.end())
  {
    pindexNew->pprev = (*miPrev).second;
    pindexNew->nHeight = pindexNew->pprev->nHeight + 1;
    forwardValidate(this->stateRoot);
    pindexNew->stateRootIndex = pindexNew->pprev->stateRootIndex;
  }

  pindexNew->nChainTrust = (pindexNew->pprev ? pindexNew->pprev->nChainTrust : 0) + pindexNew->GetBlockTrust();

  if (!pindexNew->SetStakeEntropyBit(GetStakeEntropyBit()))
  {
    return error("AddToBlockIndex() : SetStakeEntropyBit() failed");
  }

  pindexNew->hashProof = hashProof;
  uint64_t nStakeModifier = 0;
  bool fGeneratedStakeModifier = false;

  if (!ComputeNextStakeModifier(pindexNew->pprev, nStakeModifier, fGeneratedStakeModifier))
  {
    return error("AddToBlockIndex() : ComputeNextStakeModifier() failed");
  }

  pindexNew->SetStakeModifier(nStakeModifier, fGeneratedStakeModifier);
  pindexNew->nStakeModifierChecksum = GetStakeModifierChecksum(pindexNew);

  if (!CheckStakeModifierCheckpoints(pindexNew->nHeight, pindexNew->nStakeModifierChecksum))
  {
    return error("AddToBlockIndex() : Rejected by stake modifier checkpoint height=%d, modifier=0x%016" PRIx64, pindexNew->nHeight, nStakeModifier);
  }

  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;

  if (pindexNew->IsProofOfStake())
  {
    setStakeSeen.insert(make_pair(pindexNew->prevoutStake, pindexNew->nStakeTime));
  }

  pindexNew->phashBlock = &((*mi).first);
  CTxDB txdb;

  if (!txdb.TxnBegin())
  {
    return false;
  }

  dev::h256 zero;

  if(this->stateRoot != zero)
  {
    pindexNew->stateRootIndex = this->stateRoot;
  }
  else
  {
    if(pindexNew->pprev != 0)
    {
      pindexNew->stateRootIndex = pindexNew->pprev->stateRootIndex;
    }
  }

  txdb.WriteBlockIndex(CDiskBlockIndex(pindexNew));

  if (!txdb.TxnCommit())
  {
    return false;
  }

  LOCK(cs_main);

  if (pindexNew->nChainTrust > nBestChainTrust)
    if (!SetBestChain(txdb, pindexNew))
    {
      return false;
    }

  if (pindexNew == pindexBest)
  {
    static uint256 hashPrevBestCoinBase;
    UpdatedTransaction(hashPrevBestCoinBase);
    hashPrevBestCoinBase = vtx[0].GetHash();
  }

  uiInterface.NotifyBlocksChanged();
  return true;
}
bool CBlock::SignBlock(__wx__& wallet, int64_t nFees)
{
  if (!vtx[0].vout[0].IsEmpty())
  {
    return false;
  }

  if (IsProofOfStake())
  {
    return true;
  }

  static int64_t nLastCoinStakeSearchTime = GetAdjustedTime();
  CKey key;
  CTransaction txCoinStake;

  if (IsProtocolV2(nBestHeight+1))
  {
    txCoinStake.nTime &= ~STAKE_TIMESTAMP_MASK;
  }

  int64_t nSearchTime = txCoinStake.nTime;

  if (nSearchTime > nLastCoinStakeSearchTime)
  {
    int64_t nSearchInterval = IsProtocolV2(nBestHeight+1) ? 1 : nSearchTime - nLastCoinStakeSearchTime;

    if (wallet.CreateCoinStake(wallet, nBits, nSearchInterval, nFees, txCoinStake, key, pindexBest->nHeight+1))
    {
      if (txCoinStake.nTime >= max(pindexBest->GetPastTimeLimit()+1, PastDrift(pindexBest->GetBlockTime(), pindexBest->nHeight+1)))
      {
        vtx[0].nTime = nTime = txCoinStake.nTime;
        nTime = max(pindexBest->GetPastTimeLimit()+1, GetMaxTransactionTime());
        nTime = max(GetBlockTime(), PastDrift(pindexBest->GetBlockTime(), pindexBest->nHeight+1));

        for (vector<CTransaction>::iterator it = vtx.begin(); it != vtx.end();)
          if (it->nTime > nTime)
          {
            it = vtx.erase(it);
          }
          else
          {
            ++it;
          }

        vtx.insert(vtx.begin() + 1, txCoinStake);
        hashMerkleRoot = BuildMerkleTree();
        printf("CBlock::SignBlock GetHash().GetHex() %s\n", GetHash().GetHex().c_str());
        printf("CBlock::SignBlock vchBlockSig.size() %lu\n", vchBlockSig.size());
        return key.Sign(GetHash(), vchBlockSig);
      }
    }

    nLastCoinStakeSearchInterval = nSearchTime - nLastCoinStakeSearchTime;
    nLastCoinStakeSearchTime = nSearchTime;
  }

  return false;
}
bool CBlock::CheckBlockSignature() const
{
  if (IsProofOfWork())
  {
    return vchBlockSig.empty();
  }

  vector<valtype> vSolutions;
  txnouttype whichType;
  const CTxOut& txout = vtx[1].vout[1];

  if (!Solver(txout.scriptPubKey, whichType, vSolutions))
  {
    return false;
  }

  if (whichType == TX_PUBKEY)
  {
    valtype& vchPubKey = vSolutions[0];
    CKey key;

    if (!key.SetPubKey(vchPubKey))
    {
      return false;
    }

    if (vchBlockSig.empty())
    {
      return false;
    }

    return key.Verify(GetHash(), vchBlockSig);
  }

  return false;
}
