
#include "WalletDB.h"
#include "Wallet.h"
#include <boost/version.hpp>
#include <boost/filesystem.hpp>
using namespace std;
using namespace boost;
static uint64_t nAccountingEntryNumber = 0;
extern bool fWalletUnlockStakingOnly;
bool __wx__DB::WriteName(const string& strAddress, const string& aliasStr)
{
  nWalletDBUpdated++;
  return Write(make_pair(string("name"), strAddress), aliasStr);
}
bool __wx__DB::EraseName(const string& strAddress)
{
  nWalletDBUpdated++;
  return Erase(make_pair(string("name"), strAddress));
}
bool __wx__DB::ReadAccount(const string& strAccount, CAccount& account)
{
  account.SetNull();
  return Read(make_pair(string("acc"), strAccount), account);
}
bool __wx__DB::WriteAccount(const string& strAccount, const CAccount& account)
{
  return Write(make_pair(string("acc"), strAccount), account);
}
bool __wx__DB::WriteAccountingEntry(const uint64_t nAccEntryNum, const CAccountingEntry& acentry)
{
  return Write(boost::make_tuple(string("acentry"), acentry.strAccount, nAccEntryNum), acentry);
}
bool __wx__DB::WriteAccountingEntry(const CAccountingEntry& acentry)
{
  return WriteAccountingEntry(++nAccountingEntryNumber, acentry);
}
int64_t __wx__DB::GetAccountCreditDebit(const string& strAccount)
{
  list<CAccountingEntry> entries;
  ListAccountCreditDebit(strAccount, entries);
  int64_t nCreditDebit = 0;
  BOOST_FOREACH (const CAccountingEntry& entry, entries)
  nCreditDebit += entry.nCreditDebit;
  return nCreditDebit;
}
void __wx__DB::ListAccountCreditDebit(const string& strAccount, list<CAccountingEntry>& entries)
{
  bool fAllAccounts = (strAccount == "*");
  Dbc* pcursor = GetCursor();

  if (!pcursor)
  {
    throw runtime_error("__wx__DB::ListAccountCreditDebit() : cannot create DB cursor");
  }

  unsigned int fFlags = DB_SET_RANGE;

  while (true)
  {
    CDataStream ssKey(SER_DISK, CLIENT_VERSION);

    if (fFlags == DB_SET_RANGE)
    {
      ssKey << boost::make_tuple(string("acentry"), (fAllAccounts? string("") : strAccount), uint64_t(0));
    }

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    int ret = ReadAtCursor(pcursor, ssKey, ssValue, fFlags);
    fFlags = DB_NEXT;

    if (ret == DB_NOTFOUND)
    {
      break;
    }
    else if (ret != 0)
    {
      pcursor->close();
      throw runtime_error("__wx__DB::ListAccountCreditDebit() : error scanning DB");
    }

    string strType;
    ssKey >> strType;

    if (strType != "acentry")
    {
      break;
    }

    CAccountingEntry acentry;
    ssKey >> acentry.strAccount;

    if (!fAllAccounts && acentry.strAccount != strAccount)
    {
      break;
    }

    ssValue >> acentry;
    ssKey >> acentry.nEntryNo;
    entries.push_back(acentry);
  }

  pcursor->close();
}
DBErrors
__wx__DB::ReorderTransactions(__wx__* pwallet)
{
  LOCK(pwallet->cs_wallet);
  typedef pair<__wx__Tx*, CAccountingEntry*> TxPair;
  typedef multimap<int64_t, TxPair > TxItems;
  TxItems txByTime;

  for (map<uint256, __wx__Tx>::iterator it = pwallet->mapWallet.begin(); it != pwallet->mapWallet.end(); ++it)
  {
    __wx__Tx* wtx = &((*it).second);
    txByTime.insert(make_pair(wtx->nTimeReceived, TxPair(wtx, (CAccountingEntry*)0)));
  }

  list<CAccountingEntry> acentries;
  ListAccountCreditDebit("", acentries);
  BOOST_FOREACH(CAccountingEntry& entry, acentries)
  {
    txByTime.insert(make_pair(entry.nTime, TxPair((__wx__Tx*)0, &entry)));
  }
  int64_t& nOrderPosNext = pwallet->nOrderPosNext;
  nOrderPosNext = 0;
  std::vector<int64_t> nOrderPosOffsets;

  for (TxItems::iterator it = txByTime.begin(); it != txByTime.end(); ++it)
  {
    __wx__Tx *const pwtx = (*it).second.first;
    CAccountingEntry *const pacentry = (*it).second.second;
    int64_t& nOrderPos = (pwtx != 0) ? pwtx->nOrderPos : pacentry->nOrderPos;

    if (nOrderPos == -1)
    {
      nOrderPos = nOrderPosNext++;
      nOrderPosOffsets.push_back(nOrderPos);

      if (pacentry)
        if (!WriteAccountingEntry(pacentry->nEntryNo, *pacentry))
        {
          return DB_LOAD_FAIL;
        }
    }
    else
    {
      int64_t nOrderPosOff = 0;
      BOOST_FOREACH(const int64_t& nOffsetStart, nOrderPosOffsets)
      {
        if (nOrderPos >= nOffsetStart)
        {
          ++nOrderPosOff;
        }
      }
      nOrderPos += nOrderPosOff;
      nOrderPosNext = std::max(nOrderPosNext, nOrderPos + 1);

      if (!nOrderPosOff)
      {
        continue;
      }

      if (pwtx)
      {
        if (!WriteTx(pwtx->GetHash(), *pwtx))
        {
          return DB_LOAD_FAIL;
        }
      }
      else if (!WriteAccountingEntry(pacentry->nEntryNo, *pacentry))
      {
        return DB_LOAD_FAIL;
      }
    }
  }

  return DB_LOAD_OK;
}
class __wx__ScanState
{
public:
  unsigned int nKeys;
  unsigned int nCKeys;
  unsigned int nKeyMeta;
  bool fIsEncrypted;
  bool fAnyUnordered;
  int nFileVersion;
  vector<uint256> vWalletUpgrade;
  __wx__ScanState()
  {
    nKeys = nCKeys = nKeyMeta = 0;
    fIsEncrypted = false;
    fAnyUnordered = false;
    nFileVersion = 0;
  }
};
bool
ReadKeyValue(__wx__* pwallet, CDataStream& ssKey, CDataStream& ssValue,
             __wx__ScanState &wss, string& strType, string& strErr)
{
  try
  {
    ssKey >> strType;

    if (strType == "name")
    {
      string strAddress;
      ssKey >> strAddress;
      ssValue >> pwallet->mapAddressBook[cba(strAddress).Get()];
    }
    else if (strType == "tx")
    {
      uint256 hash;
      ssKey >> hash;
      __wx__Tx& wtx = pwallet->mapWallet[hash];
      ssValue >> wtx;

      if (wtx.CheckTransaction() && (wtx.GetHash() == hash))
      {
        wtx.BindWallet(pwallet);
      }
      else
      {
        pwallet->mapWallet.erase(hash);
        return false;
      }

      if (31404 <= wtx.fTimeReceivedIsTxTime && wtx.fTimeReceivedIsTxTime <= 31703)
      {
        if (!ssValue.empty())
        {
          char fTmp;
          char fUnused;
          ssValue >> fTmp >> fUnused >> wtx.strFromAccount;
          strErr = strprintf("LoadWallet() upgrading tx ver=%d %d '%s' %s",
                             wtx.fTimeReceivedIsTxTime, fTmp, wtx.strFromAccount.c_str(), hash.ToString().c_str());
          wtx.fTimeReceivedIsTxTime = fTmp;
        }
        else
        {
          strErr = strprintf("LoadWallet() repairing tx ver=%d %s", wtx.fTimeReceivedIsTxTime, hash.ToString().c_str());
          wtx.fTimeReceivedIsTxTime = 0;
        }

        wss.vWalletUpgrade.push_back(hash);
      }

      if (wtx.nOrderPos == -1)
      {
        wss.fAnyUnordered = true;
      }

# 288 "walletdb.cpp"
    }
    else if (strType == "acentry")
    {
      string strAccount;
      ssKey >> strAccount;
      uint64_t nNumber;
      ssKey >> nNumber;

      if (nNumber > nAccountingEntryNumber)
      {
        nAccountingEntryNumber = nNumber;
      }

      if (!wss.fAnyUnordered)
      {
        CAccountingEntry acentry;
        ssValue >> acentry;

        if (acentry.nOrderPos == -1)
        {
          wss.fAnyUnordered = true;
        }
      }
    }
    else if (strType == "key" || strType == "wkey")
    {
      vector<unsigned char> vchPubKey;
      ssKey >> vchPubKey;
      CKey key;

      if (strType == "key")
      {
        wss.nKeys++;
        CPrivKey pkey;
        ssValue >> pkey;
        key.SetPubKey(vchPubKey);

        if (!key.SetPrivKey(pkey))
        {
          strErr = "Error reading wallet database: CPrivKey corrupt";
          return false;
        }

        if (key.GetPubKey() != vchPubKey)
        {
          strErr = "Error reading wallet database: CPrivKey pubkey inconsistency";
          return false;
        }

        if (!key.IsValid())
        {
          strErr = "Error reading wallet database: invalid CPrivKey";
          return false;
        }
      }
      else
      {
        __wx__Key wkey;
        ssValue >> wkey;
        key.SetPubKey(vchPubKey);

        if (!key.SetPrivKey(wkey.vchPrivKey))
        {
          strErr = "Error reading wallet database: CPrivKey corrupt";
          return false;
        }

        if (key.GetPubKey() != vchPubKey)
        {
          strErr = "Error reading wallet database: __wx__Key pubkey inconsistency";
          return false;
        }

        if (!key.IsValid())
        {
          strErr = "Error reading wallet database: invalid __wx__Key";
          return false;
        }
      }

      if (!pwallet->LoadKey(key))
      {
        strErr = "Error reading wallet database: LoadKey failed";
        return false;
      }
    }
    else if (strType == "mkey")
    {
      unsigned int nID;
      ssKey >> nID;
      CMasterKey kMasterKey;
      ssValue >> kMasterKey;

      if(pwallet->mapMasterKeys.count(nID) != 0)
      {
        strErr = strprintf("Error reading wallet database: duplicate CMasterKey id %u", nID);
        return false;
      }

      pwallet->mapMasterKeys[nID] = kMasterKey;

      if (pwallet->nMasterKeyMaxID < nID)
      {
        pwallet->nMasterKeyMaxID = nID;
      }
    }
    else if (strType == "ckey")
    {
      wss.nCKeys++;
      vector<unsigned char> vchPubKey;
      ssKey >> vchPubKey;
      vector<unsigned char> vchPrivKey;
      ssValue >> vchPrivKey;

      if (!pwallet->LoadCryptedKey(vchPubKey, vchPrivKey))
      {
        strErr = "Error reading wallet database: LoadCryptedKey failed";
        return false;
      }

      wss.fIsEncrypted = true;
    }
    else if (strType == "keymeta")
    {
      CPubKey vchPubKey;
      ssKey >> vchPubKey;
      CKeyMetadata keyMeta;
      ssValue >> keyMeta;
      wss.nKeyMeta++;
      pwallet->LoadKeyMetadata(vchPubKey, keyMeta);

      if (!pwallet->nTimeFirstKey ||
          (keyMeta.nCreateTime < pwallet->nTimeFirstKey))
      {
        pwallet->nTimeFirstKey = keyMeta.nCreateTime;
      }
    }
    else if (strType == "relay")
    {
      vchType k;
      ssKey >> k;
      Relay r;
      ssValue >> r;
      pwallet->LoadRelay(k, r);
    }
    else if (strType == "defaultkey")
    {
      ssValue >> pwallet->vchDefaultKey;
    }
    else if (strType == "pool")
    {
      int64_t nIndex;
      ssKey >> nIndex;
      CKeyPool keypool;
      ssValue >> keypool;
      pwallet->setKeyPool.insert(nIndex);
      CKeyID keyid = keypool.vchPubKey.GetID();

      if (pwallet->kd.count(keyid) == 0)
      {
        pwallet->kd[keyid] = CKeyMetadata(keypool.nTime);
      }
    }
    else if (strType == "version")
    {
      ssValue >> wss.nFileVersion;

      if (wss.nFileVersion == 10300)
      {
        wss.nFileVersion = 300;
      }
    }
    else if (strType == "cscript")
    {
      uint160 hash;
      ssKey >> hash;
      CScript script;
      ssValue >> script;

      if (!pwallet->LoadCScript(script))
      {
        strErr = "Error reading wallet database: LoadCScript failed";
        return false;
      }
    }
    else if (strType == "orderposnext")
    {
      ssValue >> pwallet->nOrderPosNext;
    }
  }
  catch (...)
  {
    return false;
  }

  return true;
}
static bool IsKeyType(string strType)
{
  return (strType== "key" || strType == "wkey" ||
          strType == "mkey" || strType == "ckey");
}
DBErrors __wx__DB::LoadWallet(__wx__* pwallet)
{
  pwallet->vchDefaultKey = CPubKey();
  __wx__ScanState wss;
  bool fNoncriticalErrors = false;
  DBErrors result = DB_LOAD_OK;

  try
  {
    LOCK(pwallet->cs_wallet);
    int nMinVersion = 0;

    if (Read((string)"minversion", nMinVersion))
    {
      if (nMinVersion > CLIENT_VERSION)
      {
        return DB_TOO_NEW;
      }

      pwallet->LoadMinVersion(nMinVersion);
    }

    Dbc* pcursor = GetCursor();

    if (!pcursor)
    {
      printf("Error getting wallet database cursor\n");
      return DB_CORRUPT;
    }

    while (true)
    {
      CDataStream ssKey(SER_DISK, CLIENT_VERSION);
      CDataStream ssValue(SER_DISK, CLIENT_VERSION);
      int ret = ReadAtCursor(pcursor, ssKey, ssValue);

      if (ret == DB_NOTFOUND)
      {
        break;
      }
      else if (ret != 0)
      {
        printf("Error reading next record from wallet database\n");
        return DB_CORRUPT;
      }

      string strType, strErr;

      if (!ReadKeyValue(pwallet, ssKey, ssValue, wss, strType, strErr))
      {
        if (IsKeyType(strType))
        {
          result = DB_CORRUPT;
        }
        else
        {
          fNoncriticalErrors = true;

          if (strType == "tx")
          {
            SoftSetBoolArg("-rescan", true);
          }
        }
      }

      if (!strErr.empty())
      {
        printf("%s\n", strErr.c_str());
      }
    }

    pcursor->close();
  }
  catch (...)
  {
    result = DB_CORRUPT;
  }

  if (fNoncriticalErrors && result == DB_LOAD_OK)
  {
    result = DB_NONCRITICAL_ERROR;
  }

  if (result != DB_LOAD_OK)
  {
    return result;
  }

  printf("nFileVersion = %d\n", wss.nFileVersion);
  printf("Keys: %u plaintext, %u encrypted, %u w/ metadata, %u total\n",
         wss.nKeys, wss.nCKeys, wss.nKeyMeta, wss.nKeys + wss.nCKeys);

  if ((wss.nKeys + wss.nCKeys) != wss.nKeyMeta)
  {
    pwallet->nTimeFirstKey = 1;
  }

  BOOST_FOREACH(uint256 hash, wss.vWalletUpgrade)
  WriteTx(hash, pwallet->mapWallet[hash]);

  if (wss.fIsEncrypted && (wss.nFileVersion == 40000 || wss.nFileVersion == 50000))
  {
    return DB_NEED_REWRITE;
  }

  if (wss.nFileVersion < CLIENT_VERSION)
  {
    WriteVersion(CLIENT_VERSION);
  }

  if (wss.fAnyUnordered)
  {
    result = ReorderTransactions(pwallet);
  }

  return result;
}
bool __wx__DB::Recover(CDBEnv& dbenv, std::string filename, bool fOnlyKeys)
{
  int64_t now = GetTime();
  std::string newFilename = strprintf("wallet.%" PRId64 ".bak", now);
  int result = dbenv.dbenv.dbrename(NULL, filename.c_str(), NULL,
                                    newFilename.c_str(), DB_AUTO_COMMIT);

  if (result == 0)
  {
    printf("Renamed %s to %s\n", filename.c_str(), newFilename.c_str());
  }
  else
  {
    printf("Failed to rename %s to %s\n", filename.c_str(), newFilename.c_str());
    return false;
  }

  std::vector<CDBEnv::KeyValPair> salvagedData;
  bool allOK = dbenv.Salvage(newFilename, true, salvagedData);

  if (salvagedData.empty())
  {
    printf("Salvage(aggressive) found no records in %s.\n", newFilename.c_str());
    return false;
  }

  printf("Salvage(aggressive) found %" PRIszu " records\n", salvagedData.size());
  bool fSuccess = allOK;
  Db* pdbCopy = new Db(&dbenv.dbenv, 0);
  int ret = pdbCopy->open(NULL,
                          filename.c_str(),
                          "main",
                          DB_BTREE,
                          DB_CREATE,
                          0);

  if (ret > 0)
  {
    printf("Cannot create database file %s\n", filename.c_str());
    return false;
  }

  __wx__ dummyWallet;
  __wx__ScanState wss;
  DbTxn* ptxn = dbenv.TxnBegin();
  BOOST_FOREACH(CDBEnv::KeyValPair& row, salvagedData)
  {
    if (fOnlyKeys)
    {
      CDataStream ssKey(row.first, SER_DISK, CLIENT_VERSION);
      CDataStream ssValue(row.second, SER_DISK, CLIENT_VERSION);
      string strType, strErr;
      bool fReadOK = ReadKeyValue(&dummyWallet, ssKey, ssValue,
                                  wss, strType, strErr);

      if (!IsKeyType(strType))
      {
        continue;
      }

      if (!fReadOK)
      {
        printf("WARNING: __wx__DB::Recover skipping %s: %s\n", strType.c_str(), strErr.c_str());
        continue;
      }
    }

    Dbt datKey(&row.first[0], row.first.size());
    Dbt datValue(&row.second[0], row.second.size());
    int ret2 = pdbCopy->put(ptxn, &datKey, &datValue, DB_NOOVERWRITE);

    if (ret2 > 0)
    {
      fSuccess = false;
    }
  }
  ptxn->commit(0);
  pdbCopy->close(0);
  delete pdbCopy;
  return fSuccess;
}
bool __wx__DB::Recover(CDBEnv& dbenv, std::string filename)
{
  return __wx__DB::Recover(dbenv, filename, false);
}
DBErrors __wx__DB::ZapWalletTx(__wx__* pwallet)
{
  vector<uint256> vTxHash;
  DBErrors err = FindWalletTx(pwallet, vTxHash);

  if (err != DB_LOAD_OK)
  {
    return err;
  }

  BOOST_FOREACH(uint256& hash, vTxHash)
  {
    if (!EraseTx(hash))
    {
      return DB_CORRUPT;
    }
  }
  return DB_LOAD_OK;
}
DBErrors __wx__DB::FindWalletTx(__wx__* pwallet, vector<uint256>& vTxHash)
{
  pwallet->vchDefaultKey = CPubKey();
  bool fNoncriticalErrors = false;
  DBErrors result = DB_LOAD_OK;

  try
  {
    LOCK(pwallet->cs_wallet);
    int nMinVersion = 0;

    if (Read((string)"minversion", nMinVersion))
    {
      if (nMinVersion > CLIENT_VERSION)
      {
        return DB_TOO_NEW;
      }

      pwallet->LoadMinVersion(nMinVersion);
    }

    Dbc* pcursor = GetCursor();

    if (!pcursor)
    {
      printf("Error getting wallet database cursor\n");
      return DB_CORRUPT;
    }

    while (true)
    {
      CDataStream ssKey(SER_DISK, CLIENT_VERSION);
      CDataStream ssValue(SER_DISK, CLIENT_VERSION);
      int ret = ReadAtCursor(pcursor, ssKey, ssValue);

      if (ret == DB_NOTFOUND)
      {
        break;
      }
      else if (ret != 0)
      {
        printf("Error reading next record from wallet database\n");
        return DB_CORRUPT;
      }

      string strType;
      ssKey >> strType;

      if (strType == "tx")
      {
        uint256 hash;
        ssKey >> hash;
        vTxHash.push_back(hash);
      }
    }

    pcursor->close();
  }
  catch (boost::thread_interrupted)
  {
    throw;
  }
  catch (...)
  {
    result = DB_CORRUPT;
  }

  if (fNoncriticalErrors && result == DB_LOAD_OK)
  {
    result = DB_NONCRITICAL_ERROR;
  }

  return result;
}
