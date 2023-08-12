
#include "api_transaction.h"
#include "txdb-leveldb.h"

extern CCriticalSection cs_main;
int CMerkleTx::SetMerkleBranch(const CBlock* pblock)
{
  AssertLockHeld(cs_main);
  CBlock blockTmp;

  if (pblock == NULL)
  {
    CTxIndex txindex;

    if (!CTxDB("r").ReadTxIndex(GetHash(), txindex))
    {
      return 0;
    }

    if (!blockTmp.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos))
    {
      return 0;
    }

    pblock = &blockTmp;
  }

  hashBlock_ = pblock->GetHash();

  for (nIndex = 0; nIndex < (int)pblock->vtx.size(); nIndex++)
    if (pblock->vtx[nIndex] == *(CTransaction*)this)
    {
      break;
    }

  if (nIndex == (int)pblock->vtx.size())
  {
    vTxMerkleBranch.clear();
    nIndex = -1;
    printf("ERROR: SetMerkleBranch() : couldn't find tx in block\n");
    return 0;
  }

  vTxMerkleBranch = pblock->GetMerkleBranch(nIndex);
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock_);

  if (mi == mapBlockIndex.end())
  {
    return 0;
  }

  CBlockIndex* pindex = (*mi).second;

  if (!pindex || !pindex->IsInMainChain())
  {
    return 0;
  }

  return pindexBest->nHeight - pindex->nHeight + 1;
}
int CMerkleTx::GetDepthInMainChainINTERNAL(CBlockIndex* &pindexRet) const
{
  if (hashBlock_ == 0 || nIndex == -1)
  {
    return 0;
  }

  AssertLockHeld(cs_main);
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock_);

  if (mi == mapBlockIndex.end())
  {
    return 0;
  }

  CBlockIndex* pindex = (*mi).second;

  if (!pindex || !pindex->IsInMainChain())
  {
    return 0;
  }

  if (!fMerkleVerified)
  {
    if (CBlock::CheckMerkleBranch(GetHash(), vTxMerkleBranch, nIndex) != pindex->hashMerkleRoot)
    {
      return 0;
    }

    fMerkleVerified = true;
  }

  pindexRet = pindex;
  return pindexBest->nHeight - pindex->nHeight + 1;
}
int CMerkleTx::GetDepthInMainChain(CBlockIndex* &pindexRet) const
{
  AssertLockHeld(cs_main);
  int nResult = GetDepthInMainChainINTERNAL(pindexRet);

  if (nResult == 0 && !mempool.exists(GetHash()))
  {
    return -1;
  }

  return nResult;
}
int CMerkleTx::GetBlocksToMaturity() const
{
  if (!(IsCoinBase() || IsCoinStake()))
  {
    return 0;
  }

  return max(0, (nCoinbaseMaturity+10) - GetDepthInMainChain());
}
int CMerkleTx::GetDepthInMainChain(int& nHeightRet) const
{
  if (hashBlock_ == 0 || nIndex == -1)
  {
    return 0;
  }

  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock_);

  if (mi == mapBlockIndex.end())
  {
    return 0;
  }

  CBlockIndex* pindex = (*mi).second;

  if (!pindex || !pindex->IsInMainChain())
  {
    return 0;
  }

  if (!fMerkleVerified)
  {
    if (CBlock::CheckMerkleBranch(GetHash(), vTxMerkleBranch, nIndex) != pindex->hashMerkleRoot)
    {
      return 0;
    }

    fMerkleVerified = true;
  }

  nHeightRet = pindex->nHeight;
  return pindexBest->nHeight - pindex->nHeight + 1;
}
