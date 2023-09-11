#ifndef HASH_TRANSACTION_H
#define HASH_TRANSACTION_H

#include "ccoin/block.h"

class CMerkleTx : public CTransaction
{
private:
  int GetDepthInMainChainINTERNAL(CBlockIndex* &pindexRet) const;
public:
  uint256 hashBlock_;
  std::vector<uint256> vTxMerkleBranch;
  int nIndex;
  mutable bool fMerkleVerified;

  CMerkleTx()
  {
    Init();
  }

  CMerkleTx(const CTransaction& txIn) : CTransaction(txIn)
  {
    Init();
  }

  void Init()
  {
    hashBlock_ = 0;
    nIndex = -1;
    fMerkleVerified = false;
  }

  IMPLEMENT_SERIALIZE
  (
    nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
    nVersion = this->nVersion;
    READWRITE(hashBlock_);
    READWRITE(vTxMerkleBranch);
    READWRITE(nIndex);
  )

  int SetMerkleBranch(const CBlock* pblock=NULL);

  int GetDepthInMainChain(CBlockIndex* &pindexRet) const;
  int GetDepthInMainChain() const
  {
    CBlockIndex *pindexRet;
    return GetDepthInMainChain(pindexRet);
  }
  int GetDepthInMainChain(int& nHeightRet) const;
  bool OnMainChain() const
  {
    CBlockIndex *pindexRet;
    return GetDepthInMainChainINTERNAL(pindexRet) > 0;
  }
  int GetBlocksToMaturity() const;
  bool AcceptToMemoryPool();

  inline int
  GetHeightInMainChain() const
  {
    int nHeight;

    if (GetDepthInMainChain (nHeight) == 0)
    {
      return -1;
    }

    return nHeight;
  }
};

#endif
