#include "txdb.h"
#include "wallet.h"
#include "walletdb.h"
#include "crypter.h"
#include "ui_interface.h"
#include "base58.h"
#include "kernel.h"
#include "reference.h"
#include "coincontrol.h"
#include <boost/algorithm/string/replace.hpp>
#include "ray_shade.h"

#include "main.h"


#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/algorithm/string/predicate.hpp>
using namespace std;

using namespace boost;

static unsigned int GetStakeSplitAge()
{
  return IsProtocolV2(nBestHeight) ? (10 * 24 * 60 * 60) : (1 * 24 * 60 * 60);
}
static int64_t GetStakeCombineThreshold()
{
  return IsProtocolV2(nBestHeight) ? (50 * COIN) : (1000 * COIN);
}

bool isAliasTx(const __wx__Tx* tx);
extern __wx__* pwalletMain;
extern CScript aliasStrip(const CScript& scriptIn);
extern bool aliasScript(const CScript& script, int& op, vector<vector<unsigned char> > &vvch);

extern bool collx(const CTransaction&);





struct CompareValueOnly
{
  bool operator()(const pair<int64_t, pair<const __wx__Tx*, unsigned int> >& t1,
                  const pair<int64_t, pair<const __wx__Tx*, unsigned int> >& t2) const
  {
    return t1.first < t2.first;
  }
};
void ThreadFlushWalletDB(void* parg)
{

  RenameThread("iocoin-wallet");

  const string& strFile = ((const string*)parg)[0];
  static bool fOneThread;
  if (fOneThread)
  {
    return;
  }
  fOneThread = true;
  if (!GetBoolArg("-flushwallet", true))
  {
    return;
  }

  unsigned int nLastSeen = nWalletDBUpdated;
  unsigned int nLastFlushed = nWalletDBUpdated;
  int64_t nLastWalletUpdate = GetTime();
  while (!fShutdown)
  {
    MilliSleep(500);

    if (nLastSeen != nWalletDBUpdated)
    {
      nLastSeen = nWalletDBUpdated;
      nLastWalletUpdate = GetTime();
    }

    if (nLastFlushed != nWalletDBUpdated && GetTime() - nLastWalletUpdate >= 2)
    {
      TRY_LOCK(bitdb.cs_db,lockDb);
      if (lockDb)
      {

        int nRefCount = 0;
        map<string, int>::iterator mi = bitdb.mapFileUseCount.begin();
        while (mi != bitdb.mapFileUseCount.end())
        {
          nRefCount += (*mi).second;
          mi++;
        }

        if (nRefCount == 0 && !fShutdown)
        {
          map<string, int>::iterator mi = bitdb.mapFileUseCount.find(strFile);
          if (mi != bitdb.mapFileUseCount.end())
          {
            printf("Flushing wallet.dat\n");
            nLastFlushed = nWalletDBUpdated;
            int64_t nStart = GetTimeMillis();


            bitdb.CloseDb(strFile);
            bitdb.CheckpointLSN(strFile);

            bitdb.mapFileUseCount.erase(mi++);
            printf("Flushed wallet.dat %" PRId64 "ms\n", GetTimeMillis() - nStart);
          }
        }
      }
    }
  }
}

bool BackupWallet(const __wx__& wallet, const string& strDest)
{
  if (!wallet.fFileBacked)
  {
    return false;
  }
  while (!fShutdown)
  {
    {
      LOCK(bitdb.cs_db);
      if (!bitdb.mapFileUseCount.count(wallet.strWalletFile) || bitdb.mapFileUseCount[wallet.strWalletFile] == 0)
      {

        bitdb.CloseDb(wallet.strWalletFile);
        bitdb.CheckpointLSN(wallet.strWalletFile);
        bitdb.mapFileUseCount.erase(wallet.strWalletFile);


        boost::filesystem::path pathSrc = GetDataDir() / wallet.strWalletFile;
        boost::filesystem::path pathDest(strDest);
        if( boost::filesystem::is_directory(pathDest))
        {
          pathDest /= wallet.strWalletFile;
        }

        try
        {
#if BOOST_VERSION >= 104000
          boost::filesystem::copy_file(pathSrc, pathDest, boost::filesystem::copy_option::overwrite_if_exists);
#else
          boost::filesystem::copy_file(pathSrc, pathDest);
#endif
          printf("copied wallet.dat to %s\n", pathDest.string().c_str());
          return true;
        }
        catch(const boost::filesystem::filesystem_error &e)
        {
          printf("error copying wallet.dat to %s - %s\n", pathDest.string().c_str(), e.what());
          return false;
        }
      }
    }
    MilliSleep(100);
  }
  return false;
}

CPubKey __wx__::GenerateNewKey()
{
  AssertLockHeld(cs_wallet);
  bool fCompressed = CanSupportFeature(FEATURE_COMPRPUBKEY);

  RandAddSeedPerfmon();
  CKey key;
  key.MakeNewKey(fCompressed);


  if (fCompressed)
  {
    SetMinVersion(FEATURE_COMPRPUBKEY);
  }

  CPubKey pubkey = key.GetPubKey();


  int64_t nCreationTime = GetTime();
  kd[pubkey.GetID()] = CKeyMetadata(nCreationTime);
  if (!nTimeFirstKey || nCreationTime < nTimeFirstKey)
  {
    nTimeFirstKey = nCreationTime;
  }

  if (!ak(key))
  {
    throw std::runtime_error("__wx__::GenerateNewKey() : ak failed");
  }
  return key.GetPubKey();
}


bool __wx__::ak(const CKey& key)
{
  AssertLockHeld(cs_wallet);

  CPubKey pubkey = key.GetPubKey();

  if (!CCryptoKeyStore::ak(key))
  {
    return false;
  }
  if (!fFileBacked)
  {
    return true;
  }
  if (!IsCrypted())
  {
    return __wx__DB(strWalletFile).WriteKey(pubkey, key.GetPrivKey(), kd[pubkey.GetID()]);
  }
  return true;
}

bool __wx__::sync(const CPubKey &vchPubKey, const vector<unsigned char> &vchCryptedSecret)
{
  if (!CCryptoKeyStore::sync(vchPubKey, vchCryptedSecret))
  {
    return false;
  }
  if (!fFileBacked)
  {
    return true;
  }
  {
    LOCK(cs_wallet);
    if (pwalletdbEncryption)
    {
      return pwalletdbEncryption->WriteCryptedKey(vchPubKey, vchCryptedSecret, kd[vchPubKey.GetID()]);
    }
    else
    {
      return __wx__DB(strWalletFile).WriteCryptedKey(vchPubKey, vchCryptedSecret, kd[vchPubKey.GetID()]);
    }
  }
  return false;
}

bool __wx__::LoadKeyMetadata(const CPubKey &pubkey, const CKeyMetadata &meta)
{
  AssertLockHeld(cs_wallet);
  if (meta.nCreateTime && (!nTimeFirstKey || meta.nCreateTime < nTimeFirstKey))
  {
    nTimeFirstKey = meta.nCreateTime;
  }

  kd[pubkey.GetID()] = meta;
  return true;
}

bool __wx__::LoadRelay(const vchType& k, const Relay& r)
{
  AssertLockHeld(cs_wallet);
  lCache[k] = r;
  return true;
}

bool __wx__::envCP0(const CPubKey &pubkey, string& rsaPrivKey)
{
  AssertLockHeld(cs_wallet);
  if(kd.count(pubkey.GetID()))
  {
    if(! (kd[pubkey.GetID()].patch.scale_() || kd[pubkey.GetID()].patch.scale()))
    {
      rsaPrivKey = kd[pubkey.GetID()].patch.domainImage();
      return true;
    }
  }
  return false;
}

bool __wx__::envCP1(const CPubKey &pubkey, string& rsaPubKey)
{
  AssertLockHeld(cs_wallet);
  if(kd.count(pubkey.GetID()))
  {
    if(!(kd[pubkey.GetID()].patch.scale_() || kd[pubkey.GetID()].patch.scale()))
    {
      rsaPubKey = kd[pubkey.GetID()].patch.codomainImage();
      return true;
    }
  }

  return false;
}

bool __wx__::GetRandomKeyMetadata(const CPubKey& pubkey, vchType &r, string& r_)
{
  AssertLockHeld(cs_wallet);

  r = kd[pubkey.GetID()].random;
  r_ = kd[pubkey.GetID()].r;

  return true;
}

bool __wx__::SetRandomKeyMetadata(const CPubKey& pubkey, const vchType &r)
{
  AssertLockHeld(cs_wallet);

  kd[pubkey.GetID()].random = r;
  kd[pubkey.GetID()].r = stringFromVch(r);

  return true;
}

bool __wx__::aes_(const CPubKey& pubkey, string& f, string& aesPlainBase64)
{
  AssertLockHeld(cs_wallet);

  aesPlainBase64 = kd[pubkey.GetID()].m[f];

  if(aesPlainBase64 != "")
  {
    return true;
  }

  return false;
}

bool __wx__::aes(const CPubKey &pubkey, string& f, string& aes256KeyBase64)
{
  AssertLockHeld(cs_wallet);

  kd[pubkey.GetID()].m[f] = aes256KeyBase64;

  return true;
}

bool __wx__::vtx_(const CPubKey& pubkey, string& a)
{
  AssertLockHeld(cs_wallet);

  if(kd[pubkey.GetID()].q.size() > 0)
  {
    a = kd[pubkey.GetID()].q.back();
  }
  else
  {
    return false;
  }

  if(a != "")
  {
    return true;
  }

  return false;
}

bool __wx__::vtx(const CPubKey &pubkey, string& a)
{
  AssertLockHeld(cs_wallet);

  kd[pubkey.GetID()].q.push_back(a);

  return true;
}

bool __wx__::SetRSAMetadata(const CPubKey &pubkey)
{
  AssertLockHeld(cs_wallet);

  if(kd[pubkey.GetID()].patch.scale_() && kd[pubkey.GetID()].patch.scale())
  {
    GenerateRSAKey(kd[pubkey.GetID()].patch);

    return true;
  }

  return false;
}

bool __wx__::LoadCryptedKey(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret)
{
  return CCryptoKeyStore::sync(vchPubKey, vchCryptedSecret);
}

bool __wx__::AddCScript(const CScript& redeemScript)
{
  if (!CCryptoKeyStore::AddCScript(redeemScript))
  {
    return false;
  }
  if (!fFileBacked)
  {
    return true;
  }
  return __wx__DB(strWalletFile).WriteCScript(Hash160(redeemScript), redeemScript);
}




bool fWalletUnlockStakingOnly = false;

bool __wx__::LoadCScript(const CScript& redeemScript)
{



  if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
  {
    std::string strAddr = cba(redeemScript.GetID()).ToString();
    printf("%s: Warning: This wallet contains a redeemScript of size %" PRIszu " which exceeds maximum size %i thus can never be redeemed. Do not use address %s.\n",
           __func__, redeemScript.size(), MAX_SCRIPT_ELEMENT_SIZE, strAddr.c_str());
    return true;
  }

  return CCryptoKeyStore::AddCScript(redeemScript);
}
bool __wx__::relay(const vchType& k, Relay& r)
{
  AssertLockHeld(cs_wallet);

  if(lCache.count(k) == 0)
  {
    lCache[k] = r;
    return true;
  }

  return false;
}

bool __wx__::Unlock(const SecureString& strWalletPassphrase)
{
  if (!as())
  {
    return false;
  }

  CCrypter crypter;
  CKeyingMaterial vMasterKey;

  {
    LOCK(cs_wallet);
    BOOST_FOREACH(const MasterKeyMap::value_type& pMasterKey, mapMasterKeys)
    {
      if(!crypter.SetKeyFromPassphrase(strWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
      {
        return false;
      }
      if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
      {
        return false;
      }
      if (CCryptoKeyStore::Unlock(vMasterKey))
      {
        __transient();
        return true;
      }
    }
  }
  return false;
}


bool __wx__::ChangeWalletPassphrase(const SecureString& strOldWalletPassphrase, const SecureString& strNewWalletPassphrase)
{
  bool fWasLocked = as();

  {
    LOCK(cs_wallet);
    Lock();

    CCrypter crypter;
    CKeyingMaterial vMasterKey;
    BOOST_FOREACH(MasterKeyMap::value_type& pMasterKey, mapMasterKeys)
    {
      if(!crypter.SetKeyFromPassphrase(strOldWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
      {
        return false;
      }
      if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
      {
        return false;
      }
      if (CCryptoKeyStore::Unlock(vMasterKey))
      {
        int64_t nStartTime = GetTimeMillis();
        crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
        pMasterKey.second.nDeriveIterations = pMasterKey.second.nDeriveIterations * (100 / ((double)(GetTimeMillis() - nStartTime)));

        nStartTime = GetTimeMillis();
        crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
        pMasterKey.second.nDeriveIterations = (pMasterKey.second.nDeriveIterations + pMasterKey.second.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

        if (pMasterKey.second.nDeriveIterations < 25000)
        {
          pMasterKey.second.nDeriveIterations = 25000;
        }

        printf("Wallet passphrase changed to an nDeriveIterations of %i\n", pMasterKey.second.nDeriveIterations);

        if (!crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
        {
          return false;
        }
        if (!crypter.Encrypt(vMasterKey, pMasterKey.second.vchCryptedKey))
        {
          return false;
        }
        __wx__DB(strWalletFile).WriteMasterKey(pMasterKey.first, pMasterKey.second);
        if (fWasLocked)
        {
          Lock();
        }
        return true;
      }
    }
  }

  return false;
}

bool __wx__::relay_(const vchType& k, Relay& r)
{
  AssertLockHeld(cs_wallet);

  if(lCache.count(k))
  {
    r = lCache[k];
    return true;
  }

  return false;
}

void __wx__::SetBestChain(const CBlockLocator& loc)
{
  __wx__DB walletdb(strWalletFile);
  walletdb.WriteBestBlock(loc);
}

bool __wx__::SetMinVersion(enum WalletFeature nVersion, __wx__DB* pwalletdbIn, bool fExplicit)
{
  LOCK(cs_wallet);
  if (nWalletVersion >= nVersion)
  {
    return true;
  }


  if (fExplicit && nVersion > nWalletMaxVersion)
  {
    nVersion = FEATURE_LATEST;
  }

  nWalletVersion = nVersion;

  if (nVersion > nWalletMaxVersion)
  {
    nWalletMaxVersion = nVersion;
  }

  if (fFileBacked)
  {
    __wx__DB* pwalletdb = pwalletdbIn ? pwalletdbIn : new __wx__DB(strWalletFile);
    if (nWalletVersion > 40000)
    {
      pwalletdb->WriteMinVersion(nWalletVersion);
    }
    if (!pwalletdbIn)
    {
      delete pwalletdb;
    }
  }

  return true;
}

bool __wx__::SetMaxVersion(int nVersion)
{
  LOCK(cs_wallet);

  if (nWalletVersion > nVersion)
  {
    return false;
  }

  nWalletMaxVersion = nVersion;

  return true;
}

bool __wx__::EncryptWallet(const SecureString& strWalletPassphrase)
{
  if (IsCrypted())
  {
    return false;
  }

  CKeyingMaterial vMasterKey;
  RandAddSeedPerfmon();

  vMasterKey.resize(WALLET_CRYPTO_KEY_SIZE);
  RAND_bytes(&vMasterKey[0], WALLET_CRYPTO_KEY_SIZE);

  CMasterKey kMasterKey(nDerivationMethodIndex);

  RandAddSeedPerfmon();
  kMasterKey.vchSalt.resize(WALLET_CRYPTO_SALT_SIZE);
  RAND_bytes(&kMasterKey.vchSalt[0], WALLET_CRYPTO_SALT_SIZE);

  CCrypter crypter;
  int64_t nStartTime = GetTimeMillis();
  crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, 25000, kMasterKey.nDerivationMethod);
  kMasterKey.nDeriveIterations = 2500000 / ((double)(GetTimeMillis() - nStartTime));

  nStartTime = GetTimeMillis();
  crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod);
  kMasterKey.nDeriveIterations = (kMasterKey.nDeriveIterations + kMasterKey.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

  if (kMasterKey.nDeriveIterations < 25000)
  {
    kMasterKey.nDeriveIterations = 25000;
  }

  printf("Encrypting Wallet with an nDeriveIterations of %i\n", kMasterKey.nDeriveIterations);

  if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod))
  {
    return false;
  }
  if (!crypter.Encrypt(vMasterKey, kMasterKey.vchCryptedKey))
  {
    return false;
  }


  {
    LOCK(cs_wallet);
    mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;
    if (fFileBacked)
    {
      pwalletdbEncryption = new __wx__DB(strWalletFile);
      if (!pwalletdbEncryption->TxnBegin())
      {
        return false;
      }
      pwalletdbEncryption->WriteMasterKey(nMasterKeyMaxID, kMasterKey);
    }

    if (!EncryptKeys(vMasterKey))
    {
      if (fFileBacked)
      {
        pwalletdbEncryption->TxnAbort();
      }
      exit(1);
    }


    SetMinVersion(FEATURE_WALLETCRYPT, pwalletdbEncryption, true);

    if (fFileBacked)
    {
      if (!pwalletdbEncryption->TxnCommit())
      {
        exit(1);
      }

      delete pwalletdbEncryption;
      pwalletdbEncryption = NULL;
    }

    Lock();
    Unlock(strWalletPassphrase);
    NewKeyPool();
    Lock();



    CDB::Rewrite(strWalletFile);

  }
  NotifyStatusChanged(this);

  return true;
}

int64_t __wx__::IncOrderPosNext(__wx__DB *pwalletdb)
{
  AssertLockHeld(cs_wallet);
  int64_t nRet = nOrderPosNext++;
  if (pwalletdb)
  {
    pwalletdb->WriteOrderPosNext(nOrderPosNext);
  }
  else
  {
    __wx__DB(strWalletFile).WriteOrderPosNext(nOrderPosNext);
  }
  return nRet;
}

__wx__::TxItems __wx__::OrderedTxItems(std::list<CAccountingEntry>& acentries, std::string strAccount)
{
  AssertLockHeld(cs_wallet);
  __wx__DB walletdb(strWalletFile);


  TxItems txOrdered;



  for (map<uint256, __wx__Tx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
  {
    __wx__Tx* wtx = &((*it).second);
    txOrdered.insert(make_pair(wtx->nOrderPos, TxPair(wtx, (CAccountingEntry*)0)));
  }
  acentries.clear();
  walletdb.ListAccountCreditDebit(strAccount, acentries);
  BOOST_FOREACH(CAccountingEntry& entry, acentries)
  {
    txOrdered.insert(make_pair(entry.nOrderPos, TxPair((__wx__Tx*)0, &entry)));
  }

  return txOrdered;
}

void __wx__::WalletUpdateSpent(const CTransaction &tx, bool fBlock)
{



  {
    LOCK(cs_wallet);
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
    {
      map<uint256, __wx__Tx>::iterator mi = mapWallet.find(txin.prevout.hash);
      if (mi != mapWallet.end())
      {
        __wx__Tx& wtx = (*mi).second;
        if (txin.prevout.n >= wtx.vout.size())
        {
          printf("WalletUpdateSpent: bad wtx %s\n", wtx.GetHash().ToString().c_str());
        }
        else if (!wtx.IsSpent(txin.prevout.n) && IsMine(wtx.vout[txin.prevout.n]))
        {
          printf("WalletUpdateSpent found spent coin %s IO %s\n", FormatMoney(wtx.GetCredit()).c_str(), wtx.GetHash().ToString().c_str());
          wtx.MarkSpent(txin.prevout.n);
          wtx.WriteToDisk();
          NotifyTransactionChanged(this, txin.prevout.hash, CT_UPDATED);
        }
      }
    }

    if (fBlock)
    {
      uint256 hash = tx.GetHash();
      map<uint256, __wx__Tx>::iterator mi = mapWallet.find(hash);
      __wx__Tx& wtx = (*mi).second;

      BOOST_FOREACH(const CTxOut& txout, tx.vout)
      {
        if (IsMine(txout))
        {
          wtx.MarkUnspent(&txout - &tx.vout[0]);
          wtx.WriteToDisk();
          NotifyTransactionChanged(this, hash, CT_UPDATED);
        }
      }
    }

  }
}

void __wx__::MarkDirty()
{
  {
    LOCK(cs_wallet);
    BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item, mapWallet)
    item.second.MarkDirty();
  }
}

bool __wx__::AddToWallet(const __wx__Tx& wtxIn)
{
  uint256 hash = wtxIn.GetHash();
  {
    LOCK(cs_wallet);

    pair<map<uint256, __wx__Tx>::iterator, bool> ret = mapWallet.insert(make_pair(hash, wtxIn));
    __wx__Tx& wtx = (*ret.first).second;
    wtx.BindWallet(this);
    bool fInsertedNew = ret.second;
    if (fInsertedNew)
    {
      wtx.nTimeReceived = GetAdjustedTime();
      wtx.nOrderPos = IncOrderPosNext();

      wtx.nTimeSmart = wtx.nTimeReceived;
      if (wtxIn.hashBlock != 0)
      {
        if (mapBlockIndex.count(wtxIn.hashBlock))
        {
          unsigned int latestNow = wtx.nTimeReceived;
          unsigned int latestEntry = 0;
          {

            int64_t latestTolerated = latestNow + 300;
            std::list<CAccountingEntry> acentries;
            TxItems txOrdered = OrderedTxItems(acentries);
            for (TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
            {
              __wx__Tx *const pwtx = (*it).second.first;
              if (pwtx == &wtx)
              {
                continue;
              }
              CAccountingEntry *const pacentry = (*it).second.second;
              int64_t nSmartTime;
              if (pwtx)
              {
                nSmartTime = pwtx->nTimeSmart;
                if (!nSmartTime)
                {
                  nSmartTime = pwtx->nTimeReceived;
                }
              }
              else
              {
                nSmartTime = pacentry->nTime;
              }
              if (nSmartTime <= latestTolerated)
              {
                latestEntry = nSmartTime;
                if (nSmartTime > latestNow)
                {
                  latestNow = nSmartTime;
                }
                break;
              }
            }
          }

          unsigned int& blocktime = mapBlockIndex[wtxIn.hashBlock]->nTime;
          wtx.nTimeSmart = std::max(latestEntry, std::min(blocktime, latestNow));
        }
        else
          printf("AddToWallet() : found %s in block %s not in index\n",
                 wtxIn.GetHash().ToString().substr(0,10).c_str(),
                 wtxIn.hashBlock.ToString().c_str());
      }
    }

    bool fUpdated = false;
    if (!fInsertedNew)
    {

      if (wtxIn.hashBlock != 0 && wtxIn.hashBlock != wtx.hashBlock)
      {
        wtx.hashBlock = wtxIn.hashBlock;
        fUpdated = true;
      }
      if (wtxIn.nIndex != -1 && (wtxIn.vMerkleBranch != wtx.vMerkleBranch || wtxIn.nIndex != wtx.nIndex))
      {
        wtx.vMerkleBranch = wtxIn.vMerkleBranch;
        wtx.nIndex = wtxIn.nIndex;
        fUpdated = true;
      }
      if (wtxIn.fFromMe && wtxIn.fFromMe != wtx.fFromMe)
      {
        wtx.fFromMe = wtxIn.fFromMe;
        fUpdated = true;
      }
      fUpdated |= wtx.UpdateSpent(wtxIn.vfSpent);
    }


    printf("AddToWallet %s  %s%s\n", wtxIn.GetHash().ToString().substr(0,10).c_str(), (fInsertedNew ? "new" : ""), (fUpdated ? "update" : ""));


    if (fInsertedNew || fUpdated)
      if (!wtx.WriteToDisk())
      {
        return false;
      }
#ifndef QT_GUI

    if (vchDefaultKey.IsValid())
    {
      CScript scriptDefaultKey;
      scriptDefaultKey.SetDestination(vchDefaultKey.GetID());
      BOOST_FOREACH(const CTxOut& txout, wtx.vout)
      {
        if (txout.scriptPubKey == scriptDefaultKey)
        {
          CPubKey newDefaultKey;
          if(GetKeyFromPool(newDefaultKey, false))
          {
            SetDefaultKey(newDefaultKey);
            SetAddressBookName(vchDefaultKey.GetID(), "");
          }
        }
      }
    }
#endif

    WalletUpdateSpent(wtx, (wtxIn.hashBlock != 0));


    NotifyTransactionChanged(this, hash, fInsertedNew ? CT_NEW : CT_UPDATED);


    std::string strCmd = GetArg("-walletnotify", "");

    if ( !strCmd.empty())
    {
      boost::replace_all(strCmd, "%s", wtxIn.GetHash().GetHex());
      boost::thread t(runCommand, strCmd);
    }

  }
  return true;
}




bool __wx__::AddToWalletIfInvolvingMe(const CTransaction& tx, const CBlock* pblock, bool fUpdate, bool fFindBlock)
{
  uint256 hash = tx.GetHash();
  {
    LOCK(cs_wallet);
    bool fExisted = mapWallet.count(hash);
    if (fExisted && !fUpdate)
    {
      return false;
    }
    if (fExisted || IsMine(tx) || IsFromMe(tx) || collx(tx))
    {
      __wx__Tx wtx(this,tx);

      if (pblock)
      {
        wtx.SetMerkleBranch(pblock);
      }
      return AddToWallet(wtx);
    }
    else
    {
      WalletUpdateSpent(tx);
    }
  }
  return false;
}

bool __wx__::EraseFromWallet(uint256 hash)
{
  if (!fFileBacked)
  {
    return false;
  }
  {
    LOCK(cs_wallet);
    if (mapWallet.erase(hash))
    {
      __wx__DB(strWalletFile).EraseTx(hash);
    }
  }
  return true;
}


bool __wx__::IsMine(const CTxIn &txin) const
{
  {
    LOCK(cs_wallet);
    map<uint256, __wx__Tx>::const_iterator mi = mapWallet.find(txin.prevout.hash);
    if (mi != mapWallet.end())
    {
      const __wx__Tx& prev = (*mi).second;
      if (txin.prevout.n < prev.vout.size())
        if (IsMine(prev.vout[txin.prevout.n]))
        {
          return true;
        }
    }
  }
  return false;
}

int64_t __wx__::GetDebit(const CTxIn &txin) const
{
  {
    LOCK(cs_wallet);
    map<uint256, __wx__Tx>::const_iterator mi = mapWallet.find(txin.prevout.hash);
    if (mi != mapWallet.end())
    {
      const __wx__Tx& prev = (*mi).second;
      if (txin.prevout.n < prev.vout.size())
        if (IsMine(prev.vout[txin.prevout.n]))
        {
          return prev.vout[txin.prevout.n].nValue;
        }
    }
  }
  return 0;
}

bool __wx__::IsChange(const CTxOut& txout) const
{
  CTxDestination address;
# 1024 "wallet.cpp"
  if (ExtractDestination(txout.scriptPubKey, address) && ::IsMine(*this, address))
  {
    LOCK(cs_wallet);
    if (!mapAddressBook.count(address))
    {
      return true;
    }
  }
  return false;
}

int64_t __wx__Tx::GetTxTime() const
{
  int64_t n = nTimeSmart;
  return n ? n : nTimeReceived;
}

int __wx__Tx::GetRequestCount() const
{

  int nRequests = -1;
  {
    LOCK(pwallet->cs_wallet);
    if (IsCoinBase() || IsCoinStake())
    {

      if (hashBlock != 0)
      {
        map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
        if (mi != pwallet->mapRequestCount.end())
        {
          nRequests = (*mi).second;
        }
      }
    }
    else
    {

      map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(GetHash());
      if (mi != pwallet->mapRequestCount.end())
      {
        nRequests = (*mi).second;


        if (nRequests == 0 && hashBlock != 0)
        {
          map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
          if (mi != pwallet->mapRequestCount.end())
          {
            nRequests = (*mi).second;
          }
          else
          {
            nRequests = 1;
          }
        }
      }
    }
  }
  return nRequests;
}
void __wx__Tx::GetAmounts(list<pair<CTxDestination, int64_t> >& listReceived,
                          list<pair<CTxDestination, int64_t> >& listSent, int64_t& nFee, string& strSentAccount) const
{
  nFee = 0;
  listReceived.clear();
  listSent.clear();
  strSentAccount = strFromAccount;


  int64_t nDebit = GetDebit();
  if (nDebit > 0)
  {
    int64_t nValueOut = GetValueOut();
    nFee = nDebit - nValueOut;
  }


  BOOST_FOREACH(const CTxOut& txout, vout)
  {

    if (txout.scriptPubKey.empty())
    {
      continue;
    }

    vector<valtype> vs;
    txnouttype t;
    if(Solver(txout.scriptPubKey, t, vs))
    {
      if(t == TX_NULL_DATA)
      {
        continue;
      }
    }

    bool fIsMine;



    if (nDebit > 0)
    {

      if (pwallet->IsChange(txout))
      {
        continue;
      }
      fIsMine = pwallet->IsMine(txout);
    }
    else if (!(fIsMine = pwallet->IsMine(txout)))
    {
      continue;
    }


    CTxDestination address;

    std::vector<vchType> vvchPrevArgsRead;
    int prevOp;
    if(aliasScript(txout.scriptPubKey, prevOp, vvchPrevArgsRead))
    {
      const CScript& s1_ = aliasStrip(txout.scriptPubKey);
      if (!ExtractDestination(s1_, address))
      {
        printf("__wx__Tx::GetAmounts: Unknown transaction type found, txid %s\n",
               this->GetHash().ToString().c_str());
        address = CNoDestination();
      }
    }
    else if (!ExtractDestination(txout.scriptPubKey, address))
    {
      printf("__wx__Tx::GetAmounts: Unknown transaction type found, txid %s\n",
             this->GetHash().ToString().c_str());
      address = CNoDestination();
    }


    if (nDebit > 0)
    {
      listSent.push_back(make_pair(address, txout.nValue));
    }


    if(fIsMine)
    {
      listReceived.push_back(make_pair(address, txout.nValue));
    }
  }

}

void __wx__Tx::GetAccountAmounts(const string& strAccount, int64_t& nReceived,
                                 int64_t& nSent, int64_t& nFee) const
{
  nReceived = nSent = nFee = 0;

  int64_t allFee;
  string strSentAccount;
  list<pair<CTxDestination, int64_t> > listReceived;
  list<pair<CTxDestination, int64_t> > listSent;
  GetAmounts(listReceived, listSent, allFee, strSentAccount);

  if (strAccount == strSentAccount)
  {
    BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64_t)& s, listSent)
    nSent += s.second;
    nFee = allFee;
  }
  {
    LOCK(pwallet->cs_wallet);
    BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64_t)& r, listReceived)
    {
      if (pwallet->mapAddressBook.count(r.first))
      {
        map<CTxDestination, string>::const_iterator mi = pwallet->mapAddressBook.find(r.first);
        if (mi != pwallet->mapAddressBook.end() && (*mi).second == strAccount)
        {
          nReceived += r.second;
        }
      }
      else if (strAccount.empty())
      {
        nReceived += r.second;
      }
    }
  }
}

void __wx__Tx::AddSupportingTransactions(CTxDB& txdb)
{
  vtxPrev.clear();

  const int COPY_DEPTH = 3;
  if (SetMerkleBranch() < COPY_DEPTH)
  {
    vector<uint256> vWorkQueue;
    BOOST_FOREACH(const CTxIn& txin, vin)
    vWorkQueue.push_back(txin.prevout.hash);


    {
      LOCK(pwallet->cs_wallet);
      map<uint256, const CMerkleTx*> mapWalletPrev;
      set<uint256> setAlreadyDone;
      for (unsigned int i = 0; i < vWorkQueue.size(); i++)
      {
        uint256 hash = vWorkQueue[i];
        if (setAlreadyDone.count(hash))
        {
          continue;
        }
        setAlreadyDone.insert(hash);

        CMerkleTx tx;
        map<uint256, __wx__Tx>::const_iterator mi = pwallet->mapWallet.find(hash);
        if (mi != pwallet->mapWallet.end())
        {
          tx = (*mi).second;
          BOOST_FOREACH(const CMerkleTx& txWalletPrev, (*mi).second.vtxPrev)
          mapWalletPrev[txWalletPrev.GetHash()] = &txWalletPrev;
        }
        else if (mapWalletPrev.count(hash))
        {
          tx = *mapWalletPrev[hash];
        }
        else if (txdb.ReadDiskTx(hash, tx))
        {
          ;
        }
        else
        {
          printf("ERROR: AddSupportingTransactions() : unsupported transaction\n");
          continue;
        }

        int nDepth = tx.SetMerkleBranch();
        vtxPrev.push_back(tx);

        if (nDepth < COPY_DEPTH)
        {
          BOOST_FOREACH(const CTxIn& txin, tx.vin)
          vWorkQueue.push_back(txin.prevout.hash);
        }
      }
    }
  }

  reverse(vtxPrev.begin(), vtxPrev.end());
}

bool __wx__Tx::WriteToDisk()
{
  return __wx__DB(pwallet->strWalletFile).WriteTx(GetHash(), *this);
}




int __wx__::ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate)
{
  int ret = 0;

  CBlockIndex* pindex = pindexStart;
  {
    LOCK2(cs_main, cs_wallet);
    while (pindex)
    {


      if (nTimeFirstKey && (pindex->nTime < (nTimeFirstKey - 7200)))
      {
        pindex = pindex->pnext;
        continue;
      }

      CBlock block;
      block.ReadFromDisk(pindex, true);
      BOOST_FOREACH(CTransaction& tx, block.vtx)
      {
        if (AddToWalletIfInvolvingMe(tx, &block, fUpdate))
        {
          ret++;
        }
      }
      pindex = pindex->pnext;
    }
  }
  return ret;
}

void __wx__::ReacceptWalletTransactions()
{
  CTxDB txdb("r");
  bool fRepeat = true;
  while (fRepeat)
  {
    LOCK2(cs_main, cs_wallet);
    fRepeat = false;
    vector<CDiskTxPos> vMissingTx;
    BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item, mapWallet)
    {
      __wx__Tx& wtx = item.second;
      if ((wtx.IsCoinBase() && wtx.IsSpent(0)) || (wtx.IsCoinStake() && wtx.IsSpent(1)))
      {
        continue;
      }

      CTxIndex txindex;
      bool fUpdated = false;
      if (txdb.ReadTxIndex(wtx.GetHash(), txindex))
      {

        if (txindex.vSpent.size() != wtx.vout.size())
        {
          printf("ERROR: ReacceptWalletTransactions() : txindex.vSpent.size() %" PRIszu " != wtx.vout.size() %" PRIszu "\n", txindex.vSpent.size(), wtx.vout.size());
          continue;
        }
        for (unsigned int i = 0; i < txindex.vSpent.size(); i++)
        {
          if (wtx.IsSpent(i))
          {
            continue;
          }
          if (!txindex.vSpent[i].IsNull() && IsMine(wtx.vout[i]))
          {
            wtx.MarkSpent(i);
            fUpdated = true;
            vMissingTx.push_back(txindex.vSpent[i]);
          }
        }
        if (fUpdated)
        {
          printf("ReacceptWalletTransactions found spent coin %s IO %s\n", FormatMoney(wtx.GetCredit()).c_str(), wtx.GetHash().ToString().c_str());
          wtx.MarkDirty();
          wtx.WriteToDisk();
        }
      }
      else
      {

        if (!(wtx.IsCoinBase() || wtx.IsCoinStake()))
        {
          wtx.AcceptWalletTransaction(txdb);
        }
      }
    }
    if (!vMissingTx.empty())
    {

      if (ScanForWalletTransactions(pindexGenesisBlock))
      {
        fRepeat = true;
      }
    }
  }
}

void __wx__Tx::RelayWalletTransaction(CTxDB& txdb)
{
  BOOST_FOREACH(const CMerkleTx& tx, vtxPrev)
  {
    if (!(tx.IsCoinBase() || tx.IsCoinStake()))
    {
      uint256 hash = tx.GetHash();
      if (!txdb.ContainsTx(hash))
      {
        RelayTransaction((CTransaction)tx, hash);
      }
    }
  }
  if (!(IsCoinBase() || IsCoinStake()))
  {
    uint256 hash = GetHash();
    if (!txdb.ContainsTx(hash))
    {
      printf("Relaying wtx %s\n", hash.ToString().substr(0,10).c_str());
      RelayTransaction((CTransaction)*this, hash);
    }
  }
}

void __wx__Tx::RelayWalletTransaction()
{
  CTxDB txdb("r");
  RelayWalletTransaction(txdb);
}

void __wx__::ResendWalletTransactions(bool fForce)
{
  if (!fForce)
  {


    static int64_t nNextTime;
    if (GetTime() < nNextTime)
    {
      return;
    }
    bool fFirst = (nNextTime == 0);
    nNextTime = GetTime() + GetRand(30 * 60);
    if (fFirst)
    {
      return;
    }


    static int64_t nLastTime;
    if (nTimeBestReceived < nLastTime)
    {
      return;
    }
    nLastTime = GetTime();
  }


  printf("ResendWalletTransactions()\n");
  CTxDB txdb("r");
  {
    LOCK(cs_wallet);

    multimap<unsigned int, __wx__Tx*> mapSorted;
    BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item, mapWallet)
    {
      __wx__Tx& wtx = item.second;


      if (fForce || nTimeBestReceived - (int64_t)wtx.nTimeReceived > 5 * 60)
      {
        mapSorted.insert(make_pair(wtx.nTimeReceived, &wtx));
      }
    }
    BOOST_FOREACH(PAIRTYPE(const unsigned int, __wx__Tx*)& item, mapSorted)
    {
      __wx__Tx& wtx = *item.second;
      if (wtx.CheckTransaction())
      {
        wtx.RelayWalletTransaction(txdb);
      }
      else
      {
        printf("ResendWalletTransactions() : CheckTransaction failed for transaction %s\n", wtx.GetHash().ToString().c_str());
      }
    }
  }
}
# 1481 "wallet.cpp"
int64_t __wx__::GetBalance() const
{
  int64_t nTotal = 0;
  {
    LOCK2(cs_main, cs_wallet);
    for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
      const __wx__Tx* pcoin = &(*it).second;
      if (pcoin->IsTrusted())
      {
        nTotal += pcoin->GetAvailableCredit();
      }
    }
  }

  return nTotal;
}

int64_t __wx__::GetUnconfirmedBalance() const
{
  int64_t nTotal = 0;
  {
    LOCK2(cs_main, cs_wallet);
    for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
      const __wx__Tx* pcoin = &(*it).second;
      if (!IsFinalTx(*pcoin) || (!pcoin->IsTrusted() && pcoin->GetDepthInMainChain() == 0))
      {
        nTotal += pcoin->GetAvailableCredit();
      }
    }
  }
  return nTotal;
}

int64_t __wx__::GetImmatureBalance() const
{
  int64_t nTotal = 0;
  {
    LOCK2(cs_main, cs_wallet);
    for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
      const __wx__Tx& pcoin = (*it).second;
      if (pcoin.IsCoinBase() && pcoin.GetBlocksToMaturity() > 0 && pcoin.IsInMainChain())
      {
        nTotal += GetCredit(pcoin);
      }
    }
  }
  return nTotal;
}

bool isAliasTx(const __wx__Tx* tx)
{
  std::vector<vchType> vvch;
  int n;
  int op;
  if (aliasTx(*tx, op, n, vvch))
  {
    switch (op)
    {
    case OP_ALIAS_SET:
    case OP_ALIAS_RELAY:
    case OP_ALIAS_ENCRYPTED:
      return true;
    }
  }

  return false;

}


void __wx__::AvailableCoins(vector<COutput>& vCoins, bool fOnlyConfirmed, const CCoinControl *coinControl) const
{
  vCoins.clear();

  {
    LOCK2(cs_main, cs_wallet);
    for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
      const __wx__Tx* pcoin = &(*it).second;

      if (!IsFinalTx(*pcoin))
      {
        continue;
      }

      if (fOnlyConfirmed && !pcoin->IsTrusted())
      {
        continue;
      }

      if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0)
      {
        continue;
      }

      if(pcoin->IsCoinStake() && pcoin->GetBlocksToMaturity() > 0)
      {
        continue;
      }

      int nDepth = pcoin->GetDepthInMainChain();
      if (nDepth < 0)
      {
        continue;
      }

      for (unsigned int i = 0; i < pcoin->vout.size(); i++)
      {
        if (!(pcoin->IsSpent(i)) && IsMine(pcoin->vout[i]) && pcoin->vout[i].nValue > nMinimumInputValue &&
            (!coinControl || !coinControl->HasSelected() || coinControl->IsSelected((*it).first, i)))
        {

          vCoins.push_back(COutput(pcoin, i, nDepth));
        }
      }

    }
  }
}

void __wx__::AvailableCoinsForStaking(vector<COutput>& vCoins, unsigned int nSpendTime) const
{
  vCoins.clear();

  {
    LOCK2(cs_main, cs_wallet);
    for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
      const __wx__Tx* pcoin = &(*it).second;


      if (pcoin->GetBlocksToMaturity() > 0)
      {
        continue;
      }

      int nDepth = pcoin->GetDepthInMainChain();
      if (nDepth < 1)
      {
        continue;
      }

      if (nDepth < nStakeMinConfirmations)
      {
        continue;
      }


      for (unsigned int i = 0; i < pcoin->vout.size(); i++)
      {
        if (!(pcoin->IsSpent(i)) && IsMine(pcoin->vout[i]) && pcoin->vout[i].nValue > nMinimumInputValue)
        {
          vCoins.push_back(COutput(pcoin, i, nDepth));
        }
      }
    }
  }
}

static void ApproximateBestSubset(vector<pair<int64_t, pair<const __wx__Tx*,unsigned int> > >vValue, int64_t nTotalLower, int64_t nTargetValue,
                                  vector<char>& vfBest, int64_t& nBest, int iterations = 1000)
{
  vector<char> vfIncluded;

  vfBest.assign(vValue.size(), true);
  nBest = nTotalLower;

  seed_insecure_rand();

  for (int nRep = 0; nRep < iterations && nBest != nTargetValue; nRep++)
  {
    vfIncluded.assign(vValue.size(), false);
    int64_t nTotal = 0;
    bool fReachedTarget = false;
    for (int nPass = 0; nPass < 2 && !fReachedTarget; nPass++)
    {
      for (unsigned int i = 0; i < vValue.size(); i++)
      {






        if (nPass == 0 ? insecure_rand()&1 : !vfIncluded[i])
        {
          nTotal += vValue[i].first;
          vfIncluded[i] = true;
          if (nTotal >= nTargetValue)
          {
            fReachedTarget = true;
            if (nTotal < nBest)
            {
              nBest = nTotal;
              vfBest = vfIncluded;
            }
            nTotal -= vValue[i].first;
            vfIncluded[i] = false;
          }
        }
      }
    }
  }
}


int64_t __wx__::GetStake() const
{
  int64_t nTotal = 0;
  LOCK2(cs_main, cs_wallet);
  for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
  {
    const __wx__Tx* pcoin = &(*it).second;
    if (pcoin->IsCoinStake() && pcoin->GetBlocksToMaturity() > 0 && pcoin->GetDepthInMainChain() > 0)
    {
      nTotal += __wx__::GetCredit(*pcoin);
    }
  }
  return nTotal;
}

int64_t __wx__::GetNewMint() const
{
  int64_t nTotal = 0;
  LOCK2(cs_main, cs_wallet);
  for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
  {
    const __wx__Tx* pcoin = &(*it).second;
    if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0 && pcoin->GetDepthInMainChain() > 0)
    {
      nTotal += __wx__::GetCredit(*pcoin);
    }
  }
  return nTotal;
}

bool __wx__::SelectCoinsMinConf(int64_t nTargetValue, unsigned int nSpendTime, int nConfMine, int nConfTheirs, vector<COutput> vCoins, set<pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet) const
{
  setCoinsRet.clear();
  nValueRet = 0;

  printf("SelectCoins vCoins.size %lu\n", vCoins.size());


  pair<int64_t, pair<const __wx__Tx*,unsigned int> > coinLowestLarger;
  coinLowestLarger.first = std::numeric_limits<int64_t>::max();
  coinLowestLarger.second.first = NULL;
  vector<pair<int64_t, pair<const __wx__Tx*,unsigned int> > > vValue;
  int64_t nTotalLower = 0;

  random_shuffle(vCoins.begin(), vCoins.end(), GetRandInt);

  BOOST_FOREACH(COutput output, vCoins)
  {
    const __wx__Tx *pcoin = output.tx;

    if (output.nDepth < (pcoin->IsFromMe() ? nConfMine : nConfTheirs))
    {
      continue;
    }

    int i = output.i;

    if (pcoin->nTime > nSpendTime)
    {
      printf("pcoin->nTime %u, nSpendTime %u\n", pcoin->nTime, nSpendTime);
      continue;
    }

    int64_t n = pcoin->vout[i].nValue;

    pair<int64_t,pair<const __wx__Tx*,unsigned int> > coin = make_pair(n,make_pair(pcoin, i));

    if (n == nTargetValue)
    {
      setCoinsRet.insert(coin.second);
      nValueRet += coin.first;
      return true;
    }
    else if (n < nTargetValue + CENT)
    {
      vValue.push_back(coin);
      nTotalLower += n;
    }
    else if (n < coinLowestLarger.first)
    {
      coinLowestLarger = coin;
    }
  }

  if (nTotalLower == nTargetValue)
  {
    for (unsigned int i = 0; i < vValue.size(); ++i)
    {
      setCoinsRet.insert(vValue[i].second);
      nValueRet += vValue[i].first;
    }
    return true;
  }

  if (nTotalLower < nTargetValue)
  {
    if (coinLowestLarger.second.first == NULL)
    {
      return false;
    }

    setCoinsRet.insert(coinLowestLarger.second);
    nValueRet += coinLowestLarger.first;
    return true;
  }


  sort(vValue.rbegin(), vValue.rend(), CompareValueOnly());
  vector<char> vfBest;
  int64_t nBest;

  ApproximateBestSubset(vValue, nTotalLower, nTargetValue, vfBest, nBest, 1000);
  if (nBest != nTargetValue && nTotalLower >= nTargetValue + CENT)
  {
    ApproximateBestSubset(vValue, nTotalLower, nTargetValue + CENT, vfBest, nBest, 1000);
  }



  if (coinLowestLarger.second.first &&
      ((nBest != nTargetValue && nBest < nTargetValue + CENT) || coinLowestLarger.first <= nBest))
  {
    setCoinsRet.insert(coinLowestLarger.second);
    nValueRet += coinLowestLarger.first;
  }
  else
  {
    for (unsigned int i = 0; i < vValue.size(); i++)
      if (vfBest[i])
      {
        setCoinsRet.insert(vValue[i].second);
        nValueRet += vValue[i].first;
      }

    if (fDebug && GetBoolArg("-printpriority"))
    {

      printf("SelectCoins() best subset: ");
      for (unsigned int i = 0; i < vValue.size(); i++)
        if (vfBest[i])
        {
          printf("%s ", FormatMoney(vValue[i].first).c_str());
        }
      printf("total %s\n", FormatMoney(nBest).c_str());
    }
  }

  return true;
}

bool __wx__::SelectCoins(int64_t nTargetValue, unsigned int nSpendTime, set<pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet, const CCoinControl* coinControl) const
{
  vector<COutput> vCoins;
  AvailableCoins(vCoins, true, coinControl);


  if (coinControl && coinControl->HasSelected())
  {
    printf("SelectCoins coinControl\n");
    BOOST_FOREACH(const COutput& out, vCoins)
    {
      nValueRet += out.tx->vout[out.i].nValue;
      setCoinsRet.insert(make_pair(out.tx, out.i));
    }
    return (nValueRet >= nTargetValue);
  }

  return (SelectCoinsMinConf(nTargetValue, nSpendTime, 1, 10, vCoins, setCoinsRet, nValueRet) ||
          SelectCoinsMinConf(nTargetValue, nSpendTime, 1, 1, vCoins, setCoinsRet, nValueRet) ||
          SelectCoinsMinConf(nTargetValue, nSpendTime, 0, 1, vCoins, setCoinsRet, nValueRet));
}


bool __wx__::SelectCoinsForStaking(int64_t nTargetValue, unsigned int nSpendTime, set<pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet) const
{
  vector<COutput> vCoins;
  AvailableCoinsForStaking(vCoins, nSpendTime);

  random_shuffle(vCoins.begin(), vCoins.end(), GetRandInt);

  setCoinsRet.clear();
  nValueRet = 0;

  BOOST_FOREACH(COutput output, vCoins)
  {
    const __wx__Tx *pcoin = output.tx;
    int i = output.i;


    if (nValueRet >= nTargetValue)
    {
      break;
    }

    int64_t n = pcoin->vout[i].nValue;

    pair<int64_t,pair<const __wx__Tx*,unsigned int> > coin = make_pair(n,make_pair(pcoin, i));

    if (n >= nTargetValue)
    {


      setCoinsRet.insert(coin.second);
      nValueRet += coin.first;
      break;
    }
    else if (n < nTargetValue + CENT)
    {
      setCoinsRet.insert(coin.second);
      nValueRet += coin.first;
    }
  }

  return true;
}

bool __wx__::CreateTransaction__(const vector<pair<CScript, int64_t> >& vecSend, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl* coinControl)
{
  int64_t nValue = 0;
  BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, vecSend)
  {
    if (nValue < 0)
    {
      return false;
    }
    nValue += s.second;
  }
  if (vecSend.empty() || nValue < 0)
  {
    return false;
  }

  wtxNew.BindWallet(this);
  wtxNew.strTxInfo = strTxInfo;
  if (wtxNew.strTxInfo.length() > MAX_TX_INFO_LEN)
  {
    wtxNew.strTxInfo.resize(MAX_TX_INFO_LEN);
  }
  if (wtxNew.strTxInfo.length()>0)
  {
    wtxNew.nVersion=3;
  }
  {
    LOCK2(cs_main, cs_wallet);

    CTxDB txdb("r");
    {
      nFeeRet = CENT;
      while (true)
      {
        wtxNew.vin.clear();
        wtxNew.vout.clear();
        wtxNew.fFromMe = true;

        int64_t nTotalValue = nValue + nFeeRet;
        double dPriority = 0;

        BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, vecSend)
        wtxNew.vout.push_back(CTxOut(s.second, s.first));


        set<pair<const __wx__Tx*,unsigned int> > setCoins;
        int64_t nValueIn = 0;
        if (!SelectCoins(nTotalValue, wtxNew.nTime, setCoins, nValueIn, coinControl))
        {
          return false;
        }
        BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
        {
          int64_t nCredit = pcoin.first->vout[pcoin.second].nValue;
          dPriority += (double)nCredit * pcoin.first->GetDepthInMainChain();
        }

        int64_t nChange = nValueIn - nValue - nFeeRet;



        if (nFeeRet < MIN_TX_FEE && nChange > 0 && nChange < CENT)
        {
          int64_t nMoveToFee = min(nChange, MIN_TX_FEE - nFeeRet);
          nChange -= nMoveToFee;
          nFeeRet += nMoveToFee;
        }

        if (nChange > 0)
        {



          CScript scriptChange;


          if (coinControl && !boost::get<CNoDestination>(&coinControl->destChange))
          {
            scriptChange.SetDestination(coinControl->destChange);
          }


          else
          {
# 1998 "wallet.cpp"
            CPubKey vchPubKey;
            assert(reservekey.GetReservedKey(vchPubKey));

            scriptChange.SetDestination(vchPubKey.GetID());
          }


          vector<CTxOut>::iterator position = wtxNew.vout.begin()+GetRandInt(wtxNew.vout.size());
          wtxNew.vout.insert(position, CTxOut(nChange, scriptChange));
        }
        else
        {
          reservekey.ReturnKey();
        }


        BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
        wtxNew.vin.push_back(CTxIn(coin.first->GetHash(),coin.second));


        int nIn = 0;
        BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
        if (!SignSignature(*this, *coin.first, wtxNew, nIn++))
        {
          return false;
        }


        unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK, PROTOCOL_VERSION);
        if (nBytes >= MAX_STANDARD_TX_SIZE)
        {
          return false;
        }
        dPriority /= nBytes;


        int64_t nPayFee = nTransactionFee * (1 + (int64_t)nBytes / 1000);
        int64_t nMinFee = wtxNew.GetMinFee(1, GMF_SEND, nBytes);

        if (nFeeRet < max(nPayFee, nMinFee))
        {
          nFeeRet = max(nPayFee, nMinFee);
          continue;
        }


        wtxNew.AddSupportingTransactions(txdb);
        wtxNew.fTimeReceivedIsTxTime = true;

        break;
      }
    }
  }
  return true;
}

bool __wx__::CreateTransaction(const vector<pair<CScript, int64_t> >& vecSend, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl* coinControl)
{
  int64_t nValue = 0;
  BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, vecSend)
  {
    if (nValue < 0)
    {
      return false;
    }
    nValue += s.second;
  }
  if (vecSend.empty() || nValue < 0)
  {
    return false;
  }

  wtxNew.BindWallet(this);
  wtxNew.strTxInfo = strTxInfo;
  if (wtxNew.strTxInfo.length() > MAX_TX_INFO_LEN)
  {
    wtxNew.strTxInfo.resize(MAX_TX_INFO_LEN);
  }
  if (wtxNew.strTxInfo.length()>0)
  {
    wtxNew.nVersion=3;
  }
  {
    LOCK2(cs_main, cs_wallet);

    CTxDB txdb("r");
    {
      nFeeRet = S_MIN_TX_FEE;
      while (true)
      {
        wtxNew.vin.clear();
        wtxNew.vout.clear();
        wtxNew.fFromMe = true;

        int64_t nTotalValue = nValue + nFeeRet;
        double dPriority = 0;

        BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, vecSend)
        wtxNew.vout.push_back(CTxOut(s.second, s.first));


        set<pair<const __wx__Tx*,unsigned int> > setCoins;
        int64_t nValueIn = 0;
        if (!SelectCoins(nTotalValue, wtxNew.nTime, setCoins, nValueIn, coinControl))
        {
          return false;
        }
        BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
        {
          int64_t nCredit = pcoin.first->vout[pcoin.second].nValue;
          dPriority += (double)nCredit * pcoin.first->GetDepthInMainChain();
        }

        int64_t nChange = nValueIn - nValue - nFeeRet;



        if (nFeeRet < MIN_TX_FEE && nChange > 0 && nChange < CENT)
        {
          int64_t nMoveToFee = min(nChange, MIN_TX_FEE - nFeeRet);
          nChange -= nMoveToFee;
          nFeeRet += nMoveToFee;
        }

        if (nChange > 0)
        {



          CScript scriptChange;


          if (coinControl && !boost::get<CNoDestination>(&coinControl->destChange))
          {
            scriptChange.SetDestination(coinControl->destChange);
          }


          else
          {
# 2146 "wallet.cpp"
            CPubKey vchPubKey;
            assert(reservekey.GetReservedKey(vchPubKey));

            scriptChange.SetDestination(vchPubKey.GetID());
          }


          vector<CTxOut>::iterator position = wtxNew.vout.begin()+GetRandInt(wtxNew.vout.size());
          wtxNew.vout.insert(position, CTxOut(nChange, scriptChange));
        }
        else
        {
          reservekey.ReturnKey();
        }


        BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
        wtxNew.vin.push_back(CTxIn(coin.first->GetHash(),coin.second));


        int nIn = 0;
        BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
        {
          if (!SignSignature(*this, *coin.first, wtxNew, nIn++))
          {
            return false;
          }
        }


        unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK, PROTOCOL_VERSION);
        if (nBytes >= MAX_STANDARD_TX_SIZE)
        {
          return false;
        }
        dPriority /= nBytes;


        int64_t nPayFee = nTransactionFee * (1 + (int64_t)nBytes / 1000);
        int64_t nMinFee = wtxNew.GetMinFee(1, GMF_SEND, nBytes);

        if (nFeeRet < max(nPayFee, nMinFee))
        {
          nFeeRet = max(nPayFee, nMinFee);
          continue;
        }


        wtxNew.AddSupportingTransactions(txdb);
        wtxNew.fTimeReceivedIsTxTime = true;

        break;
      }
    }
  }
  return true;
}

bool __wx__::CreateTransaction__(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl* coinControl)
{
  vector< pair<CScript, int64_t> > vecSend;
  vecSend.push_back(make_pair(scriptPubKey, nValue));
  return CreateTransaction__(vecSend, wtxNew, reservekey, nFeeRet, strTxInfo, coinControl);
}

bool __wx__::CreateTransaction(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string strTxInfo, const CCoinControl* coinControl)
{
  vector< pair<CScript, int64_t> > vecSend;
  vecSend.push_back(make_pair(scriptPubKey, nValue));
  return CreateTransaction(vecSend, wtxNew, reservekey, nFeeRet, strTxInfo, coinControl);
}

bool __wx__::GetStakeWeight(uint64_t& nWeight)
{

  int64_t nBalance = GetBalance();

  if (nBalance <= nReserveBalance)
  {
    return false;
  }

  vector<const __wx__Tx*> vwtxPrev;

  set<pair<const __wx__Tx*,unsigned int> > setCoins;
  int64_t nValueIn = 0;

  if (!SelectCoinsForStaking(nBalance - nReserveBalance, GetTime(), setCoins, nValueIn))
  {
    return false;
  }

  if (setCoins.empty())
  {
    return false;
  }

  nWeight = 0;

  CTxDB txdb("r");

  LOCK2(cs_main, cs_wallet);
  BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
  {
    CTxIndex txindex;
    if (!txdb.ReadTxIndex(pcoin.first->GetHash(), txindex))
    {
      continue;
    }

    if(pcoin.first->GetDepthInMainChain() >= nStakeMinConfirmations)
    {
      nWeight += pcoin.first->vout[pcoin.second].nValue;
    }
  }

  return true;
}

bool __wx__::CreateCoinStake(const CKeyStore& keystore, unsigned int nBits, int64_t nSearchInterval, int64_t nFees, CTransaction& txNew, CKey& key, int nHeight)
{
  CBlockIndex* pindexPrev = pindexBest;
  CBigNum bnTargetPerCoinDay;
  bnTargetPerCoinDay.SetCompact(nBits);

  txNew.vin.clear();
  txNew.vout.clear();


  CScript scriptEmpty;
  scriptEmpty.clear();
  txNew.vout.push_back(CTxOut(0, scriptEmpty));


  int64_t nBalance = GetBalance();

  if (nBalance <= nReserveBalance)
  {
    return false;
  }

  vector<const __wx__Tx*> vwtxPrev;

  set<pair<const __wx__Tx*,unsigned int> > setCoins;
  int64_t nValueIn = 0;


  if (!SelectCoinsForStaking(nBalance - nReserveBalance, txNew.nTime, setCoins, nValueIn))
  {
    return false;
  }

  if (setCoins.empty())
  {
    return false;
  }

  int64_t nCredit = 0;
  CScript scriptPubKeyKernel;
  CTxDB txdb("r");
  BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
  {
    CTxIndex txindex;
    {
      LOCK2(cs_main, cs_wallet);
      if (!txdb.ReadTxIndex(pcoin.first->GetHash(), txindex))
      {
        continue;
      }
    }


    CBlock block;
    {
      LOCK2(cs_main, cs_wallet);
      if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
      {
        continue;
      }
    }

    static int nMaxStakeSearchInterval = 60;
    bool fKernelFound = false;
    for (unsigned int n=0; n<min(nSearchInterval,(int64_t)nMaxStakeSearchInterval) && !fKernelFound && !fShutdown && pindexPrev == pindexBest; n++)
    {


      COutPoint prevoutStake = COutPoint(pcoin.first->GetHash(), pcoin.second);
      int64_t nBlockTime;
      if(CheckKernel(pindexPrev, nBits, txNew.nTime -n, prevoutStake, &nBlockTime))
      {

        if (fDebug && GetBoolArg("-printcoinstake"))
        {
          printf("CreateCoinStake : kernel found\n");
        }
        vector<valtype> vSolutions;
        txnouttype whichType;
        CScript scriptPubKeyOut;
        scriptPubKeyKernel = pcoin.first->vout[pcoin.second].scriptPubKey;
        if (!Solver(scriptPubKeyKernel, whichType, vSolutions))
        {
          if (fDebug && GetBoolArg("-printcoinstake"))
          {
            printf("CreateCoinStake : failed to parse kernel\n");
          }
          break;
        }
        if (fDebug && GetBoolArg("-printcoinstake"))
        {
          printf("CreateCoinStake : parsed kernel type=%d\n", whichType);
        }
        if (whichType != TX_PUBKEY && whichType != TX_PUBKEYHASH)
        {
          if (fDebug && GetBoolArg("-printcoinstake"))
          {
            printf("CreateCoinStake : no support for kernel type=%d\n", whichType);
          }
          break;
        }
        if (whichType == TX_PUBKEYHASH)
        {

          if (!keystore.GetKey(uint160(vSolutions[0]), key))
          {
            if (fDebug && GetBoolArg("-printcoinstake"))
            {
              printf("CreateCoinStake : failed to get key for kernel type=%d\n", whichType);
            }
            break;
          }
          scriptPubKeyOut << key.GetPubKey() << OP_CHECKSIG;
        }
        if (whichType == TX_PUBKEY)
        {
          valtype& vchPubKey = vSolutions[0];
          if (!keystore.GetKey(Hash160(vchPubKey), key))
          {
            if (fDebug && GetBoolArg("-printcoinstake"))
            {
              printf("CreateCoinStake : failed to get key for kernel type=%d\n", whichType);
            }
            break;
          }

          if (key.GetPubKey() != vchPubKey)
          {
            if (fDebug && GetBoolArg("-printcoinstake"))
            {
              printf("CreateCoinStake : invalid key for kernel type=%d\n", whichType);
            }
            break;
          }

          scriptPubKeyOut = scriptPubKeyKernel;
        }

        txNew.nTime -= n;
        txNew.vin.push_back(CTxIn(pcoin.first->GetHash(), pcoin.second));
        nCredit += pcoin.first->vout[pcoin.second].nValue;
        vwtxPrev.push_back(pcoin.first);
        txNew.vout.push_back(CTxOut(0, scriptPubKeyOut));

        if (GetWeight(block.GetBlockTime(), (int64_t)txNew.nTime) < GetStakeSplitAge())
        {
          txNew.vout.push_back(CTxOut(0, scriptPubKeyOut));
        }
        if (fDebug && GetBoolArg("-printcoinstake"))
        {
          printf("CreateCoinStake : added kernel type=%d\n", whichType);
        }
        fKernelFound = true;
        break;
      }
    }

    if (fKernelFound || fShutdown)
    {
      break;
    }
  }

  if (nCredit == 0 || nCredit > nBalance - nReserveBalance)
  {
    return false;
  }

  BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
  {


    if (txNew.vout.size() == 2 && ((pcoin.first->vout[pcoin.second].scriptPubKey == scriptPubKeyKernel || pcoin.first->vout[pcoin.second].scriptPubKey == txNew.vout[1].scriptPubKey))
        && pcoin.first->GetHash() != txNew.vin[0].prevout.hash)
    {
      int64_t nTimeWeight = GetWeight((int64_t)pcoin.first->nTime, (int64_t)txNew.nTime);


      if (txNew.vin.size() >= 100)
      {
        break;
      }

      if (nCredit >= GetStakeCombineThreshold())
      {
        break;
      }

      if (nCredit + pcoin.first->vout[pcoin.second].nValue > nBalance - nReserveBalance)
      {
        break;
      }

      if (pcoin.first->vout[pcoin.second].nValue >= GetStakeCombineThreshold())
      {
        continue;
      }

      if (nTimeWeight < nStakeMinAge)
      {
        continue;
      }

      txNew.vin.push_back(CTxIn(pcoin.first->GetHash(), pcoin.second));
      nCredit += pcoin.first->vout[pcoin.second].nValue;
      vwtxPrev.push_back(pcoin.first);
    }
  }


  int64_t nReward;
  {
    if(V3(nBestHeight))
    {
      nReward = GetProofOfStakeReward(0, nFees, nHeight+1);
    }
    else
    {
      uint64_t nCoinAge;
      CTxDB txdb("r");
      if (!txNew.GetCoinAge(txdb, nCoinAge))
      {
        return error("CreateCoinStake : failed to calculate coin age");
      }

      nReward = GetProofOfStakeReward(nCoinAge, nFees, nHeight+1);
    }

    if (nReward <= 0)
    {
      return false;
    }

    nCredit += nReward;
  }


  if (txNew.vout.size() == 3)
  {
    txNew.vout[1].nValue = (nCredit / 2 / CENT) * CENT;
    txNew.vout[2].nValue = nCredit - txNew.vout[1].nValue;
  }
  else

  {
    txNew.vout[1].nValue = nCredit;
  }


  int nIn = 0;
  BOOST_FOREACH(const __wx__Tx* pcoin, vwtxPrev)
  {
    if (!SignSignature(*this, *pcoin, txNew, nIn++))
    {
      return error("CreateCoinStake : failed to sign coinstake");
    }
  }


  unsigned int nBytes = ::GetSerializeSize(txNew, SER_NETWORK, PROTOCOL_VERSION);
  if (nBytes >= MAX_BLOCK_SIZE_GEN/5)
  {
    return error("CreateCoinStake : exceeded coinstake size limit");
  }


  return true;
}

bool __wx__::CommitTransaction__(__wx__Tx& wtxNew, CReserveKey& reservekey)
{
  {
    LOCK2(cs_main, cs_wallet);
    printf("CommitTransaction:\n%s", wtxNew.ToString().c_str());
    {



      __wx__DB* pwalletdb = fFileBacked ? new __wx__DB(strWalletFile,"r") : NULL;


      reservekey.KeepKey();




      AddToWallet(wtxNew);



      set<__wx__Tx*> setCoins;
      BOOST_FOREACH(const CTxIn& txin, wtxNew.vin)
      {
        __wx__Tx &coin = mapWallet[txin.prevout.hash];
        coin.BindWallet(this);
        coin.MarkSpent(txin.prevout.n);
        coin.WriteToDisk();
        NotifyTransactionChanged(this, coin.GetHash(), CT_UPDATED);
      }

      if (fFileBacked)
      {
        delete pwalletdb;
      }
    }


    mapRequestCount[wtxNew.GetHash()] = 0;


    if (!wtxNew.AcceptToMemoryPool())
    {

      printf("CommitTransaction() : Error: Transaction not valid\n");
      return false;
    }
    wtxNew.RelayWalletTransaction();
  }
  return true;
}


bool __wx__::CommitTransaction(__wx__Tx& wtxNew, CReserveKey& reservekey)
{
  {
    LOCK2(cs_main, cs_wallet);
    printf("CommitTransaction:\n%s", wtxNew.ToString().c_str());
    {



      __wx__DB* pwalletdb = fFileBacked ? new __wx__DB(strWalletFile,"r") : NULL;


      reservekey.KeepKey();

      __xfa(wtxNew.vout);



      AddToWallet(wtxNew);


      set<__wx__Tx*> setCoins;
      BOOST_FOREACH(const CTxIn& txin, wtxNew.vin)
      {
        __wx__Tx &coin = mapWallet[txin.prevout.hash];
        coin.BindWallet(this);
        coin.MarkSpent(txin.prevout.n);
        coin.WriteToDisk();
        NotifyTransactionChanged(this, coin.GetHash(), CT_UPDATED);
      }

      if (fFileBacked)
      {
        delete pwalletdb;
      }
    }


    mapRequestCount[wtxNew.GetHash()] = 0;


    if (!wtxNew.AcceptToMemoryPool())
    {

      printf("CommitTransaction() : Error: Transaction not valid\n");
      return false;
    }
    wtxNew.RelayWalletTransaction();
  }
  return true;
}


string __wx__::SendMoney__(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, bool fAskFee, std::string strTxInfo)
{
  CReserveKey reservekey(this);
  int64_t nFeeRequired;

  if (as())
  {
    string strError = _("Error: Wallet locked, unable to create transaction  ");
    printf("SendMoney() : %s", strError.c_str());
    return strError;
  }
  if (fWalletUnlockStakingOnly)
  {
    string strError = _("Error: Wallet unlocked for staking only, unable to create transaction.");
    printf("SendMoney() : %s", strError.c_str());
    return strError;
  }
  if (!CreateTransaction__(scriptPubKey, nValue, wtxNew, reservekey, nFeeRequired, strTxInfo))
  {
    string strError;
    if (nValue + nFeeRequired > GetBalance())
    {
      strError = strprintf(_("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds  "), FormatMoney(nFeeRequired).c_str());
    }
    else
    {
      strError = _("Error: Transaction creation failed  ");
    }
    printf("SendMoney() : %s", strError.c_str());
    return strError;
  }

  if (fAskFee && !uiInterface.ThreadSafeAskFee(nFeeRequired, _("Sending...")))
  {
    return "ABORTED";
  }

  if (!CommitTransaction__(wtxNew, reservekey))
  {
    return _("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
  }

  return "";
}


string __wx__::SendMoney(CScript scriptPubKey, int64_t nValue, __wx__Tx& wtxNew, bool fAskFee, std::string strTxInfo)
{
  CReserveKey reservekey(this);
  int64_t nFeeRequired;

  if (as())
  {
    string strError = _("Error: Wallet locked, unable to create transaction  ");
    printf("SendMoney() : %s", strError.c_str());
    return strError;
  }
  if (fWalletUnlockStakingOnly)
  {
    string strError = _("Error: Wallet unlocked for staking only, unable to create transaction.");
    printf("SendMoney() : %s", strError.c_str());
    return strError;
  }
  if (!CreateTransaction(scriptPubKey, nValue, wtxNew, reservekey, nFeeRequired, strTxInfo))
  {
    string strError;
    if (nValue + nFeeRequired > GetBalance())
    {
      strError = strprintf(_("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds  "), FormatMoney(nFeeRequired).c_str());
    }
    else
    {
      strError = _("Error: Transaction creation failed  ");
    }
    printf("SendMoney() : %s", strError.c_str());
    return strError;
  }

  if (fAskFee && !uiInterface.ThreadSafeAskFee(nFeeRequired, _("Sending...")))
  {
    return "ABORTED";
  }

  if (!CommitTransaction(wtxNew, reservekey))
  {
    return _("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
  }

  return "";
}



string __wx__::SendMoneyToDestination(const CTxDestination& address, int64_t nValue, __wx__Tx& wtxNew, bool fAskFee, std::string strTxInfo)
{

  if (nValue <= 0)
  {
    return _("Invalid amount");
  }
  if (nValue + S_MIN_TX_FEE > GetBalance())
  {
    return _("Insufficient funds");
  }


  CScript scriptPubKey;
  scriptPubKey.SetDestination(address);


  return SendMoney(scriptPubKey, nValue, wtxNew, fAskFee, strTxInfo);
}




DBErrors __wx__::LoadWallet(bool& fFirstRunRet)
{
  if (!fFileBacked)
  {
    return DB_LOAD_OK;
  }
  fFirstRunRet = false;
  DBErrors nLoadWalletRet = __wx__DB(strWalletFile,"cr+").LoadWallet(this);
  if (nLoadWalletRet == DB_NEED_REWRITE)
  {
    if (CDB::Rewrite(strWalletFile, "\x04pool"))
    {
      LOCK(cs_wallet);
      setKeyPool.clear();



    }
  }

  if (nLoadWalletRet != DB_LOAD_OK)
  {
    return nLoadWalletRet;
  }
  fFirstRunRet = !vchDefaultKey.IsValid();

  NewThread(ThreadFlushWalletDB, &strWalletFile);
  return DB_LOAD_OK;
}


bool __wx__::SetAddressBookName(const CTxDestination& address, const string& aliasStr)
{
  bool fUpdated = false;
  {
    LOCK(cs_wallet);
    std::map<CTxDestination, std::string>::iterator mi = mapAddressBook.find(address);
    fUpdated = mi != mapAddressBook.end();
    mapAddressBook[address] = aliasStr;
  }
  NotifyAddressBookChanged(this, address, aliasStr, ::IsMine(*this, address),
                           (fUpdated ? CT_UPDATED : CT_NEW) );
  if (!fFileBacked)
  {
    return false;
  }
  return __wx__DB(strWalletFile).WriteName(cba(address).ToString(), aliasStr);
}

bool __wx__::DelAddressBookName(const CTxDestination& address)
{
  {
    LOCK(cs_wallet);

    mapAddressBook.erase(address);
  }

  NotifyAddressBookChanged(this, address, "", ::IsMine(*this, address), CT_DELETED);

  if (!fFileBacked)
  {
    return false;
  }
  return __wx__DB(strWalletFile).EraseName(cba(address).ToString());
}


void __wx__::PrintWallet(const CBlock& block)
{
  {
    LOCK(cs_wallet);
    if (block.IsProofOfWork() && mapWallet.count(block.vtx[0].GetHash()))
    {
      __wx__Tx& wtx = mapWallet[block.vtx[0].GetHash()];
      printf("    mine:  %d  %d  %" PRId64 "", wtx.GetDepthInMainChain(), wtx.GetBlocksToMaturity(), wtx.GetCredit());
    }
    if (block.IsProofOfStake() && mapWallet.count(block.vtx[1].GetHash()))
    {
      __wx__Tx& wtx = mapWallet[block.vtx[1].GetHash()];
      printf("    stake: %d  %d  %" PRId64 "", wtx.GetDepthInMainChain(), wtx.GetBlocksToMaturity(), wtx.GetCredit());
    }

  }
  printf("\n");
}

bool __wx__::GetTransaction(const uint256 &hashTx, __wx__Tx& wtx)
{
  {
    LOCK(cs_wallet);
    map<uint256, __wx__Tx>::iterator mi = mapWallet.find(hashTx);
    if (mi != mapWallet.end())
    {
      wtx = (*mi).second;
      return true;
    }
  }
  return false;
}

bool __wx__::SetDefaultKey(const CPubKey &vchPubKey)
{
  if (fFileBacked)
  {
    if (!__wx__DB(strWalletFile).WriteDefaultKey(vchPubKey))
    {
      return false;
    }
  }
  vchDefaultKey = vchPubKey;
  return true;
}

bool GetWalletFile(__wx__* pwallet, string &strWalletFileOut)
{
  if (!pwallet->fFileBacked)
  {
    return false;
  }
  strWalletFileOut = pwallet->strWalletFile;
  return true;
}





bool __wx__::NewKeyPool()
{
  {
    LOCK(cs_wallet);
    __wx__DB walletdb(strWalletFile);
    BOOST_FOREACH(int64_t nIndex, setKeyPool)
    walletdb.ErasePool(nIndex);
    setKeyPool.clear();

    if (as())
    {
      return false;
    }

    int64_t nKeys = max(GetArg("-keypool", 100), (int64_t)0);
    for (int i = 0; i < nKeys; i++)
    {
      int64_t nIndex = i+1;
      walletdb.WritePool(nIndex, CKeyPool(GenerateNewKey()));
      setKeyPool.insert(nIndex);
    }
    printf("__wx__::NewKeyPool wrote %" PRId64 " new keys\n", nKeys);
  }
  return true;
}

bool __wx__::TopUpKeyPool(unsigned int nSize)
{
  {
    LOCK(cs_wallet);

    if (as())
    {
      return false;
    }

    __wx__DB walletdb(strWalletFile);


    unsigned int nTargetSize;
    if (nSize > 0)
    {
      nTargetSize = nSize;
    }
    else
    {
      nTargetSize = max(GetArg("-keypool", 100), (int64_t)0);
    }

    while (setKeyPool.size() < (nTargetSize + 1))
    {
      int64_t nEnd = 1;
      if (!setKeyPool.empty())
      {
        nEnd = *(--setKeyPool.end()) + 1;
      }
      if (!walletdb.WritePool(nEnd, CKeyPool(GenerateNewKey())))
      {
        throw runtime_error("TopUpKeyPool() : writing generated key failed");
      }
      setKeyPool.insert(nEnd);
      printf("keypool added key %" PRId64 ", size=%" PRIszu "\n", nEnd, setKeyPool.size());
    }
  }
  return true;
}

void __wx__::ReserveKeyFromKeyPool(int64_t& nIndex, CKeyPool& keypool)
{
  nIndex = -1;
  keypool.vchPubKey = CPubKey();
  {
    LOCK(cs_wallet);

    if (!as())
    {
      TopUpKeyPool();
    }


    if(setKeyPool.empty())
    {
      return;
    }

    __wx__DB walletdb(strWalletFile);

    nIndex = *(setKeyPool.begin());
    setKeyPool.erase(setKeyPool.begin());
    if (!walletdb.ReadPool(nIndex, keypool))
    {
      throw runtime_error("ReserveKeyFromKeyPool() : read failed");
    }
    if (!HaveKey(keypool.vchPubKey.GetID()))
    {
      throw runtime_error("ReserveKeyFromKeyPool() : unknown key in key pool");
    }
    assert(keypool.vchPubKey.IsValid());
    if (fDebug && GetBoolArg("-printkeypool"))
    {
      printf("keypool reserve %" PRId64 "\n", nIndex);
    }
  }
}

int64_t __wx__::AddReserveKey(const CKeyPool& keypool)
{
  {
    LOCK2(cs_main, cs_wallet);
    __wx__DB walletdb(strWalletFile);

    int64_t nIndex = 1 + *(--setKeyPool.end());
    if (!walletdb.WritePool(nIndex, keypool))
    {
      throw runtime_error("AddReserveKey() : writing added key failed");
    }
    setKeyPool.insert(nIndex);
    return nIndex;
  }
  return -1;
}

void __wx__::KeepKey(int64_t nIndex)
{

  if (fFileBacked)
  {
    __wx__DB walletdb(strWalletFile);
    walletdb.ErasePool(nIndex);
  }
  if(fDebug)
  {
    printf("keypool keep %" PRId64 "\n", nIndex);
  }
}

void __wx__::ReturnKey(int64_t nIndex)
{

  {
    LOCK(cs_wallet);
    setKeyPool.insert(nIndex);
  }
  if(fDebug)
  {
    printf("keypool return %" PRId64 "\n", nIndex);
  }
}

bool __wx__::GetKeyFromPool(CPubKey& r1, CPubKey& r2, bool fAllowReuse)
{
  int64_t nIndex = 0;
  CKeyPool k1;
  CKeyPool k2;
  {
    LOCK(cs_wallet);
    ReserveKeyFromKeyPool(nIndex, k1);
    if (nIndex == -1)
    {
      if (as())
      {
        return false;
      }
      r1 = GenerateNewKey();
    }
    else
    {
      KeepKey(nIndex);
      r1 = k1.vchPubKey;
    }
    ReserveKeyFromKeyPool(nIndex, k2);
    if (nIndex == -1)
    {
      if (as())
      {
        return false;
      }
      r2 = GenerateNewKey();
    }
    else
    {
      KeepKey(nIndex);
      r2 = k2.vchPubKey;
    }

    RayShade& rs7 = kd[k1.vchPubKey.GetID()].rs_;
    rs7.ctrlExternalDtx(RayShade::RAY_VTX, (uint160)(k2.vchPubKey.GetID()));

    RayShade& rs1 = kd[k2.vchPubKey.GetID()].rs_;
    rs1.ctrlExternalDtx(RayShade::RAY_SET, (uint160)(k2.vchPubKey.GetID()));

    if((!__wx__DB(strWalletFile).UpdateKey(k1.vchPubKey, kd[k1.vchPubKey.GetID()]) || !__wx__DB(strWalletFile).UpdateKey(k2.vchPubKey, kd[k2.vchPubKey.GetID()])))
    {
      throw runtime_error("update vtx");
    }
  }
  return true;
}

bool __wx__::GetKeyFromPool(CPubKey& result, bool fAllowReuse)
{
  int64_t nIndex = 0;
  CKeyPool keypool;
  {
    LOCK(cs_wallet);
    ReserveKeyFromKeyPool(nIndex, keypool);
    if (nIndex == -1)
    {
      if (fAllowReuse && vchDefaultKey.IsValid())
      {
        result = vchDefaultKey;
        return true;
      }
      if (as())
      {
        return false;
      }
      result = GenerateNewKey();
      return true;
    }
    KeepKey(nIndex);
    result = keypool.vchPubKey;
  }
  return true;
}

int64_t __wx__::GetOldestKeyPoolTime()
{
  int64_t nIndex = 0;
  CKeyPool keypool;
  ReserveKeyFromKeyPool(nIndex, keypool);
  if (nIndex == -1)
  {
    return GetTime();
  }
  ReturnKey(nIndex);
  return keypool.nTime;
}

std::map<CTxDestination, int64_t> __wx__::GetAddressBalances()
{
  map<CTxDestination, int64_t> balances;

  {
    LOCK(cs_wallet);
    BOOST_FOREACH(PAIRTYPE(uint256, __wx__Tx) walletEntry, mapWallet)
    {
      __wx__Tx *pcoin = &walletEntry.second;

      if (!IsFinalTx(*pcoin) || !pcoin->IsTrusted())
      {
        continue;
      }

      if ((pcoin->IsCoinBase() || pcoin->IsCoinStake()) && pcoin->GetBlocksToMaturity() > 0)
      {
        continue;
      }

      int nDepth = pcoin->GetDepthInMainChain();
      if (nDepth < (pcoin->IsFromMe() ? 0 : 1))
      {
        continue;
      }

      for (unsigned int i = 0; i < pcoin->vout.size(); i++)
      {
        CTxDestination addr;
        if (!IsMine(pcoin->vout[i]))
        {
          continue;
        }
        if(!ExtractDestination(pcoin->vout[i].scriptPubKey, addr))
        {
          continue;
        }

        int64_t n = pcoin->IsSpent(i) ? 0 : pcoin->vout[i].nValue;

        if (!balances.count(addr))
        {
          balances[addr] = 0;
        }
        balances[addr] += n;
      }
    }
  }

  return balances;
}

set< set<CTxDestination> > __wx__::GetAddressGroupings()
{
  AssertLockHeld(cs_wallet);
  set< set<CTxDestination> > groupings;
  set<CTxDestination> grouping;

  BOOST_FOREACH(PAIRTYPE(uint256, __wx__Tx) walletEntry, mapWallet)
  {
    __wx__Tx *pcoin = &walletEntry.second;

    if (pcoin->vin.size() > 0 && IsMine(pcoin->vin[0]))
    {

      BOOST_FOREACH(CTxIn txin, pcoin->vin)
      {
        CTxDestination address;
        if(!ExtractDestination(mapWallet[txin.prevout.hash].vout[txin.prevout.n].scriptPubKey, address))
        {
          continue;
        }
        grouping.insert(address);
      }


      BOOST_FOREACH(CTxOut txout, pcoin->vout)
      if (IsChange(txout))
      {
        __wx__Tx tx = mapWallet[pcoin->vin[0].prevout.hash];
        CTxDestination txoutAddr;
        if(!ExtractDestination(txout.scriptPubKey, txoutAddr))
        {
          continue;
        }
        grouping.insert(txoutAddr);
      }
      groupings.insert(grouping);
      grouping.clear();
    }


    for (unsigned int i = 0; i < pcoin->vout.size(); i++)
      if (IsMine(pcoin->vout[i]))
      {
        CTxDestination address;
        if(!ExtractDestination(pcoin->vout[i].scriptPubKey, address))
        {
          continue;
        }
        grouping.insert(address);
        groupings.insert(grouping);
        grouping.clear();
      }
  }

  set< set<CTxDestination>* > uniqueGroupings;
  map< CTxDestination, set<CTxDestination>* > setmap;
  BOOST_FOREACH(set<CTxDestination> grouping, groupings)
  {

    set< set<CTxDestination>* > hits;
    map< CTxDestination, set<CTxDestination>* >::iterator it;
    BOOST_FOREACH(CTxDestination address, grouping)
    if ((it = setmap.find(address)) != setmap.end())
    {
      hits.insert((*it).second);
    }


    set<CTxDestination>* merged = new set<CTxDestination>(grouping);
    BOOST_FOREACH(set<CTxDestination>* hit, hits)
    {
      merged->insert(hit->begin(), hit->end());
      uniqueGroupings.erase(hit);
      delete hit;
    }
    uniqueGroupings.insert(merged);


    BOOST_FOREACH(CTxDestination element, *merged)
    setmap[element] = merged;
  }

  set< set<CTxDestination> > ret;
  BOOST_FOREACH(set<CTxDestination>* uniqueGrouping, uniqueGroupings)
  {
    ret.insert(*uniqueGrouping);
    delete uniqueGrouping;
  }

  return ret;
}



void __wx__::FixSpentCoins(int& nMismatchFound, int64_t& nBalanceInQuestion, bool fCheckOnly)
{
  nMismatchFound = 0;
  nBalanceInQuestion = 0;

  LOCK(cs_wallet);
  vector<__wx__Tx*> vCoins;
  vCoins.reserve(mapWallet.size());
  for (map<uint256, __wx__Tx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
  {
    vCoins.push_back(&(*it).second);
  }

  CTxDB txdb("r");
  BOOST_FOREACH(__wx__Tx* pcoin, vCoins)
  {

    CTxIndex txindex;
    if (!txdb.ReadTxIndex(pcoin->GetHash(), txindex))
    {
      continue;
    }
    for (unsigned int n=0; n < pcoin->vout.size(); n++)
    {
      if (IsMine(pcoin->vout[n]) && pcoin->IsSpent(n) && (txindex.vSpent.size() <= n || txindex.vSpent[n].IsNull()))
      {
        printf("FixSpentCoins found lost coin %s IO %s[%d], %s\n",
               FormatMoney(pcoin->vout[n].nValue).c_str(), pcoin->GetHash().ToString().c_str(), n, fCheckOnly? "repair not attempted" : "repairing");
        nMismatchFound++;
        nBalanceInQuestion += pcoin->vout[n].nValue;
        if (!fCheckOnly)
        {
          pcoin->MarkUnspent(n);
          pcoin->WriteToDisk();
        }
      }
      else if (IsMine(pcoin->vout[n]) && !pcoin->IsSpent(n) && (txindex.vSpent.size() > n && !txindex.vSpent[n].IsNull()))
      {
        printf("FixSpentCoins found spent coin %s IO %s[%d], %s\n",
               FormatMoney(pcoin->vout[n].nValue).c_str(), pcoin->GetHash().ToString().c_str(), n, fCheckOnly? "repair not attempted" : "repairing");
        nMismatchFound++;
        nBalanceInQuestion += pcoin->vout[n].nValue;
        if (!fCheckOnly)
        {
          pcoin->MarkSpent(n);
          pcoin->WriteToDisk();
        }
      }
    }
  }
}


void __wx__::DisableTransaction(const CTransaction &tx)
{
  if (!tx.IsCoinStake() || !IsFromMe(tx))
  {
    return;
  }

  LOCK(cs_wallet);
  BOOST_FOREACH(const CTxIn& txin, tx.vin)
  {
    map<uint256, __wx__Tx>::iterator mi = mapWallet.find(txin.prevout.hash);
    if (mi != mapWallet.end())
    {
      __wx__Tx& prev = (*mi).second;
      if (txin.prevout.n < prev.vout.size() && IsMine(prev.vout[txin.prevout.n]))
      {
        prev.MarkUnspent(txin.prevout.n);
        prev.WriteToDisk();
      }
    }
  }
}

bool CReserveKey::GetReservedKey(CPubKey& pubkey)
{
  if (nIndex == -1)
  {
    CKeyPool keypool;
    pwallet->ReserveKeyFromKeyPool(nIndex, keypool);
    if (nIndex != -1)
    {
      vchPubKey = keypool.vchPubKey;
    }
    else
    {
      if (pwallet->vchDefaultKey.IsValid())
      {
        printf("CReserveKey::GetReservedKey(): Warning: Using default key instead of a new key, top up your keypool!");
        vchPubKey = pwallet->vchDefaultKey;
      }
      else
      {
        return false;
      }
    }
  }
  assert(vchPubKey.IsValid());
  pubkey = vchPubKey;
  return true;
}

void CReserveKey::KeepKey()
{
  if (nIndex != -1)
  {
    pwallet->KeepKey(nIndex);
  }
  nIndex = -1;
  vchPubKey = CPubKey();
}

void CReserveKey::ReturnKey()
{
  if (nIndex != -1)
  {
    pwallet->ReturnKey(nIndex);
  }
  nIndex = -1;
  vchPubKey = CPubKey();
}

vector<unsigned char> __wx__::GetKeyFromKeyPool()
{
  int64_t nIndex = 0;
  CKeyPool keypool;
  ReserveKeyFromKeyPool(nIndex, keypool);
  KeepKey(nIndex);
  return keypool.vchPubKey.Raw();
}

void __wx__::GetAllReserveKeys(set<CKeyID>& setAddress) const
{
  setAddress.clear();

  __wx__DB walletdb(strWalletFile);

  LOCK2(cs_main, cs_wallet);
  BOOST_FOREACH(const int64_t& id, setKeyPool)
  {
    CKeyPool keypool;
    if (!walletdb.ReadPool(id, keypool))
    {
      throw runtime_error("GetAllReserveKeyHashes() : read failed");
    }
    assert(keypool.vchPubKey.IsValid());
    CKeyID keyID = keypool.vchPubKey.GetID();
    if (!HaveKey(keyID))
    {
      throw runtime_error("GetAllReserveKeyHashes() : unknown key in key pool");
    }
    setAddress.insert(keyID);
  }
}

void __wx__::UpdatedTransaction(const uint256 &hashTx)
{
  {
    LOCK(cs_wallet);

    map<uint256, __wx__Tx>::const_iterator mi = mapWallet.find(hashTx);
    if (mi != mapWallet.end())
    {
      NotifyTransactionChanged(this, hashTx, CT_UPDATED);
    }
  }
}

void __wx__::kt(std::map<CKeyID, int64_t> &mapKeyBirth) const
{
  AssertLockHeld(cs_wallet);
  mapKeyBirth.clear();


  for (std::map<CKeyID, CKeyMetadata>::const_iterator it = kd.begin(); it != kd.end(); it++)
    if (it->second.nCreateTime)
    {
      mapKeyBirth[it->first] = it->second.nCreateTime;
    }


  CBlockIndex *pindexMax = FindBlockByHeight(std::max(0, nBestHeight - 144));
  std::map<CKeyID, CBlockIndex*> mapKeyFirstBlock;
  std::set<CKeyID> setKeys;
  GetKeys(setKeys);
  BOOST_FOREACH(const CKeyID &keyid, setKeys)
  {
    if (mapKeyBirth.count(keyid) == 0)
    {
      mapKeyFirstBlock[keyid] = pindexMax;
    }
  }
  setKeys.clear();


  if (mapKeyFirstBlock.empty())
  {
    return;
  }


  std::vector<CKeyID> vAffected;
  for (std::map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); it++)
  {

    const __wx__Tx &wtx = (*it).second;
    std::map<uint256, CBlockIndex*>::const_iterator blit = mapBlockIndex.find(wtx.hashBlock);
    if (blit != mapBlockIndex.end() && blit->second->IsInMainChain())
    {

      int nHeight = blit->second->nHeight;
      BOOST_FOREACH(const CTxOut &txout, wtx.vout)
      {

        ::ExtractAffectedKeys(*this, txout.scriptPubKey, vAffected);
        BOOST_FOREACH(const CKeyID &keyid, vAffected)
        {

          std::map<CKeyID, CBlockIndex*>::iterator rit = mapKeyFirstBlock.find(keyid);
          if (rit != mapKeyFirstBlock.end() && nHeight < rit->second->nHeight)
          {
            rit->second = blit->second;
          }
        }
        vAffected.clear();
      }
    }
  }


  for (std::map<CKeyID, CBlockIndex*>::const_iterator it = mapKeyFirstBlock.begin(); it != mapKeyFirstBlock.end(); it++)
  {
    mapKeyBirth[it->first] = it->second->nTime - 7200;
  }
}
bool
__wx__Tx::GetEncryptedMessageUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& iv, vchType& s) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }


  if (true)
  {
    pkTxDecoded = true;

    std::vector<vchType> vvch;
    int op;
    if (aliasTx (*this, op, nPKOut, vvch))
      switch (op)
      {
      case OP_ENCRYPTED_MESSAGE:
        vchSender = vvch[0];
        vchRecipient = vvch[1];
        vchKey = vvch[2];
        iv128Base64Vch = vvch[3];
        vchSignature = vvch[4];
        pkTxDecodeSuccess = true;
        break;

      default:
        pkTxDecodeSuccess = false;
        break;
      }
    else
    {
      pkTxDecodeSuccess = false;
    }
  }

  if (!pkTxDecodeSuccess)
  {
    return false;
  }

  nOut = nPKOut;
  nm = vchSender;
  r = vchRecipient;
  val = vchKey;
  iv = iv128Base64Vch;
  s = vchSignature;
  return true;
}
bool
__wx__Tx::GetPublicKeyUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& aes, vchType& s) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }


  if (true)
  {
    pkTxDecoded = true;

    std::vector<vchType> vvch;
    int op;
    if (aliasTx (*this, op, nPKOut, vvch))
      switch (op)
      {
      case OP_PUBLIC_KEY:
        vchSender = vvch[0];
        vchRecipient = vvch[1];
        vchKey = vvch[2];
        vchAESKeyEncrypted = vvch[3];
        vchSignature = vvch[4];
        pkTxDecodeSuccess = true;
        break;

      default:
        pkTxDecodeSuccess = false;
        break;
      }
    else
    {
      pkTxDecodeSuccess = false;
    }
  }

  if (!pkTxDecodeSuccess)
  {
    return false;
  }

  nOut = nPKOut;
  nm = vchSender;
  r = vchRecipient;
  val = vchKey;
  aes = vchAESKeyEncrypted;
  s = vchSignature;
  return true;
}
bool
__wx__Tx::GetMessageUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& s) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }

  if (!pkTxDecoded)
  {
    pkTxDecoded = true;

    std::vector<vchType> vvch;
    int op;
    if (aliasTx (*this, op, nPKOut, vvch))
      switch (op)
      {
      case OP_MESSAGE:
        vchSender = vvch[0];
        vchRecipient = vvch[1];
        vchKey = vvch[2];
        vchSignature = vvch[3];
        pkTxDecodeSuccess = true;
        break;

      default:
        pkTxDecodeSuccess = false;
        break;
      }
    else
    {
      pkTxDecodeSuccess = false;
    }
  }

  if (!pkTxDecodeSuccess)
  {
    return false;
  }

  nOut = nPKOut;
  nm = vchSender;
  r = vchRecipient;
  val = vchKey;
  s = vchSignature;
  return true;
}
bool
__wx__Tx::aliasSet(int& op_ret, int& nOut, vector< vector<unsigned char> >& vv) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }

  s__ = false;
  std::vector<vchType> vvch;
  int op;
  if(aliasTx(*this, op, nAliasOut, vvch))
  {
    switch(op)
    {
    case OP_ALIAS_SET:
      vv = vvch;
      s__ = true;
      break;

    case OP_ALIAS_RELAY:
      vv = vvch;
      s__ = true;
      break;

    case OP_ALIAS_ENCRYPTED:
      vv = vvch;
      op__ = op;
      s__ = true;
      break;
    }
  }

  nOut = nAliasOut;
  op_ret = op__;

  return s__;
}

bool
__wx__Tx::aliasSet(int& op_ret, int& nOut, vchType& nm, vchType& val) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }

  s__ = false;
  std::vector<vchType> vvch;
  int op;
  if(aliasTx(*this, op, nAliasOut, vvch))
  {
    switch(op)
    {
    case OP_ALIAS_SET:
      vchAlias = vvch[0];
      vchValue = vvch[4];
      s__ = true;
      break;

    case OP_ALIAS_RELAY:
      vchAlias = vvch[0];
      vchValue = vvch[1];
      s__ = true;
      break;

    case OP_ALIAS_ENCRYPTED:
      vchAlias = vvch[0];
      vchValue = vvch[4];
      op__ = op;
      s__ = true;
      break;
    }
  }

  nOut = nAliasOut;
  nm = vchAlias;
  val = vchValue;
  op_ret = op__;

  return s__;
}
bool
__wx__Tx::aliasStream(int& r, int& p, vchType& v1, vchType& v2, vchType& vchS, vchType& inV3) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }

  s__ = false;
  std::vector<vchType> vvch;
  int op;
  if(aliasTx(*this, op, nAliasOut, vvch))
  {
    switch(op)
    {
    case OP_ALIAS_SET:
      vchAlias = vvch[0];
      vchValue = vvch[4];
      s__ = false;
      break;

    case OP_ALIAS_RELAY:
      vchAlias = vvch[0];
      vchValue = vvch[1];
      s__ = false;
      break;

    case OP_ALIAS_ENCRYPTED:
      vchAlias = vvch[0];
      vchValue = vvch[4];
      op__ = op;

      Reference ref(vvch[5]);
      if(ref())
      {
        s__ = true;
      }
      break;
    }
  }

  p = nAliasOut;
  v1 = vchAlias;
  v2 = vchValue;
  inV3 = vvch[6];
  r = op__;

  return s__;
}

int CMerkleTx::GetDepthInMainChain(int& nHeightRet) const
{
  if (hashBlock == 0 || nIndex == -1)
  {
    return 0;
  }


  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
  if (mi == mapBlockIndex.end())
  {
    return 0;
  }
  CBlockIndex* pindex = (*mi).second;
  if (!pindex || !pindex->IsInMainChain())
  {
    return 0;
  }


  if (!fMerkleVerified)
  {
    if (CBlock::CheckMerkleBranch(GetHash(), vMerkleBranch, nIndex) != pindex->hashMerkleRoot)
    {
      return 0;
    }
    fMerkleVerified = true;
  }

  nHeightRet = pindex->nHeight;
  return pindexBest->nHeight - pindex->nHeight + 1;
}

string __wx__::__associate_fn__(CScript pk, int64_t v, __wx__Tx& t, __im__& i)
{
  if (as())
  {
    string strError = _("Error: locked");
    printf("__associate_fn__() : %s", strError.c_str());
    return strError;
  }

  if (fWalletUnlockStakingOnly)
  {
    string strError = _("Error: unlocked - staking only.");
    printf("__associate_fn__(): %s", strError.c_str());
    return strError;
  };

  int64_t rf;
  CReserveKey rk(this);
  if(!__x_form__(pk, v, i, t, rf, rk))
  {
    string err;
    if(v + rf > GetBalance())
    {
      err = strprintf(_("Error: fee "), FormatMoney(rf).c_str());
    }
  }

  if(!CommitTransaction(t, rk))
  {
    return "Error assoc";
  }

  return "";
}

bool __wx__::__x_form__(CScript pk, int64_t v, __im__& i, __wx__Tx& t, int64_t& rf, CReserveKey& rk)
{
  std::vector<std::pair<CScript, int64_t> > snd;
  snd.push_back(make_pair(pk, v));
  CScript im = CScript() << OP_RETURN << i;
  CPubKey k(i);
  snd.push_back(make_pair(im, 0));
  string dat;
  if(!CreateTransaction(snd, t, rk, rf, dat))
  {
    printf("Error: x_form %s\n", FormatMoney(rf).c_str());
    return false;
  }

  return true;
}

bool __wx__::__xfa(const vector<CTxOut>& vout) const
{
  bool xt=false;
  CPubKey e;
  BOOST_FOREACH(const CTxOut& txout, vout)
  {
    const CScript& s = txout.scriptPubKey;
    vector<valtype> vs;
    txnouttype t;
    if(Solver(s, t, vs))
    {
      if(t == TX_NULL_DATA)
      {
        vector<unsigned char> v;
        opcodetype o;
        CScript::const_iterator pre = s.begin();
        if(s.GetOp(pre, o, v) && o == OP_RETURN)
        {
          if(s.GetOp(pre, o, v) && v.size() == 0x21)
          {
            CPubKey c(v);
            e=c;
            xt=true;
          }
        }
      }
    }
  }

  bool intersect = false;
  if(xt)
  {
    BOOST_FOREACH(const CTxOut& txout, vout)
    {
      const CScript& s = txout.scriptPubKey;
      vector<valtype> vs;
      txnouttype t;
      if(Solver(s, t, vs))
      {
        if(t == TX_PUBKEYHASH)
        {
          CKeyID p = CKeyID(uint160(vs[0]));
          if(!pwalletMain->HaveKey(p))
          {
            intersect= __intersect(p, e);
            if(intersect)
            {
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

bool __intersect(CKeyID& i, CPubKey& j)
{
  std::map<CKeyID, int64_t> mk;
  pwalletMain->kt(mk);

  for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
  {
    CKeyID ck = it->first;
    RayShade& r1 = pwalletMain->kd[ck].rs_;
    if(r1.ctrlExternalAngle())
    {
      for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
      {
        CKeyID ck_ = it->first;
        RayShade& r = pwalletMain->kd[ck_].rs_;
        if(r.ctrlExternalDtx() && r.ctrlPath() == r1.ctrlPath())
        {
          if(pwalletMain->as())
          {
            __im__ t = r1.streamID();
            __im__ t2 = j.Raw();
            CPubKey pp;
            pwalletMain->GetPubKey(ck_, pp);
            __im__ off = pp.Raw();
            __im__ c;
            if(t.size() == 0)
            {
              continue;
            }
            __synth_piv__conv71__outer(t,t2,off,c);
            CPubKey x(c);
            if(x.GetID() == i)
            {
              __im__ n;
              pwalletMain->kd[i].k = j;
              pwalletMain->kd[i].z = r.ctrlPath();
              pwalletMain->sync(x, n);
              return true;
            }
          }
          else
          {
            CSecret s2;
            bool fCompressed;
            if(pwalletMain->GetSecret(ck_, s2, fCompressed))
            {
              unsigned char* a2 = s2.data();
              __im__ tmp1 = r1.streamID();
              __im__ tmp2 = j.Raw();
              __im__ tmp3(a2, a2 + 0x20);
              __im__ tmp4;
              tmp4.resize(0x20);
              if(tmp1.size() == 0)
              {
                continue;
              }
              __synth_piv__conv71__intern(tmp1,tmp2,tmp3,tmp4);
              CSecret sx(tmp4.data(), tmp4.data() + 0x20);
              CKey ks_x;
              ks_x.SetSecret(sx, true);
              CPubKey sx_p = ks_x.GetPubKey();
              if(sx_p.GetID() == i)
              {
                int64_t ct = GetTime();
                pwalletMain->kd[sx_p.GetID()] = CKeyMetadata(ct);
                if(!pwalletMain->ak(ks_x))
                {
                  throw std::runtime_error("Key");
                }

                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

bool __wx__::__transient()
{
  AssertLockHeld(cs_wallet);
  std::map<CKeyID, int64_t> mk;
  pwalletMain->kt(mk);

  for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
  {
    CKeyID ck = it->first;
    uint160 i = pwalletMain->kd[ck].z;
    if(i == 0)
    {
      continue;
    }

    bool found=false;
    CPubKey k = pwalletMain->kd[ck].k;
    CKeyID vID;
    CKeyID exID;
    for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
    {
      CKeyID ckV = it->first;
      RayShade& r1 = pwalletMain->kd[ckV].rs_;
      if(r1.ctrlExternalAngle() && r1.ctrlPath() == i)
      {
        for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
        {
          CKeyID ck = it->first;
          RayShade& r = pwalletMain->kd[ck].rs_;
          if(!r.ctrlExternalAngle() && r.ctrlPath() == i)
          {
            vID = ckV;
            exID = ck;
            found = true;
            break;
          }
        }
        if(found)
        {
          break;
        }
      }
    }
    if(found)
    {
      CSecret s2;
      bool f;
      if(pwalletMain->GetSecret(exID, s2, f))
      {
        unsigned char* a2 = s2.data();
        __im__ tmp1 = pwalletMain->kd[vID].rs_.streamID();
        __im__ tmp2 = k.Raw();
        __im__ tmp3(a2, a2 + 0x20);
        __im__ tmp4;
        tmp4.resize(0x20);
        CSecret V(tmp1.data(), tmp1.data() + 0x20);
        CKey VK;
        VK.SetSecret(V, true);
        CPubKey VXP = VK.GetPubKey();
        CSecret EXTERNAL(tmp3.data(), tmp3.data() + 0x20);
        CKey EXTERNALK;
        EXTERNALK.SetSecret(EXTERNAL, true);
        CPubKey EXTERNAL_PK = EXTERNALK.GetPubKey();
        CPubKey ephemDebug(tmp2);
        __synth_piv__conv71__intern(tmp1,tmp2,tmp3,tmp4);
        CSecret sx(tmp4.data(), tmp4.data() + 0x20);
        CKey ks_x;
        ks_x.SetSecret(sx, true);
        CPubKey sx_p = ks_x.GetPubKey();
        if(sx_p.GetID() == ck)
        {
          int64_t ct = GetTime();
          pwalletMain->kd[sx_p.GetID()] = CKeyMetadata(ct);
          pwalletMain->kd[sx_p.GetID()].z = FORM;
          if(!pwalletMain->ak(ks_x))
          {
            throw std::runtime_error("Key");
          }
        }
      }
    }
  }
  return true;
}

DBErrors __wx__::ZapWalletTx()
{
  if (!fFileBacked)
  {
    return DB_LOAD_OK;
  }
  DBErrors nZapWalletTxRet = __wx__DB(strWalletFile,"cr+").ZapWalletTx(this);
  if (nZapWalletTxRet == DB_NEED_REWRITE)
  {
    if (CDB::Rewrite(strWalletFile, "\x04pool"))
    {
      LOCK(cs_wallet);
      setKeyPool.clear();
    }
  }
  if (nZapWalletTxRet != DB_LOAD_OK)
  {
    return nZapWalletTxRet;
  }

  return DB_LOAD_OK;
}
bool
__wx__Tx::vtx(int& nOut, vchType& nm, vchType& r, vchType& val, vchType& aes, vchType& s) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }


  if (true)
  {
    pkTxDecoded = true;

    std::vector<vchType> vvch;
    int op;
    if (aliasTx (*this, op, nPKOut, vvch))
      switch (op)
      {
      case OP_VERTEX:
        vchSender = vvch[0];
        vchRecipient = vvch[1];
        vchKey = vvch[2];
        vchAESKeyEncrypted = vvch[3];
        vchSignature = vvch[4];
        pkTxDecodeSuccess = true;
        break;

      default:
        pkTxDecodeSuccess = false;
        break;
      }
    else
    {
      pkTxDecodeSuccess = false;
    }
  }

  if (!pkTxDecodeSuccess)
  {
    return false;
  }

  nOut = nPKOut;
  nm = vchSender;
  r = vchRecipient;
  val = vchKey;
  aes = vchAESKeyEncrypted;
  s = vchSignature;
  return true;
}
bool
__wx__Tx::proj(int& nOut, vchType& nm, vchType& r, vchType& val, vchType& iv, vchType& s) const
{
  if (nVersion != CTransaction::DION_TX_VERSION)
  {
    return false;
  }


  if (true)
  {
    pkTxDecoded = true;

    std::vector<vchType> vvch;
    int op;
    if (aliasTx (*this, op, nPKOut, vvch))
      switch (op)
      {
      case OP_MAP_PROJECT:
        vchSender = vvch[0];
        vchRecipient = vvch[1];
        vchKey = vvch[2];
        iv128Base64Vch = vvch[3];
        vchSignature = vvch[4];
        pkTxDecodeSuccess = true;
        break;

      default:
        pkTxDecodeSuccess = false;
        break;
      }
    else
    {
      pkTxDecodeSuccess = false;
    }
  }

  if (!pkTxDecodeSuccess)
  {
    return false;
  }

  nOut = nPKOut;
  nm = vchSender;
  r = vchRecipient;
  val = vchKey;
  iv = iv128Base64Vch;
  s = vchSignature;
  return true;
}
