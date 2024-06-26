#pragma once

#include "ccoin/Process.h"

#include <map>
#include <string>
#include <vector>

#include <db_cxx.h>

#include "TxDBLevelDB.h"
#include "dions/Dions.h"

class CAddress;
class CAddrMan;
class CBlockLocator;
class CDiskBlockIndex;
class CDiskTxPos;
class CMasterKey;
class COutPoint;
class CTxIndex;
class __wx__;
class __wx__Tx;
class AliasIndex;
class CTxDB;

extern unsigned int nWalletDBUpdated;

void ThreadFlushWalletDB(void* parg);
bool BackupWallet(const __wx__& wallet, const std::string& strDest);

extern bool aliasTx(const CTransaction& tx, int& op, int& nOut, vector<vector<unsigned char> >& vvch );
extern CScript aliasStrip(const CScript& scriptIn);
extern bool txTrace(CDiskTxPos& txPos, vector<unsigned char>& vchValue, uint256& hash, int& nHeight);

class CDBEnv
{
private:
  bool fDbEnvInit;
  bool fMockDb;
  boost::filesystem::path pathEnv;
  std::string strPath;

  void EnvShutdown();

public:
  mutable CCriticalSection cs_db;
  DbEnv dbenv;
  std::map<std::string, int> mapFileUseCount;
  std::map<std::string, Db*> mapDb;

  CDBEnv();
  ~CDBEnv();
  void MakeMock();
  bool IsMock()
  {
    return fMockDb;
  };







  enum VerifyResult { VERIFY_OK, RECOVER_OK, RECOVER_FAIL };
  VerifyResult Verify(std::string strFile, bool (*recoverFunc)(CDBEnv& dbenv, std::string strFile));







  typedef std::pair<std::vector<unsigned char>, std::vector<unsigned char> > KeyValPair;
  bool Salvage(std::string strFile, bool fAggressive, std::vector<KeyValPair>& vResult);

  bool Open(boost::filesystem::path pathEnv_);
  void Close();
  void Flush(bool fShutdown);
  void CheckpointLSN(std::string strFile);

  void CloseDb(const std::string& strFile);
  bool RemoveDb(const std::string& strFile);

  DbTxn *TxnBegin(int flags=DB_TXN_WRITE_NOSYNC)
  {
    DbTxn* ptxn = NULL;
    int ret = dbenv.txn_begin(NULL, &ptxn, flags);
    if (!ptxn || ret != 0)
    {
      return NULL;
    }
    return ptxn;
  }
};

extern CDBEnv bitdb;



class CDB
{
protected:
  Db* pdb;
  std::string strFile;
  DbTxn *activeTxn;
  bool fReadOnly;

  explicit CDB(const char* pszFile, const char* pszMode="r+");
  ~CDB()
  {
    Close();
  }
public:
  void Close();
private:
  CDB(const CDB&);
  void operator=(const CDB&);

public:
  template<typename K, typename T>
  bool Read(const K& key, T& value)
  {
    if (!pdb)
    {
      return false;
    }


    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());


    Dbt datValue;
    datValue.set_flags(DB_DBT_MALLOC);
    int ret = pdb->get(activeTxn, &datKey, &datValue, 0);
    memset(datKey.get_data(), 0, datKey.get_size());
    if (datValue.get_data() == NULL)
    {
      return false;
    }


    try
    {
      CDataStream ssValue((char*)datValue.get_data(), (char*)datValue.get_data() + datValue.get_size(), SER_DISK, CLIENT_VERSION);
      ssValue >> value;
    }
    catch (std::exception &e)
    {
      return false;
    }


    memset(datValue.get_data(), 0, datValue.get_size());
    free(datValue.get_data());
    return (ret == 0);
  }

  template<typename K, typename T>
  bool Write(const K& key, const T& value, bool fOverwrite=true)
  {
    if (!pdb)
    {
      return false;
    }
    if (fReadOnly)
    {
      assert(!"Write called on database in read-only mode");
    }


    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());


    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(10000);
    ssValue << value;
    Dbt datValue(&ssValue[0], ssValue.size());


    int ret = pdb->put(activeTxn, &datKey, &datValue, (fOverwrite ? 0 : DB_NOOVERWRITE));


    memset(datKey.get_data(), 0, datKey.get_size());
    memset(datValue.get_data(), 0, datValue.get_size());
    return (ret == 0);
  }

  template<typename K>
  bool Erase(const K& key)
  {
    if (!pdb)
    {
      return false;
    }
    if (fReadOnly)
    {
      assert(!"Erase called on database in read-only mode");
    }


    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());


    int ret = pdb->del(activeTxn, &datKey, 0);


    memset(datKey.get_data(), 0, datKey.get_size());
    return (ret == 0 || ret == DB_NOTFOUND);
  }

  template<typename K>
  bool Exists(const K& key)
  {
	    std::cout << "Exists 0" << std::endl;
    if (!pdb)
    {
	    std::cout << "Exists 1" << std::endl;
      return false;
    }


	    std::cout << "Exists 2" << std::endl;
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(1000);
    ssKey << key;
    Dbt datKey(&ssKey[0], ssKey.size());


    int ret = pdb->exists(activeTxn, &datKey, 0);


	    std::cout << "Exists 3" << std::endl;
    memset(datKey.get_data(), 0, datKey.get_size());
    return (ret == 0);
  }

  Dbc* GetCursor()
  {
    if (!pdb)
    {
      return NULL;
    }
    Dbc* pcursor = NULL;
    int ret = pdb->cursor(NULL, &pcursor, 0);
    if (ret != 0)
    {
      return NULL;
    }
    return pcursor;
  }

  int ReadAtCursor(Dbc* pcursor, CDataStream& ssKey, CDataStream& ssValue, unsigned int fFlags=DB_NEXT)
  {

    Dbt datKey;
    if (fFlags == DB_SET || fFlags == DB_SET_RANGE || fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE)
    {
      datKey.set_data(&ssKey[0]);
      datKey.set_size(ssKey.size());
    }
    Dbt datValue;
    if (fFlags == DB_GET_BOTH || fFlags == DB_GET_BOTH_RANGE)
    {
      datValue.set_data(&ssValue[0]);
      datValue.set_size(ssValue.size());
    }
    datKey.set_flags(DB_DBT_MALLOC);
    datValue.set_flags(DB_DBT_MALLOC);
    int ret = pcursor->get(&datKey, &datValue, fFlags);
    if (ret != 0)
    {
      return ret;
    }
    else if (datKey.get_data() == NULL || datValue.get_data() == NULL)
    {
      return 99999;
    }


    ssKey.SetType(SER_DISK);
    ssKey.clear();
    ssKey.write((char*)datKey.get_data(), datKey.get_size());
    ssValue.SetType(SER_DISK);
    ssValue.clear();
    ssValue.write((char*)datValue.get_data(), datValue.get_size());


    memset(datKey.get_data(), 0, datKey.get_size());
    memset(datValue.get_data(), 0, datValue.get_size());
    free(datKey.get_data());
    free(datValue.get_data());
    return 0;
  }

public:
  bool TxnBegin()
  {
    if (!pdb || activeTxn)
    {
      return false;
    }
    DbTxn* ptxn = bitdb.TxnBegin();
    if (!ptxn)
    {
      return false;
    }
    activeTxn = ptxn;
    return true;
  }

  bool TxnCommit()
  {
    if (!pdb || !activeTxn)
    {
      return false;
    }
    int ret = activeTxn->commit(0);
    activeTxn = NULL;
    return (ret == 0);
  }

  bool TxnAbort()
  {
    if (!pdb || !activeTxn)
    {
      return false;
    }
    int ret = activeTxn->abort();
    activeTxn = NULL;
    return (ret == 0);
  }

  bool ReadVersion(int& nVersion)
  {
    nVersion = 0;
    return Read(std::string("version"), nVersion);
  }

  bool WriteVersion(int nVersion)
  {
    return Write(std::string("version"), nVersion);
  }

  bool static Rewrite(const std::string& strFile, const char* pszSkip = NULL);
};



class CAddrDB
{
private:
  boost::filesystem::path pathAddr;
public:
  CAddrDB();
  bool Write(const CAddrMan& addr);
  bool Read(CAddrMan& addr);
};
class AliasIndex
{
public:
  CDiskTxPos txPos;
  int nHeight;
  std::vector<unsigned char> vValue;
  std::string vAddress;

  AliasIndex()
  {
  }

  AliasIndex(CDiskTxPos txPosIn, unsigned int nHeightIn, std::vector<unsigned char> vValueIn, std::string vAddressIn)
  {
    txPos = txPosIn;
    nHeight = nHeightIn;
    vValue = vValueIn;
    vAddress = vAddressIn;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(txPos);
    READWRITE(nHeight);
    READWRITE(vValue);
    READWRITE(vAddress);
  )
};




class VertexNodeDB : public CDB
{
public:
  VertexNodeDB(const char* pszMode="cr+") : CDB("vertexcache.dat", pszMode)
  {
  }


  bool lPut(const vchType& vertex, const vchType& reference)
  {
    return Write(make_pair(std::string("vertex_"), vertex), reference, true);
  }


  bool lGet(const vchType& vertex, vchType& reference)
  {
    return Read(make_pair(std::string("vertex_"), vertex), reference);
  }

  bool lKey(const vchType& vertex)
  {
    return Exists(make_pair(std::string("vertex_"), vertex));
  }

  bool EraseName(const vchType& vertex)
  {
    return Erase(make_pair(std::string("vertex_"), vertex));
  }

  bool test();
  void filter(CBlockIndex*);
  void filter()
  {
    return;
  };
};

class LocatorNodeDB : public CDB
{
public:
  LocatorNodeDB(const char* pszMode="cr+") : CDB("aliascache.dat", pszMode)
  {
  }

  bool lPut(const vchType& alias, const std::vector<AliasIndex>& vtxPos)
  {
    return Write(make_pair(std::string("alias_"), alias), vtxPos, true);
  }

  bool lGet(const vchType& alias, std::vector<AliasIndex>& vtxPos)
  {
    return Read(make_pair(std::string("alias_"), alias), vtxPos);
  }

  bool lKey(const vchType& alias)
  {
	  std::cout << "LocatorNodeDB::lKey :" << stringFromVch(alias) << ":" << std::endl;
	  std::cout << "---------" << std::endl;
    return Exists(make_pair(std::string("alias_"), alias));
  }

  bool EraseName(const vchType& alias)
  {
    return Erase(make_pair(std::string("alias_"), alias));
  }

  bool ydwiWhldw();
  bool test();
  void filter(CBlockIndex*);
  void filter()
  {
    return;
  };
};
