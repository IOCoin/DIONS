#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"
#include "hashblock.h"
#include "ptrie/FixedHash.h"

#include <list>


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

#include "constants.h"


extern bool fReindex;

static const int LAST_POW_BLOCK = 12815;


#define LR_R 0x32


static const unsigned int MAX_BLOCK_SIZE = 4.0 * 1024 * 1024;



static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE;


static const unsigned int MAX_STANDARD_TX_SIZE = 0.6 * MAX_BLOCK_SIZE_GEN;

static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;

static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;

static const unsigned int MAX_INV_SZ = 50000;

static const int64_t MIN_TX_FEE = 10000;

static const int64_t MIN_RELAY_TX_FEE = MIN_TX_FEE;
static const int64_t S_MIN_TX_FEE = 100000;

static const int64_t MAX_MONEY = std::numeric_limits<int64_t>::max();
inline bool MoneyRange(int64_t nValue)
{
  return (nValue >= 0 && nValue <= MAX_MONEY);
}

static const unsigned int LOCKTIME_THRESHOLD = 500000000;
static const unsigned int MAX_TX_INFO_LEN = 140;
static const int64_t MIN_COIN_YEAR_REWARD = 1 * CENT;
static const int64_t MAX_COIN_YEAR_REWARD = 3 * CENT;


static const uint256 hashGenesisBlock("0x00000afad2d5833b50b59a9784fdc59869b688680e1670a52c52e3c2c04c1fe8");
static const uint256 hashGenesisBlockTestNet("5f97300cd3dc3d2215dd38ce6d99bf7d5984bb62b2777060d3b5564298bd5484");
static const uint256 hashGenesisMerkleRoot("0xcd5029ac01fb6cd7da8ff00ff1e82f3aca6bf3ecce5fb60623ee807fa83d1795");

const int SHADE_FEATURE_UPDATE = 75 * 500 + 1860837;
const int BASELINE_LOCK = 0x00ff0;
const int BLOCK_REWARD_HALVING = 0x2dc6c0;

inline bool IsProtocolV2(int nHeight)
{
  return nHeight > 65000;
}
inline bool V3(int nHeight)
{
  return fTestNet ? nHeight >= 0x0a : nHeight >= 1625000;
}
inline bool V4(int nHeight)
{
  return fTestNet ? nHeight >= 0x10 : nHeight >= SHADE_FEATURE_UPDATE;
}
inline bool V5(int nHeight)
{
  return fTestNet ? nHeight >= 0x20 : nHeight >= BLOCK_REWARD_HALVING;
}

inline int64_t PastDrift(int64_t nTime, int nHeight)
{
  return IsProtocolV2(nHeight) ? nTime : nTime - 10 * 60;
}
inline int64_t FutureDrift(int64_t nTime, int nHeight)
{
  return IsProtocolV2(nHeight) ? nTime + 15 : nTime + 10 * 60;
}

inline unsigned int GetTargetSpacing(int nHeight)
{
  return IsProtocolV2(nHeight) ? 64 : 60;
}


extern unsigned int nCoinCacheSize;
extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern std::map<uint256, CBlockIndex*> mapBlockIndex;
extern std::set<std::pair<COutPoint, unsigned int> > setStakeSeen;
extern CBlockIndex* pindexGenesisBlock;
extern unsigned int nStakeMinAge;
extern unsigned int nStakeMaxAge;
extern unsigned int nNodeLifespan;
extern int nCoinbaseMaturity;
extern int nBestHeight;
extern uint256 nBestChainTrust;
extern uint256 nBestInvalidTrust;
extern uint256 hashBestChain;
extern CBlockIndex* pindexBest;
extern unsigned int nTransactionsUpdated;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockSize;
extern int64_t nLastCoinStakeSearchInterval;
extern const std::string strMessageMagic;
extern int64_t nTimeBestReceived;
extern CCriticalSection cs_setpwalletRegistered;
extern std::set<__wx__*> setpwalletRegistered;
extern unsigned char pchMessageStart[4];
extern std::map<uint256, CBlock*> mapOrphanBlocks;


extern int64_t nTransactionFee;
extern int64_t nReserveBalance;
extern int64_t nMinimumInputValue;
extern bool fUseFastIndex;
extern unsigned int nDerivationMethodIndex;

extern bool fEnforceCanonical;
extern int nStakeMinConfirmations;

static const uint64_t nMinDiskSpace = 52428800;

class CReserveKey;
class CTxDB;
class CTxIndex;

void RegisterWallet(__wx__* pwalletIn);
void UnregisterWallet(__wx__* pwalletIn);
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock = NULL, bool fUpdate = false, bool fConnect = true);
bool ProcessBlock(CNode* pfrom, CBlock* pblock);
bool CheckDiskSpace(uint64_t nAdditionalBytes=0);
FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
FILE* AppendBlockFile(unsigned int& nFileRet);
bool LoadBlockIndex(bool fAllowNew=true);
void PrintBlockTree();
CBlockIndex* FindBlockByHeight(int nHeight);
bool ProcessMessages(CNode* pfrom);
bool SendMessages(CNode* pto, bool fSendTrickle);
bool LoadExternalBlockFile(FILE* fileIn);

int GetPowHeight(const CBlockIndex* pindex);
bool CheckProofOfWork(uint256 hash, unsigned int nBits);
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake, int64_t nFees);
int64_t GetProofOfWorkReward(int64_t nPowHeight, int64_t nFees);
int64_t GetProofOfStakeInterest(int nHeight);
int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees, int nHeight);
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime);
unsigned int ComputeMinStake(unsigned int nBase, int64_t nTime, unsigned int nBlockTime);
bool minBase(const CTxIndex& txindex, const CBlockIndex* pindexFrom, int nMaxDepth, int& nActualDepth);
int GetNumBlocksOfPeers();
bool IsInitialBlockDownload();
std::string GetWarnings(std::string strFor);
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock, bool s=false);
uint256 WantedByOrphan(const CBlock* pblockOrphan);
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake);
void StakeMiner(__wx__ *pwallet);
void ResendWalletTransactions(bool fForce = false);



bool AcceptToMemoryPool(CTxMemPool& pool, CTransaction &tx,
                        bool* pfMissingInputs);
# 201 "main.h"
bool GetWalletFile(__wx__* pwallet, std::string &strWalletFileOut);


class CDiskTxPos
{
public:
  unsigned int nFile;
  unsigned int nBlockPos;
  unsigned int nTxPos;

  CDiskTxPos()
  {
    SetNull();
  }

  CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
  {
    nFile = nFileIn;
    nBlockPos = nBlockPosIn;
    nTxPos = nTxPosIn;
  }

  IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
  void SetNull()
  {
    nFile = (unsigned int) -1;
    nBlockPos = 0;
    nTxPos = 0;
  }
  bool IsNull() const
  {
    return (nFile == (unsigned int) -1);
  }

  friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
  {
    return (a.nFile == b.nFile &&
            a.nBlockPos == b.nBlockPos &&
            a.nTxPos == b.nTxPos);
  }

  friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
  {
    return !(a == b);
  }


  std::string ToString() const
  {
    if (IsNull())
    {
      return "null";
    }
    else
    {
      return strprintf("(nFile=%u, nBlockPos=%u, nTxPos=%u)", nFile, nBlockPos, nTxPos);
    }
  }

  void print() const
  {
    printf("%s", ToString().c_str());
  }
};




class CInPoint
{
public:
  CTransaction* ptx;
  unsigned int n;

  CInPoint()
  {
    SetNull();
  }
  CInPoint(CTransaction* ptxIn, unsigned int nIn)
  {
    ptx = ptxIn;
    n = nIn;
  }
  void SetNull()
  {
    ptx = NULL;
    n = (unsigned int) -1;
  }
  bool IsNull() const
  {
    return (ptx == NULL && n == (unsigned int) -1);
  }
};




class COutPoint
{
public:
  uint256 hash;
  unsigned int n;

  COutPoint()
  {
    SetNull();
  }
  COutPoint(uint256 hashIn, unsigned int nIn)
  {
    hash = hashIn;
    n = nIn;
  }
  IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
  void SetNull()
  {
    hash = 0;
    n = (unsigned int) -1;
  }
  bool IsNull() const
  {
    return (hash == 0 && n == (unsigned int) -1);
  }

  unsigned int GetIndex() const
  {
    return n;
  }
  const char* GetHash() const
  {
    return hash.ToString().c_str();
  };

  friend bool operator<(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
  }

  friend bool operator==(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash == b.hash && a.n == b.n);
  }

  friend bool operator!=(const COutPoint& a, const COutPoint& b)
  {
    return !(a == b);
  }

  std::string ToString() const
  {
    return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10).c_str(), n);
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};
class CTxIn
{
public:
  COutPoint prevout;
  CScript scriptSig;
  unsigned int nSequence;

  CTxIn()
  {
    nSequence = std::numeric_limits<unsigned int>::max();
  }

  explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(prevout);
    READWRITE(scriptSig);
    READWRITE(nSequence);
  )

  bool IsFinal() const
  {
    return (nSequence == std::numeric_limits<unsigned int>::max());
  }

  CTxIn& operator=(const CTxIn& o)
  {
    this->prevout = o.prevout;
    this->scriptSig = o.scriptSig;
    this->nSequence = o.nSequence;
    return *this;
  }

  friend bool operator==(const CTxIn& a, const CTxIn& b)
  {
    return (a.prevout == b.prevout &&
            a.scriptSig == b.scriptSig &&
            a.nSequence == b.nSequence);
  }

  friend bool operator!=(const CTxIn& a, const CTxIn& b)
  {
    return !(a == b);
  }

  std::string ToStringShort() const
  {
    return strprintf(" %s %d", prevout.hash.ToString().c_str(), prevout.n);
  }

  std::string ToString() const
  {
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
    {
      str += strprintf(", coinbase %s", HexStr(scriptSig).c_str());
    }
    else
    {
      str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24).c_str());
    }
    if (nSequence != std::numeric_limits<unsigned int>::max())
    {
      str += strprintf(", nSequence=%u", nSequence);
    }
    str += ")";
    return str;
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};







class CTxOut
{
public:
  int64_t nValue;
  CScript scriptPubKey;

  CTxOut()
  {
    SetNull();
  }

  CTxOut(int64_t nValueIn, CScript scriptPubKeyIn)
  {
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(nValue);
    READWRITE(scriptPubKey);
  )

  void SetNull()
  {
    nValue = -1;
    scriptPubKey.clear();
  }

  bool IsNull() const
  {
    return (nValue == -1);
  }

  void SetEmpty()
  {
    nValue = 0;
    scriptPubKey.clear();
  }

  bool IsEmpty() const
  {
    return (nValue == 0 && scriptPubKey.empty());
  }

  uint256 GetHash() const
  {
    return SerializeHash(*this);
  }

  bool IsUnspendable() const
  {
    return IsEmpty() ||
           (scriptPubKey.size() > 0 && *scriptPubKey.begin() == OP_RETURN);
  }

  CTxOut& operator=(const CTxOut& o)
  {
    this->nValue = o.nValue;
    this->scriptPubKey = o.scriptPubKey;
    return *this;
  }

  friend bool operator==(const CTxOut& a, const CTxOut& b)
  {
    return (a.nValue == b.nValue &&
            a.scriptPubKey == b.scriptPubKey);
  }

  friend bool operator!=(const CTxOut& a, const CTxOut& b)
  {
    return !(a == b);
  }

  std::string ToStringShort() const
  {
    return strprintf(" out %s %s", FormatMoney(nValue).c_str(), scriptPubKey.ToString(true).c_str());
  }

  std::string ToString() const
  {
    if (IsEmpty())
    {
      return "CTxOut(empty)";
    }
    return strprintf("CTxOut(nValue=%s, scriptPubKey=%s)", FormatMoney(nValue).c_str(), scriptPubKey.ToString().c_str());
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};




enum GetMinFee_mode
{
  GMF_BLOCK,
  GMF_RELAY,
  GMF_SEND,
};

typedef std::map<uint256, std::pair<CTxIndex, CTransaction> > MapPrevTx;





class CTransaction
{
public:
  static const int CURRENT_VERSION=1;
  static const int VERSION_WITH_INFO=3;
  static const int DION_TX_VERSION=5;
  int nVersion;
  unsigned int nTime;
  std::vector<CTxIn> vin;
  std::vector<CTxOut> vout;
  unsigned int nLockTime;
  std::string strTxInfo;

  mutable int nDoS;
  bool DoS(int nDoSIn, bool fIn) const
  {
    nDoS += nDoSIn;
    return fIn;
  }

  CTransaction()
  {
    SetNull();
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->nVersion);
    nVersion = this->nVersion;
    READWRITE(nTime);
    READWRITE(vin);
    READWRITE(vout);
    READWRITE(nLockTime);
    if (this->nVersion >= VERSION_WITH_INFO)
{
  READWRITE(strTxInfo);
  }
  )

  void SetNull()
  {
    nVersion = CTransaction::CURRENT_VERSION;
    nTime = GetAdjustedTime();
    vin.clear();
    vout.clear();
    nLockTime = 0;
    strTxInfo.clear();
    nDoS = 0;
  }

  bool IsNull() const
  {
    return (vin.empty() && vout.empty());
  }

  uint256 GetHash() const
  {
    return SerializeHash(*this);
  }

  bool IsNewerThan(const CTransaction& old) const
  {
    if (vin.size() != old.vin.size())
    {
      return false;
    }
    for (unsigned int i = 0; i < vin.size(); i++)
      if (vin[i].prevout != old.vin[i].prevout)
      {
        return false;
      }

    bool fNewer = false;
    unsigned int nLowest = std::numeric_limits<unsigned int>::max();
    for (unsigned int i = 0; i < vin.size(); i++)
    {
      if (vin[i].nSequence != old.vin[i].nSequence)
      {
        if (vin[i].nSequence <= nLowest)
        {
          fNewer = false;
          nLowest = vin[i].nSequence;
        }
        if (old.vin[i].nSequence < nLowest)
        {
          fNewer = true;
          nLowest = old.vin[i].nSequence;
        }
      }
    }
    return fNewer;
  }

  bool IsCoinBase() const
  {
    return (vin.size() == 1 && vin[0].prevout.IsNull() && vout.size() >= 1);
  }

  bool IsCoinStake() const
  {

    return (vin.size() > 0 && (!vin[0].prevout.IsNull()) && vout.size() >= 2 && vout[0].IsEmpty());
  }






  bool AreInputsStandard(const MapPrevTx& mapInputs) const;






  unsigned int GetLegacySigOpCount() const;







  unsigned int GetP2SHSigOpCount(const MapPrevTx& mapInputs) const;





  int64_t GetValueOut() const
  {
    int64_t nValueOut = 0;
    BOOST_FOREACH(const CTxOut& txout, vout)
    {
      nValueOut += txout.nValue;
      if (!MoneyRange(txout.nValue) || !MoneyRange(nValueOut))
      {
        throw std::runtime_error("CTransaction::GetValueOut() : value out of range");
      }
    }
    return nValueOut;
  }
# 709 "main.h"
  int64_t GetValueIn(const MapPrevTx& mapInputs) const;


  int64_t GetMinFee(unsigned int nBlockSize=1, enum GetMinFee_mode mode=GMF_BLOCK, unsigned int nBytes = 0) const;

  bool ReadFromDisk(CDiskTxPos pos, FILE** pfileRet=NULL)
  {
    CAutoFile filein = CAutoFile(OpenBlockFile(pos.nFile, 0, pfileRet ? "rb+" : "rb"), SER_DISK, CLIENT_VERSION);
    if (!filein)
    {
      return error("CTransaction::ReadFromDisk() : OpenBlockFile failed");
    }


    if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
    {
      return error("CTransaction::ReadFromDisk() : fseek failed");
    }

    try
    {
      filein >> *this;
    }
    catch (std::exception &e)
    {
      return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }


    if (pfileRet)
    {
      if (fseek(filein, pos.nTxPos, SEEK_SET) != 0)
      {
        return error("CTransaction::ReadFromDisk() : second fseek failed");
      }
      *pfileRet = filein.release();
    }
    return true;
  }

  friend bool operator==(const CTransaction& a, const CTransaction& b)
  {
    return (a.nVersion == b.nVersion &&
            a.nTime == b.nTime &&
            a.vin == b.vin &&
            a.vout == b.vout &&
            a.nLockTime == b.nLockTime);
  }

  friend bool operator!=(const CTransaction& a, const CTransaction& b)
  {
    return !(a == b);
  }

  std::string ToStringShort() const
  {
    std::string str;
    str += strprintf("%s %s", GetHash().ToString().c_str(), IsCoinBase()? "base" : (IsCoinStake()? "stake" : "user"));
    return str;
  }

  std::string ToString() const
  {
    std::string str;
    str += IsCoinBase()? "Coinbase" : (IsCoinStake()? "Coinstake" : "CTransaction");
    str += strprintf("(hash=%s, nTime=%d, ver=%d, vin.size=%" PRIszu ", vout.size=%" PRIszu ", nLockTime=%d, strTxInfo=%s)\n",
                     GetHash().ToString().substr(0,10).c_str(),
                     nTime,
                     nVersion,
                     vin.size(),
                     vout.size(),
                     nLockTime,
                     strTxInfo.substr(0,16).c_str());

    for (unsigned int i = 0; i < vin.size(); i++)
    {
      str += "    " + vin[i].ToString() + "\n";
    }
    for (unsigned int i = 0; i < vout.size(); i++)
    {
      str += "    " + vout[i].ToString() + "\n";
    }
    return str;
  }


  void print() const
  {
    printf("%s", ToString().c_str());
  }


  bool ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet);
  bool ReadFromDisk(CTxDB& txdb, COutPoint prevout);
  bool ReadFromDisk(COutPoint prevout);
  bool DisconnectInputs(CTxDB& txdb);
# 816 "main.h"
  bool FetchInputs(CTxDB& txdb, const std::map<uint256, CTxIndex>& mapTestPool,
                   bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid);
# 830 "main.h"
  bool ConnectInputs(CTxDB& txdb, MapPrevTx inputs,
                     std::map<uint256, CTxIndex>& mapTestPool, CDiskTxPos& posThisTx,
                     CBlockIndex* pindexBlock, bool fBlock, bool fMiner, int flags);
  bool CheckTransaction() const;
  bool GetCoinAge(CTxDB& txdb, uint64_t& nCoinAge) const;
  bool GetCoinAge(CTxDB& txdb, const CBlockIndex* pindexPrev, uint64_t& nCoinAge) const;


protected:
  const CTxOut& GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const;
};




bool IsStandardTx(const CTransaction& tx);

bool IsFinalTx(const CTransaction &tx, int nBlockHeight = 0, int64_t nBlockTime = 0);




class CMerkleTx : public CTransaction
{
private:
  int GetDepthInMainChainINTERNAL(CBlockIndex* &pindexRet) const;
public:
  uint256 hashBlock;
  std::vector<uint256> vMerkleBranch;
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
    hashBlock = 0;
    nIndex = -1;
    fMerkleVerified = false;
  }


  IMPLEMENT_SERIALIZE
  (
    nSerSize += SerReadWrite(s, *(CTransaction*)this, nType, nVersion, ser_action);
    nVersion = this->nVersion;
    READWRITE(hashBlock);
    READWRITE(vMerkleBranch);
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
  bool IsInMainChain() const
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
# 934 "main.h"
class CTxIndex
{
public:
  CDiskTxPos pos;
  std::vector<CDiskTxPos> vSpent;

  CTxIndex()
  {
    SetNull();
  }

  CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs)
  {
    pos = posIn;
    vSpent.resize(nOutputs);
  }

  IMPLEMENT_SERIALIZE
  (
    if (!(nType & SER_GETHASH))
    READWRITE(nVersion);
    READWRITE(pos);
    READWRITE(vSpent);
  )

    void SetNull()
  {
    pos.SetNull();
    vSpent.clear();
  }

  bool IsNull()
  {
    return pos.IsNull();
  }

  friend bool operator==(const CTxIndex& a, const CTxIndex& b)
  {
    return (a.pos == b.pos &&
            a.vSpent == b.vSpent);
  }

  friend bool operator!=(const CTxIndex& a, const CTxIndex& b)
  {
    return !(a == b);
  }
  int GetDepthInMainChain() const;

};
# 998 "main.h"
class CBlock
{
public:

  static const int CURRENT_VERSION = 8;
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

private:
  bool SetBestChainInner(CTxDB& txdb, CBlockIndex *pindexNew);
};
# 1315 "main.h"
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
# 1664 "main.h"
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
# 1810 "main.h"
class CTxMemPool
{
public:
  mutable CCriticalSection cs;
  std::map<uint256, CTransaction> mapTx;
  std::map<COutPoint, CInPoint> mapNextTx;

  bool addUnchecked(const uint256& hash, CTransaction &tx);
  bool remove(const CTransaction &tx, bool fRecursive = false);
  bool removeConflicts(const CTransaction &tx);
  void clear();
  void queryHashes(std::vector<uint256>& vtxid);

  unsigned long size() const
  {
    LOCK(cs);
    return mapTx.size();
  }

  bool exists(uint256 hash) const
  {
    LOCK(cs);
    return (mapTx.count(hash) != 0);
  }

  bool lookup(uint256 hash, CTransaction& result) const
  {
    LOCK(cs);
    std::map<uint256, CTransaction>::const_iterator i = mapTx.find(hash);
    if (i == mapTx.end())
    {
      return false;
    }
    result = i->second;
    return true;
  }
};

extern CTxMemPool mempool;

#endif
