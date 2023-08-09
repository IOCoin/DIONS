#ifndef DIONS_BLOCK_H
#define DIONS_BLOCK_H

#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"
#include "hashblock.h"
#include "ptrie/FixedHash.h"
#include "ptrie/TrieDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "transaction.h"

#include <list>
extern FILE* AppendBlockFile(unsigned int& nFileRet);
extern bool IsInitialBlockDownload();
extern bool CheckProofOfWork(uint256 hash, unsigned int nBits);
extern CBlockIndex* pindexBest;
extern bool IsProtocolV2(int nHeight);
extern bool fUseFastIndex;
extern std::map<uint256, CBlockIndex*> mapBlockIndex;
const uint256 hashGenesisBlock("0x00000afad2d5833b50b59a9784fdc59869b688680e1670a52c52e3c2c04c1fe8");
const uint256 hashGenesisBlockTestNet("5f97300cd3dc3d2215dd38ce6d99bf7d5984bb62b2777060d3b5564298bd5484");
const uint256 hashGenesisMerkleRoot("0xcd5029ac01fb6cd7da8ff00ff1e82f3aca6bf3ecce5fb60623ee807fa83d1795");
extern CBlockIndex* pindexGenesisBlock;
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
class CTxDB;
class CTxIndex;

class CBlock
{
public:

  static const int CURRENT_VERSION = 8;
  static const unsigned int MAX_BLOCK_SIZE = 4.0 * 1024 * 1024;
  static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE;
  int nVersion;
  uint256 hashPrevBlock;
  uint256 hashMerkleRoot;
  unsigned int nTime;
  unsigned int nBits;
  unsigned int nNonce;


  dev::h256 stateRoot;


  std::vector<CTransaction> vtx;


  std::vector<unsigned char> vchBlockSig;


  mutable std::vector<uint256> vMerkleTree;


  mutable int nDoS;
  bool DoS(int nDoSIn, bool fIn) const
  {
    nDoS += nDoSIn;
    return fIn;
  }

  CBlock()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->nVersion);
    nVersion = this->nVersion;
    READWRITE(hashPrevBlock);
    READWRITE(hashMerkleRoot);
    READWRITE(nTime);
    READWRITE(nBits);
    READWRITE(nNonce);


    if (!(nType & (SER_GETHASH|SER_BLOCKHEADERONLY)))
    {
      READWRITE(vtx);
      READWRITE(vchBlockSig);
    }
    else if (fRead)
    {
      const_cast<CBlock*>(this)->vtx.clear();
      const_cast<CBlock*>(this)->vchBlockSig.clear();
    }
  )

  void SetNull()
  {
    nVersion = CBlock::CURRENT_VERSION;
    hashPrevBlock = 0;
    hashMerkleRoot = 0;
    nTime = 0;
    nBits = 0;
    nNonce = 0;
    vtx.clear();
    vchBlockSig.clear();
    vMerkleTree.clear();
    nDoS = 0;
  }

  bool IsNull() const
  {
    return (nBits == 0);
  }

  uint256 GetHash() const
  {
    if (nVersion > 6)
    {
      return Hash(BEGIN(nVersion), END(nNonce));
    }
    else
    {
      return GetPoWHash();
    }
  }

  uint256 GetPoWHash() const
  {
    return Hash9(BEGIN(nVersion), END(nNonce));
  }

  int64_t GetBlockTime() const
  {
    return (int64_t)nTime;
  }

  void UpdateTime(const CBlockIndex* pindexPrev);


  unsigned int GetStakeEntropyBit() const
  {
    unsigned int nEntropyBit = ((GetHash().Get64()) & 1llu);

    if (fDebug && GetBoolArg("-printstakemodifier"))
    {
      printf("GetStakeEntropyBit: hashBlock=%s nEntropyBit=%u\n", GetHash().ToString().c_str(), nEntropyBit);
    }

    return nEntropyBit;
  }


  bool IsProofOfStake() const
  {
    return (vtx.size() > 1 && vtx[1].IsCoinStake());
  }

  bool IsProofOfWork() const
  {
    return !IsProofOfStake();
  }

  std::pair<COutPoint, unsigned int> GetProofOfStake() const
  {
    return IsProofOfStake()? std::make_pair(vtx[1].vin[0].prevout, vtx[1].nTime) : std::make_pair(COutPoint(), (unsigned int)0);
  }


  int64_t GetMaxTransactionTime() const
  {
    int64_t maxTransactionTime = 0;
    BOOST_FOREACH(const CTransaction& tx, vtx)
    maxTransactionTime = std::max(maxTransactionTime, (int64_t)tx.nTime);
    return maxTransactionTime;
  }

  uint256 BuildMerkleTree() const
  {
    vMerkleTree.clear();
    BOOST_FOREACH(const CTransaction& tx, vtx)
    vMerkleTree.push_back(tx.GetHash());
    int j = 0;

    for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
    {
      for (int i = 0; i < nSize; i += 2)
      {
        int i2 = std::min(i+1, nSize-1);
        vMerkleTree.push_back(Hash(BEGIN(vMerkleTree[j+i]), END(vMerkleTree[j+i]),
                                   BEGIN(vMerkleTree[j+i2]), END(vMerkleTree[j+i2])));
      }

      j += nSize;
    }
    /////////////////////////////////////////////////////////////////////////////////
    //  vtx 6     
    //  hash0 hash1 hash2 hash3 hash4 hash5
    //  hash0 hash1 hash2 hash3 hash4 hash5 hash01 hash23 hash45
    //  hash0 hash1 hash2 hash3 hash4 hash5 hash01 hash23 hash45 hash0123 hash4545
    //  hash0 hash1 hash2 hash3 hash4 hash5 hash01 hash23 hash45 hash0123 hash4545 hash01234545
    //
    //  j=0
    //  for nSize=6
    //    for i=0 i<6 i+=2
    //      i2 = min 1, 5  = 1
    //      tree push Hash(tree[0 + 0] and tree[0 + 1])
    //      i += 2
    //      i2 = min 3, 5  = 3
    //      tree push Hash(tree[0 + 2] and tree[0 + 3])
    //      i += 2
    //      i2 = min 5, 5  = 5
    //      tree push Hash(tree[0 + 4] and tree[0 + 5])
    //      i += 2 <- 6 end
    //
    //      tree has now 9 elements
    //
    //    j += nSize <- 6
    //    nSize = (6 + 1) / 2 = 3
    //    for i=0 i<3 i+=2
    //      i2 = min 1, 3-1  = 1
    //      tree push Hash(tree[6 + 0] and tree[6 + 1])
    //      i += 2
    //      i2 = min 2, 3-1  = 2
    //      tree push Hash(tree[6 + 2] and tree[6 + 2])
    //      i+=2 <- 4 end
    //
    //      tree has now 11 elements
    //    j += nSize <- 6 + 3 = 9
    //    nSize = (3 + 1) / 2 = 2
    //    for i=0 i<2 i+=2
    //      i2 = min 1, 2-1  = 1
    //      tree push Hash(tree[9 + 0] and tree[9 + 1])
    //      i+=2 <- 2 end
    //  
    //      tree has now 12 elements
    //    j += nSize <- 9 + 2 = 11
    //    nSize = (2+1)/2 = 1 end
    //
    //
    //
    /////////////////////////////////////////////////////////////////////////////////

    return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
  }

  std::vector<uint256> GetMerkleBranch(int nIndex) const
  {
    if (vMerkleTree.empty())
    {
      BuildMerkleTree();
    }

    std::vector<uint256> vMerkleBranch;
    int j = 0;

    for (int nSize = vtx.size(); nSize > 1; nSize = (nSize + 1) / 2)
    {
      int i = std::min(nIndex^1, nSize-1);
      vMerkleBranch.push_back(vMerkleTree[j+i]);
      nIndex >>= 1;
      j += nSize;
    }
    ////////////////////////////////////////////////////////////////////////
    //
    //  tree
    //  hash0 hash1 hash2 hash3 hash4 hash5 hash01 hash23 hash45 hash0123 hash4545 hash01234545
    //
    //  suppose nIndex = 3
    //  bramch
    //  hash2 hash01 hash4545
    //
    //                                        012345
    //                                  0123          4545
    //                               01      23     45      45
    //                              0  1    2  3   4  5    4  5 
    //
    //
    //  j=0
    //
    //  for nSize=6 nSize>1 nSize = (nSize + 1) / 2
    //    i = min nIndex xor 1, nSize-1 = min 2, 5
    //    branch push tree[0 + 2] 
    //    nIndex>>=1 <- 1
    //    j+=nSize <- 6
    //    nSize = (6 + 1) / 2 <- 3
    //    i = min nIndex xor 1, nSize-1 = min 0, 2 = 0
    //    branch push tree[6 + 0] 
    //    nIndex>>=1 <- 0
    //    j+=nSize <- 6 + 3 = 9
    //    nSize = (3 + 1) / 2 <- 2
    //    i = min nIndex xor 1, nSize-1 = min 1, 1 = 1
    //    branch push tree[9 + 1] 
    //    nIndex>>=1 <- 0
    //    j+=nSize <- 9 + 2 = 11 
    //    nSize = (2 + 1) / 2 <- 1 end
    //
    //
    //
    //
    ////////////////////////////////////////////////////////////////////////

    return vMerkleBranch;
  }

  static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex)
  {
    if (nIndex == -1)
    {
      return 0;
    }

    BOOST_FOREACH(const uint256& otherside, vMerkleBranch)
    {
      if (nIndex & 1)
      {
        hash = Hash(BEGIN(otherside), END(otherside), BEGIN(hash), END(hash));
      }
      else
      {
        hash = Hash(BEGIN(hash), END(hash), BEGIN(otherside), END(otherside));
      }

      nIndex >>= 1;
    }
    return hash;
  }


  bool WriteToDisk(unsigned int& nFileRet, unsigned int& nBlockPosRet)
  {
    CAutoFile fileout = CAutoFile(AppendBlockFile(nFileRet), SER_DISK, CLIENT_VERSION);

    if (!fileout)
    {
      return error("CBlock::WriteToDisk() : AppendBlockFile failed");
    }

    unsigned int nSize = fileout.GetSerializeSize(*this);
    fileout << FLATDATA(pchMessageStart) << nSize;
    long fileOutPos = ftell(fileout);

    if (fileOutPos < 0)
    {
      return error("CBlock::WriteToDisk() : ftell failed");
    }

    nBlockPosRet = fileOutPos;
    fileout << *this;
    fflush(fileout);

    if (!IsInitialBlockDownload() || (nBestHeight+1) % 500 == 0)
    {
      FileCommit(fileout);
    }

    return true;
  }

  bool ReadFromDisk(unsigned int nFile, unsigned int nBlockPos, bool fReadTransactions=true)
  {
    SetNull();
    CAutoFile filein = CAutoFile(OpenBlockFile(nFile, nBlockPos, "rb"), SER_DISK, CLIENT_VERSION);

    if (!filein)
    {
      return error("CBlock::ReadFromDisk() : OpenBlockFile failed");
    }

    if (!fReadTransactions)
    {
      filein.nType |= SER_BLOCKHEADERONLY;
    }

    try
    {
      filein >> *this;
    }
    catch (std::exception &e)
    {
      return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }

    if (fReadTransactions && IsProofOfWork() && !CheckProofOfWork(GetPoWHash(), nBits))
    {
      return error("CBlock::ReadFromDisk() : errors in block header");
    }

    return true;
  }



  void print() const
  {
    printf("CBlock(hash=%s, ver=%d, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%" PRIszu ", vchBlockSig=%s)\n",
           GetHash().ToString().c_str(),
           nVersion,
           hashPrevBlock.ToString().c_str(),
           hashMerkleRoot.ToString().c_str(),
           nTime, nBits, nNonce,
           vtx.size(),
           HexStr(vchBlockSig.begin(), vchBlockSig.end()).c_str());

    for (unsigned int i = 0; i < vtx.size(); i++)
    {
      printf("  ");
      vtx[i].print();
    }

    printf("  vMerkleTree: ");

    for (unsigned int i = 0; i < vMerkleTree.size(); i++)
    {
      printf("%s ", vMerkleTree[i].ToString().substr(0,10).c_str());
    }

    printf("\n");
  }


  bool DisconnectBlock(CTxDB& txdb, CBlockIndex* pindex);
  bool ConnectBlock(CTxDB& txdb, CBlockIndex* pindex, bool fJustCheck=false);
  bool ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions=true);
  bool SetBestChain(CTxDB& txdb, CBlockIndex* pindexNew);
  bool AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos, const uint256& hashProof);
  bool CheckBlock(bool fCheckPoW=true,bool fCheckMerkleRoot=true,bool fCheckSig=true) const;
  bool AcceptBlock();
  bool SignBlock(__wx__& keystore, int64_t nFees);
  bool CheckBlockSignature() const;
  dev::OverlayDB const* db() const { return (dev::OverlayDB const*)state__->db(); }

  dev::SecureTrieDB<dev::Address, dev::OverlayDB>* state__;
private:
  bool SetBestChainInner(CTxDB& txdb, CBlockIndex *pindexNew);
};
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
class CBlockLocator
{
protected:
  std::vector<uint256> vHave;
public:

  CBlockLocator()
  {
  }

  explicit CBlockLocator(const CBlockIndex* pindex)
  {
    Set(pindex);
  }

  explicit CBlockLocator(uint256 hashBlock)
  {
    std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);

    if (mi != mapBlockIndex.end())
    {
      Set((*mi).second);
    }
  }

  CBlockLocator(const std::vector<uint256>& vHaveIn)
  {
    vHave = vHaveIn;
  }

  IMPLEMENT_SERIALIZE
  (

    if (!(nType & SER_GETHASH))
    READWRITE(nVersion);
    READWRITE(vHave);
  )

    void SetNull()
  {
    vHave.clear();
  }

  bool IsNull()
  {
    return vHave.empty();
  }

  void Set(const CBlockIndex* pindex)
  {
    vHave.clear();
    int nStep = 1;

    while (pindex)
    {
      vHave.push_back(pindex->GetBlockHash());

      for (int i = 0; pindex && i < nStep; i++)
      {
        pindex = pindex->pprev;
      }

      if (vHave.size() > 10)
      {
        nStep *= 2;
      }
    }

    vHave.push_back((!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
  }

  int GetDistanceBack()
  {
    int nDistance = 0;
    int nStep = 1;
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);

      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;

        if (pindex->IsInMainChain())
        {
          return nDistance;
        }
      }

      nDistance += nStep;

      if (nDistance > 10)
      {
        nStep *= 2;
      }
    }
    return nDistance;
  }

  CBlockIndex* GetBlockIndex()
  {
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);

      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;

        if (pindex->IsInMainChain())
        {
          return pindex;
        }
      }
    }
    return pindexGenesisBlock;
  }

  uint256 GetBlockHash()
  {
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);

      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;

        if (pindex->IsInMainChain())
        {
          return hash;
        }
      }
    }
    return (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet);
  }

  int GetHeight()
  {
    CBlockIndex* pindex = GetBlockIndex();

    if (!pindex)
    {
      return 0;
    }

    return pindex->nHeight;
  }
};


#endif
