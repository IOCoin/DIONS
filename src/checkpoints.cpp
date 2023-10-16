
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "CheckPoints.h"
#include "BlockIndex.h"
#include "TxDB.h"
#include "ccoin/process.h"
#include "core/uint256.h"
static const int nCheckpointSpan = 10;
namespace Checkpoints
{
typedef std::map<int, uint256> MapCheckpoints;
# 28 "checkpoints.cpp"
static MapCheckpoints mapCheckpoints =
  boost::assign::map_list_of
  ( 0, hashGenesisBlock )
  ( 2000, uint256("0x0000000000630a62ab7f79eb3a7a18ea2ce52cb7fe9d0b42f8d96d299eebd0ed") )
  ( 4000, uint256("0x0000000000592856ce322caa56c59baae91d508131a1757e44f9b98cd59596aa") )
  ( 6000, uint256("0x31cef59ceff4d5208abbc1260e03daabfc88b5364fdba6bd5ea526bbf2c3a789") )
  ( 8000, uint256("0x00000000006d3b5b2de92d470eaa3a97a1453aa19d7c6502e444e2d3b67b5de4") )
  ( 10000, uint256("0x000000000026bd8477cb22d0bfed736a753f70db69f6d7ce0fb25d256db16ee9") )
  ( 200000, uint256("ed8c09902c98d140d5f6cac92e4cadd75667ab42d68a6341929be9b4562ed6c2") )
  ( 400000, uint256("e0734018c7bd135290e9c44ff138d3110456322d478f76d0cb8a6d1bae5b9192") )
  ( 800000, uint256("5c5b978e0d1a27210130b1fc172b760e0a6a205a525b01d04c4a0c0e55afb82f") )
  ( 1000000, uint256("a4e2dec54d19dd796ee8ede3c22ee5a52109e0b697c8f85913aa91642eeea62c") )
  ( 1200000, uint256("e4f107cd496ff700c0a887a6f449525175cff20acf4a9373628e1a1f647bfa9d") )
  ( 1600000, uint256("4fa856def879a0574e47891f93699d2f4c53b045a584a53d95b1fc1e1ff49162") )
  ( 1625000, uint256("2ded826175cd8dee6333f4ec198597bb6992f972e01ec1b710f9c6149b065a7f") )
  ( 1800000, uint256("805fdf08fa517c4b9ce0f366be5112cb3d190e478a75818dd317a7f75db29cc3") )
  ( 1860837, uint256("11ca928dce7fc4c5d467865a801df58e5db94ef9f96006bb58e94dcf86654372") )
  ( 1898337, uint256("d2fe23992ce9939af0d08e48f9f1efac5b4bf53b5ee130d83b6d3beac8401ae3") )
  ;
static MapCheckpoints mapCheckpointsTestnet =
  boost::assign::map_list_of
  ( 0, hashGenesisBlockTestNet )
  ;
bool CheckHardened(int nHeight, const uint256& hash)
{
  MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);
  MapCheckpoints::const_iterator i = checkpoints.find(nHeight);

  if (i == checkpoints.end())
  {
    return true;
  }

  return hash == i->second;
}
int GetTotalBlocksEstimate()
{
  MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);
  return checkpoints.rbegin()->first;
}
CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
{
  MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);
  BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
  {
    const uint256& hash = i.second;
    std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);

    if (t != mapBlockIndex.end())
    {
      return t->second;
    }
  }
  return NULL;
}
uint256 hashSyncCheckpoint = 0;
uint256 hashPendingCheckpoint = 0;
CSyncCheckpoint checkpointMessage;
CSyncCheckpoint checkpointMessagePending;
uint256 hashInvalidCheckpoint = 0;
CCriticalSection cs_hashSyncCheckpoint;
CBlockIndex* GetLastSyncCheckpoint()
{
  LOCK(cs_hashSyncCheckpoint);

  if (!mapBlockIndex.count(hashSyncCheckpoint))
  {
    error("GetSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
  }
  else
  {
    return mapBlockIndex[hashSyncCheckpoint];
  }

  return NULL;
}
bool ValidateSyncCheckpoint(uint256 hashCheckpoint)
{
  if (!mapBlockIndex.count(hashSyncCheckpoint))
  {
    return error("ValidateSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
  }

  if (!mapBlockIndex.count(hashCheckpoint))
  {
    return error("ValidateSyncCheckpoint: block index missing for received sync-checkpoint %s", hashCheckpoint.ToString().c_str());
  }

  CBlockIndex* pindexSyncCheckpoint = mapBlockIndex[hashSyncCheckpoint];
  CBlockIndex* pindexCheckpointRecv = mapBlockIndex[hashCheckpoint];

  if (pindexCheckpointRecv->nHeight <= pindexSyncCheckpoint->nHeight)
  {
    CBlockIndex* pindex = pindexSyncCheckpoint;

    while (pindex->nHeight > pindexCheckpointRecv->nHeight)
      if (!(pindex = pindex->pprev))
      {
        return error("ValidateSyncCheckpoint: pprev null - block index structure failure");
      }

    if (pindex->GetBlockHash() != hashCheckpoint)
    {
      hashInvalidCheckpoint = hashCheckpoint;
      return error("ValidateSyncCheckpoint: new sync-checkpoint %s is conflicting with current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
    }

    return false;
  }

  CBlockIndex* pindex = pindexCheckpointRecv;

  while (pindex->nHeight > pindexSyncCheckpoint->nHeight)
    if (!(pindex = pindex->pprev))
    {
      return error("ValidateSyncCheckpoint: pprev2 null - block index structure failure");
    }

  if (pindex->GetBlockHash() != hashSyncCheckpoint)
  {
    hashInvalidCheckpoint = hashCheckpoint;
    return error("ValidateSyncCheckpoint: new sync-checkpoint %s is not a descendant of current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
  }

  return true;
}
bool WriteSyncCheckpoint(const uint256& hashCheckpoint)
{
  CTxDB txdb;
  txdb.TxnBegin();

  if (!txdb.WriteSyncCheckpoint(hashCheckpoint))
  {
    txdb.TxnAbort();
    return error("WriteSyncCheckpoint(): failed to write to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
  }

  if (!txdb.TxnCommit())
  {
    return error("WriteSyncCheckpoint(): failed to commit to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
  }

  Checkpoints::hashSyncCheckpoint = hashCheckpoint;
  return true;
}
bool AcceptPendingSyncCheckpoint()
{
  LOCK(cs_hashSyncCheckpoint);

  if (hashPendingCheckpoint != 0 && mapBlockIndex.count(hashPendingCheckpoint))
  {
    if (!ValidateSyncCheckpoint(hashPendingCheckpoint))
    {
      hashPendingCheckpoint = 0;
      checkpointMessagePending.SetNull();
      return false;
    }

    CTxDB txdb;
    CBlockIndex* pindexCheckpoint = mapBlockIndex[hashPendingCheckpoint];

    if (!pindexCheckpoint->IsInMainChain())
    {
      CBlock block;

      if (!block.ReadFromDisk(pindexCheckpoint))
      {
        return error("AcceptPendingSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
      }

      if (!block.SetBestChain(txdb, pindexCheckpoint))
      {
        hashInvalidCheckpoint = hashPendingCheckpoint;
        return error("AcceptPendingSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
      }
    }

    if (!WriteSyncCheckpoint(hashPendingCheckpoint))
    {
      return error("AcceptPendingSyncCheckpoint(): failed to write sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
    }

    hashPendingCheckpoint = 0;
    checkpointMessage = checkpointMessagePending;
    checkpointMessagePending.SetNull();
    printf("AcceptPendingSyncCheckpoint : sync-checkpoint at %s\n", hashSyncCheckpoint.ToString().c_str());

    if (!checkpointMessage.IsNull())
    {
      BOOST_FOREACH(CNode* pnode, vNodes)
      checkpointMessage.RelayTo(pnode);
    }

    return true;
  }

  return false;
}
uint256 AutoSelectSyncCheckpoint()
{
  const CBlockIndex *pindex = pindexBest;

  while (pindex->pprev && (pindex->GetBlockTime() + nCheckpointSpan * GetTargetSpacing(nBestHeight) > pindexBest->GetBlockTime() || pindex->nHeight + nCheckpointSpan > pindexBest->nHeight))
  {
    pindex = pindex->pprev;
  }

  return pindex->GetBlockHash();
}
bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev)
{
  if (fTestNet)
  {
    return true;
  }

  int nHeight = pindexPrev->nHeight + 1;
  LOCK(cs_hashSyncCheckpoint);
  assert(mapBlockIndex.count(hashSyncCheckpoint));
  const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];

  if (nHeight > pindexSync->nHeight)
  {
    const CBlockIndex* pindex = pindexPrev;

    while (pindex->nHeight > pindexSync->nHeight)
      if (!(pindex = pindex->pprev))
      {
        return error("CheckSync: pprev null - block index structure failure");
      }

    if (pindex->nHeight < pindexSync->nHeight || pindex->GetBlockHash() != hashSyncCheckpoint)
    {
      return false;
    }
  }

  if (nHeight == pindexSync->nHeight && hashBlock != hashSyncCheckpoint)
  {
    return false;
  }

  if (nHeight < pindexSync->nHeight && !mapBlockIndex.count(hashBlock))
  {
    return false;
  }

  return true;
}
bool WantedByPendingSyncCheckpoint(uint256 hashBlock)
{
  LOCK(cs_hashSyncCheckpoint);

  if (hashPendingCheckpoint == 0)
  {
    return false;
  }

  if (hashBlock == hashPendingCheckpoint)
  {
    return true;
  }

  if (mapOrphanBlocks.count(hashPendingCheckpoint)
      && hashBlock == WantedByOrphan(mapOrphanBlocks[hashPendingCheckpoint]))
  {
    return true;
  }

  return false;
}
bool ResetSyncCheckpoint()
{
  LOCK(cs_hashSyncCheckpoint);
  const uint256& hash = mapCheckpoints.rbegin()->second;

  if (mapBlockIndex.count(hash) && !mapBlockIndex[hash]->IsInMainChain())
  {
    printf("ResetSyncCheckpoint: SetBestChain to hardened checkpoint %s\n", hash.ToString().c_str());
    CTxDB txdb;
    CBlock block;

    if (!block.ReadFromDisk(mapBlockIndex[hash]))
    {
      return error("ResetSyncCheckpoint: ReadFromDisk failed for hardened checkpoint %s", hash.ToString().c_str());
    }

    if (!block.SetBestChain(txdb, mapBlockIndex[hash]))
    {
      return error("ResetSyncCheckpoint: SetBestChain failed for hardened checkpoint %s", hash.ToString().c_str());
    }
  }
  else if(!mapBlockIndex.count(hash))
  {
    hashPendingCheckpoint = hash;
    checkpointMessagePending.SetNull();
    printf("ResetSyncCheckpoint: pending for sync-checkpoint %s\n", hashPendingCheckpoint.ToString().c_str());
  }

  BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
  {
    const uint256& hash = i.second;

    if (mapBlockIndex.count(hash) && mapBlockIndex[hash]->IsInMainChain())
    {
      if (!WriteSyncCheckpoint(hash))
      {
        return error("ResetSyncCheckpoint: failed to write sync checkpoint %s", hash.ToString().c_str());
      }

      printf("ResetSyncCheckpoint: sync-checkpoint reset to %s\n", hashSyncCheckpoint.ToString().c_str());
      return true;
    }
  }
  return false;
}
void AskForPendingSyncCheckpoint(CNode* pfrom)
{
  LOCK(cs_hashSyncCheckpoint);

  if (pfrom && hashPendingCheckpoint != 0 && (!mapBlockIndex.count(hashPendingCheckpoint)) && (!mapOrphanBlocks.count(hashPendingCheckpoint)))
  {
    pfrom->AskFor(CInv(MSG_BLOCK, hashPendingCheckpoint));
  }
}
bool SetCheckpointPrivKey(std::string strPrivKey)
{
  CSyncCheckpoint checkpoint;
  checkpoint.hashCheckpoint = !fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet;
  CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
  sMsg << (CUnsignedSyncCheckpoint)checkpoint;
  checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());
  std::vector<unsigned char> vchPrivKey = ParseHex(strPrivKey);
  CKey key;
  key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()));

  if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
  {
    return false;
  }

  CSyncCheckpoint::strMasterPrivKey = strPrivKey;
  return true;
}
bool SendSyncCheckpoint(uint256 hashCheckpoint)
{
  CSyncCheckpoint checkpoint;
  checkpoint.hashCheckpoint = hashCheckpoint;
  CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
  sMsg << (CUnsignedSyncCheckpoint)checkpoint;
  checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

  if (CSyncCheckpoint::strMasterPrivKey.empty())
  {
    return error("SendSyncCheckpoint: Checkpoint master key unavailable.");
  }

  std::vector<unsigned char> vchPrivKey = ParseHex(CSyncCheckpoint::strMasterPrivKey);
  CKey key;
  key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()));

  if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
  {
    return error("SendSyncCheckpoint: Unable to sign checkpoint, check private key?");
  }

  if(!checkpoint.ProcessSyncCheckpoint(NULL))
  {
    printf("WARNING: SendSyncCheckpoint: Failed to process checkpoint.\n");
    return false;
  }

  {
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    checkpoint.RelayTo(pnode);
  }

  return true;
}
bool IsMatureSyncCheckpoint()
{
  LOCK(cs_hashSyncCheckpoint);
  assert(mapBlockIndex.count(hashSyncCheckpoint));
  const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];
  return (nBestHeight >= pindexSync->nHeight + nCoinbaseMaturity ||
          pindexSync->GetBlockTime() + nStakeMinAge < GetAdjustedTime());
}
}
const std::string CSyncCheckpoint::strMasterPubKey = "0400df1d33140bd7ef27eaafb91ef1b7bf1162d1b71ee97025398725041d665c49f442ae5f8803f92a8d71d36f4144e9f325cf4250d065e95ac3735108189e0509";
std::string CSyncCheckpoint::strMasterPrivKey = "";
bool CSyncCheckpoint::CheckSignature()
{
  CKey key;

  if (!key.SetPubKey(ParseHex(CSyncCheckpoint::strMasterPubKey)))
  {
    return error("CSyncCheckpoint::CheckSignature() : SetPubKey failed");
  }

  if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
  {
    return error("CSyncCheckpoint::CheckSignature() : verify signature failed");
  }

  CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
  sMsg >> *(CUnsignedSyncCheckpoint*)this;
  return true;
}
bool CSyncCheckpoint::ProcessSyncCheckpoint(CNode* pfrom)
{
  if (!CheckSignature())
  {
    return false;
  }

  LOCK(Checkpoints::cs_hashSyncCheckpoint);

  if (!mapBlockIndex.count(hashCheckpoint))
  {
    Checkpoints::hashPendingCheckpoint = hashCheckpoint;
    Checkpoints::checkpointMessagePending = *this;
    printf("ProcessSyncCheckpoint: pending for sync-checkpoint %s\n", hashCheckpoint.ToString().c_str());

    if (pfrom)
    {
      pfrom->PushGetBlocks(pindexBest, hashCheckpoint);
      pfrom->AskFor(CInv(MSG_BLOCK, mapOrphanBlocks.count(hashCheckpoint)? WantedByOrphan(mapOrphanBlocks[hashCheckpoint]) : hashCheckpoint));
    }

    return false;
  }

  if (!Checkpoints::ValidateSyncCheckpoint(hashCheckpoint))
  {
    return false;
  }

  CTxDB txdb;
  CBlockIndex* pindexCheckpoint = mapBlockIndex[hashCheckpoint];

  if (!pindexCheckpoint->IsInMainChain())
  {
    CBlock block;

    if (!block.ReadFromDisk(pindexCheckpoint))
    {
      return error("ProcessSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
    }

    if (!block.SetBestChain(txdb, pindexCheckpoint))
    {
      Checkpoints::hashInvalidCheckpoint = hashCheckpoint;
      return error("ProcessSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
    }
  }

  if (!Checkpoints::WriteSyncCheckpoint(hashCheckpoint))
  {
    return error("ProcessSyncCheckpoint(): failed to write sync checkpoint %s", hashCheckpoint.ToString().c_str());
  }

  Checkpoints::checkpointMessage = *this;
  Checkpoints::hashPendingCheckpoint = 0;
  Checkpoints::checkpointMessagePending.SetNull();
  printf("ProcessSyncCheckpoint: sync-checkpoint at %s\n", hashCheckpoint.ToString().c_str());
  return true;
}
