#pragma once

#include "ccoin/Block.h"

class CBlockIndex
{
public:
  const uint256* phashBlock;
  CBlockIndex* pprev;
  CBlockIndex* pnext;
  unsigned int nFile;
  unsigned int nBlockPos;
  uint256 nChainTrust;
  int nHeight;

  int64_t nMint;
  int64_t nMoneySupply;

  unsigned int nFlags;
  enum
  {
    BLOCK_PROOF_OF_STAKE = (1 << 0),
    BLOCK_STAKE_ENTROPY = (1 << 1),
    BLOCK_STAKE_MODIFIER = (1 << 2),
  };

  uint64_t nStakeModifier;
  unsigned int nStakeModifierChecksum;

  COutPoint prevoutStake;
  unsigned int nStakeTime;

  uint256 hashProof;


  dev::h256 stateRootIndex;


  int nVersion;
  uint256 hashMerkleRoot;
  unsigned int nTime;
  unsigned int nBits;
  unsigned int nNonce;

  CBlockIndex()
  {
    phashBlock = NULL;
    pprev = NULL;
    pnext = NULL;
    nFile = 0;
    nBlockPos = 0;
    nHeight = 0;
    nChainTrust = 0;
    nMint = 0;
    nMoneySupply = 0;
    nFlags = 0;
    nStakeModifier = 0;
    nStakeModifierChecksum = 0;
    hashProof = 0;
    prevoutStake.SetNull();
    nStakeTime = 0;
    nVersion = 0;
    hashMerkleRoot = 0;
    nTime = 0;
    nBits = 0;
    nNonce = 0;
  }

  CBlockIndex(unsigned int nFileIn, unsigned int nBlockPosIn, CBlock& block)
  {
    phashBlock = NULL;
    pprev = NULL;
    pnext = NULL;
    nFile = nFileIn;
    nBlockPos = nBlockPosIn;
    nHeight = 0;
    nChainTrust = 0;
    nMint = 0;
    nMoneySupply = 0;
    nFlags = 0;
    nStakeModifier = 0;
    nStakeModifierChecksum = 0;
    hashProof = 0;

    if (block.IsProofOfStake())
    {
      SetProofOfStake();
      prevoutStake = block.vtx[1].vin[0].prevout;
      nStakeTime = block.vtx[1].nTime;
    }
    else
    {
      prevoutStake.SetNull();
      nStakeTime = 0;
    }

    nVersion = block.nVersion;
    hashMerkleRoot = block.hashMerkleRoot;
    nTime = block.nTime;
    nBits = block.nBits;
    nNonce = block.nNonce;
  }

  CBlock GetBlockHeader() const
  {
    CBlock block;
    block.nVersion = nVersion;

    if (pprev)
    {
      block.hashPrevBlock = pprev->GetBlockHash();
    }

    block.hashMerkleRoot = hashMerkleRoot;
    block.nTime = nTime;
    block.nBits = nBits;
    block.nNonce = nNonce;
    return block;
  }

  uint256 GetBlockHash() const
  {
    return *phashBlock;
  }

  int64_t GetBlockTime() const
  {
    return (int64_t)nTime;
  }

  uint256 GetBlockTrust() const;

  bool IsInMainChain() const
  {
    return (pnext || this == pindexBest);
  }

  bool CheckIndex() const
  {
    return true;
  }

  int64_t GetPastTimeLimit() const
  {
    if (IsProtocolV2(nHeight))
    {
      return GetBlockTime();
    }
    else
    {
      return GetMedianTimePast();
    }
  }

  enum { nMedianTimeSpan=11 };

  int64_t GetMedianTimePast() const
  {
    int64_t pmedian[nMedianTimeSpan];
    int64_t* pbegin = &pmedian[nMedianTimeSpan];
    int64_t* pend = &pmedian[nMedianTimeSpan];
    const CBlockIndex* pindex = this;

    for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
    {
      *(--pbegin) = pindex->GetBlockTime();
    }

    std::sort(pbegin, pend);
    return pbegin[(pend - pbegin)/2];
  }

  static bool IsSuperMajority(int minVersion, const CBlockIndex* pstart,
                              unsigned int nRequired, unsigned int nToCheck);

  bool IsProofOfWork() const
  {
    return !(nFlags & BLOCK_PROOF_OF_STAKE);
  }

  bool IsProofOfStake() const
  {
    return (nFlags & BLOCK_PROOF_OF_STAKE);
  }

  void SetProofOfStake()
  {
    nFlags |= BLOCK_PROOF_OF_STAKE;
  }

  unsigned int GetStakeEntropyBit() const
  {
    return ((nFlags & BLOCK_STAKE_ENTROPY) >> 1);
  }

  bool SetStakeEntropyBit(unsigned int nEntropyBit)
  {
    if (nEntropyBit > 1)
    {
      return false;
    }

    nFlags |= (nEntropyBit? BLOCK_STAKE_ENTROPY : 0);
    return true;
  }

  bool GeneratedStakeModifier() const
  {
    return (nFlags & BLOCK_STAKE_MODIFIER);
  }

  void SetStakeModifier(uint64_t nModifier, bool fGeneratedStakeModifier)
  {
    nStakeModifier = nModifier;

    if (fGeneratedStakeModifier)
    {
      nFlags |= BLOCK_STAKE_MODIFIER;
    }
  }

  std::string ToString() const
  {
    return strprintf("CBlockIndex(nprev=%p, pnext=%p, nFile=%u, nBlockPos=%-6d nHeight=%d, nMint=%s, nMoneySupply=%s, nFlags=(%s)(%d)(%s), nStakeModifier=%016" PRIx64 ", nStakeModifierChecksum=%08x, hashProof=%s, prevoutStake=(%s), nStakeTime=%d merkle=%s, hashBlock=%s)",
                     pprev, pnext, nFile, nBlockPos, nHeight,
                     FormatMoney(nMint).c_str(), FormatMoney(nMoneySupply).c_str(),
                     GeneratedStakeModifier() ? "MOD" : "-", GetStakeEntropyBit(), IsProofOfStake()? "PoS" : "PoW",
                     nStakeModifier, nStakeModifierChecksum,
                     hashProof.ToString().c_str(),
                     prevoutStake.ToString().c_str(), nStakeTime,
                     hashMerkleRoot.ToString().c_str(),
                     GetBlockHash().ToString().c_str());
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};

class CDiskBlockIndex : public CBlockIndex
{
private:
  uint256 blockHash;

public:
  uint256 hashPrev;
  uint256 hashNext;

  CDiskBlockIndex()
  {
    hashPrev = 0;
    hashNext = 0;
    blockHash = 0;
  }

  explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex)
  {
    hashPrev = (pprev ? pprev->GetBlockHash() : 0);
    hashNext = (pnext ? pnext->GetBlockHash() : 0);
  }

  IMPLEMENT_SERIALIZE
  (

    if (!(nType & SER_GETHASH))
    READWRITE(nVersion);

    READWRITE(hashNext);
    READWRITE(nFile);
    READWRITE(nBlockPos);
    READWRITE(nHeight);
    READWRITE(nMint);
    READWRITE(nMoneySupply);
    READWRITE(nFlags);
    READWRITE(nStakeModifier);
    if (IsProofOfStake())
  {
    READWRITE(prevoutStake);
      READWRITE(nStakeTime);
    }
    else if (fRead)
  {
    const_cast<CDiskBlockIndex*>(this)->prevoutStake.SetNull();
      const_cast<CDiskBlockIndex*>(this)->nStakeTime = 0;
    }
  READWRITE(hashProof);


  READWRITE(this->nVersion);
  READWRITE(hashPrev);
  READWRITE(hashMerkleRoot);
  READWRITE(nTime);
  READWRITE(nBits);
  READWRITE(nNonce);
  READWRITE(blockHash);

  READWRITE(stateRootIndex);
  )

  uint256 GetBlockHash() const
  {
    if (fUseFastIndex && (nTime < GetAdjustedTime() - 24 * 60 * 60) && blockHash != 0)
    {
      return blockHash;
    }

    CBlock block;
    block.nVersion = nVersion;
    block.hashPrevBlock = hashPrev;
    block.hashMerkleRoot = hashMerkleRoot;
    block.nTime = nTime;
    block.nBits = nBits;
    block.nNonce = nNonce;
    const_cast<CDiskBlockIndex*>(this)->blockHash = block.GetHash();
    return blockHash;
  }

  std::string ToString() const
  {
    std::string str = "CDiskBlockIndex(";
    str += CBlockIndex::ToString();
    str += strprintf("\n                hashBlock=%s, hashPrev=%s, hashNext=%s)",
                     GetBlockHash().ToString().c_str(),
                     hashPrev.ToString().c_str(),
                     hashNext.ToString().c_str());
    return str;
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};

