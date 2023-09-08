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
#include "wallet/transaction.h"

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

#endif
