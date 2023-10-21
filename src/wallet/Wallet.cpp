
#include "TxDB.h"
#include "Wallet.h"
#include "WalletDB.h"
#include "Crypter.h"
#include "UIInterface.h"
#include "core/Base58.h"
#include "Kernel.h"
#include "dions/Reference.h"
#include <boost/algorithm/string/replace.hpp>
#include "RayShade.h"
#include "ccoin/Process.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "dions/Dions.h"
#include <iostream>
#include <fstream>
#include "rpc/Client.h"
#include "CoinControl.h"
#include "core/Base58.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/variant/get.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#define printf OutputDebugStringF
using namespace json_spirit;
using namespace std;
using namespace std;
using namespace boost;
extern __wx__* pwalletMainId;
extern ConfigurationState globalState;
extern CTxMemPool mempool;
static unsigned int GetStakeSplitAge()
{
  return IsProtocolV2(nBestHeight) ? (10 * 24 * 60 * 60) : (1 * 24 * 60 * 60);
}
static int64_t GetStakeCombineThreshold()
{
  return IsProtocolV2(nBestHeight) ? (50 * COIN) : (1000 * COIN);
}
bool isAliasTx(const __wx__Tx* tx);
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
namespace bt = boost::posix_time;
using namespace json_spirit;
using namespace std;
void EnsureWalletIsUnlocked();
namespace bt = boost::posix_time;
extern __wx__* pwalletMainId;
extern int checkAddress(string addr, cba& a);
const std::locale formats[] =
{
  std::locale(std::locale::classic(),new bt::time_input_facet("%Y-%m-%dT%H:%M:%SZ")),
  std::locale(std::locale::classic(),new bt::time_input_facet("%Y-%m-%d %H:%M:%S")),
  std::locale(std::locale::classic(),new bt::time_input_facet("%Y/%m/%d %H:%M:%S")),
  std::locale(std::locale::classic(),new bt::time_input_facet("%d.%m.%Y %H:%M:%S")),
  std::locale(std::locale::classic(),new bt::time_input_facet("%Y-%m-%d"))
};
const size_t formats_n = sizeof(formats)/sizeof(formats[0]);
std::time_t pt_to_time_t(const bt::ptime& pt)
{
  bt::ptime timet_start(boost::gregorian::date(1970,1,1));
  bt::time_duration diff = pt - timet_start;
  return diff.ticks()/bt::time_duration::rep_type::ticks_per_second;
}
class CTxDump
{
public:
  CBlockIndex *pindex;
  int64_t nValue;
  bool fSpent;
  __wx__Tx* ptx;
  int nOut;
  CTxDump(__wx__Tx* ptx = NULL, int nOut = -1)
  {
    pindex = NULL;
    nValue = 0;
    fSpent = false;
    this->ptx = ptx;
    this->nOut = nOut;
  }
};
int64_t DecodeDumpTime(const std::string& s)
{
  bt::ptime pt;

  for(size_t i=0; i<formats_n; ++i)
  {
    std::istringstream is(s);
    is.imbue(formats[i]);
    is >> pt;

    if(pt != bt::ptime())
    {
      break;
    }
  }

  return pt_to_time_t(pt);
}
std::string static EncodeDumpTime(int64_t nTime)
{
  return DateTimeStrFormat("%Y-%m-%dT%H:%M:%SZ", nTime);
}
std::string static EncodeDumpString(const std::string &str)
{
  std::stringstream ret;
  BOOST_FOREACH(unsigned char c, str)
  {
    if (c <= 32 || c >= 128 || c == '%')
    {
      ret << '%' << HexStr(&c, &c + 1);
    }
    else
    {
      ret << c;
    }
  }
  return ret.str();
}
std::string DecodeDumpString(const std::string &str)
{
  std::stringstream ret;

  for (unsigned int pos = 0; pos < str.length(); pos++)
  {
    unsigned char c = str[pos];

    if (c == '%' && pos+2 < str.length())
    {
      c = (((str[pos+1]>>6)*9+((str[pos+1]-'0')&15)) << 4) |
          ((str[pos+2]>>6)*9+((str[pos+2]-'0')&15));
      pos += 2;
    }

    ret << c;
  }

  return ret.str();
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

      if (wtxIn.hashBlock_ != 0)
      {
        if (mapBlockIndex.count(wtxIn.hashBlock_))
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
          unsigned int& blocktime = mapBlockIndex[wtxIn.hashBlock_]->nTime;
          wtx.nTimeSmart = std::max(latestEntry, std::min(blocktime, latestNow));
        }
        else
          printf("AddToWallet() : found %s in block %s not in index\n",
                 wtxIn.GetHash().ToString().substr(0,10).c_str(),
                 wtxIn.hashBlock_.ToString().c_str());
      }
    }

    bool fUpdated = false;

    if (!fInsertedNew)
    {
      if (wtxIn.hashBlock_ != 0 && wtxIn.hashBlock_ != wtx.hashBlock_)
      {
        wtx.hashBlock_ = wtxIn.hashBlock_;
        fUpdated = true;
      }

      if (wtxIn.nIndex != -1 && (wtxIn.vTxMerkleBranch != wtx.vTxMerkleBranch || wtxIn.nIndex != wtx.nIndex))
      {
        wtx.vTxMerkleBranch = wtxIn.vTxMerkleBranch;
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
    WalletUpdateSpent(wtx, (wtxIn.hashBlock_ != 0));
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
      if (this->hashBlock_ != 0)
      {
        map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(this->hashBlock_);

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

        if (nRequests == 0 && this->hashBlock_ != 0)
        {
          map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(this->hashBlock_);

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
int64_t __wx__::ImmatureBalance() const
{
  int64_t nTotal = 0;
  {
    LOCK2(cs_main, cs_wallet);

    for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
    {
      const __wx__Tx& pcoin = (*it).second;

      if (pcoin.IsCoinBase() && pcoin.GetBlocksToMaturity() > 0 && pcoin.OnMainChain())
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

      if (nDepth < ConfigurationState::nStakeMinConfirmations)
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

  if (wtxNew.strTxInfo.length() > ConfigurationState::MAX_TX_INFO_LEN)
  {
    wtxNew.strTxInfo.resize(ConfigurationState::MAX_TX_INFO_LEN);
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

        if (nFeeRet < CTransaction::MIN_TX_FEE && nChange > 0 && nChange < CENT)
        {
          int64_t nMoveToFee = min(nChange, CTransaction::MIN_TX_FEE - nFeeRet);
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

        if (nBytes >= ConfigurationState::MAX_STANDARD_TX_SIZE)
        {
          return false;
        }

        dPriority /= nBytes;
        int64_t nPayFee = globalState.nTransactionFee * (1 + (int64_t)nBytes / 1000);
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

  if (wtxNew.strTxInfo.length() > ConfigurationState::MAX_TX_INFO_LEN)
  {
    wtxNew.strTxInfo.resize(ConfigurationState::MAX_TX_INFO_LEN);
  }

  if (wtxNew.strTxInfo.length()>0)
  {
    wtxNew.nVersion=3;
  }

  {
    LOCK2(cs_main, cs_wallet);
    CTxDB txdb("r");
    {
      nFeeRet = CTransaction::S_MIN_TX_FEE;

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

        if (nFeeRet < CTransaction::MIN_TX_FEE && nChange > 0 && nChange < CENT)
        {
          int64_t nMoveToFee = min(nChange, CTransaction::MIN_TX_FEE - nFeeRet);
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

        if (nBytes >= ConfigurationState::MAX_STANDARD_TX_SIZE)
        {
          return false;
        }

        dPriority /= nBytes;
        int64_t nPayFee = globalState.nTransactionFee * (1 + (int64_t)nBytes / 1000);
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

    if(pcoin.first->GetDepthInMainChain() >= ConfigurationState::nStakeMinConfirmations)
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

  if (nBytes >= CBlock::MAX_BLOCK_SIZE_GEN/5)
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

    if (!mempool.accept(wtxNew,NULL))
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

    if (!mempool.accept(wtxNew,NULL))
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

  if (nValue + CTransaction::S_MIN_TX_FEE > GetBalance())
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
    std::map<uint256, CBlockIndex*>::const_iterator blit = mapBlockIndex.find(wtx.hashBlock_);

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
bool __wx__::__xfa(const vector<CTxOut>& vout) 
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

          if(!this->HaveKey(p))
          {
            intersect= this->__intersect(p, e);

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
bool __wx__::__intersect(CKeyID& i, CPubKey& j) 
{
  std::map<CKeyID, int64_t> mk;
  this->kt(mk);

  for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
  {
    CKeyID ck = it->first;
    RayShade& r1 = this->kd[ck].rs_;

    if(r1.ctrlExternalAngle())
    {
      for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
      {
        CKeyID ck_ = it->first;
        RayShade& r = this->kd[ck_].rs_;

        if(r.ctrlExternalDtx() && r.ctrlPath() == r1.ctrlPath())
        {
          if(this->as())
          {
            __im__ t = r1.streamID();
            __im__ t2 = j.Raw();
            CPubKey pp;
            this->GetPubKey(ck_, pp);
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
              this->kd[i].k = j;
              this->kd[i].z = r.ctrlPath();
              this->sync(x, n);
              return true;
            }
          }
          else
          {
            CSecret s2;
            bool fCompressed;

            if(this->GetSecret(ck_, s2, fCompressed))
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
                this->kd[sx_p.GetID()] = CKeyMetadata(ct);

                if(!this->ak(ks_x))
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
  this->kt(mk);

  for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
  {
    CKeyID ck = it->first;
    uint160 i = this->kd[ck].z;

    if(i == 0)
    {
      continue;
    }

    bool found=false;
    CPubKey k = this->kd[ck].k;
    CKeyID vID;
    CKeyID exID;

    for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
    {
      CKeyID ckV = it->first;
      RayShade& r1 = this->kd[ckV].rs_;

      if(r1.ctrlExternalAngle() && r1.ctrlPath() == i)
      {
        for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
        {
          CKeyID ck = it->first;
          RayShade& r = this->kd[ck].rs_;

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

      if(this->GetSecret(exID, s2, f))
      {
        unsigned char* a2 = s2.data();
        __im__ tmp1 = this->kd[vID].rs_.streamID();
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
          this->kd[sx_p.GetID()] = CKeyMetadata(ct);
          this->kd[sx_p.GetID()].z = FORM;

          if(!this->ak(ks_x))
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
void ReadOrderPos(int64_t& nOrderPos, mapValue_t& mapValue)
{
    if (!mapValue.count("n"))
    {
        nOrderPos = -1;
        return;
    }
    nOrderPos = atoi64(mapValue["n"].c_str());
}


void WriteOrderPos(const int64_t& nOrderPos, mapValue_t& mapValue)
{
    if (nOrderPos == -1)
        return;
    mapValue["n"] = i64tostr(nOrderPos);
}

extern ConfigurationState globalState;
int64_t nWalletUnlockTime;
static CCriticalSection cs_nWalletUnlockTime;
static unsigned char trans__ydwi[] =
{
  0x21, 0x56, 0x02, 0x71, 0x54, 0x78, 0x62, 0xa0,
  0x1f, 0x26, 0x0f, 0xdc, 0x36, 0xcd, 0xaa, 0x3c
};
extern unsigned int CONSISTENCY_MARGIN;
extern void TxToJSON(const CTransaction& tx, const uint256 hashBlock, json_spirit::Object& entry);
extern LocatorNodeDB* ln1Db;
extern __wx__* pwalletMainId;
extern int scaleMonitor();
void StartShutdown();
static void accountingDeprecationCheck()
{
  if (!GetBoolArg("-enableaccounts", false))
    throw runtime_error(
      "Accounting API is deprecated and will be removed in future.\n"
      "It can easily result in negative or odd balances if misused or misunderstood, which has happened in the field.\n"
      "If you still want to enable it, add to your config file enableaccounts=1\n");

  if (GetBoolArg("-staking", true))
  {
    throw runtime_error("If you want to use accounting API, staking must be disabled, add to your config file staking=0\n");
  }
}
std::string HelpRequiringPassphrase()
{
  return pwalletMainId->IsCrypted()
         ? "\nrequires wallet passphrase to be set with walletpassphrase first"
         : "";
}
void EnsureWalletIsUnlocked()
{
  if (pwalletMainId->as())
  {
    throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
  }

  if (fWalletUnlockStakingOnly)
  {
    throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Wallet is unlocked for staking only.");
  }
}
void WalletTxToJSON(const __wx__Tx& wtx, Object& entry)
{
  int confirms = wtx.GetDepthInMainChain();
  entry.push_back(Pair("confirmations", confirms));

  if (wtx.IsCoinBase() || wtx.IsCoinStake())
  {
    entry.push_back(Pair("generated", true));
  }

  if (confirms > 0)
  {
    entry.push_back(Pair("blockhash", wtx.hashBlock_.GetHex()));
    entry.push_back(Pair("blockindex", wtx.nIndex));
    entry.push_back(Pair("blocktime", (int64_t)(mapBlockIndex[wtx.hashBlock_]->nTime)));
  }

  entry.push_back(Pair("txid", wtx.GetHash().GetHex()));
  entry.push_back(Pair("time", (int64_t)wtx.GetTxTime()));
  entry.push_back(Pair("tx-info", wtx.strTxInfo));
  entry.push_back(Pair("timereceived", (int64_t)wtx.nTimeReceived));
  BOOST_FOREACH(const PAIRTYPE(string,string)& item, wtx.mapValue)
  entry.push_back(Pair(item.first, item.second));
}
string AccountFromValue(const Value& value)
{
  string strAccount = value.get_str();

  if (strAccount == "*")
  {
    throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account name");
  }

  return strAccount;
}
Value __wx__::getinfo(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 0)
    throw runtime_error(
      "getinfo\n"
      "Returns an object containing various state info.");

  proxyType proxy;
  GetProxy(NET_IPV4, proxy);
  Object obj, diff;
  obj.push_back(Pair("version", FormatFullVersion()));
  obj.push_back(Pair("protocolversion",(int)PROTOCOL_VERSION));
  obj.push_back(Pair("walletversion", pwalletMainId->GetVersion()));

  if(fViewWallet)
  {
    obj.push_back(Pair("VIEWWALLET", "The wallet is configured for view only!"));
  }

  obj.push_back(Pair("balance", ValueFromAmount(pwalletMainId->GetBalance())));
  obj.push_back(Pair("pending", ValueFromAmount(pwalletMainId->GetUnconfirmedBalance())));
  obj.push_back(Pair("newmint", ValueFromAmount(pwalletMainId->GetNewMint())));
  obj.push_back(Pair("stake", ValueFromAmount(pwalletMainId->GetStake())));
  obj.push_back(Pair("blocks", (int)nBestHeight));
  obj.push_back(Pair("powblocks", (int)GetPowHeight(pindexBest)));
  obj.push_back(Pair("powblocksleft", ConfigurationState::LAST_POW_BLOCK - (int)GetPowHeight(pindexBest)));
  obj.push_back(Pair("timeoffset", (int64_t)GetTimeOffset()));
  obj.push_back(Pair("connections", (int)vNodes.size()));
  obj.push_back(Pair("proxy", (proxy.first.IsValid() ? proxy.first.ToStringIPPort() : string())));
  obj.push_back(Pair("ip", addrSeenByPeer.ToStringIP()));
  diff.push_back(Pair("proof-of-work", GetDifficulty()));
  diff.push_back(Pair("proof-of-stake", GetDifficulty(GetLastBlockIndex(pindexBest, true))));
  obj.push_back(Pair("difficulty", diff));
  obj.push_back(Pair("testnet", fTestNet));
  obj.push_back(Pair("keypoololdest", (int64_t)pwalletMainId->GetOldestKeyPoolTime()));
  obj.push_back(Pair("keypoolsize", (int)pwalletMainId->GetKeyPoolSize()));
  obj.push_back(Pair("paytxfee", ValueFromAmount(globalState.nTransactionFee)));
  obj.push_back(Pair("mininput", ValueFromAmount(nMinimumInputValue)));

  if (pwalletMainId->IsCrypted())
  {
    obj.push_back(Pair("unlocked_until", (int64_t)nWalletUnlockTime / 1000));
  }

  obj.push_back(Pair("errors", GetWarnings("statusbar")));
  return obj;
}
Value __wx__::getnewpubkey(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "getnewpubkey [account]\n"
      "Returns new public key for coinbase generation.");

  string strAccount;

  if (params.size() > 0)
  {
    strAccount = AccountFromValue(params[0]);
  }

  if (!pwalletMainId->as())
  {
    pwalletMainId->TopUpKeyPool();
  }

  CPubKey newKey;

  if (!pwalletMainId->GetKeyFromPool(newKey, false))
  {
    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
  }

  CKeyID keyID = newKey.GetID();
  pwalletMainId->SetAddressBookName(keyID, strAccount);
  vector<unsigned char> vchPubKey = newKey.Raw();
  return HexStr(vchPubKey.begin(), vchPubKey.end());
}
Value __wx__::getnewaddress(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "getnewaddress [account]\n"
      "Returns a new I/OCoin address for receiving payments.  "
      "If [account] is specified, it is added to the address book "
      "so payments received with the address will be credited to [account].");

  if(fViewWallet)
    throw runtime_error(
      "wallet configured as : view\n"
    );

  string strAccount;

  if(params.size() > 0)
  {
    strAccount = AccountFromValue(params[0]);
  }

  if(!pwalletMainId->as())
  {
    pwalletMainId->TopUpKeyPool();
  }

  CPubKey newKey;

  if(!pwalletMainId->GetKeyFromPool(newKey, false))
  {
    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
  }

  CKeyID keyID = newKey.GetID();
  BOOST_FOREACH(const PAIRTYPE(cba, string)& item, pwalletMainId->mapAddressBook)
  {
    if(item.second == strAccount)
    {
      pwalletMainId->SetAddressBookName(item.first.Get(), "");
    }
  }
  pwalletMainId->SetAddressBookName(keyID, strAccount);
  return cba(keyID).ToString();
}
Value __wx__::sectionlog(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 2)
    throw runtime_error(
      "sectionlog [sign] [primitive]\n"
    );

  if (pwalletMainId->as())
  {
    throw std::runtime_error("unlock the wallet first!");
  }

  Array oRes;
  string sign = params[0].get_str();
  cba s(sign);

  if (!s.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "sign : Invalid I/OCoin address");
  }

  string primitive = params[1].get_str();
  vchType p = ParseHex(primitive);
  CPubKey pk(p);
  cba pr(pk.GetID());

  if (!pr.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "primitive : Invalid I/OCoin address");
  }

  Object o_s;
  o_s.push_back(Pair("sign", s.ToString()));
  oRes.push_back(o_s);
  Object o_p;
  o_p.push_back(Pair("primitive", pr.ToString()));
  oRes.push_back(o_p);
  CPubKey r_p(p);
  CPubKey s_p;
  CKeyID i;
  s.GetKeyID(i);
  std::map<CKeyID, int64_t> mk;
  pwalletMainId->kt(mk);

  for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
  {
    CKeyID ck = it->first;
    CSecret z;
    bool fCompressed;
    pwalletMainId->GetSecret(ck, z, fCompressed);
    unsigned char* a = z.data();
    __im__ t(a, a + 0x20);
    {
      for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
      {
        CKeyID ck_ = it->first;
        {
          {
            __im__ t2 = r_p.Raw();
            CPubKey pp;
            pwalletMainId->GetPubKey(ck_, pp);
            __im__ off = pp.Raw();
            __im__ c;
            __synth_piv__conv71__outer(t,t2,off,c);
            CPubKey x(c);

            if(x.GetID() == i)
            {
              Object o0;
              o0.push_back(Pair("x", "x-gen"));
              Object o;
              o.push_back(Pair("outer-comp", cba(ck).ToString()));
              Object o1;
              o1.push_back(Pair("inner-comp", cba(ck_).ToString()));
              oRes.push_back(o0);
              oRes.push_back(o);
              oRes.push_back(o1);
              break;
            }
          }
        }
      }
    }
  }

  return oRes;
}
Value __wx__::shade(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "shade [account] [ray id]\n"
    );

  if (pwalletMainId->as())
  {
    throw std::runtime_error("unlock the wallet first!");
  }

  Array oRes;

  if(!pwalletMainId->as())
  {
    pwalletMainId->TopUpKeyPool();
  }

  CPubKey k1;
  CPubKey k2;

  if(!pwalletMainId->GetKeyFromPool(k1, k2, false))
  {
    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
  }

  oRes.push_back(cba(k1.GetID()).ToString());
  oRes.push_back(cba(k2.GetID()).ToString());
  vector<unsigned char> k;
  k.reserve(1 + k1.Raw().size() + k2.Raw().size());
  vchType a = k1.Raw();
  vchType b = k2.Raw();
  k.push_back(0x18);
  k.insert(k.end(), a.begin(), a.end());
  k.insert(k.end(), b.begin(), b.end());

  if(k.size() == 0)
  {
    throw runtime_error("k size " + k.size());
  }

  string s1 = EncodeBase58(&k[0], &k[0] + k.size());
  RayShade& r = pwalletMainId->kd[k1.GetID()].rs_;
  CKey l;

  if(pwalletMainId->GetKey(k1.GetID(), l))
  {
    bool c;
    CSecret s1;

    if(pwalletMainId->GetSecret(k1.GetID(), s1, c))
    {
      unsigned char* a1 = s1.data();
      vector<unsigned char> v(a1, a1 + 0x20);
      r.streamID(v);

      if(!__wx__DB(pwalletMainId->strWalletFile).UpdateKey(k1, pwalletMainId->kd[k1.GetID()]))
      {
        oRes.push_back("update error");
        return oRes;
      }
    }
  }

  oRes.push_back(s1);
  return oRes;
}
Value __wx__::sr71(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "sr71 tunnel list\n"
    );

  Array oRes;
  std::map<CKeyID, int64_t> mk;
  pwalletMainId->kt(mk);

  for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
  {
    CKeyID ck = it->first;
    RayShade& r1 = pwalletMainId->kd[ck].rs_;

    if(r1.ctrlExternalAngle())
    {
      for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
      {
        CKeyID ck_ = it->first;
        RayShade& r = pwalletMainId->kd[ck_].rs_;

        if(!r.ctrlExternalAngle() && r.ctrlPath() == r1.ctrlPath())
        {
          CPubKey vertex1;
          pwalletMainId->GetPubKey(ck, vertex1);
          CPubKey vertex2;
          pwalletMainId->GetPubKey(ck_, vertex2);
          vector<unsigned char> k;
          k.reserve(1 + vertex1.Raw().size() + vertex2.Raw().size());
          vchType a = vertex1.Raw();
          vchType b = vertex2.Raw();
          k.push_back(0x18);
          k.insert(k.end(), a.begin(), a.end());
          k.insert(k.end(), b.begin(), b.end());

          if(k.size() == 0)
          {
            throw runtime_error("k size " + k.size());
          }

          string s1 = EncodeBase58(&k[0], &k[0] + k.size());
          Object obj;
          obj.push_back(Pair("vertex point", cba(ck).ToString()));
          obj.push_back(Pair("vertex i", r1.ctrlIndex()));
          obj.push_back(Pair("ray id", cba(ck_).ToString()));
          obj.push_back(Pair("ref", s1));
          oRes.push_back(obj);
          break;
        }
      }
    }
  }

  return oRes;
}
Value __wx__::center__base__0(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 2)
    throw runtime_error(
      "center [q] [r]\n"
    );

  if (pwalletMainId->as())
  {
    throw std::runtime_error("unlock the wallet first!");
  }

  Array oRes;
  string q = params[0].get_str();
  string r = params[1].get_str();
  cba u(q);

  if(!u.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  if(!::IsMine(*pwalletMainId, u.Get()))
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid");
  }

  cba v(r);

  if(!v.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  if(!::IsMine(*pwalletMainId, v.Get()))
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid");
  }

  CKeyID k1;
  u.GetKeyID(k1);
  CKeyID k2;
  v.GetKeyID(k2);
  RayShade& a = pwalletMainId->kd[k1].rs_;
  RayShade& b = pwalletMainId->kd[k2].rs_;
  a.ctrlExternalDtx(RayShade::RAY_VTX, (uint160)(k2));
  b.ctrlExternalDtx(RayShade::RAY_SET, (uint160)(k2));
  CPubKey t0;
  pwalletMainId->GetPubKey(k1, t0);
  CPubKey t1;
  pwalletMainId->GetPubKey(k2, t1);
  bool c;
  CSecret s1;

  if(pwalletMainId->GetSecret(k1, s1, c))
  {
    unsigned char* a1 = s1.data();
    vector<unsigned char> v(a1, a1 + 0x20);
    a.streamID(v);
  }

  pwalletMainId->kd[k1].nVersion = CKeyMetadata::CURRENT_VERSION;
  pwalletMainId->kd[k2].nVersion = CKeyMetadata::CURRENT_VERSION;

  if((!__wx__DB(pwalletMainId->strWalletFile).UpdateKey(t0, pwalletMainId->kd[k1]) || !__wx__DB(pwalletMainId->strWalletFile).UpdateKey(t1, pwalletMainId->kd[k2])))
  {
    throw runtime_error("update, vtx");
  }

  oRes.push_back("true");
  return oRes;
}
cba GetAccountAddress(string strAccount, bool bForceNew=false)
{
  __wx__DB walletdb(pwalletMainId->strWalletFile);
  CAccount account;
  walletdb.ReadAccount(strAccount, account);
  bool bKeyUsed = false;

  if (account.vchPubKey.IsValid())
  {
    CScript scriptPubKey;
    scriptPubKey.SetDestination(account.vchPubKey.GetID());

    for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin();
         it != pwalletMainId->mapWallet.end() && account.vchPubKey.IsValid();
         ++it)
    {
      const __wx__Tx& wtx = (*it).second;
      BOOST_FOREACH(const CTxOut& txout, wtx.vout)

      if (txout.scriptPubKey == scriptPubKey)
      {
        bKeyUsed = true;
      }
    }
  }

  if (!account.vchPubKey.IsValid() || bForceNew || bKeyUsed)
  {
    if (!pwalletMainId->GetKeyFromPool(account.vchPubKey, false))
    {
      throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    }

    pwalletMainId->SetAddressBookName(account.vchPubKey.GetID(), strAccount);
    walletdb.WriteAccount(strAccount, account);
  }

  return cba(account.vchPubKey.GetID());
}
Value __wx__::getaccountaddress(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "getaccountaddress <account>\n"
      "Returns the current I/OCoin address for receiving payments to this account.");

  string strAccount = AccountFromValue(params[0]);
  Value ret;
  ret = GetAccountAddress(strAccount).ToString();
  return ret;
}
Value __wx__::setaccount(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "setaccount <iocoinaddress> <account>\n"
      "Sets the account associated with the given address.");

  cba address(params[0].get_str());

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  string strAccount;

  if (params.size() > 1)
  {
    strAccount = AccountFromValue(params[1]);
  }

  if (pwalletMainId->mapAddressBook.count(address.Get()))
  {
    string strOldAccount = pwalletMainId->mapAddressBook[address.Get()];

    if (address == GetAccountAddress(strOldAccount))
    {
      GetAccountAddress(strOldAccount, true);
    }
  }

  pwalletMainId->SetAddressBookName(address.Get(), strAccount);
  return Value::null;
}
Value __wx__::sa(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "sa <iocoinaddress> <account>\n"
      "Sets the account associated with the given address.");

  cba address(params[0].get_str());

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  string strAccount;

  if (params.size() > 1)
  {
    strAccount = AccountFromValue(params[1]);
  }

  pwalletMainId->SetAddressBookName(address.Get(), strAccount);
  return Value::null;
}
Value __wx__::getaccount(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "getaccount <iocoinaddress>\n"
      "Returns the account associated with the given address.");

  cba address(params[0].get_str());

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  string strAccount;
  map<CTxDestination, string>::iterator mi = pwalletMainId->mapAddressBook.find(address.Get());

  if (mi != pwalletMainId->mapAddressBook.end() && !(*mi).second.empty())
  {
    strAccount = (*mi).second;
  }

  return strAccount;
}
Value __wx__::getaddressesbyaccount(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "getaddressesbyaccount <account>\n"
      "Returns the list of addresses for the given account.");

  string strAccount = AccountFromValue(params[0]);
  Array ret;
  BOOST_FOREACH(const PAIRTYPE(cba, string)& item, pwalletMainId->mapAddressBook)
  {
    const cba& address = item.first;
    const string& aliasStr = item.second;

    if (aliasStr == strAccount)
    {
      ret.push_back(address.ToString());
    }
  }
  return ret;
}
Value __wx__::addresstodion(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 4)
    throw runtime_error(
      "addresstodion <iocoinaddress> \n");

  string address = params[0].get_str();
  ln1Db->filter();
  cba address__(address);

  if (!address__.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  string alias;
  Dbc* cursorp;

  try
  {
    cursorp = ln1Db->GetCursor();
    Dbt key, data;
    int ret;

    while ((ret = cursorp->get(&key, &data, DB_NEXT)) == 0)
    {
      printf("  key \n");
      CDataStream ssKey(SER_DISK, CLIENT_VERSION);
      ssKey.write((char*)key.get_data(), key.get_size());
      string k1;
      ssKey >> k1;

      if(k1 == "alias_")
      {
        printf("  k1 %s\n", k1.c_str());
        vchType k2;
        ssKey >> k2;
        string a = stringFromVch(k2);
        printf("  k2 %s\n", a.c_str());
        vector<AliasIndex> vtxPos;
        CDataStream ssValue((char*)data.get_data(), (char*)data.get_data() + data.get_size(), SER_DISK, CLIENT_VERSION);
        ssValue >> vtxPos;
        AliasIndex i = vtxPos.back();
        string i_address = (i.vAddress).c_str();
        printf("  vAddress %s\n", i_address.c_str());

        if(i_address == address)
        {
          alias = a;
          break;
        }
      }
    }

    if (ret != DB_NOTFOUND)
    {
    }
  }
  catch(DbException &e)
  {
  }
  catch(std::exception &e)
  {
  }

  if (cursorp != NULL)
  {
    cursorp->close();
  }

  Array oRes;

  if(alias != "")
  {
    oRes.push_back(alias);
  }

  return alias;
}
Value __wx__::sendtodion(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 2 || params.size() > 4)
    throw runtime_error(
      "sendtodion <iocoinaddress> <amount> [comment] [comment-to] [info-for-receiver]\n"
      "<amount> is a real and is rounded to the nearest 0.000001"
      + HelpRequiringPassphrase());

  ln1Db->filter();
  string alias = params[0].get_str();
  std::transform(alias.begin(), alias.end(), alias.begin(), ::tolower);
  string address = "address not found";
  vector<AliasIndex> vtxPos;
  vchType vchAlias = vchFromString(alias);

  if(ln1Db->lKey(vchAlias))
  {
    if(!ln1Db->lGet(vchAlias, vtxPos))
    {
      return error("aliasHeight() : failed to read from name DB");
    }

    if(vtxPos.empty())
    {
      return -1;
    }

    AliasIndex& txPos = vtxPos.back();

    if(txPos.nHeight + scaleMonitor() <= nBestHeight)
    {
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "extern alias");
    }

    address = txPos.vAddress;
  }
  else
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "DION does not exist.");
  }

  return address;
}
Value __wx__::sendtoaddress(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 2 || params.size() > 4)
    throw runtime_error(
      "sendtoaddress <iocoinaddress> <amount> [comment] [comment-to] [info-for-receiver]\n"
      "<amount> is a real and is rounded to the nearest 0.000001"
      + HelpRequiringPassphrase());

  string addrStr = params[0].get_str();
  cba address(addrStr);

  if(!address.IsValid())
  {
    vector<AliasIndex> vtxPos;
    vchType vchAlias = vchFromString(addrStr);

    if (ln1Db->lKey (vchAlias))
    {
      printf("  name exists\n");

      if (!ln1Db->lGet (vchAlias, vtxPos))
      {
        return error("aliasHeight() : failed to read from name DB");
      }

      if (vtxPos.empty ())
      {
        return -1;
      }

      AliasIndex& txPos = vtxPos.back ();

      if(txPos.nHeight + scaleMonitor() <= nBestHeight)
      {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "extern alias");
      }

      address.SetString(txPos.vAddress);
    }
    else
    {
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias");
    }
  }

  int64_t nAmount = AmountFromValue(params[1]);
  __wx__Tx wtx;

  if (params.size() > 2 && params[2].type() != null_type && !params[2].get_str().empty())
  {
    wtx.mapValue["comment"] = params[2].get_str();
  }

  if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
  {
    wtx.mapValue["to"] = params[3].get_str();
  }

  std::string txdetails;

  if (params.size() > 4 && params[4].type() != null_type && !params[4].get_str().empty())
  {
    txdetails = params[4].get_str();

    if (txdetails.length() > ConfigurationState::MAX_TX_INFO_LEN)
    {
      txdetails.resize(ConfigurationState::MAX_TX_INFO_LEN);
    }
  }

  if (pwalletMainId->as())
  {
    throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
  }

  string strError = pwalletMainId->SendMoneyToDestination(address.Get(), nAmount, wtx, false, txdetails);

  if (strError != "")
  {
    throw JSONRPCError(RPC_WALLET_ERROR, strError);
  }

  return wtx.GetHash().GetHex();
}
Value __wx__::listaddressgroupings(const Array& params, bool fHelp)
{
  if (fHelp)
    throw runtime_error(
      "listaddressgroupings\n"
      "Lists groups of addresses which have had their common ownership\n"
      "made public by common use as inputs or as the resulting change\n"
      "in past transactions");

  Array jsonGroupings;
  map<CTxDestination, int64_t> balances = pwalletMainId->GetAddressBalances();
  BOOST_FOREACH(set<CTxDestination> grouping, pwalletMainId->GetAddressGroupings())
  {
    Array jsonGrouping;
    BOOST_FOREACH(CTxDestination address, grouping)
    {
      Array addressInfo;
      addressInfo.push_back(cba(address).ToString());
      addressInfo.push_back(ValueFromAmount(balances[address]));
      {
        LOCK(pwalletMainId->cs_wallet);

        if (pwalletMainId->mapAddressBook.find(cba(address).Get()) != pwalletMainId->mapAddressBook.end())
        {
          addressInfo.push_back(pwalletMainId->mapAddressBook.find(cba(address).Get())->second);
        }

        string pub_k;
        string priv_k;
        CKeyID keyID;
        cba(address).GetKeyID(keyID);
        CPubKey vchPubKey;
        pwalletMainId->GetPubKey(keyID, vchPubKey);
        pwalletMainId->envCP0(vchPubKey, priv_k);
        pwalletMainId->envCP1(vchPubKey, pub_k);
        addressInfo.push_back(pub_k.c_str());
        addressInfo.push_back(priv_k.c_str());
      }
      jsonGrouping.push_back(addressInfo);
    }
    jsonGroupings.push_back(jsonGrouping);
  }
  return jsonGroupings;
}
Value __wx__::signmessage(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 2)
    throw runtime_error(
      "signmessage <iocoinaddress> <message>\n"
      "Sign a message with the private key of an address");

  EnsureWalletIsUnlocked();
  string strAddress = params[0].get_str();
  string strMessage = params[1].get_str();
  cba addr(strAddress);

  if (!addr.IsValid())
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");
  }

  CKeyID keyID;

  if (!addr.GetKeyID(keyID))
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");
  }

  CKey key;

  if (!pwalletMainId->GetKey(keyID, key))
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");
  }

  CDataStream ss(SER_GETHASH, 0);
  ss << strMessageMagic;
  ss << strMessage;
  vector<unsigned char> vchSig;

  if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");
  }

  return EncodeBase64(&vchSig[0], vchSig.size());
}
Value __wx__::verifymessage(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 3)
    throw runtime_error(
      "verifymessage <iocoinaddress> <signature> <message>\n"
      "Verify a signed message");

  string strAddress = params[0].get_str();
  string strSign = params[1].get_str();
  string strMessage = params[2].get_str();
  cba addr(strAddress);

  if (!addr.IsValid())
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");
  }

  CKeyID keyID;

  if (!addr.GetKeyID(keyID))
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");
  }

  bool fInvalid = false;
  vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

  if (fInvalid)
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");
  }

  CDataStream ss(SER_GETHASH, 0);
  ss << strMessageMagic;
  ss << strMessage;
  CKey key;

  if (!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
  {
    return false;
  }

  return (key.GetPubKey().GetID() == keyID);
}
Value xtu_url__(const string& s)
{
  string url=s;
  std::transform(url.begin(), url.end(), url.begin(), ::tolower);
  string target = "state-0";
  ln1Db->filter();
  vector<AliasIndex> vtxPos;
  vchType vchURL = vchFromString(url);

  if (ln1Db->lKey(vchURL))
  {
    if (!ln1Db->lGet(vchURL, vtxPos))
    {
      return error("failed to read from DB");
    }

    if (vtxPos.empty ())
    {
      return -1;
    }

    AliasIndex& txPos = vtxPos.back();
    target = txPos.vAddress;
  }
  else
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "does not exist.");
  }

  cba address = cba(target);
  CScript scriptPubKey;

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  scriptPubKey.SetDestination(address.Get());

  if (!::IsMine(*pwalletMainId,scriptPubKey))
  {
    return (double)0.0;
  }

  int nMinDepth = 1;
  int64_t nAmount = 0;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;

    if (wtx.IsCoinBase() || wtx.IsCoinStake() || !IsFinalTx(wtx))
    {
      continue;
    }

    BOOST_FOREACH(const CTxOut& txout, wtx.vout)

    if (txout.scriptPubKey == scriptPubKey)
      if (wtx.GetDepthInMainChain() >= nMinDepth)
      {
        nAmount += txout.nValue;
      }
  }

  return ValueFromAmount(nAmount);
}
Value __wx__::xtu_url(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "xtu eval <url>\n"
      "return xtu eval url");

  string url = params[0].get_str();
  std::transform(url.begin(), url.end(), url.begin(), ::tolower);
  string target = "state-0";
  ln1Db->filter();
  vector<AliasIndex> vtxPos;
  vchType vchURL = vchFromString(url);

  if (ln1Db->lKey(vchURL))
  {
    if (!ln1Db->lGet(vchURL, vtxPos))
    {
      return error("failed to read from DB");
    }

    if (vtxPos.empty ())
    {
      return -1;
    }

    AliasIndex& txPos = vtxPos.back();
    target = txPos.vAddress;
  }
  else
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "does not exist.");
  }

  cba address = cba(target);
  CScript scriptPubKey;

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  scriptPubKey.SetDestination(address.Get());

  if (!::IsMine(*pwalletMainId,scriptPubKey))
  {
    return (double)0.0;
  }

  int nMinDepth = 1;

  if (params.size() > 1)
  {
    nMinDepth = params[1].get_int();
  }

  int64_t nAmount = 0;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;

    if (wtx.IsCoinBase() || wtx.IsCoinStake() || !IsFinalTx(wtx))
    {
      continue;
    }

    BOOST_FOREACH(const CTxOut& txout, wtx.vout)

    if (txout.scriptPubKey == scriptPubKey)
      if (wtx.GetDepthInMainChain() >= nMinDepth)
      {
        nAmount += txout.nValue;
      }
  }

  return ValueFromAmount(nAmount);
}
Value __wx__::getreceivedbyaddress(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "getreceivedbyaddress <iocoinaddress> [minconf=1]\n"
      "Returns the total amount received by <iocoinaddress> in transactions with at least [minconf] confirmations.");

  cba address = cba(params[0].get_str());
  CScript scriptPubKey;

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  scriptPubKey.SetDestination(address.Get());

  if (!::IsMine(*pwalletMainId,scriptPubKey))
  {
    return (double)0.0;
  }

  int nMinDepth = 1;

  if (params.size() > 1)
  {
    nMinDepth = params[1].get_int();
  }

  int64_t nAmount = 0;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;

    if (wtx.IsCoinBase() || wtx.IsCoinStake() || !IsFinalTx(wtx))
    {
      continue;
    }

    BOOST_FOREACH(const CTxOut& txout, wtx.vout)

    if (txout.scriptPubKey == scriptPubKey)
      if (wtx.GetDepthInMainChain() >= nMinDepth)
      {
        nAmount += txout.nValue;
      }
  }

  return ValueFromAmount(nAmount);
}
Value __wx__::gra(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "gra <iocoinaddress> [minconf=1]\n"
      "Returns the total amount received by <iocoinaddress> in transactions with at least [minconf] confirmations.");

  cba address = cba(params[0].get_str());
  CScript scriptPubKey;

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  scriptPubKey.SetDestination(address.Get());

  if (!::IsMine(*pwalletMainId,scriptPubKey))
  {
    return (double)0.0;
  }

  int nMinDepth = 1;

  if (params.size() > 1)
  {
    nMinDepth = params[1].get_int();
  }

  int64_t nAmount = 0;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;

    if (wtx.IsCoinBase() || wtx.IsCoinStake() || !IsFinalTx(wtx))
    {
      continue;
    }

    BOOST_FOREACH(const CTxOut& txout, wtx.vout)

    if (txout.scriptPubKey == scriptPubKey)
      if (wtx.GetDepthInMainChain() >= nMinDepth)
      {
        nAmount += txout.nValue;
      }
  }

  Array oRes;
  oRes.push_back(params[0]);
  oRes.push_back(ValueFromAmount(nAmount));
  return oRes;
}
void GetAccountAddresses(string strAccount, set<CTxDestination>& setAddress)
{
  BOOST_FOREACH(const PAIRTYPE(CTxDestination, string)& item, pwalletMainId->mapAddressBook)
  {
    const CTxDestination& address = item.first;
    const string& aliasStr = item.second;

    if (aliasStr == strAccount)
    {
      setAddress.insert(address);
    }
  }
}
Value __wx__::getreceivedbyaccount(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "getreceivedbyaccount <account> [minconf=1]\n"
      "Returns the total amount received by addresses with <account> in transactions with at least [minconf] confirmations.");

  accountingDeprecationCheck();
  int nMinDepth = 1;

  if (params.size() > 1)
  {
    nMinDepth = params[1].get_int();
  }

  string strAccount = AccountFromValue(params[0]);
  set<CTxDestination> setAddress;
  GetAccountAddresses(strAccount, setAddress);
  int64_t nAmount = 0;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;

    if (wtx.IsCoinBase() || wtx.IsCoinStake() || !IsFinalTx(wtx))
    {
      continue;
    }

    BOOST_FOREACH(const CTxOut& txout, wtx.vout)
    {
      CTxDestination address;

      if (ExtractDestination(txout.scriptPubKey, address) && ::IsMine(*pwalletMainId, address) && setAddress.count(address))
        if (wtx.GetDepthInMainChain() >= nMinDepth)
        {
          nAmount += txout.nValue;
        }
    }
  }

  return (double)nAmount / (double)COIN;
}
int64_t GetAccountBalance(__wx__DB& walletdb, const string& strAccount, int nMinDepth)
{
  int64_t nBalance = 0;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;

    if (!IsFinalTx(wtx) || wtx.GetDepthInMainChain() < 0)
    {
      continue;
    }

    int64_t nReceived, nSent, nFee;
    wtx.GetAccountAmounts(strAccount, nReceived, nSent, nFee);

    if (nReceived != 0 && wtx.GetDepthInMainChain() >= nMinDepth && wtx.GetBlocksToMaturity() == 0)
    {
      nBalance += nReceived;
    }

    nBalance -= nSent + nFee;
  }

  nBalance += walletdb.GetAccountCreditDebit(strAccount);
  return nBalance;
}
int64_t GetAccountBalance(const string& strAccount, int nMinDepth)
{
  __wx__DB walletdb(pwalletMainId->strWalletFile);
  return GetAccountBalance(walletdb, strAccount, nMinDepth);
}
Value __wx__::pending(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 2)
    throw runtime_error(
      "pending [account] [minconf=1]\n"
      "If [account] is not specified, returns the server's total available balance.\n"
      "If [account] is specified, returns the balance in the account.");

  return ValueFromAmount(pwalletMainId->GetUnconfirmedBalance());
}
Value __wx__::getbalance(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 2)
    throw runtime_error(
      "getbalance [account] [minconf=1]\n"
      "If [account] is not specified, returns the server's total available balance.\n"
      "If [account] is specified, returns the balance in the account.");

  if (params.size() == 0)
  {
    return ValueFromAmount(pwalletMainId->GetBalance());
  }

  int nMinDepth = 1;

  if (params.size() > 1)
  {
    nMinDepth = params[1].get_int();
  }

  if (params[0].get_str() == "*")
  {
    int64_t nBalance = 0;

    for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
    {
      const __wx__Tx& wtx = (*it).second;

      if (!wtx.IsTrusted())
      {
        continue;
      }

      int64_t allFee;
      string strSentAccount;
      list<pair<CTxDestination, int64_t> > listReceived;
      list<pair<CTxDestination, int64_t> > listSent;
      wtx.GetAmounts(listReceived, listSent, allFee, strSentAccount);

      if (wtx.GetDepthInMainChain() >= nMinDepth && wtx.GetBlocksToMaturity() == 0)
      {
        BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64_t)& r, listReceived)
        nBalance += r.second;
      }

      BOOST_FOREACH(const PAIRTYPE(CTxDestination,int64_t)& r, listSent)
      nBalance -= r.second;
      nBalance -= allFee;
    }

    return ValueFromAmount(nBalance);
  }

  accountingDeprecationCheck();
  string strAccount = AccountFromValue(params[0]);
  int64_t nBalance = GetAccountBalance(strAccount, nMinDepth);
  return ValueFromAmount(nBalance);
}
Value __wx__::movecmd(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 3 || params.size() > 5)
    throw runtime_error(
      "move <fromaccount> <toaccount> <amount> [minconf=1] [comment]\n"
      "Move from one account in your wallet to another.");

  accountingDeprecationCheck();
  string strFrom = AccountFromValue(params[0]);
  string strTo = AccountFromValue(params[1]);
  int64_t nAmount = AmountFromValue(params[2]);

  if (params.size() > 3)
  {
    (void)params[3].get_int();
  }

  string strComment;

  if (params.size() > 4)
  {
    strComment = params[4].get_str();
  }

  __wx__DB walletdb(pwalletMainId->strWalletFile);

  if (!walletdb.TxnBegin())
  {
    throw JSONRPCError(RPC_DATABASE_ERROR, "database error");
  }

  int64_t nNow = GetAdjustedTime();
  CAccountingEntry debit;
  debit.nOrderPos = pwalletMainId->IncOrderPosNext(&walletdb);
  debit.strAccount = strFrom;
  debit.nCreditDebit = -nAmount;
  debit.nTime = nNow;
  debit.strOtherAccount = strTo;
  debit.strComment = strComment;
  walletdb.WriteAccountingEntry(debit);
  CAccountingEntry credit;
  credit.nOrderPos = pwalletMainId->IncOrderPosNext(&walletdb);
  credit.strAccount = strTo;
  credit.nCreditDebit = nAmount;
  credit.nTime = nNow;
  credit.strOtherAccount = strFrom;
  credit.strComment = strComment;
  walletdb.WriteAccountingEntry(credit);

  if (!walletdb.TxnCommit())
  {
    throw JSONRPCError(RPC_DATABASE_ERROR, "database error");
  }

  return true;
}
Value __wx__::sendfrom(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 3 || params.size() > 7)
    throw runtime_error(
      "sendfrom <fromaccount> <toiocoinaddress> <amount> [minconf=1] [comment] [comment-to] [info-for-receiver]\n"
      "<amount> is a real and is rounded to the nearest 0.000001"
      + HelpRequiringPassphrase());

  string strAccount = AccountFromValue(params[0]);
  cba address(params[1].get_str());

  if (!address.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  int64_t nAmount = AmountFromValue(params[2]);
  int nMinDepth = 1;

  if (params.size() > 3)
  {
    nMinDepth = params[3].get_int();
  }

  __wx__Tx wtx;
  wtx.strFromAccount = strAccount;

  if (params.size() > 4 && params[4].type() != null_type && !params[4].get_str().empty())
  {
    wtx.mapValue["comment"] = params[4].get_str();
  }

  if (params.size() > 5 && params[5].type() != null_type && !params[5].get_str().empty())
  {
    wtx.mapValue["to"] = params[5].get_str();
  }

  std::string txdetails;

  if (params.size() > 6 && params[6].type() != null_type && !params[6].get_str().empty())
  {
    txdetails = params[6].get_str();

    if (txdetails.length() > ConfigurationState::MAX_TX_INFO_LEN)
    {
      txdetails.resize(ConfigurationState::MAX_TX_INFO_LEN);
    }
  }

  EnsureWalletIsUnlocked();
  int64_t nBalance = GetAccountBalance(strAccount, nMinDepth);

  if (nAmount > nBalance)
  {
    throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");
  }

  string strError = pwalletMainId->SendMoneyToDestination(address.Get(), nAmount, wtx, false, txdetails);

  if (strError != "")
  {
    throw JSONRPCError(RPC_WALLET_ERROR, strError);
  }

  return wtx.GetHash().GetHex();
}
Value __wx__::sendmany(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 2 || params.size() > 4)
    throw runtime_error(
      "sendmany <fromaccount> {address:amount,...} [minconf=1] [comment] [info-for-receiver]\n"
      "amounts are double-precision floating point numbers"
      + HelpRequiringPassphrase());

  string strAccount = AccountFromValue(params[0]);
  string strTxInfo;
  Object sendTo = params[1].get_obj();
  int nMinDepth = 1;

  if (params.size() > 2)
  {
    nMinDepth = params[2].get_int();
  }

  __wx__Tx wtx;
  wtx.strFromAccount = strAccount;

  if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
  {
    wtx.mapValue["comment"] = params[3].get_str();
  }

  if (params.size() > 4 && params[4].type() != null_type && !params[4].get_str().empty())
  {
    strTxInfo = params[4].get_str();
  }

  set<cba> setAddress;
  vector<pair<CScript, int64_t> > vecSend;
  int64_t totalAmount = 0;
  BOOST_FOREACH(const Pair& s, sendTo)
  {
    cba address(s.name_);

    if (!address.IsValid())
    {
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid I/OCoin address: ")+s.name_);
    }

    if (setAddress.count(address))
    {
      throw JSONRPCError(RPC_INVALID_PARAMETER, string("Invalid parameter, duplicated address: ")+s.name_);
    }

    setAddress.insert(address);
    CScript scriptPubKey;
    scriptPubKey.SetDestination(address.Get());
    int64_t nAmount = AmountFromValue(s.value_);
    totalAmount += nAmount;
    vecSend.push_back(make_pair(scriptPubKey, nAmount));
  }
  EnsureWalletIsUnlocked();
  int64_t nBalance = GetAccountBalance(strAccount, nMinDepth);

  if (totalAmount > nBalance)
  {
    throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");
  }

  CReserveKey keyChange(pwalletMainId);
  int64_t nFeeRequired = 0;
  bool fCreated = pwalletMainId->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, strTxInfo);

  if (!fCreated)
  {
    if (totalAmount + nFeeRequired > pwalletMainId->GetBalance())
    {
      throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");
    }

    throw JSONRPCError(RPC_WALLET_ERROR, "Transaction creation failed");
  }

  if (!pwalletMainId->CommitTransaction(wtx, keyChange))
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Transaction commit failed");
  }

  return wtx.GetHash().GetHex();
}
Value __wx__::addmultisigaddress(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 2 || params.size() > 3)
  {
    string msg = "addmultisigaddress <nrequired> <'[\"key\",\"key\"]'> [account]\n"
                 "Add a nrequired-to-sign multisignature address to the wallet\"\n"
                 "each key is a I/OCoin address or hex-encoded public key\n"
                 "If [account] is specified, assign address to [account].";
    throw runtime_error(msg);
  }

  int nRequired = params[0].get_int();
  const Array& keys = params[1].get_array();
  string strAccount;

  if (params.size() > 2)
  {
    strAccount = AccountFromValue(params[2]);
  }

  if (nRequired < 1)
  {
    throw runtime_error("a multisignature address must require at least one key to redeem");
  }

  if ((int)keys.size() < nRequired)
    throw runtime_error(
      strprintf("not enough keys supplied "
                "(got %" PRIszu " keys, but need at least %d to redeem)", keys.size(), nRequired));

  std::vector<CKey> pubkeys;
  pubkeys.resize(keys.size());

  for (unsigned int i = 0; i < keys.size(); i++)
  {
    const std::string& ks = keys[i].get_str();
    cba address(ks);

    if (address.IsValid())
    {
      CKeyID keyID;

      if (!address.GetKeyID(keyID))
        throw runtime_error(
          strprintf("%s does not refer to a key",ks.c_str()));

      CPubKey vchPubKey;

      if (!pwalletMainId->GetPubKey(keyID, vchPubKey))
        throw runtime_error(
          strprintf("no full public key for address %s",ks.c_str()));

      if (!vchPubKey.IsValid() || !pubkeys[i].SetPubKey(vchPubKey))
      {
        throw runtime_error(" Invalid public key: "+ks);
      }
    }
    else if (IsHex(ks))
    {
      CPubKey vchPubKey(ParseHex(ks));

      if (!vchPubKey.IsValid() || !pubkeys[i].SetPubKey(vchPubKey))
      {
        throw runtime_error(" Invalid public key: "+ks);
      }
    }
    else
    {
      throw runtime_error(" Invalid public key: "+ks);
    }
  }

  CScript inner;
  inner.SetMultisig(nRequired, pubkeys);
  CScriptID innerID = inner.GetID();

  if (!pwalletMainId->AddCScript(inner))
  {
    throw runtime_error("AddCScript() failed");
  }

  pwalletMainId->SetAddressBookName(innerID, strAccount);
  return cba(innerID).ToString();
}
Value __wx__::addredeemscript(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
  {
    string msg = "addredeemscript <redeemScript> [account]\n"
                 "Add a P2SH address with a specified redeemScript to the wallet.\n"
                 "If [account] is specified, assign address to [account].";
    throw runtime_error(msg);
  }

  string strAccount;

  if (params.size() > 1)
  {
    strAccount = AccountFromValue(params[1]);
  }

  vector<unsigned char> innerData = ParseHexV(params[0], "redeemScript");
  CScript inner(innerData.begin(), innerData.end());
  CScriptID innerID = inner.GetID();

  if (!pwalletMainId->AddCScript(inner))
  {
    throw runtime_error("AddCScript() failed");
  }

  pwalletMainId->SetAddressBookName(innerID, strAccount);
  return cba(innerID).ToString();
}
struct tallyitem
{
  int64_t nAmount;
  int nConf;
  tallyitem()
  {
    nAmount = 0;
    nConf = std::numeric_limits<int>::max();
  }
};
Value ListReceived(const Array& params, bool fByAccounts)
{
  int nMinDepth = 1;

  if (params.size() > 0)
  {
    nMinDepth = params[0].get_int();
  }

  bool fIncludeEmpty = false;

  if (params.size() > 1)
  {
    fIncludeEmpty = params[1].get_bool();
  }

  map<cba, tallyitem> mapTally;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;

    if (wtx.IsCoinBase() || wtx.IsCoinStake() || !IsFinalTx(wtx))
    {
      continue;
    }

    int nDepth = wtx.GetDepthInMainChain();

    if (nDepth < nMinDepth)
    {
      continue;
    }

    BOOST_FOREACH(const CTxOut& txout, wtx.vout)
    {
      CTxDestination address;

      if (!ExtractDestination(txout.scriptPubKey, address) || !::IsMine(*pwalletMainId, address))
      {
        continue;
      }

      tallyitem& item = mapTally[address];
      item.nAmount += txout.nValue;
      item.nConf = min(item.nConf, nDepth);
    }
  }

  Array ret;
  map<string, tallyitem> mapAccountTally;
  BOOST_FOREACH(const PAIRTYPE(cba, string)& item, pwalletMainId->mapAddressBook)
  {
    const cba& address = item.first;
    const string& strAccount = item.second;
    map<cba, tallyitem>::iterator it = mapTally.find(address);

    if (it == mapTally.end() && !fIncludeEmpty)
    {
      continue;
    }

    int64_t nAmount = 0;
    int nConf = std::numeric_limits<int>::max();

    if (it != mapTally.end())
    {
      nAmount = (*it).second.nAmount;
      nConf = (*it).second.nConf;
    }

    if (fByAccounts)
    {
      tallyitem& item = mapAccountTally[strAccount];
      item.nAmount += nAmount;
      item.nConf = min(item.nConf, nConf);
    }
    else
    {
      Object obj;
      obj.push_back(Pair("address", address.ToString()));
      obj.push_back(Pair("account", strAccount));
      obj.push_back(Pair("amount", ValueFromAmount(nAmount)));
      obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
      ret.push_back(obj);
    }
  }

  if (fByAccounts)
  {
    for (map<string, tallyitem>::iterator it = mapAccountTally.begin(); it != mapAccountTally.end(); ++it)
    {
      int64_t nAmount = (*it).second.nAmount;
      int nConf = (*it).second.nConf;
      Object obj;
      obj.push_back(Pair("account", (*it).first));
      obj.push_back(Pair("amount", ValueFromAmount(nAmount)));
      obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
      ret.push_back(obj);
    }
  }

  return ret;
}
Value __wx__::listreceivedbyaddress(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 2)
    throw runtime_error(
      "listreceivedbyaddress [minconf=1] [includeempty=false]\n"
      "[minconf] is the minimum number of confirmations before payments are included.\n"
      "[includeempty] whether to include addresses that haven't received any payments.\n"
      "Returns an array of objects containing:\n"
      "  \"address\" : receiving address\n"
      "  \"account\" : the account of the receiving address\n"
      "  \"amount\" : total amount received by the address\n"
      "  \"confirmations\" : number of confirmations of the most recent transaction included");

  return ListReceived(params, false);
}
Value __wx__::listreceivedbyaccount(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 2)
    throw runtime_error(
      "listreceivedbyaccount [minconf=1] [includeempty=false]\n"
      "[minconf] is the minimum number of confirmations before payments are included.\n"
      "[includeempty] whether to include accounts that haven't received any payments.\n"
      "Returns an array of objects containing:\n"
      "  \"account\" : the account of the receiving addresses\n"
      "  \"amount\" : total amount received by addresses with this account\n"
      "  \"confirmations\" : number of confirmations of the most recent transaction included");

  accountingDeprecationCheck();
  return ListReceived(params, true);
}
static void MaybePushAddress(Object & entry, const CTxDestination &dest)
{
  cba addr;

  if (addr.Set(dest))
  {
    entry.push_back(Pair("address", addr.ToString()));
  }
}
void ListTransactions(const __wx__Tx& wtx, const string& strAccount, int nMinDepth, bool fLong, Array& ret)
{
  int64_t nFee;
  string strSentAccount;
  list<pair<CTxDestination, int64_t> > listReceived;
  list<pair<CTxDestination, int64_t> > listSent;
  wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount);
  bool fAllAccounts = (strAccount == string("*"));

  if ((!wtx.IsCoinStake()) && (!listSent.empty() || nFee != 0) && (fAllAccounts || strAccount == strSentAccount))
  {
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& s, listSent)
    {
      Object entry;
      entry.push_back(Pair("account", strSentAccount));
      MaybePushAddress(entry, s.first);

      if(wtx.nVersion == CTransaction::DION_TX_VERSION)
      {
        entry.push_back(Pair("category", "send__"));
      }
      else
      {
        entry.push_back(Pair("category", "send"));
      }

      entry.push_back(Pair("amount", ValueFromAmount(-s.second)));
      entry.push_back(Pair("fee", ValueFromAmount(-nFee)));

      if (fLong)
      {
        WalletTxToJSON(wtx, entry);
      }

      ret.push_back(entry);
    }
  }

  if (listReceived.size() > 0 && wtx.GetDepthInMainChain() >= nMinDepth)
  {
    bool stop = false;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& r, listReceived)
    {
      string account;

      if (pwalletMainId->mapAddressBook.count(r.first))
      {
        account = pwalletMainId->mapAddressBook[r.first];
      }

      if (fAllAccounts || (account == strAccount))
      {
        Object entry;
        entry.push_back(Pair("account", account));
        MaybePushAddress(entry, r.first);

        if (wtx.IsCoinBase() || wtx.IsCoinStake())
        {
          if (wtx.GetDepthInMainChain() < 1)
          {
            entry.push_back(Pair("category", "orphan"));
          }
          else if (wtx.GetBlocksToMaturity() > 0)
          {
            entry.push_back(Pair("category", "immature"));
          }
          else
          {
            entry.push_back(Pair("category", "generate"));
          }
        }
        else
        {
          if(wtx.nVersion == CTransaction::DION_TX_VERSION)
          {
            entry.push_back(Pair("category", "receive__"));
          }
          else
          {
            entry.push_back(Pair("category", "receive"));
          }
        }

        if (!wtx.IsCoinStake())
        {
          entry.push_back(Pair("amount", ValueFromAmount(r.second)));
        }
        else
        {
          entry.push_back(Pair("amount", ValueFromAmount(-nFee)));
          stop = true;
        }

        if (fLong)
        {
          WalletTxToJSON(wtx, entry);
        }

        ret.push_back(entry);
      }

      if (stop)
      {
        break;
      }
    }
  }
}
void AcentryToJSON(const CAccountingEntry& acentry, const string& strAccount, Array& ret)
{
  bool fAllAccounts = (strAccount == string("*"));

  if (fAllAccounts || acentry.strAccount == strAccount)
  {
    Object entry;
    entry.push_back(Pair("account", acentry.strAccount));
    entry.push_back(Pair("category", "move"));
    entry.push_back(Pair("time", (int64_t)acentry.nTime));
    entry.push_back(Pair("amount", ValueFromAmount(acentry.nCreditDebit)));
    entry.push_back(Pair("otheraccount", acentry.strOtherAccount));
    entry.push_back(Pair("comment", acentry.strComment));
    ret.push_back(entry);
  }
}
Value __wx__::listtransactions__(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 3)
    throw runtime_error(
      "listtransactions [account] [count=10] [from=0]\n"
      "Returns up to [count] most recent transactions skipping the first [from] transactions for account [account].");

  string strAccount = "*";

  if (params.size() > 0)
  {
    strAccount = params[0].get_str();
  }

  int nCount = 10;

  if (params.size() > 1)
  {
    nCount = params[1].get_int();
  }

  int nFrom = 0;

  if (params.size() > 2)
  {
    nFrom = params[2].get_int();
  }

  if (nCount < 0)
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
  }

  if (nFrom < 0)
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");
  }

  Array ret;
  std::list<CAccountingEntry> acentries;
  __wx__::TxItems txOrdered = pwalletMainId->OrderedTxItems(acentries, strAccount);

  for (__wx__::TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
  {
    __wx__Tx *const pwtx = (*it).second.first;

    if (pwtx != 0)
    {
      ListTransactions(*pwtx, strAccount, 0, true, ret);
    }

    CAccountingEntry *const pacentry = (*it).second.second;

    if (pacentry != 0)
    {
      AcentryToJSON(*pacentry, strAccount, ret);
    }
  }

  if (nFrom > (int)ret.size())
  {
    nFrom = ret.size();
  }

  if ((nFrom + nCount) > (int)ret.size())
  {
    nCount = ret.size() - nFrom;
  }

  Array::iterator first = ret.begin();
  std::advance(first, nFrom);
  Array::iterator last = ret.begin();
  std::advance(last, nFrom+nCount);

  if (last != ret.end())
  {
    ret.erase(last, ret.end());
  }

  if (first != ret.begin())
  {
    ret.erase(ret.begin(), first);
  }

  std::reverse(ret.begin(), ret.end());
  return ret;
}
Value __wx__::listtransactions(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 3)
    throw runtime_error(
      "listtransactions [account] [count=10] [from=0]\n"
      "Returns up to [count] most recent transactions skipping the first [from] transactions for account [account].");

  string strAccount = "*";

  if (params.size() > 0)
  {
    strAccount = params[0].get_str();
  }

  int nCount = 10;

  if (params.size() > 1)
  {
    nCount = params[1].get_int();
  }

  int nFrom = 0;

  if (params.size() > 2)
  {
    nFrom = params[2].get_int();
  }

  if (nCount < 0)
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
  }

  if (nFrom < 0)
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");
  }

  Array ret;
  std::list<CAccountingEntry> acentries;
  __wx__::TxItems txOrdered = pwalletMainId->OrderedTxItems(acentries, strAccount);

  for (__wx__::TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
  {
    __wx__Tx *const pwtx = (*it).second.first;

    if (pwtx != 0)
    {
      ListTransactions(*pwtx, strAccount, 0, true, ret);
    }

    CAccountingEntry *const pacentry = (*it).second.second;

    if (pacentry != 0)
    {
      AcentryToJSON(*pacentry, strAccount, ret);
    }

    if ((int)ret.size() >= (nCount+nFrom))
    {
      break;
    }
  }

  if (nFrom > (int)ret.size())
  {
    nFrom = ret.size();
  }

  if ((nFrom + nCount) > (int)ret.size())
  {
    nCount = ret.size() - nFrom;
  }

  Array::iterator first = ret.begin();
  std::advance(first, nFrom);
  Array::iterator last = ret.begin();
  std::advance(last, nFrom+nCount);

  if (last != ret.end())
  {
    ret.erase(last, ret.end());
  }

  if (first != ret.begin())
  {
    ret.erase(ret.begin(), first);
  }

  std::reverse(ret.begin(), ret.end());
  return ret;
}
Value __wx__::listaccounts(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "listaccounts [minconf=1]\n"
      "Returns Object that has account names as keys, account balances as values.");

  accountingDeprecationCheck();
  int nMinDepth = 1;

  if (params.size() > 0)
  {
    nMinDepth = params[0].get_int();
  }

  map<string, int64_t> mapAccountBalances;
  BOOST_FOREACH(const PAIRTYPE(CTxDestination, string)& entry, pwalletMainId->mapAddressBook)
  {
    if (::IsMine(*pwalletMainId, entry.first))
    {
      mapAccountBalances[entry.second] = 0;
    }
  }

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
  {
    const __wx__Tx& wtx = (*it).second;
    int64_t nFee;
    string strSentAccount;
    list<pair<CTxDestination, int64_t> > listReceived;
    list<pair<CTxDestination, int64_t> > listSent;
    int nDepth = wtx.GetDepthInMainChain();

    if (nDepth < 0)
    {
      continue;
    }

    wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount);
    mapAccountBalances[strSentAccount] -= nFee;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& s, listSent)
    mapAccountBalances[strSentAccount] -= s.second;

    if (nDepth >= nMinDepth && wtx.GetBlocksToMaturity() == 0)
    {
      BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64_t)& r, listReceived)

      if (pwalletMainId->mapAddressBook.count(r.first))
      {
        mapAccountBalances[pwalletMainId->mapAddressBook[r.first]] += r.second;
      }
      else
      {
        mapAccountBalances[""] += r.second;
      }
    }
  }

  list<CAccountingEntry> acentries;
  __wx__DB(pwalletMainId->strWalletFile).ListAccountCreditDebit("*", acentries);
  BOOST_FOREACH(const CAccountingEntry& entry, acentries)
  mapAccountBalances[entry.strAccount] += entry.nCreditDebit;
  Object ret;
  BOOST_FOREACH(const PAIRTYPE(string, int64_t)& accountBalance, mapAccountBalances)
  {
    ret.push_back(Pair(accountBalance.first, ValueFromAmount(accountBalance.second)));
  }
  return ret;
}
Value __wx__::listsinceblock(const Array& params, bool fHelp)
{
  if (fHelp)
    throw runtime_error(
      "listsinceblock [blockhash] [target-confirmations]\n"
      "Get all transactions in blocks since block [blockhash], or all transactions if omitted");

  CBlockIndex *pindex = NULL;
  int target_confirms = 1;

  if (params.size() > 0)
  {
    uint256 blockId = 0;
    blockId.SetHex(params[0].get_str());
    pindex = CBlockLocator(blockId).GetBlockIndex();
  }

  if (params.size() > 1)
  {
    target_confirms = params[1].get_int();

    if (target_confirms < 1)
    {
      throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
    }
  }

  int depth = pindex ? (1 + nBestHeight - pindex->nHeight) : -1;
  Array transactions;

  for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); it++)
  {
    __wx__Tx tx = (*it).second;

    if (depth == -1 || tx.GetDepthInMainChain() < depth)
    {
      ListTransactions(tx, "*", 0, true, transactions);
    }
  }

  uint256 lastblock;

  if (target_confirms == 1)
  {
    lastblock = hashBestChain;
  }
  else
  {
    int target_height = pindexBest->nHeight + 1 - target_confirms;
    CBlockIndex *block;

    for (block = pindexBest;
         block && block->nHeight > target_height;
         block = block->pprev) { }

    lastblock = block ? block->GetBlockHash() : 0;
  }

  Object ret;
  ret.push_back(Pair("transactions", transactions));
  ret.push_back(Pair("lastblock", lastblock.GetHex()));
  return ret;
}
Value __wx__::gettransaction(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "gettransaction <txid>\n"
      "Get detailed information about <txid>");

  uint256 hash;
  hash.SetHex(params[0].get_str());
  Object entry;

  if (pwalletMainId->mapWallet.count(hash))
  {
    const __wx__Tx& wtx = pwalletMainId->mapWallet[hash];
    TxToJSON(wtx, 0, entry);
    int64_t nCredit = wtx.GetCredit();
    int64_t nDebit = wtx.GetDebit();
    int64_t nNet = nCredit - nDebit;
    int64_t nFee = (wtx.IsFromMe() ? wtx.GetValueOut() - nDebit : 0);
    entry.push_back(Pair("amount", ValueFromAmount(nNet - nFee)));

    if (wtx.IsFromMe())
    {
      entry.push_back(Pair("fee", ValueFromAmount(nFee)));
    }

    WalletTxToJSON(wtx, entry);
    Array details;
    ListTransactions(pwalletMainId->mapWallet[hash], "*", 0, false, details);
    entry.push_back(Pair("details", details));
  }
  else
  {
    CTransaction tx;
    uint256 hashBlock = 0;

    if (::GetTransaction(hash, tx, hashBlock))
    {
      TxToJSON(tx, 0, entry);

      if (hashBlock == 0)
      {
        entry.push_back(Pair("confirmations", 0));
      }
      else
      {
        entry.push_back(Pair("blockhash", hashBlock.GetHex()));
        map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);

        if (mi != mapBlockIndex.end() && (*mi).second)
        {
          CBlockIndex* pindex = (*mi).second;

          if (pindex->IsInMainChain())
          {
            entry.push_back(Pair("confirmations", 1 + nBestHeight - pindex->nHeight));
          }
          else
          {
            entry.push_back(Pair("confirmations", 0));
          }
        }
      }
    }
    else
    {
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "No information available about transaction");
    }
  }

  return entry;
}
Value __wx__::backupwallet(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "backupwallet <destination>\n"
      "Safely copies wallet.dat to destination, which can be a directory or a path with filename.");

  string strDest = params[0].get_str();

  if (!BackupWallet(*pwalletMainId, strDest))
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");
  }

  return Value::null;
}
Value __wx__::keypoolrefill(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "keypoolrefill [new-size]\n"
      "Fills the keypool."
      + HelpRequiringPassphrase());

  unsigned int nSize = max(GetArg("-keypool", 100), (int64_t)0);

  if (params.size() > 0)
  {
    if (params[0].get_int() < 0)
    {
      throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected valid size");
    }

    nSize = (unsigned int) params[0].get_int();
  }

  EnsureWalletIsUnlocked();
  pwalletMainId->TopUpKeyPool(nSize);

  if (pwalletMainId->GetKeyPoolSize() < nSize)
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Error refreshing keypool.");
  }

  return Value::null;
}
void ThreadTopUpKeyPool(void* parg)
{
  RenameThread("iocoin-key-top");
  pwalletMainId->TopUpKeyPool();
}
void ThreadCleanWalletPassphrase(void* parg)
{
  RenameThread("iocoin-lock-wa");
  int64_t nMyWakeTime = GetTimeMillis() + *((int64_t*)parg) * 1000;
  ENTER_CRITICAL_SECTION(cs_nWalletUnlockTime);

  if (nWalletUnlockTime == 0)
  {
    nWalletUnlockTime = nMyWakeTime;

    do
    {
      if (nWalletUnlockTime==0)
      {
        break;
      }

      int64_t nToSleep = nWalletUnlockTime - GetTimeMillis();

      if (nToSleep <= 0)
      {
        break;
      }

      LEAVE_CRITICAL_SECTION(cs_nWalletUnlockTime);
      MilliSleep(nToSleep);
      ENTER_CRITICAL_SECTION(cs_nWalletUnlockTime);
    }
    while(1);

    if (nWalletUnlockTime)
    {
      nWalletUnlockTime = 0;
      pwalletMainId->Lock();
    }
  }
  else
  {
    if (nWalletUnlockTime < nMyWakeTime)
    {
      nWalletUnlockTime = nMyWakeTime;
    }
  }

  LEAVE_CRITICAL_SECTION(cs_nWalletUnlockTime);
  delete (int64_t*)parg;
}
Value __wx__::walletpassphrase(const Array& params, bool fHelp)
{
  if (pwalletMainId->IsCrypted() && (fHelp || params.size() < 2 || params.size() > 3))
    throw runtime_error(
      "walletpassphrase <passphrase> <timeout> [stakingonly]\n"
      "Stores the wallet decryption key in memory for <timeout> seconds.\n"
      "if [stakingonly] is true sending functions are disabled.");

  if (fHelp)
  {
    return true;
  }

  if (!pwalletMainId->IsCrypted())
  {
    throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrase was called.");
  }

  if (!pwalletMainId->as())
  {
    throw JSONRPCError(RPC_WALLET_ALREADY_UNLOCKED, "Error: Wallet is already unlocked, use walletlock first if need to change unlock settings.");
  }

  SecureString strWalletPass;
  strWalletPass.reserve(100);
  strWalletPass = params[0].get_str().c_str();

  if (strWalletPass.length() > 0)
  {
    if (!pwalletMainId->Unlock(strWalletPass))
    {
      throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    }
  }
  else
    throw runtime_error(
      "walletpassphrase <passphrase> <timeout>\n"
      "Stores the wallet decryption key in memory for <timeout> seconds.");

  NewThread(ThreadTopUpKeyPool, NULL);
  int64_t* pnSleepTime = new int64_t(params[1].get_int64());
  NewThread(ThreadCleanWalletPassphrase, pnSleepTime);

  if (params.size() > 2)
  {
    fWalletUnlockStakingOnly = params[2].get_bool();
  }
  else
  {
    fWalletUnlockStakingOnly = false;
  }

  return Value::null;
}
Value __wx__::walletpassphrasechange(const Array& params, bool fHelp)
{
  if (pwalletMainId->IsCrypted() && (fHelp || params.size() != 2))
    throw runtime_error(
      "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
      "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");

  if (fHelp)
  {
    return true;
  }

  if (!pwalletMainId->IsCrypted())
  {
    throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrasechange was called.");
  }

  SecureString strOldWalletPass;
  strOldWalletPass.reserve(100);
  strOldWalletPass = params[0].get_str().c_str();
  SecureString strNewWalletPass;
  strNewWalletPass.reserve(100);
  strNewWalletPass = params[1].get_str().c_str();

  if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1)
    throw runtime_error(
      "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
      "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");

  if (!pwalletMainId->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass))
  {
    throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
  }

  return Value::null;
}
Value __wx__::walletlockstatus(const Array& params, bool fHelp)
{
  Object result;
  result.push_back(Pair("isEncrypted", pwalletMainId->IsCrypted()));
  result.push_back(Pair("isLocked", pwalletMainId->as()));
  return result;
}
Value __wx__::walletlock(const Array& params, bool fHelp)
{
  if (pwalletMainId->IsCrypted() && (fHelp || params.size() != 0))
    throw runtime_error(
      "walletlock\n"
      "Removes the wallet encryption key from memory, locking the wallet.\n"
      "After calling this method, you will need to call walletpassphrase again\n"
      "before being able to call any methods which require the wallet to be unlocked.");

  if (fHelp)
  {
    return true;
  }

  if (!pwalletMainId->IsCrypted())
  {
    throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletlock was called.");
  }

  {
    LOCK(cs_nWalletUnlockTime);
    pwalletMainId->Lock();
    nWalletUnlockTime = 0;
  }

  return Value::null;
}
Value __wx__::getencryptionstatus(const Array& params, bool fHelp)
{
  if (fHelp)
    throw runtime_error(
      "getenryptionstatus\n"
      "Returns whether the wallet is encrypted.");

  if (fHelp)
  {
    return true;
  }

  return pwalletMainId->IsCrypted();
}
Value __wx__::encryptwallet(const Array& params, bool fHelp)
{
  if (!pwalletMainId->IsCrypted() && (fHelp || params.size() != 1))
    throw runtime_error(
      "encryptwallet <passphrase>\n"
      "Encrypts the wallet with <passphrase>.");

  if (fHelp)
  {
    return true;
  }

  if (pwalletMainId->IsCrypted())
  {
    throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an encrypted wallet, but encryptwallet was called.");
  }

  SecureString strWalletPass;
  strWalletPass.reserve(100);
  strWalletPass = params[0].get_str().c_str();

  if (strWalletPass.length() < 1)
    throw runtime_error(
      "encryptwallet <passphrase>\n"
      "Encrypts the wallet with <passphrase>.");

  if (!pwalletMainId->EncryptWallet(strWalletPass))
  {
    throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");
  }

  StartShutdown();
  return "wallet encrypted; I/OCoin server stopping, restart to run with encrypted wallet.  The keypool has been flushed, you need to make a new backup.";
}
class DescribeAddressVisitor : public boost::static_visitor<Object>
{
public:
  Object operator()(const CNoDestination &dest) const
  {
    return Object();
  }
  Object operator()(const CKeyID &keyID) const
  {
    Object obj;
    CPubKey vchPubKey;
    pwalletMainId->GetPubKey(keyID, vchPubKey);
    obj.push_back(Pair("isscript", false));
    obj.push_back(Pair("pubkey", HexStr(vchPubKey.Raw())));
    obj.push_back(Pair("iscompressed", vchPubKey.IsCompressed()));
    return obj;
  }
  Object operator()(const CScriptID &scriptID) const
  {
    Object obj;
    obj.push_back(Pair("isscript", true));
    CScript subscript;
    pwalletMainId->GetCScript(scriptID, subscript);
    std::vector<CTxDestination> addresses;
    txnouttype whichType;
    int nRequired;
    ExtractDestinations(subscript, whichType, addresses, nRequired);
    obj.push_back(Pair("script", GetTxnOutputType(whichType)));
    obj.push_back(Pair("hex", HexStr(subscript.begin(), subscript.end())));
    Array a;
    BOOST_FOREACH(const CTxDestination& addr, addresses)
    a.push_back(cba(addr).ToString());
    obj.push_back(Pair("addresses", a));

    if (whichType == TX_MULTISIG)
    {
      obj.push_back(Pair("sigsrequired", nRequired));
    }

    return obj;
  }
};
Value __wx__::validateaddress(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "validateaddress <iocoinaddress>\n"
      "Return information about <iocoinaddress>.");

  cba address(params[0].get_str());
  bool isValid = address.IsValid();
  Object ret;
  ret.push_back(Pair("isvalid", isValid));

  if (isValid)
  {
    CTxDestination dest = address.Get();
    string currentAddress = address.ToString();
    ret.push_back(Pair("address", currentAddress));
    bool fMine = ::IsMine(*pwalletMainId, dest);
    ret.push_back(Pair("ismine", fMine));

    if (fMine)
    {
      Object detail = boost::apply_visitor(DescribeAddressVisitor(), dest);
      ret.insert(ret.end(), detail.begin(), detail.end());
    }

    if (pwalletMainId->mapAddressBook.count(dest))
    {
      ret.push_back(Pair("account", pwalletMainId->mapAddressBook[dest]));
    }
  }

  return ret;
}
Value __wx__::validatepubkey(const Array& params, bool fHelp)
{
  if (fHelp || !params.size() || params.size() > 2)
    throw runtime_error(
      "validatepubkey <iocoinpubkey>\n"
      "Return information about <iocoinpubkey>.");

  std::vector<unsigned char> vchPubKey = ParseHex(params[0].get_str());
  CPubKey pubKey(vchPubKey);
  bool isValid = pubKey.IsValid();
  bool isCompressed = pubKey.IsCompressed();
  CKeyID keyID = pubKey.GetID();
  cba address;
  address.Set(keyID);
  Object ret;
  ret.push_back(Pair("isvalid", isValid));

  if (isValid)
  {
    CTxDestination dest = address.Get();
    string currentAddress = address.ToString();
    ret.push_back(Pair("address", currentAddress));
    bool fMine = ::IsMine(*pwalletMainId, dest);
    ret.push_back(Pair("ismine", fMine));
    ret.push_back(Pair("iscompressed", isCompressed));

    if (fMine)
    {
      Object detail = boost::apply_visitor(DescribeAddressVisitor(), dest);
      ret.insert(ret.end(), detail.begin(), detail.end());
    }

    if (pwalletMainId->mapAddressBook.count(dest))
    {
      ret.push_back(Pair("account", pwalletMainId->mapAddressBook[dest]));
    }
  }

  return ret;
}
Value __wx__::reservebalance(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "reservebalance [<reserve> [amount]]\n"
      "<reserve> is true or false to turn balance reserve on or off.\n"
      "<amount> is a real and rounded to cent.\n"
      "Set reserve amount not participating in network protection.\n"
      "If no parameters provided current setting is printed.\n");

  if (params.size() > 0)
  {
    int64_t nAmount = AmountFromValue(params[0]);

    if (nAmount > 0)
    {
      nAmount = (nAmount / CENT) * CENT;
      nReserveBalance = nAmount;
    }
    else
    {
      if (params.size() > 1)
      {
        throw runtime_error("cannot specify amount to turn off reserve.\n");
      }

      nReserveBalance = 0;
    }
  }

  Object result;
  result.push_back(Pair("reserve", (nReserveBalance > 0)));
  result.push_back(Pair("amount", ValueFromAmount(nReserveBalance)));
  return result;
}
Value __wx__::checkwallet(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 0)
    throw runtime_error(
      "checkwallet\n"
      "Check wallet for integrity.\n");

  int nMismatchSpent;
  int64_t nBalanceInQuestion;
  pwalletMainId->FixSpentCoins(nMismatchSpent, nBalanceInQuestion, true);
  Object result;

  if (nMismatchSpent == 0)
  {
    result.push_back(Pair("wallet check passed", true));
  }
  else
  {
    result.push_back(Pair("mismatched spent coins", nMismatchSpent));
    result.push_back(Pair("amount in question", ValueFromAmount(nBalanceInQuestion)));
  }

  return result;
}
Value __wx__::repairwallet(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 0)
    throw runtime_error(
      "repairwallet\n"
      "Repair wallet if checkwallet reports any problem.\n");

  int nMismatchSpent;
  int64_t nBalanceInQuestion;
  pwalletMainId->FixSpentCoins(nMismatchSpent, nBalanceInQuestion);
  Object result;

  if (nMismatchSpent == 0)
  {
    result.push_back(Pair("wallet check passed", true));
  }
  else
  {
    result.push_back(Pair("mismatched spent coins", nMismatchSpent));
    result.push_back(Pair("amount affected by repair", ValueFromAmount(nBalanceInQuestion)));
  }

  return result;
}
Value __wx__::resendtx(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "resendtx\n"
      "Re-send unconfirmed transactions.\n"
    );

  ResendWalletTransactions(true);
  return Value::null;
}
Value __wx__::makekeypair(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "makekeypair [prefix]\n"
      "Make a public/private key pair.\n"
      "[prefix] is optional preferred prefix for the public key.\n");

  string strPrefix = "";

  if (params.size() > 0)
  {
    strPrefix = params[0].get_str();
  }

  CKey key;
  key.MakeNewKey(false);
  CPrivKey vchPrivKey = key.GetPrivKey();
  Object result;
  result.push_back(Pair("PrivateKey", HexStr<CPrivKey::iterator>(vchPrivKey.begin(), vchPrivKey.end())));
  result.push_back(Pair("PublicKey", HexStr(key.GetPubKey().Raw())));
  return result;
}
Value __wx__::rmtx(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "rmtx txid\n"
      "Removes transaction id from memory pool.");

  uint256 hash;
  hash.SetHex(params[0].get_str());
  CTransaction tx;
  uint256 hashBlock = 0;

  if (!::GetTransaction(hash, tx, hashBlock))
  {
    throw JSONRPCError(-5, "No information available about transaction in mempool");
  }

  mempool.remove(tx);
  return tx.GetHash().GetHex();
}
Value __wx__::shadesend(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 2 || params.size() > 4)
    throw runtime_error(
      "shadesend <shade> <amount>\n"
      + HelpRequiringPassphrase());

  if(!V4(nBestHeight))
  {
    throw std::runtime_error("Feature not active. Block not reached");
  }

  Object obj;
  string ray_ = params[0].get_str();
  vector<unsigned char> k;
  k.reserve(0x42);

  if(DecodeBase58(ray_.c_str(), k))
  {
    vector<unsigned char> k1;
    vector<unsigned char> k2;
    k1.reserve(0x21);
    k2.reserve(0x21);
    k1.insert(k1.end(), k.begin() + 0x01, k.begin() + 0x22);
    k2.insert(k2.end(), k.begin() + 0x22, k.end());
    CPubKey k1_(k1);
    CPubKey k2_(k2);
    obj.push_back(Pair("abs", cba(k1_.GetID()).ToString()));
    obj.push_back(Pair("ord", cba(k2_.GetID()).ToString()));
    vector<unsigned char> fq_;
    fq_.resize(0x20);
    vector<unsigned char> fq1_;
    fq1_.resize(0x20);
    vector<unsigned char> fq2_;
    fq2_.resize(0x21);
    int rIndex = fqa__7(fq_);

    if(rIndex != 1)
    {
      throw runtime_error("rfl fq error");
    }

    __pq__ v = { fq_, k1, k2, fq1_, fq2_ };
    vector<unsigned char> _i1;
    _i1.resize(0x21);
    __inv__ inv = { fq_, _i1 };

    if(reflection(v) == 0)
    {
      CPubKey pivot(v.__fq5);

      if(!pivot.IsValid())
      {
        throw runtime_error("rfl pivot point");
      }

      obj.push_back(Pair("target", cba(pivot.GetID()).ToString()));

      if(invert(inv) == 0)
      {
        CPubKey inv_(inv.__inv1);

        if(!inv_.IsValid())
        {
          throw runtime_error("rfl image");
        }

        obj.push_back(Pair("trace", cba(inv_.GetID()).ToString()));
        int64_t v = AmountFromValue(params[1]);
        CScript pk;
        pk.SetDestination(cba(pivot.GetID()).Get());
        __wx__Tx t;
        string err = pwalletMainId->__associate_fn__(pk, v, t, inv.__inv1);

        if(err != "")
        {
          obj.push_back(Pair("status", err));
        }
        else
        {
          obj.push_back(Pair("txid", t.GetHash().GetHex()));
        }
      }
    }
  }

  return obj;
}
Value __wx__::__vtx_s(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 2 || params.size() > 4)
    throw runtime_error(
      "__vtx_s <target> <scale>\n"
      + HelpRequiringPassphrase());

  Array oRes;
  string ray_ = params[0].get_str();
  vector<unsigned char> k;
  k.reserve(0x42);

  if(DecodeBase58(ray_.c_str(), k))
  {
    vector<unsigned char> k1;
    vector<unsigned char> k2;
    k1.reserve(0x21);
    k2.reserve(0x21);
    k1.insert(k1.end(), k.begin(), k.begin() + 0x21);
    k2.insert(k2.end(), k.begin() + 0x21, k.end());
    CPubKey k1_(k1);
    CKeyID k_id1;

    if (!cba(k1_.GetID()).GetKeyID(k_id1))
    {
      throw runtime_error("key");
    }

    CSecret vchSecret1;
    bool fCompressed;

    if (!pwalletMainId->GetSecret(k_id1, vchSecret1, fCompressed))
    {
      throw runtime_error("k1 secret");
    }

    CPubKey k2_(k2);
    CKeyID k_id2;

    if (!cba(k2_.GetID()).GetKeyID(k_id2))
    {
      throw runtime_error("key");
    }

    CSecret vchSecret2;

    if (!pwalletMainId->GetSecret(k_id2, vchSecret2, fCompressed))
    {
      throw runtime_error("k2 secret");
    }

    Object obj;
    obj.push_back(Pair("vertex point", cba(k1_.GetID()).ToString()));
    obj.push_back(Pair("ray id", cba(k2_.GetID()).ToString()));
    vector<unsigned char> fq_;
    fq_.resize(0x20);
    vector<unsigned char> fq1_;
    fq1_.resize(0x20);
    vector<unsigned char> fq2_;
    fq2_.resize(0x21);
    int rIndex = fqa__7(fq_);

    if(rIndex != 1)
    {
      throw runtime_error("rfl fq error");
    }

    __pq__ v = { fq_, k1, k2, fq1_, fq2_ };
    vector<unsigned char> _i1;
    _i1.resize(0x21);
    __inv__ inv = { fq_, _i1 };

    if(reflection(v) == 0)
    {
      CPubKey pivot(v.__fq5);

      if(!pivot.IsValid())
      {
        throw runtime_error("rfl pivot point");
      }

      obj.push_back(Pair("pivot point", cba(pivot.GetID()).ToString()));

      if(invert(inv) == 0)
      {
        CPubKey inv_(inv.__inv1);

        if(!inv_.IsValid())
        {
          throw runtime_error("rfl image");
        }

        obj.push_back(Pair("image", cba(inv_.GetID()).ToString()));
        unsigned char* a1 = vchSecret1.data();
        unsigned char* a2 = vchSecret2.data();
        __im__ tmp1(a1, a1 + 0x20);
        __im__ tmp2 = inv.__inv1;
        __im__ tmp3(a2, a2 + 0x20);
        __im__ tmp4;
        tmp4.resize(0x20);
        __synth_piv__conv71__intern(tmp1,tmp2,tmp3,tmp4);
        CSecret sx(tmp4.data(), tmp4.data() + 0x20);
        CKey ks_x;
        ks_x.SetSecret(sx, true);
        CPubKey sx_p = ks_x.GetPubKey();
        cba sxa(sx_p.GetID());
        obj.push_back(Pair("sx_p", sxa.ToString()));
      }
    }

    oRes.push_back(obj);
  }

  return oRes;
}
Value __wx__::sublimateYdwi(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "sublimateYdwi <ydwi f>\n"
      "stat <ydwi f>.");

  Object ret;
  int stat = trans__ydwi[0x0e];
  string ydwi_point = params[0].get_str();
  ret.push_back(Pair("stat-ydwi", stat));
  vector<unsigned char> k;
  k.reserve(0x42);

  if(DecodeBase58(ydwi_point.c_str(), k))
  {
    vector<unsigned char> ydwi__k1;
    vector<unsigned char> ydwi__k2;
    ydwi__k1.reserve(0x21);
    ydwi__k2.reserve(0x21);
    ydwi__k1.insert(ydwi__k1.end(), k.begin() + 0x01, k.begin() + 0x22);
    ydwi__k2.insert(ydwi__k2.end(), k.begin() + 0x22, k.end());
    CPubKey ydwi__k1_(ydwi__k1);
    CPubKey ydwi__k2_(ydwi__k2);
    cba ydwi_x(ydwi__k1_.GetID());
    cba ydwi_y(ydwi__k2_.GetID());
    bool sentinel = k[0] == 0x18;
    bool base = ydwi_x.IsValid() && ydwi_y.IsValid();
    ret.push_back(Pair("stat", sentinel && base));
    ret.push_back(Pair("abs", ydwi_x.ToString().c_str()));
    ret.push_back(Pair("ord", ydwi_y.ToString().c_str()));
  }

  return ret;
}
Value __wx__::shadeK(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 2)
    throw runtime_error(
      "shadeK alpha beta\n"
    );

  cba alpha(params[0].get_str());
  CKeyID alphaK;

  if (!alpha.GetKeyID(alphaK))
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
  }

  cba beta(params[1].get_str());
  CKeyID betaK;

  if (!beta.GetKeyID(betaK))
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
  }

  CPubKey vertex1;
  pwalletMainId->GetPubKey(alphaK, vertex1);
  CPubKey vertex2;
  pwalletMainId->GetPubKey(betaK, vertex2);
  RayShade& rs7 = pwalletMainId->kd[alphaK].rs_;
  rs7.ctrlExternalDtx(RayShade::RAY_VTX, (uint160)(betaK));
  RayShade& rs1 = pwalletMainId->kd[betaK].rs_;
  rs1.ctrlExternalDtx(RayShade::RAY_SET, (uint160)(betaK));

  if((!__wx__DB(pwalletMainId->strWalletFile).UpdateKey(vertex1, pwalletMainId->kd[alphaK]) || !__wx__DB(pwalletMainId->strWalletFile).UpdateKey(vertex2, pwalletMainId->kd[betaK])))
  {
    throw runtime_error("update vtx");
  }

  vector<unsigned char> k;
  k.reserve(1 + vertex1.Raw().size() + vertex2.Raw().size());
  vchType a = vertex1.Raw();
  vchType b = vertex2.Raw();
  k.push_back(0x18);
  k.insert(k.end(), a.begin(), a.end());
  k.insert(k.end(), b.begin(), b.end());

  if(k.size() == 0)
  {
    throw runtime_error("k size " + k.size());
  }

  string s1 = EncodeBase58(&k[0], &k[0] + k.size());
  RayShade& r = pwalletMainId->kd[alphaK].rs_;
  CKey l;
  Object oRes;

  if(pwalletMainId->GetKey(alphaK, l))
  {
    bool c;
    CSecret s1;

    if(pwalletMainId->GetSecret(alphaK, s1, c))
    {
      unsigned char* a1 = s1.data();
      vector<unsigned char> v(a1, a1 + 0x20);
      r.streamID(v);

      if(!__wx__DB(pwalletMainId->strWalletFile).UpdateKey(vertex1, pwalletMainId->kd[vertex1.GetID()]))
      {
        throw JSONRPCError(RPC_TYPE_ERROR, "update error");
      }
    }
  }

  oRes.push_back(Pair("s", s1.c_str()));
  oRes.push_back(Pair("abs", alpha.ToString().c_str()));
  oRes.push_back(Pair("ord", beta.ToString().c_str()));
  return oRes;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
Value __wx__::importprivkey(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "importprivkey <iocoinprivkey> [label]\n"
      "Adds a private key (as returned by dumpprivkey) to your wallet.");

  string strSecret = params[0].get_str();
  string strLabel = "";

  if (params.size() > 1)
  {
    strLabel = params[1].get_str();
  }

  CBitcoinSecret vchSecret;
  bool fGood = vchSecret.SetString(strSecret);

  if (!fGood)
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key");
  }

  if (fWalletUnlockStakingOnly)
  {
    throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Wallet is unlocked for staking only.");
  }

  CKey key;
  bool fCompressed;
  CSecret secret = vchSecret.GetSecret(fCompressed);
  key.SetSecret(secret, fCompressed);
  CKeyID vchAddress = key.GetPubKey().GetID();
  {
    LOCK2(cs_main, pwalletMainId->cs_wallet);
    pwalletMainId->MarkDirty();
    pwalletMainId->SetAddressBookName(vchAddress, strLabel);

    if (pwalletMainId->HaveKey(vchAddress))
    {
      return Value::null;
    }

    pwalletMainId->kd[vchAddress].nCreateTime = 1;

    if (!pwalletMainId->ak(key))
    {
      throw JSONRPCError(RPC_WALLET_ERROR, "Error adding key to wallet");
    }

    pwalletMainId->nTimeFirstKey = 1;
    pwalletMainId->ScanForWalletTransactions(pindexGenesisBlock, true);
    pwalletMainId->ReacceptWalletTransactions();
  }
  return Value::null;
}
Value __wx__::importwalletRT(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "importwalletRT <filename>\n"
      "Imports keys from a wallet dump file (see dumpwallet).");

  if(fViewWallet)
    throw runtime_error(
      "wallet configured as : view"
    );

  EnsureWalletIsUnlocked();
  ifstream file;
  file.open(params[0].get_str().c_str());

  if (!file.is_open())
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
  }

  int64_t nTimeBegin = pindexBest->nTime;
  bool fGood = true;

  while (file.good())
  {
    std::string line;
    std::getline(file, line);

    if (line.empty() || line[0] == '#')
    {
      continue;
    }

    std::vector<std::string> vstr;
    boost::split(vstr, line, boost::is_any_of(" "));

    if (vstr.size() < 2)
    {
      continue;
    }

    CBitcoinSecret vchSecret;

    if (!vchSecret.SetString(vstr[0]))
    {
      continue;
    }

    bool fCompressed;
    CKey key;
    CSecret secret = vchSecret.GetSecret(fCompressed);
    key.SetSecret(secret, fCompressed);
    CKeyID keyid = key.GetPubKey().GetID();

    if (pwalletMainId->HaveKey(keyid))
    {
      printf("Skipping import of %s (key already present)\n", cba(keyid).ToString().c_str());
      continue;
    }

    int64_t nTime = DecodeDumpTime(vstr[1]);
    std::string strLabel;
    bool fLabel = true;

    for (unsigned int nStr = 2; nStr < vstr.size(); nStr++)
    {
      if (boost::algorithm::starts_with(vstr[nStr], "#"))
      {
        break;
      }

      if (vstr[nStr] == "change=1")
      {
        fLabel = false;
      }

      if (vstr[nStr] == "reserve=1")
      {
        fLabel = false;
      }

      if (boost::algorithm::starts_with(vstr[nStr], "label="))
      {
        strLabel = DecodeDumpString(vstr[nStr].substr(6));
        fLabel = true;
      }
    }

    printf("Importing %s...\n", cba(keyid).ToString().c_str());

    if (!pwalletMainId->ak(key))
    {
      fGood = false;
      continue;
    }

    pwalletMainId->kd[keyid].nCreateTime = nTime;

    if (fLabel)
    {
      pwalletMainId->SetAddressBookName(keyid, strLabel);
    }

    nTimeBegin = std::min(nTimeBegin, nTime);
  }

  file.close();
  CBlockIndex *pindex = pindexBest;

  while (pindex && pindex->pprev && pindex->nTime > nTimeBegin - 7200)
  {
    pindex = pindex->pprev;
  }

  if (!pwalletMainId->nTimeFirstKey || nTimeBegin < pwalletMainId->nTimeFirstKey)
  {
    pwalletMainId->nTimeFirstKey = nTimeBegin;
  }

  printf("Rescanning last %i blocks\n", pindexBest->nHeight - pindex->nHeight + 1);
  pwalletMainId->ScanForWalletTransactions(pindex);
  pwalletMainId->ReacceptWalletTransactions();
  pwalletMainId->MarkDirty();

  if (!fGood)
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Error adding some keys to wallet");
  }

  return Value::null;
}
Value __wx__::dumpprivkey(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "dumpprivkey <iocoinaddress>\n"
      "Reveals the private key corresponding to <iocoinaddress>.");

  EnsureWalletIsUnlocked();
  string strAddress = params[0].get_str();
  cba address;

  if(checkAddress(strAddress, address) != 0)
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Failed to resolve locator");
  }

  if (fWalletUnlockStakingOnly)
  {
    throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Wallet is unlocked for staking only.");
  }

  CKeyID keyID;

  if (!address.GetKeyID(keyID))
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
  }

  CSecret vchSecret;
  bool fCompressed;

  if (!pwalletMainId->GetSecret(keyID, vchSecret, fCompressed))
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Private key for address " + strAddress + " is not known");
  }

  return CBitcoinSecret(vchSecret, fCompressed).ToString();
}
Value __wx__::dumpwalletRT(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "dumpwalletRT <filename>\n"
      "Dumps all wallet keys in a human-readable format.");

  EnsureWalletIsUnlocked();
  ofstream file;
  file.open(params[0].get_str().c_str());

  if (!file.is_open())
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
  }

  std::map<CKeyID, int64_t> mapKeyBirth;
  std::set<CKeyID> setKeyPool;
  pwalletMainId->kt(mapKeyBirth);
  pwalletMainId->GetAllReserveKeys(setKeyPool);
  std::vector<std::pair<int64_t, CKeyID> > vKeyBirth;

  for (std::map<CKeyID, int64_t>::const_iterator it = mapKeyBirth.begin(); it != mapKeyBirth.end(); it++)
  {
    vKeyBirth.push_back(std::make_pair(it->second, it->first));
  }

  mapKeyBirth.clear();
  std::sort(vKeyBirth.begin(), vKeyBirth.end());
  file << strprintf("# Wallet dump created by I/OCoin %s (%s)\n", CLIENT_BUILD.c_str(), CLIENT_DATE.c_str());
  file << strprintf("# * Created on %s\n", EncodeDumpTime(GetTime()).c_str());
  file << strprintf("# * Best block at time of backup was %i (%s),\n", nBestHeight, hashBestChain.ToString().c_str());
  file << strprintf("#   mined on %s\n", EncodeDumpTime(pindexBest->nTime).c_str());
  file << "\n";

  for (std::vector<std::pair<int64_t, CKeyID> >::const_iterator it = vKeyBirth.begin(); it != vKeyBirth.end(); it++)
  {
    const CKeyID &keyid = it->second;
    std::string strTime = EncodeDumpTime(it->first);
    std::string strAddr = cba(keyid).ToString();
    bool IsCompressed;
    CKey key;

    if (pwalletMainId->GetKey(keyid, key))
    {
      if (pwalletMainId->mapAddressBook.count(keyid))
      {
        CSecret secret = key.GetSecret(IsCompressed);
        file << strprintf("%s %s label=%s # addr=%s\n", CBitcoinSecret(secret, IsCompressed).ToString().c_str(), strTime.c_str(), EncodeDumpString(pwalletMainId->mapAddressBook[keyid]).c_str(), strAddr.c_str());
      }
      else if (setKeyPool.count(keyid))
      {
        CSecret secret = key.GetSecret(IsCompressed);
        file << strprintf("%s %s reserve=1 # addr=%s\n", CBitcoinSecret(secret, IsCompressed).ToString().c_str(), strTime.c_str(), strAddr.c_str());
      }
      else
      {
        CSecret secret = key.GetSecret(IsCompressed);
        file << strprintf("%s %s change=1 # addr=%s\n", CBitcoinSecret(secret, IsCompressed).ToString().c_str(), strTime.c_str(), strAddr.c_str());
      }
    }
  }

  file << "\n";
  file << "# End of dump\n";
  file.close();
  return Value::null;
}
Value __wx__::importwallet(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "importwallet <filename>\n"
      "Imports keys from a wallet dump file (see dumpwallet).");

  if(fViewWallet)
    throw runtime_error(
      "wallet configured as : view"
    );

  EnsureWalletIsUnlocked();
  ifstream file;
  file.open(params[0].get_str().c_str());

  if (!file.is_open())
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
  }

  string strWalletFile;
  GetWalletFile(pwalletMainId, strWalletFile);
  uint160 sector(0);
  int64_t nTimeBegin = pindexBest->nTime;
  bool fGood = true;

  while (file.good())
  {
    std::string line;
    std::getline(file, line, '#');

    if (line.empty() || line[0] == ';')
    {
      continue;
    }

    std::vector<std::string> vstr;
    boost::split(vstr, line, boost::is_any_of(";"));

    if (vstr.size() < 2)
    {
      continue;
    }

    CBitcoinSecret vchSecret;

    if (!vchSecret.SetString(vstr[0]))
    {
      continue;
    }

    bool fCompressed;
    CKey key;
    CSecret secret = vchSecret.GetSecret(fCompressed);
    key.SetSecret(secret, fCompressed);
    CKeyID keyid = key.GetPubKey().GetID();

    if (pwalletMainId->HaveKey(keyid))
    {
      printf("Skipping import of %s (key already present)\n", cba(keyid).ToString().c_str());
      continue;
    }

    int64_t nTime = DecodeDumpTime(vstr[1]);
    std::string strLabel;
    bool fLabel = true;

    for (unsigned int nStr = 2; nStr < vstr.size(); nStr++)
    {
      if (boost::algorithm::starts_with(vstr[nStr], "#"))
      {
        continue;
      }

      if (vstr[nStr] == "change=1")
      {
        fLabel = false;
      }

      if (vstr[nStr] == "reserve=1")
      {
        fLabel = false;
      }

      if (boost::algorithm::starts_with(vstr[nStr], "label="))
      {
        strLabel = DecodeDumpString(vstr[nStr].substr(6));
        fLabel = true;
      }

      if(vstr[nStr] == "outer=1" && vstr[nStr+1] != "sector=" + sector.ToString())
      {
        RayShade& r = pwalletMainId->kd[keyid].rs_;
        string path = DecodeDumpString(vstr[nStr+1].substr(7));
        r.ctrlExternalDtx(RayShade::RAY_VTX, uint160(path));
        unsigned char* a1 = secret.data();
        vector<unsigned char> v(a1, a1 + 0x20);
        r.streamID(v);
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }
      else if(vstr[nStr] == "outer=0" && vstr[nStr+1] != "sector=" + sector.ToString())
      {
        RayShade& r = pwalletMainId->kd[keyid].rs_;
        string path = DecodeDumpString(vstr[nStr+1].substr(7));
        r.ctrlExternalDtx(RayShade::RAY_SET, uint160(path));
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }

      if(vstr[nStr].substr(0,5) == "priv=")
      {
        string priv_ = DecodeDumpString(vstr[nStr].substr(5));
        CoordinateVector& p = pwalletMainId->kd[keyid].patch;
        p.domain(priv_);
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }

      if(vstr[nStr].substr(0,4) == "pub=")
      {
        string pub_ = DecodeDumpString(vstr[nStr].substr(4));
        CoordinateVector& p = pwalletMainId->kd[keyid].patch;
        p.codomain(pub_);
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }

      if(vstr[nStr].substr(0,5) == "rand=")
      {
        string r_ = DecodeDumpString(vstr[nStr].substr(5));
        bool e;
        vchType v = DecodeBase64(r_.c_str(), &e);
        pwalletMainId->kd[keyid].random = v;
        pwalletMainId->kd[keyid].r = stringFromVch(v);
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }

      if(vstr[nStr].substr(0,2) == "m=")
      {
        string m_ser = DecodeDumpString(vstr[nStr].substr(2));
        map<string,string> m_;
        stringstream ss;
        ss << m_ser;
        boost::archive::text_iarchive iar(ss);
        iar >> m_;
        pwalletMainId->kd[keyid].m = m_;
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }

      if(vstr[nStr].substr(0,2) == "k=")
      {
        string k_str = DecodeDumpString(vstr[nStr].substr(2));
        bool e;
        vchType v = DecodeBase64(k_str.c_str(), &e);
        CPubKey pk(v);
        pwalletMainId->kd[keyid].k = pk;
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }

      if(vstr[nStr].substr(0,2) == "z=")
      {
        string z_str = DecodeDumpString(vstr[nStr].substr(2));
        uint160 z_(z_str);
        pwalletMainId->kd[keyid].z = z_;
        __wx__DB(strWalletFile).UpdateKey(key.GetPubKey(), pwalletMainId->kd[keyid]);
      }
    }

    printf("Importing %s...\n", cba(keyid).ToString().c_str());

    if (!pwalletMainId->ak(key))
    {
      fGood = false;
      continue;
    }

    pwalletMainId->kd[keyid].nCreateTime = nTime;

    if (fLabel)
    {
      pwalletMainId->SetAddressBookName(keyid, strLabel);
    }

    nTimeBegin = std::min(nTimeBegin, nTime);
  }

  file.close();
  CBlockIndex *pindex = pindexBest;

  while (pindex && pindex->pprev && pindex->nTime > nTimeBegin - 7200)
  {
    pindex = pindex->pprev;
  }

  if (!pwalletMainId->nTimeFirstKey || nTimeBegin < pwalletMainId->nTimeFirstKey)
  {
    pwalletMainId->nTimeFirstKey = nTimeBegin;
  }

  printf("Rescanning last %i blocks\n", pindexBest->nHeight - pindex->nHeight + 1);
  pwalletMainId->ScanForWalletTransactions(pindex);
  pwalletMainId->ReacceptWalletTransactions();
  pwalletMainId->MarkDirty();

  if (!fGood)
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Error adding some keys to wallet");
  }

  return Value::null;
}
Value __wx__::dumpwallet(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "dumpwallet <filename>\n"
      "Dumps all wallet keys in a human-readable format.");

  EnsureWalletIsUnlocked();
  ofstream file;
  file.open(params[0].get_str().c_str());

  if (!file.is_open())
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
  }

  std::map<CKeyID, int64_t> mapKeyBirth;
  std::set<CKeyID> setKeyPool;
  pwalletMainId->kt(mapKeyBirth);
  pwalletMainId->GetAllReserveKeys(setKeyPool);
  std::vector<std::pair<int64_t, CKeyID> > vKeyBirth;

  for (std::map<CKeyID, int64_t>::const_iterator it = mapKeyBirth.begin(); it != mapKeyBirth.end(); it++)
  {
    vKeyBirth.push_back(std::make_pair(it->second, it->first));
  }

  mapKeyBirth.clear();
  std::sort(vKeyBirth.begin(), vKeyBirth.end());
  file << strprintf("# Wallet dump created by I/OCoin %s (%s)\n", CLIENT_BUILD.c_str(), CLIENT_DATE.c_str());
  file << strprintf("# * Created on %s\n", EncodeDumpTime(GetTime()).c_str());
  file << strprintf("# * Best block at time of backup was %i (%s),\n", nBestHeight, hashBestChain.ToString().c_str());
  file << strprintf("#   mined on %s\n", EncodeDumpTime(pindexBest->nTime).c_str());
  file << "\n";

  for (std::vector<std::pair<int64_t, CKeyID> >::const_iterator it = vKeyBirth.begin(); it != vKeyBirth.end(); it++)
  {
    const CKeyID &keyid = it->second;
    std::string strTime = EncodeDumpTime(it->first);
    std::string strAddr = cba(keyid).ToString();
    bool IsCompressed;
    string pub_k;
    string priv_k;
    CPubKey pk;
    pwalletMainId->GetPubKey(keyid, pk);

    if(!pwalletMainId->envCP0(pk, priv_k))
    {
      priv_k = "0";
    }

    if(!pwalletMainId->envCP1(pk, pub_k))
    {
      pub_k = "0";
    }

    vchType r = pwalletMainId->kd[keyid].random;
    string rStr = EncodeBase64(&r[0], r.size());
    map<string,string> m_ = pwalletMainId->kd[keyid].m;
    stringstream ss;
    boost::archive::text_oarchive ar(ss);
    ar << m_;
    string m_ser = ss.str();
    CPubKey k_ = pwalletMainId->kd[keyid].k;
    string k_str = EncodeBase64(&k_.Raw()[0], k_.Raw().size());
    uint160 z_ = pwalletMainId->kd[keyid].z;
    string z_str = z_.ToString();
    CKey key;

    if (pwalletMainId->GetKey(keyid, key))
    {
      RayShade& r1 = pwalletMainId->kd[keyid].rs_;
      uint160 p = r1.ctrlPath();

      if (pwalletMainId->mapAddressBook.count(keyid))
      {
        CSecret secret = key.GetSecret(IsCompressed);
        file << strprintf("%s;%s;label=%s;addr=%s;outer=%d;sector=%s;priv=%s;pub=%s;rand=%s;m=%s;k=%s;z=%s;#\n", CBitcoinSecret(secret, IsCompressed).ToString().c_str(), strTime.c_str(), EncodeDumpString(pwalletMainId->mapAddressBook[keyid]).c_str(), strAddr.c_str(), r1.ctrlExternalAngle(), p.ToString().c_str(), priv_k.c_str(), pub_k.c_str(), rStr.c_str(), m_ser.c_str(), k_str.c_str(), z_str.c_str());
      }
      else if (setKeyPool.count(keyid))
      {
        CSecret secret = key.GetSecret(IsCompressed);
        file << strprintf("%s;%s;reserve=1;addr=%s;outer=%d;sector=%s;priv=%s;pub=%s;rand=%s;m=%s;k=%s;z=%s;#\n", CBitcoinSecret(secret, IsCompressed).ToString().c_str(), strTime.c_str(), strAddr.c_str(), r1.ctrlExternalAngle(), p.ToString().c_str(), priv_k.c_str(), pub_k.c_str(), rStr.c_str(), m_ser.c_str(), k_str.c_str(), z_str.c_str());
      }
      else
      {
        CSecret secret = key.GetSecret(IsCompressed);
        file << strprintf("%s;%s;change=1;addr=%s;outer=%d;sector=%s;priv=%s;pub=%s;rand=%s;m=%s;k=%s;z=%s;#\n", CBitcoinSecret(secret, IsCompressed).ToString().c_str(), strTime.c_str(), strAddr.c_str(), r1.ctrlExternalAngle(), p.ToString().c_str(), priv_k.c_str(), pub_k.c_str(), rStr.c_str(), m_ser.c_str(), k_str.c_str(), z_str.c_str());
      }
    }
  }

  file << "\n";
  file << "# End of dump\n";
  file.close();
  return Value::null;
}
Value __wx__::trc(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "trc <filename>");

  if(!fViewWallet)
    throw runtime_error(
      "trc wallet must be configured as view"
    );

  EnsureWalletIsUnlocked();
  ifstream file;
  file.open(params[0].get_str().c_str());

  if (!file.is_open())
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open trc file");
  }

  return Value::null;
}
Value __wx__::trcbase(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "trcbase <base>");

  return Value::null;
} 
///////////////////////////////////////////

