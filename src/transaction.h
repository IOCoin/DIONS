#ifndef DIONS_TRANSACTION_H
#define DIONS_TRANSACTION_H
#include<cstdio>
#include<cstdint>
#include"serialize.h"
#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"

extern bool MoneyRange(int64_t nValue);
extern FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
class CTxDB;
enum GetMinFee_mode
{
  GMF_BLOCK,
  GMF_RELAY,
  GMF_SEND,
};
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
typedef std::map<uint256, std::pair<CTxIndex, CTransaction> > MapPrevTx;
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
  bool FetchInputs(CTxDB& txdb, const std::map<uint256, CTxIndex>& mapTestPool,
                   bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid);
  bool ConnectInputs(CTxDB& txdb, MapPrevTx inputs,
                     std::map<uint256, CTxIndex>& mapTestPool, CDiskTxPos& posThisTx,
                     CBlockIndex* pindexBlock, bool fBlock, bool fMiner, int flags);
  bool CheckTransaction() const;
  bool GetCoinAge(CTxDB& txdb, uint64_t& nCoinAge) const;
  bool GetCoinAge(CTxDB& txdb, const CBlockIndex* pindexPrev, uint64_t& nCoinAge) const;


protected:
  const CTxOut& GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const;
};


#endif
