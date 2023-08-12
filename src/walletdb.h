#ifndef BITCOIN_WALLETDB_H
#define BITCOIN_WALLETDB_H 

#include "db.h"
#include "base58.h"

#include "relay.h"
#include "disk_block_locator.h"

class CKeyPool;
class CAccount;
class CAccountingEntry;


enum DBErrors
{
  DB_LOAD_OK,
  DB_CORRUPT,
  DB_NONCRITICAL_ERROR,
  DB_TOO_NEW,
  DB_LOAD_FAIL,
  DB_NEED_REWRITE
};

enum RelayForward
{
  STATIC,
  FORM
};



class CKeyMetadata
{
public:
  typedef std::map<string, string> CMap;
  typedef std::vector<string> CVector;
  typedef std::map<long, long> Sentinel;
  static const int BASE_DIONS_VERSION=1;
  static const int SHADE_VERSION=2;
  static const int CURRENT_VERSION=SHADE_VERSION;
  int nVersion;
  CoordinateVector patch;
  RayShade rs_;
  CMap m;
  CVector q;
  vchType random;
  string r;
  Sentinel rlweIndex;
  int64_t nCreateTime;
  uint160 z;
  uint160 gamma;
  CPubKey k;

  CKeyMetadata()
  {
    SetNull();
  }
  CKeyMetadata(int64_t nCreateTime_)
  {
    nVersion = CKeyMetadata::CURRENT_VERSION;
    nCreateTime = nCreateTime_;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->nVersion);
    nVersion = this->nVersion;
    READWRITE(nCreateTime);
    READWRITE(this->patch);
    READWRITE(this->m);
    READWRITE(this->random);
    READWRITE(this->rlweIndex);
    if(nVersion >= SHADE_VERSION)
{
  READWRITE(this->r);
    READWRITE(this->z);
    READWRITE(this->k);
    READWRITE(this->rs_);
    READWRITE(this->q);
  }
  )

  void SetNull()
  {
    nVersion = CKeyMetadata::CURRENT_VERSION;
    nCreateTime = 0;
    z=0;
  }
};



class __wx__DB : public CDB
{
public:
  __wx__DB(std::string strFilename, const char* pszMode="r+") : CDB(strFilename.c_str(), pszMode)
  {
  }
private:
  __wx__DB(const __wx__DB&);
  void operator=(const __wx__DB&);
public:
  bool WriteName(const std::string& strAddress, const std::string& aliasStr);

  bool EraseName(const std::string& strAddress);

  bool WriteTx(uint256 hash, const __wx__Tx& wtx)
  {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("tx"), hash), wtx);
  }

  bool EraseTx(uint256 hash)
  {
    nWalletDBUpdated++;
    return Erase(std::make_pair(std::string("tx"), hash));
  }

  bool UpdateKey(const CPubKey& vchPubKey, const CKeyMetadata &keyMeta)
  {
    nWalletDBUpdated++;

    if(!Write(std::make_pair(std::string("keymeta"), vchPubKey), keyMeta, true))
    {
      return false;
    }

    return true;
  }

  bool UpdateKey(const vchType& k, const Relay& r)
  {
    nWalletDBUpdated++;

    if(!Write(std::make_pair(std::string("relay"), k), r, true))
    {
      return false;
    }

    return true;
  }

  bool WriteKey(const CPubKey& vchPubKey, const CPrivKey& vchPrivKey, const CKeyMetadata &keyMeta)
  {
    nWalletDBUpdated++;

    if(!Write(std::make_pair(std::string("keymeta"), vchPubKey), keyMeta))
    {
      return false;
    }

    return Write(std::make_pair(std::string("key"), vchPubKey.Raw()), vchPrivKey, false);
  }

  bool WriteCryptedKey(const CPubKey& vchPubKey, const std::vector<unsigned char>& vchCryptedSecret, CKeyMetadata &keyMeta)
  {
    nWalletDBUpdated++;
    bool fEraseUnencryptedKey = true;

    if(!Write(std::make_pair(std::string("keymeta"), vchPubKey), keyMeta))
    {
      return false;
    }

    bool __synRelay=false;
    if(keyMeta.z == FORM )
    {
      keyMeta.z=0;
      __synRelay=true;
    }
    if (!Write(std::make_pair(std::string("ckey"), vchPubKey.Raw()), vchCryptedSecret, __synRelay))
    {
      return false;
    }
    if (fEraseUnencryptedKey)
    {
      Erase(std::make_pair(std::string("key"), vchPubKey.Raw()));
      Erase(std::make_pair(std::string("wkey"), vchPubKey.Raw()));
    }
    return true;
  }

  bool WriteMasterKey(unsigned int nID, const CMasterKey& kMasterKey)
  {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("mkey"), nID), kMasterKey, true);
  }

  bool WriteCScript(const uint160& hash, const CScript& redeemScript)
  {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("cscript"), hash), redeemScript, false);
  }

  bool WriteBestBlock(const CBlockLocator& locator)
  {
    nWalletDBUpdated++;
    return Write(std::string("bestblock"), locator);
  }

  bool ReadBestBlock(CBlockLocator& locator)
  {
    return Read(std::string("bestblock"), locator);
  }

  bool WriteOrderPosNext(int64_t nOrderPosNext)
  {
    nWalletDBUpdated++;
    return Write(std::string("orderposnext"), nOrderPosNext);
  }

  bool WriteDefaultKey(const CPubKey& vchPubKey)
  {
    nWalletDBUpdated++;
    return Write(std::string("defaultkey"), vchPubKey.Raw());
  }

  bool ReadPool(int64_t nPool, CKeyPool& keypool)
  {
    return Read(std::make_pair(std::string("pool"), nPool), keypool);
  }

  bool WritePool(int64_t nPool, const CKeyPool& keypool)
  {
    nWalletDBUpdated++;
    return Write(std::make_pair(std::string("pool"), nPool), keypool);
  }

  bool ErasePool(int64_t nPool)
  {
    nWalletDBUpdated++;
    return Erase(std::make_pair(std::string("pool"), nPool));
  }

  bool WriteMinVersion(int nVersion)
  {
    return Write(std::string("minversion"), nVersion);
  }

  bool ReadAccount(const std::string& strAccount, CAccount& account);
  bool WriteAccount(const std::string& strAccount, const CAccount& account);
private:
  bool WriteAccountingEntry(const uint64_t nAccEntryNum, const CAccountingEntry& acentry);
public:
  bool WriteAccountingEntry(const CAccountingEntry& acentry);
  int64_t GetAccountCreditDebit(const std::string& strAccount);
  void ListAccountCreditDebit(const std::string& strAccount, std::list<CAccountingEntry>& acentries);

  DBErrors ReorderTransactions(__wx__*);
  DBErrors LoadWallet(__wx__* pwallet);
  DBErrors FindWalletTx(__wx__* pwallet, std::vector<uint256>& vTxHash);
  DBErrors ZapWalletTx(__wx__* pwallet);
  static bool Recover(CDBEnv& dbenv, std::string filename, bool fOnlyKeys);
  static bool Recover(CDBEnv& dbenv, std::string filename);
};

#endif
