#ifndef DIONS_TRANSACTION_H
#define DIONS_TRANSACTION_H
#include<cstdio>
#include<cstdint>
#include"serialize.h"
#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"
#include "output_point.h"
#include "disk_tx_pos.h"

extern bool MoneyRange(int64_t nValue);
extern FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode="rb");
class CTxDB;
class CTransaction
{
public:
  static const int CURRENT_VERSION=1;
  static const int VERSION_WITH_INFO=3;
  static const int DION_TX_VERSION=5;
  static const int64_t MIN_TX_FEE = 10000;
  static const int64_t MIN_RELAY_TX_FEE = MIN_TX_FEE;
  static const int64_t S_MIN_TX_FEE = 100000;
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
