// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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


using namespace std;


static unsigned int GetStakeSplitAge() { return IsProtocolV2(nBestHeight) ? (10 * 24 * 60 * 60) : (1 * 24 * 60 * 60); }
static int64_t GetStakeCombineThreshold() { return IsProtocolV2(nBestHeight) ? (50 * COIN) : (1000 * COIN); }

bool isAliasTx(const __wx__Tx* tx);
extern __wx__* pwalletMain;
extern CScript aliasStrip(const CScript& scriptIn);
extern bool aliasScript(const CScript& script, int& op, vector<vector<unsigned char> > &vvch);
//////////////////////////////////////////////////////////////////////////////
//
// mapWallet
//

struct CompareValueOnly
{
    bool operator()(const pair<int64_t, pair<const __wx__Tx*, unsigned int> >& t1,
	const pair<int64_t, pair<const __wx__Tx*, unsigned int> >& t2) const
    {
  return t1.first < t2.first;
    }
};

CPubKey __wx__::GenerateNewKey()
{
    AssertLockHeld(cs_wallet); // kd
    bool fCompressed = CanSupportFeature(FEATURE_COMPRPUBKEY); // default to compressed public keys if we want 0.6.0 wallets

    RandAddSeedPerfmon();
    CKey key;
    key.MakeNewKey(fCompressed);

    // Compressed public keys were introduced in version 0.6.0
    if (fCompressed)
  SetMinVersion(FEATURE_COMPRPUBKEY);

    CPubKey pubkey = key.GetPubKey();

    // Create new metadata
    int64_t nCreationTime = GetTime();
    kd[pubkey.GetID()] = CKeyMetadata(nCreationTime);
    if (!nTimeFirstKey || nCreationTime < nTimeFirstKey)
  nTimeFirstKey = nCreationTime;

    if (!ak(key))
  throw std::runtime_error("__wx__::GenerateNewKey() : ak failed");
    return key.GetPubKey();
}


bool __wx__::ak(const CKey& key)
{
    AssertLockHeld(cs_wallet); // kd

    CPubKey pubkey = key.GetPubKey();

    if (!CCryptoKeyStore::ak(key))
  return false;
    if (!fFileBacked)
  return true;
    if (!IsCrypted())
  return __wx__DB(strWalletFile).WriteKey(pubkey, key.GetPrivKey(), kd[pubkey.GetID()]);
    return true;
}

bool __wx__::sync(const CPubKey &vchPubKey, const vector<unsigned char> &vchCryptedSecret)
{
    if (!CCryptoKeyStore::sync(vchPubKey, vchCryptedSecret))
  return false;
    if (!fFileBacked)
  return true;
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
    AssertLockHeld(cs_wallet); // kd
    if (meta.nCreateTime && (!nTimeFirstKey || meta.nCreateTime < nTimeFirstKey))
  nTimeFirstKey = meta.nCreateTime;

    kd[pubkey.GetID()] = meta;
    return true;
}

bool __wx__::LoadRelay(const vchType& k, const Relay& r)
{
    AssertLockHeld(cs_wallet); // kd
    lCache[k] = r;
    return true;
}

bool __wx__::envCP0(const CPubKey &pubkey, string& rsaPrivKey)
{
    AssertLockHeld(cs_wallet); // kd
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
    AssertLockHeld(cs_wallet); // kd
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
    AssertLockHeld(cs_wallet); // kd

    r = kd[pubkey.GetID()].random;
    r_ = kd[pubkey.GetID()].r;

    return true;
}

bool __wx__::SetRandomKeyMetadata(const CPubKey& pubkey, const vchType &r)
{
    AssertLockHeld(cs_wallet); // kd

    kd[pubkey.GetID()].random = r;
    kd[pubkey.GetID()].r = stringFromVch(r);

    return true;
}

bool __wx__::aes_(const CPubKey& pubkey, string& f, string& aesPlainBase64)
{
    AssertLockHeld(cs_wallet); // kd

    aesPlainBase64 = kd[pubkey.GetID()].m[f];
    
    if(aesPlainBase64 != "")
      return true;

    return false;
}

bool __wx__::aes(const CPubKey &pubkey, string& f, string& aes256KeyBase64)
{
    AssertLockHeld(cs_wallet); // kd

    kd[pubkey.GetID()].m[f] = aes256KeyBase64;

    return true;
}

bool __wx__::vtx_(const CPubKey& pubkey, string& a)
{
    AssertLockHeld(cs_wallet); // kd

    a = kd[pubkey.GetID()].q.back();
    
    if(a != "")
      return true;

    return false;
}

bool __wx__::vtx(const CPubKey &pubkey, string& a)
{
    AssertLockHeld(cs_wallet); // kd

    kd[pubkey.GetID()].q.push_back(a);

    return true;
}

bool __wx__::SetRSAMetadata(const CPubKey &pubkey)
{
    AssertLockHeld(cs_wallet); // kd

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
  return false;
    if (!fFileBacked)
  return true;
    return __wx__DB(strWalletFile).WriteCScript(Hash160(redeemScript), redeemScript);
}

// optional setting to unlock wallet for staking only
// serves to disable the trivial sendmoney when OS account compromised
// provides no real security
bool fWalletUnlockStakingOnly = false;

bool __wx__::LoadCScript(const CScript& redeemScript)
{
    /* A sanity check was added in pull #3843 to avoid adding redeemScripts
     * that never can be redeemed. However, old wallets may still contain
     * these. Do not add them to the wallet and warn. */
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
    {
  std::string strAddr = cba(redeemScript.GetID()).ToString();
  printf("%s: Warning: This wallet contains a redeemScript of size %"PRIszu" which exceeds maximum size %i thus can never be redeemed. Do not use address %s.\n",
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
  return false;

    CCrypter crypter;
    CKeyingMaterial vMasterKey;

    {
  LOCK(cs_wallet);
  BOOST_FOREACH(const MasterKeyMap::value_type& pMasterKey, mapMasterKeys)
  {
      if(!crypter.SetKeyFromPassphrase(strWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
    return false;
      if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
    return false;
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
    return false;
      if (!crypter.Decrypt(pMasterKey.second.vchCryptedKey, vMasterKey))
    return false;
      if (CCryptoKeyStore::Unlock(vMasterKey))
      {
    int64_t nStartTime = GetTimeMillis();
    crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
    pMasterKey.second.nDeriveIterations = pMasterKey.second.nDeriveIterations * (100 / ((double)(GetTimeMillis() - nStartTime)));

    nStartTime = GetTimeMillis();
    crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod);
    pMasterKey.second.nDeriveIterations = (pMasterKey.second.nDeriveIterations + pMasterKey.second.nDeriveIterations * 100 / ((double)(GetTimeMillis() - nStartTime))) / 2;

    if (pMasterKey.second.nDeriveIterations < 25000)
	pMasterKey.second.nDeriveIterations = 25000;

    printf("Wallet passphrase changed to an nDeriveIterations of %i\n", pMasterKey.second.nDeriveIterations);

    if (!crypter.SetKeyFromPassphrase(strNewWalletPassphrase, pMasterKey.second.vchSalt, pMasterKey.second.nDeriveIterations, pMasterKey.second.nDerivationMethod))
	return false;
    if (!crypter.Encrypt(vMasterKey, pMasterKey.second.vchCryptedKey))
	return false;
    __wx__DB(strWalletFile).WriteMasterKey(pMasterKey.first, pMasterKey.second);
    if (fWasLocked)
	Lock();
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
    LOCK(cs_wallet); // nWalletVersion
    if (nWalletVersion >= nVersion)
  return true;

    // when doing an explicit upgrade, if we pass the max version permitted, upgrade all the way
    if (fExplicit && nVersion > nWalletMaxVersion)
      nVersion = FEATURE_LATEST;

    nWalletVersion = nVersion;

    if (nVersion > nWalletMaxVersion)
  nWalletMaxVersion = nVersion;

    if (fFileBacked)
    {
  __wx__DB* pwalletdb = pwalletdbIn ? pwalletdbIn : new __wx__DB(strWalletFile);
  if (nWalletVersion > 40000)
      pwalletdb->WriteMinVersion(nWalletVersion);
  if (!pwalletdbIn)
      delete pwalletdb;
    }

    return true;
}

bool __wx__::SetMaxVersion(int nVersion)
{
    LOCK(cs_wallet); // nWalletVersion, nWalletMaxVersion
    // cannot downgrade below current version
    if (nWalletVersion > nVersion)
  return false;

    nWalletMaxVersion = nVersion;

    return true;
}

bool __wx__::EncryptWallet(const SecureString& strWalletPassphrase)
{
    if (IsCrypted())
  return false;

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
  kMasterKey.nDeriveIterations = 25000;

    printf("Encrypting Wallet with an nDeriveIterations of %i\n", kMasterKey.nDeriveIterations);

    if (!crypter.SetKeyFromPassphrase(strWalletPassphrase, kMasterKey.vchSalt, kMasterKey.nDeriveIterations, kMasterKey.nDerivationMethod))
  return false;
    if (!crypter.Encrypt(vMasterKey, kMasterKey.vchCryptedKey))
  return false;

    {
  LOCK(cs_wallet);
  mapMasterKeys[++nMasterKeyMaxID] = kMasterKey;
  if (fFileBacked)
  {
      pwalletdbEncryption = new __wx__DB(strWalletFile);
      if (!pwalletdbEncryption->TxnBegin())
    return false;
      pwalletdbEncryption->WriteMasterKey(nMasterKeyMaxID, kMasterKey);
  }

  if (!EncryptKeys(vMasterKey))
  {
      if (fFileBacked)
    pwalletdbEncryption->TxnAbort();
      exit(1); //We now probably have half of our keys encrypted in memory, and half not...die and let the user reload their unencrypted wallet.
  }

  // Encryption was introduced in version 0.4.0
  SetMinVersion(FEATURE_WALLETCRYPT, pwalletdbEncryption, true);

  if (fFileBacked)
  {
      if (!pwalletdbEncryption->TxnCommit())
    exit(1); //We now have keys encrypted in memory, but no on disk...die to avoid confusion and let the user reload their unencrypted wallet.

      delete pwalletdbEncryption;
      pwalletdbEncryption = NULL;
  }

  Lock();
  Unlock(strWalletPassphrase);
  NewKeyPool();
  Lock();

  // Need to completely rewrite the wallet file; if we don't, bdb might keep
  // bits of the unencrypted private key in slack space in the database file.
  CDB::Rewrite(strWalletFile);

    }
    NotifyStatusChanged(this);

    return true;
}

int64_t __wx__::IncOrderPosNext(__wx__DB *pwalletdb)
{
    AssertLockHeld(cs_wallet); // nOrderPosNext
    int64_t nRet = nOrderPosNext++;
    if (pwalletdb) {
  pwalletdb->WriteOrderPosNext(nOrderPosNext);
    } else {
  __wx__DB(strWalletFile).WriteOrderPosNext(nOrderPosNext);
    }
    return nRet;
}

__wx__::TxItems __wx__::OrderedTxItems(std::list<CAccountingEntry>& acentries, std::string strAccount)
{
    AssertLockHeld(cs_wallet); // mapWallet
    __wx__DB walletdb(strWalletFile);

    // First: get all __wx__Tx and CAccountingEntry into a sorted-by-order multimap.
    TxItems txOrdered;

    // Note: maintaining indices in the database of (account,time) --> txid and (account, time) --> acentry
    // would make this much faster for applications that do this a lot.
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
    // Anytime a signature is successfully verified, it's proof the outpoint is spent.
    // Update the wallet spent flag if it doesn't know due to wallet.dat being
    // restored from backup or the user making copies of wallet.dat.
    {
  LOCK(cs_wallet);
  BOOST_FOREACH(const CTxIn& txin, tx.vin)
  {
      map<uint256, __wx__Tx>::iterator mi = mapWallet.find(txin.prevout.hash);
      if (mi != mapWallet.end())
      {
    __wx__Tx& wtx = (*mi).second;
    if (txin.prevout.n >= wtx.vout.size())
	printf("WalletUpdateSpent: bad wtx %s\n", wtx.GetHash().ToString().c_str());
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
  // Inserts only if not already there, returns tx inserted or tx found
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
      // Tolerate times up to the last timestamp in the wallet not more than 5 minutes into the future
      int64_t latestTolerated = latestNow + 300;
      std::list<CAccountingEntry> acentries;
      TxItems txOrdered = OrderedTxItems(acentries);
      for (TxItems::reverse_iterator it = txOrdered.rbegin(); it != txOrdered.rend(); ++it)
      {
	  __wx__Tx *const pwtx = (*it).second.first;
	  if (pwtx == &wtx)
	continue;
	  CAccountingEntry *const pacentry = (*it).second.second;
	  int64_t nSmartTime;
	  if (pwtx)
	  {
	nSmartTime = pwtx->nTimeSmart;
	if (!nSmartTime)
	    nSmartTime = pwtx->nTimeReceived;
	  }
	  else
	nSmartTime = pacentry->nTime;
	  if (nSmartTime <= latestTolerated)
	  {
	latestEntry = nSmartTime;
	if (nSmartTime > latestNow)
	    latestNow = nSmartTime;
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
      // Merge
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

  //// debug print
  printf("AddToWallet %s  %s%s\n", wtxIn.GetHash().ToString().substr(0,10).c_str(), (fInsertedNew ? "new" : ""), (fUpdated ? "update" : ""));

  // Write to disk
  if (fInsertedNew || fUpdated)
      if (!wtx.WriteToDisk())
    return false;
#ifndef QT_GUI
  // If default receiving address gets used, replace it with a new one
  if (vchDefaultKey.IsValid()) {
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
  // since AddToWallet is called directly for self-originating transactions, check for consumption of own coins
  WalletUpdateSpent(wtx, (wtxIn.hashBlock != 0));

  // Notify UI of new or updated transaction
  NotifyTransactionChanged(this, hash, fInsertedNew ? CT_NEW : CT_UPDATED);

  // notify an external script when a wallet transaction comes in or is updated
  std::string strCmd = GetArg("-walletnotify", "");

  if ( !strCmd.empty())
  {
      boost::replace_all(strCmd, "%s", wtxIn.GetHash().GetHex());
      boost::thread t(runCommand, strCmd); // thread runs free
  }

    }
    return true;
}

// Add a transaction to the wallet, or update it.
// pblock is optional, but should be provided if the transaction is known to be in a block.
// If fUpdate is true, existing transactions will be updated.
bool __wx__::AddToWalletIfInvolvingMe(const CTransaction& tx, const CBlock* pblock, bool fUpdate, bool fFindBlock)
{
    uint256 hash = tx.GetHash();
    {
  LOCK(cs_wallet);
  bool fExisted = mapWallet.count(hash);
  if (fExisted && !fUpdate) return false;
  if (fExisted || IsMine(tx) || IsFromMe(tx))
  {
      __wx__Tx wtx(this,tx);
      // Get merkle branch if transaction was found in a block
      if (pblock)
    wtx.SetMerkleBranch(pblock);
      return AddToWallet(wtx);
  }
  else
      WalletUpdateSpent(tx);
    }
    return false;
}

bool __wx__::EraseFromWallet(uint256 hash)
{
    if (!fFileBacked)
  return false;
    {
  LOCK(cs_wallet);
  if (mapWallet.erase(hash))
      __wx__DB(strWalletFile).EraseTx(hash);
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
	return prev.vout[txin.prevout.n].nValue;
  }
    }
    return 0;
}

bool __wx__::IsChange(const CTxOut& txout) const
{
  CTxDestination address;

  // TODO: fix handling of 'change' outputs. The assumption is that any
  // payment to a TX_PUBKEYHASH that is mine but isn't in the address book
  // is change. That assumption is likely to break when we implement multisignature
  // wallets that return change back into a multi-signature-protected address;
  // a better way of identifying which outputs are 'the send' and which are
  // 'the change' will need to be implemented (maybe extend __wx__Tx to remember
  // which output, if any, was change).
  if (ExtractDestination(txout.scriptPubKey, address) && ::IsMine(*this, address))
  {
    LOCK(cs_wallet);
    if (!mapAddressBook.count(address))
	return true;
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
	      // Returns -1 if it wasn't being tracked
	      int nRequests = -1;
	      {
	    LOCK(pwallet->cs_wallet);
	    if (IsCoinBase() || IsCoinStake())
	    {
		// Generated block
		if (hashBlock != 0)
		{
	      map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
	      if (mi != pwallet->mapRequestCount.end())
		  nRequests = (*mi).second;
	  }
      }
      else
      {
	  // Did anyone request this transaction?
	  map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(GetHash());
	  if (mi != pwallet->mapRequestCount.end())
	  {
	      nRequests = (*mi).second;

	      // How about the block it's in?
	      if (nRequests == 0 && hashBlock != 0)
	      {
		  map<uint256, int>::const_iterator mi = pwallet->mapRequestCount.find(hashBlock);
		  if (mi != pwallet->mapRequestCount.end())
		      nRequests = (*mi).second;
		  else
		      nRequests = 1; // If it's in someone else's block it must have got out
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

  // Compute fee:
  int64_t nDebit = GetDebit();
  if (nDebit > 0) // debit>0 means we signed/sent this transaction
  {
      int64_t nValueOut = GetValueOut();
      nFee = nDebit - nValueOut;
  }

  // Sent/received.
  BOOST_FOREACH(const CTxOut& txout, vout)
  {
      // Skip special stake out
      if (txout.scriptPubKey.empty())
	  continue;

      vector<valtype> vs;
      txnouttype t;
      if(Solver(txout.scriptPubKey, t, vs))
      {
        if(t == TX_NULL_DATA)
          continue;
      }
     
      bool fIsMine;
      // Only need to handle txouts if AT LEAST one of these is true:
      //   1) they debit from us (sent)
      //   2) the output is to us (received)
      if (nDebit > 0)
      {
	  // Don't report 'change' txouts
	  if (pwallet->IsChange(txout))
	      continue;
	  fIsMine = pwallet->IsMine(txout);
      }
      else if (!(fIsMine = pwallet->IsMine(txout)))
	  continue;

      // In either case, we need to get the destination address
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

      // If we are debited by the transaction, add the output as a "sent" entry
      if (nDebit > 0)
	  listSent.push_back(make_pair(address, txout.nValue));

      // If we are receiving the output, add it as a "received" entry
      if(fIsMine)
	  listReceived.push_back(make_pair(address, txout.nValue));
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
		  nReceived += r.second;
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

      // This critsect is OK because txdb is already open
      {
	  LOCK(pwallet->cs_wallet);
	  map<uint256, const CMerkleTx*> mapWalletPrev;
	  set<uint256> setAlreadyDone;
	  for (unsigned int i = 0; i < vWorkQueue.size(); i++)
	  {
	      uint256 hash = vWorkQueue[i];
	      if (setAlreadyDone.count(hash))
		  continue;
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

// Scan the block chain (starting in pindexStart) for transactions
// from or to us. If fUpdate is true, found transactions that already
// exist in the wallet will be updated.
int __wx__::ScanForWalletTransactions(CBlockIndex* pindexStart, bool fUpdate)
{
  int ret = 0;

  CBlockIndex* pindex = pindexStart;
  {
      LOCK2(cs_main, cs_wallet);
      while (pindex)
      {
	  // no need to read and scan block, if block was created before
	  // our wallet birthday (as adjusted for block time variability)
	  if (nTimeFirstKey && (pindex->nTime < (nTimeFirstKey - 7200))) {
	      pindex = pindex->pnext;
	      continue;
	  }

	  CBlock block;
	  block.ReadFromDisk(pindex, true);
	  BOOST_FOREACH(CTransaction& tx, block.vtx)
	  {
	      if (AddToWalletIfInvolvingMe(tx, &block, fUpdate))
		  ret++;
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
	      continue;

	  CTxIndex txindex;
	  bool fUpdated = false;
	  if (txdb.ReadTxIndex(wtx.GetHash(), txindex))
	  {
	      // Update fSpent if a tx got spent somewhere else by a copy of wallet.dat
	      if (txindex.vSpent.size() != wtx.vout.size())
	      {
		  printf("ERROR: ReacceptWalletTransactions() : txindex.vSpent.size() %"PRIszu" != wtx.vout.size() %"PRIszu"\n", txindex.vSpent.size(), wtx.vout.size());
		  continue;
	      }
	      for (unsigned int i = 0; i < txindex.vSpent.size(); i++)
	      {
		  if (wtx.IsSpent(i))
		      continue;
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
	      // Re-accept any txes of ours that aren't already in a block
	      if (!(wtx.IsCoinBase() || wtx.IsCoinStake()))
		  wtx.AcceptWalletTransaction(txdb);
	  }
      }
      if (!vMissingTx.empty())
      {
	  // TODO: optimize this to scan just part of the block chain?
	  if (ScanForWalletTransactions(pindexGenesisBlock))
	      fRepeat = true;  // Found missing transactions: re-do re-accept.
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
	      RelayTransaction((CTransaction)tx, hash);
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
      // Do this infrequently and randomly to avoid giving away
      // that these are our transactions.
      static int64_t nNextTime;
      if (GetTime() < nNextTime)
	  return;
      bool fFirst = (nNextTime == 0);
      nNextTime = GetTime() + GetRand(30 * 60);
      if (fFirst)
	  return;

      // Only do it if there's been a new block since last time
      static int64_t nLastTime;
      if (nTimeBestReceived < nLastTime)
	  return;
      nLastTime = GetTime();
  }

  // Rebroadcast any of our txes that aren't in a block yet
  printf("ResendWalletTransactions()\n");
  CTxDB txdb("r");
  {
      LOCK(cs_wallet);
      // Sort them in chronological order
      multimap<unsigned int, __wx__Tx*> mapSorted;
      BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item, mapWallet)
      {
	  __wx__Tx& wtx = item.second;
	  // Don't rebroadcast until it's had plenty of time that
	  // it should have gotten in already by now.
	  if (fForce || nTimeBestReceived - (int64_t)wtx.nTimeReceived > 5 * 60)
	      mapSorted.insert(make_pair(wtx.nTimeReceived, &wtx));
      }
      BOOST_FOREACH(PAIRTYPE(const unsigned int, __wx__Tx*)& item, mapSorted)
      {
	  __wx__Tx& wtx = *item.second;
	  if (wtx.CheckTransaction())
	      wtx.RelayWalletTransaction(txdb);
	  else
	      printf("ResendWalletTransactions() : CheckTransaction failed for transaction %s\n", wtx.GetHash().ToString().c_str());
      }
  }
}






//////////////////////////////////////////////////////////////////////////////
//
// Actions
//


int64_t __wx__::GetBalance() const
{
  int64_t nTotal = 0;
  {
      LOCK2(cs_main, cs_wallet);
      for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
      {
	  const __wx__Tx* pcoin = &(*it).second;
	  if (pcoin->IsTrusted())
	      nTotal += pcoin->GetAvailableCredit();
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
	      nTotal += pcoin->GetAvailableCredit();
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
	      nTotal += GetCredit(pcoin);
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

// populate vCoins with vector of spendable COutputs
void __wx__::AvailableCoins(vector<COutput>& vCoins, bool fOnlyConfirmed, const CCoinControl *coinControl) const
{
  vCoins.clear();

  {
      LOCK2(cs_main, cs_wallet);
      for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
      {
	  const __wx__Tx* pcoin = &(*it).second;

	  if (!IsFinalTx(*pcoin))
	      continue;
 
	  if (fOnlyConfirmed && !pcoin->IsTrusted())
	      continue;

	  if (pcoin->IsCoinBase() && pcoin->GetBlocksToMaturity() > 0)
	      continue;

	  if(pcoin->IsCoinStake() && pcoin->GetBlocksToMaturity() > 0)
	      continue;

	  int nDepth = pcoin->GetDepthInMainChain();
	  if (nDepth < 0)
	      continue;

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
	    continue;

	  int nDepth = pcoin->GetDepthInMainChain();
	  if (nDepth < 1)
	      continue;

          if (nDepth < nStakeMinConfirmations)
            continue;


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
	      //The solver here uses a randomized algorithm,
	      //the randomness serves no real security purpose but is just
	      //needed to prevent degenerate behavior and it is important
	      //that the rng fast. We do not use a constant random sequence,
	      //because there may be some privacy improvement by making
	      //the selection random.
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

// ppcoin: total coins staked (non-spendable until maturity)
int64_t __wx__::GetStake() const
{
  int64_t nTotal = 0;
  LOCK2(cs_main, cs_wallet);
  for (map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
  {
      const __wx__Tx* pcoin = &(*it).second;
      if (pcoin->IsCoinStake() && pcoin->GetBlocksToMaturity() > 0 && pcoin->GetDepthInMainChain() > 0)
	  nTotal += __wx__::GetCredit(*pcoin);
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
	  nTotal += __wx__::GetCredit(*pcoin);
  }
  return nTotal;
}

bool __wx__::SelectCoinsMinConf(int64_t nTargetValue, unsigned int nSpendTime, int nConfMine, int nConfTheirs, vector<COutput> vCoins, set<pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet) const
{
  setCoinsRet.clear();
  nValueRet = 0;

  printf("SelectCoins vCoins.size %lu\n", vCoins.size());

  // List of values less than target
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
	  printf("pcoin->nTime %"PRId64", nSpendTime %"PRId64"\n", pcoin->nTime, nSpendTime);
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

  // Solve subset sum by stochastic approximation
  sort(vValue.rbegin(), vValue.rend(), CompareValueOnly());
  vector<char> vfBest;
  int64_t nBest;

  ApproximateBestSubset(vValue, nTotalLower, nTargetValue, vfBest, nBest, 1000);
  if (nBest != nTargetValue && nTotalLower >= nTargetValue + CENT)
      ApproximateBestSubset(vValue, nTotalLower, nTargetValue + CENT, vfBest, nBest, 1000);

  // If we have a bigger coin and (either the stochastic approximation didn't find a good solution,
  //                                   or the next bigger coin is closer), return the bigger coin
  if (coinLowestLarger.second.first &&
      ((nBest != nTargetValue && nBest < nTargetValue + CENT) || coinLowestLarger.first <= nBest))
  {
      setCoinsRet.insert(coinLowestLarger.second);
      nValueRet += coinLowestLarger.first;
  }
  else {
      for (unsigned int i = 0; i < vValue.size(); i++)
	  if (vfBest[i])
	  {
	      setCoinsRet.insert(vValue[i].second);
	      nValueRet += vValue[i].first;
	  }

      if (fDebug && GetBoolArg("-printpriority"))
      {
	  //// debug print
	  printf("SelectCoins() best subset: ");
	  for (unsigned int i = 0; i < vValue.size(); i++)
	      if (vfBest[i])
		  printf("%s ", FormatMoney(vValue[i].first).c_str());
	  printf("total %s\n", FormatMoney(nBest).c_str());
      }
  }

  return true;
}

bool __wx__::SelectCoins(int64_t nTargetValue, unsigned int nSpendTime, set<pair<const __wx__Tx*,unsigned int> >& setCoinsRet, int64_t& nValueRet, const CCoinControl* coinControl) const
{
  vector<COutput> vCoins;
  AvailableCoins(vCoins, true, coinControl);

  // coin control -> return all selected outputs (we want all selected to go into the transaction for sure)
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

// Select some coins without random shuffle or best subset approximation
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

      // Stop if we've chosen enough inputs
      if (nValueRet >= nTargetValue)
	  break;

      int64_t n = pcoin->vout[i].nValue;

      pair<int64_t,pair<const __wx__Tx*,unsigned int> > coin = make_pair(n,make_pair(pcoin, i));

      if (n >= nTargetValue)
      {
	  // If input value is greater or equal to target then simply insert
	  //    it into the current subset and exit
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
	  return false;
      nValue += s.second;
  }
  if (vecSend.empty() || nValue < 0)
      return false;

  wtxNew.BindWallet(this);
  wtxNew.strTxInfo = strTxInfo;
  if (wtxNew.strTxInfo.length() > MAX_TX_INFO_LEN)
    {wtxNew.strTxInfo.resize(MAX_TX_INFO_LEN);}
  if (wtxNew.strTxInfo.length()>0)
     {wtxNew.nVersion=3;}
  {
      LOCK2(cs_main, cs_wallet);
      // txdb must be opened before the mapWallet lock
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
	      // vouts to the payees
	      BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, vecSend)
		  wtxNew.vout.push_back(CTxOut(s.second, s.first));

	      // Choose coins to use
	      set<pair<const __wx__Tx*,unsigned int> > setCoins;
	      int64_t nValueIn = 0;
	      if (!SelectCoins(nTotalValue, wtxNew.nTime, setCoins, nValueIn, coinControl))
		  return false;
	      BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
	      {
		  int64_t nCredit = pcoin.first->vout[pcoin.second].nValue;
		  dPriority += (double)nCredit * pcoin.first->GetDepthInMainChain();
	      }

	      int64_t nChange = nValueIn - nValue - nFeeRet;
	      // if sub-cent change is required, the fee must be raised to at least MIN_TX_FEE
	      // or until nChange becomes zero
	      // NOTE: this depends on the exact behaviour of GetMinFee
	      if (nFeeRet < MIN_TX_FEE && nChange > 0 && nChange < CENT)
	      {
		  int64_t nMoveToFee = min(nChange, MIN_TX_FEE - nFeeRet);
		  nChange -= nMoveToFee;
		  nFeeRet += nMoveToFee;
	      }

	      if (nChange > 0)
	      {
		  // Fill a vout to ourself
		  // TODO: pass in scriptChange instead of reservekey so
		  // change transaction isn't always pay-to-bitcoin-address
		  CScript scriptChange;

		  // coin control: send change to custom address
		  if (coinControl && !boost::get<CNoDestination>(&coinControl->destChange))
		      scriptChange.SetDestination(coinControl->destChange);

		  // no coin control: send change to newly generated address
		  else
		  {
		      // Note: We use a new key here to keep it from being obvious which side is the change.
		      //  The drawback is that by not reusing a previous key, the change may be lost if a
		      //  backup is restored, if the backup doesn't have the new private key for the change.
		      //  If we reused the old key, it would be possible to add code to look for and
		      //  rediscover unknown transactions that were written with keys of ours to recover
		      //  post-backup change.

		      // Reserve a new key pair from key pool
		      CPubKey vchPubKey;
		      assert(reservekey.GetReservedKey(vchPubKey)); // should never fail, as we just unlocked

		      scriptChange.SetDestination(vchPubKey.GetID());
		  }

		  // Insert change txn at random position:
		  vector<CTxOut>::iterator position = wtxNew.vout.begin()+GetRandInt(wtxNew.vout.size());
		  wtxNew.vout.insert(position, CTxOut(nChange, scriptChange));
	      }
	      else
		  reservekey.ReturnKey();

	      // Fill vin
	      BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
		  wtxNew.vin.push_back(CTxIn(coin.first->GetHash(),coin.second));

	      // Sign
	      int nIn = 0;
	      BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
		  if (!SignSignature(*this, *coin.first, wtxNew, nIn++))
		      return false;

	      // Limit size
	      unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK, PROTOCOL_VERSION);
	      if (nBytes >= MAX_STANDARD_TX_SIZE)
		  return false;
	      dPriority /= nBytes;

	      // Check that enough fee is included
	      int64_t nPayFee = nTransactionFee * (1 + (int64_t)nBytes / 1000);
	      int64_t nMinFee = wtxNew.GetMinFee(1, GMF_SEND, nBytes);

	      if (nFeeRet < max(nPayFee, nMinFee))
	      {
		  nFeeRet = max(nPayFee, nMinFee);
		  continue;
	      }

	      // Fill vtxPrev by copying from previous transactions vtxPrev
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
	  return false;
      nValue += s.second;
  }
  if (vecSend.empty() || nValue < 0)
      return false;

  wtxNew.BindWallet(this);
  wtxNew.strTxInfo = strTxInfo;
  if (wtxNew.strTxInfo.length() > MAX_TX_INFO_LEN)
    {wtxNew.strTxInfo.resize(MAX_TX_INFO_LEN);}
  if (wtxNew.strTxInfo.length()>0)
     {wtxNew.nVersion=3;}
  {
      LOCK2(cs_main, cs_wallet);
      // txdb must be opened before the mapWallet lock
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
	      // vouts to the payees
	      BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, vecSend)
		  wtxNew.vout.push_back(CTxOut(s.second, s.first));

	      // Choose coins to use
	      set<pair<const __wx__Tx*,unsigned int> > setCoins;
	      int64_t nValueIn = 0;
	      if (!SelectCoins(nTotalValue, wtxNew.nTime, setCoins, nValueIn, coinControl))
		  return false;
	      BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
	      {
		  int64_t nCredit = pcoin.first->vout[pcoin.second].nValue;
		  dPriority += (double)nCredit * pcoin.first->GetDepthInMainChain();
	      }

	      int64_t nChange = nValueIn - nValue - nFeeRet;
	      // if sub-cent change is required, the fee must be raised to at least MIN_TX_FEE
	      // or until nChange becomes zero
	      // NOTE: this depends on the exact behaviour of GetMinFee
	      if (nFeeRet < MIN_TX_FEE && nChange > 0 && nChange < CENT)
	      {
		  int64_t nMoveToFee = min(nChange, MIN_TX_FEE - nFeeRet);
		  nChange -= nMoveToFee;
		  nFeeRet += nMoveToFee;
	      }

	      if (nChange > 0)
	      {
		  // Fill a vout to ourself
		  // TODO: pass in scriptChange instead of reservekey so
		  // change transaction isn't always pay-to-bitcoin-address
		  CScript scriptChange;

		  // coin control: send change to custom address
		  if (coinControl && !boost::get<CNoDestination>(&coinControl->destChange))
		      scriptChange.SetDestination(coinControl->destChange);

		  // no coin control: send change to newly generated address
		  else
		  {
		      // Note: We use a new key here to keep it from being obvious which side is the change.
		      //  The drawback is that by not reusing a previous key, the change may be lost if a
		      //  backup is restored, if the backup doesn't have the new private key for the change.
		      //  If we reused the old key, it would be possible to add code to look for and
		      //  rediscover unknown transactions that were written with keys of ours to recover
		      //  post-backup change.

		      // Reserve a new key pair from key pool
		      CPubKey vchPubKey;
		      assert(reservekey.GetReservedKey(vchPubKey)); // should never fail, as we just unlocked

		      scriptChange.SetDestination(vchPubKey.GetID());
		  }

		  // Insert change txn at random position:
		  vector<CTxOut>::iterator position = wtxNew.vout.begin()+GetRandInt(wtxNew.vout.size());
		  wtxNew.vout.insert(position, CTxOut(nChange, scriptChange));
	      }
	      else
		  reservekey.ReturnKey();

	      // Fill vin
	      BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
		  wtxNew.vin.push_back(CTxIn(coin.first->GetHash(),coin.second));

	      // Sign
	      int nIn = 0;
	      BOOST_FOREACH(const PAIRTYPE(const __wx__Tx*,unsigned int)& coin, setCoins)
		  if (!SignSignature(*this, *coin.first, wtxNew, nIn++))
		      return false;

	      // Limit size
	      unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK, PROTOCOL_VERSION);
	      if (nBytes >= MAX_STANDARD_TX_SIZE)
		  return false;
	      dPriority /= nBytes;

	      // Check that enough fee is included
	      int64_t nPayFee = nTransactionFee * (1 + (int64_t)nBytes / 1000);
	      int64_t nMinFee = wtxNew.GetMinFee(1, GMF_SEND, nBytes);

	      if (nFeeRet < max(nPayFee, nMinFee))
	      {
		  nFeeRet = max(nPayFee, nMinFee);
		  continue;
	      }

	      // Fill vtxPrev by copying from previous transactions vtxPrev
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
  // Choose coins to use
  int64_t nBalance = GetBalance();

  if (nBalance <= nReserveBalance)
      return false;

  vector<const __wx__Tx*> vwtxPrev;

  set<pair<const __wx__Tx*,unsigned int> > setCoins;
  int64_t nValueIn = 0;

  if (!SelectCoinsForStaking(nBalance - nReserveBalance, GetTime(), setCoins, nValueIn))
      return false;

  if (setCoins.empty())
      return false;

  nWeight = 0;

  int64_t nCurrentTime = GetTime();
  CTxDB txdb("r");

  LOCK2(cs_main, cs_wallet);
  BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
  {
      CTxIndex txindex;
      if (!txdb.ReadTxIndex(pcoin.first->GetHash(), txindex))
	  continue;

    if(pcoin.first->GetDepthInMainChain() >= nStakeMinConfirmations)
      nWeight += pcoin.first->vout[pcoin.second].nValue;
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

  // Mark coin stake transaction
  CScript scriptEmpty;
  scriptEmpty.clear();
  txNew.vout.push_back(CTxOut(0, scriptEmpty));

  // Choose coins to use
  int64_t nBalance = GetBalance();

  if (nBalance <= nReserveBalance)
      return false;

  vector<const __wx__Tx*> vwtxPrev;

  set<pair<const __wx__Tx*,unsigned int> > setCoins;
  int64_t nValueIn = 0;

  // Select coins with suitable depth
  if (!SelectCoinsForStaking(nBalance - nReserveBalance, txNew.nTime, setCoins, nValueIn))
      return false;

  if (setCoins.empty())
      return false;

  int64_t nCredit = 0;
  CScript scriptPubKeyKernel;
  CTxDB txdb("r");
  BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
  {
      CTxIndex txindex;
      {
	  LOCK2(cs_main, cs_wallet);
	  if (!txdb.ReadTxIndex(pcoin.first->GetHash(), txindex))
	      continue;
      }

      // Read block header
      CBlock block;
      {
	  LOCK2(cs_main, cs_wallet);
	  if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
	      continue;
      }

      static int nMaxStakeSearchInterval = 60;
      bool fKernelFound = false;
      for (unsigned int n=0; n<min(nSearchInterval,(int64_t)nMaxStakeSearchInterval) && !fKernelFound && !fShutdown && pindexPrev == pindexBest; n++)
      {
	  // Search backward in time from the given txNew timestamp
	  // Search nSearchInterval seconds back up to nMaxStakeSearchInterval
	  COutPoint prevoutStake = COutPoint(pcoin.first->GetHash(), pcoin.second);
          int64_t nBlockTime;
	  if(CheckKernel(pindexPrev,  nBits, txNew.nTime -n, prevoutStake, &nBlockTime))
	  {
	      // Found a kernel
	      if (fDebug && GetBoolArg("-printcoinstake"))
		  printf("CreateCoinStake : kernel found\n");
	      vector<valtype> vSolutions;
	      txnouttype whichType;
	      CScript scriptPubKeyOut;
	      scriptPubKeyKernel = pcoin.first->vout[pcoin.second].scriptPubKey;
	      if (!Solver(scriptPubKeyKernel, whichType, vSolutions))
	      {
		  if (fDebug && GetBoolArg("-printcoinstake"))
		      printf("CreateCoinStake : failed to parse kernel\n");
		  break;
	      }
	      if (fDebug && GetBoolArg("-printcoinstake"))
		  printf("CreateCoinStake : parsed kernel type=%d\n", whichType);
	      if (whichType != TX_PUBKEY && whichType != TX_PUBKEYHASH)
	      {
		  if (fDebug && GetBoolArg("-printcoinstake"))
		      printf("CreateCoinStake : no support for kernel type=%d\n", whichType);
		  break;  // only support pay to public key and pay to address
	      }
	      if (whichType == TX_PUBKEYHASH) // pay to address type
	      {
		  // convert to pay to public key type
		  if (!keystore.GetKey(uint160(vSolutions[0]), key))
		  {
		      if (fDebug && GetBoolArg("-printcoinstake"))
			  printf("CreateCoinStake : failed to get key for kernel type=%d\n", whichType);
		      break;  // unable to find corresponding public key
		  }
		  scriptPubKeyOut << key.GetPubKey() << OP_CHECKSIG;
	      }
	      if (whichType == TX_PUBKEY)
	      {
		  valtype& vchPubKey = vSolutions[0];
		  if (!keystore.GetKey(Hash160(vchPubKey), key))
		  {
		      if (fDebug && GetBoolArg("-printcoinstake"))
			  printf("CreateCoinStake : failed to get key for kernel type=%d\n", whichType);
		      break;  // unable to find corresponding public key
		  }

	      if (key.GetPubKey() != vchPubKey)
	      {
		  if (fDebug && GetBoolArg("-printcoinstake"))
		      printf("CreateCoinStake : invalid key for kernel type=%d\n", whichType);
		      break; // keys mismatch
		  }

		  scriptPubKeyOut = scriptPubKeyKernel;
	      }

	      txNew.nTime -= n;
	      txNew.vin.push_back(CTxIn(pcoin.first->GetHash(), pcoin.second));
	      nCredit += pcoin.first->vout[pcoin.second].nValue;
	      vwtxPrev.push_back(pcoin.first);
	      txNew.vout.push_back(CTxOut(0, scriptPubKeyOut));

	      if (GetWeight(block.GetBlockTime(), (int64_t)txNew.nTime) < GetStakeSplitAge())
		  txNew.vout.push_back(CTxOut(0, scriptPubKeyOut)); //split stake
	      if (fDebug && GetBoolArg("-printcoinstake"))
		  printf("CreateCoinStake : added kernel type=%d\n", whichType);
	      fKernelFound = true;
	      break;
	  }
      }

      if (fKernelFound || fShutdown)
	  break; // if kernel is found stop searching
  }

  if (nCredit == 0 || nCredit > nBalance - nReserveBalance)
    return false;

  BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int) pcoin, setCoins)
  {
      // Attempt to add more inputs
      // Only add coins of the same key/address as kernel
      if (txNew.vout.size() == 2 && ((pcoin.first->vout[pcoin.second].scriptPubKey == scriptPubKeyKernel || pcoin.first->vout[pcoin.second].scriptPubKey == txNew.vout[1].scriptPubKey))
	  && pcoin.first->GetHash() != txNew.vin[0].prevout.hash)
      {
	  int64_t nTimeWeight = GetWeight((int64_t)pcoin.first->nTime, (int64_t)txNew.nTime);

	  // Stop adding more inputs if already too many inputs
	  if (txNew.vin.size() >= 100)
	      break;
	  // Stop adding more inputs if value is already pretty significant
	  if (nCredit >= GetStakeCombineThreshold())
	      break;
	  // Stop adding inputs if reached reserve limit
	  if (nCredit + pcoin.first->vout[pcoin.second].nValue > nBalance - nReserveBalance)
	      break;
	  // Do not add additional significant input
	  if (pcoin.first->vout[pcoin.second].nValue >= GetStakeCombineThreshold())
	      continue;
	  // Do not add input that is still too young
	  if (nTimeWeight < nStakeMinAge)
	      continue;

	  txNew.vin.push_back(CTxIn(pcoin.first->GetHash(), pcoin.second));
	  nCredit += pcoin.first->vout[pcoin.second].nValue;
	  vwtxPrev.push_back(pcoin.first);
      }
  }

  //Calculate reward
  int64_t nReward;
  {
      if(V3(nBestHeight))
        nReward = GetProofOfStakeReward(0, nFees, nHeight+1);
      else
      {
        uint64_t nCoinAge;
        CTxDB txdb("r");
        if (!txNew.GetCoinAge(txdb, nCoinAge))
            return error("CreateCoinStake : failed to calculate coin age");

        nReward = GetProofOfStakeReward(nCoinAge, nFees, nHeight+1);
      }

      if (nReward <= 0)
	return false;

      nCredit += nReward;
  }

  // Set output amount
  if (txNew.vout.size() == 3)
  {
      txNew.vout[1].nValue = (nCredit / 2 / CENT) * CENT;
      txNew.vout[2].nValue = nCredit - txNew.vout[1].nValue;
  }
  else

      txNew.vout[1].nValue = nCredit;

  // Sign
  int nIn = 0;
  BOOST_FOREACH(const __wx__Tx* pcoin, vwtxPrev)
  {
      if (!SignSignature(*this, *pcoin, txNew, nIn++))
	  return error("CreateCoinStake : failed to sign coinstake");
  }

  // Limit size
  unsigned int nBytes = ::GetSerializeSize(txNew, SER_NETWORK, PROTOCOL_VERSION);
  if (nBytes >= MAX_BLOCK_SIZE_GEN/5)
      return error("CreateCoinStake : exceeded coinstake size limit");

  // Successfully generated coinstake
  return true;
}

bool __wx__::CommitTransaction__(__wx__Tx& wtxNew, CReserveKey& reservekey)
{
  {
      LOCK2(cs_main, cs_wallet);
      printf("CommitTransaction:\n%s", wtxNew.ToString().c_str());
      {
	  // This is only to keep the database open to defeat the auto-flush for the
	  // duration of this scope.  This is the only place where this optimization
	  // maybe makes sense; please don't do it anywhere else.
	  __wx__DB* pwalletdb = fFileBacked ? new __wx__DB(strWalletFile,"r") : NULL;

	  // Take key pair from key pool so it won't be used again
	  reservekey.KeepKey();


	  // Add tx to wallet, because if it has change it's also ours,
	  // otherwise just for transaction history.
	  AddToWallet(wtxNew);
          
  
	  // Mark old coins as spent
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
	      delete pwalletdb;
      }

      // Track how many getdata requests our transaction gets
      mapRequestCount[wtxNew.GetHash()] = 0;

      // Broadcast
      if (!wtxNew.AcceptToMemoryPool())
      {
	  // This must not fail. The transaction has already been signed and recorded.
	  printf("CommitTransaction() : Error: Transaction not valid\n");
	  return false;
      }
      wtxNew.RelayWalletTransaction();
  }
  return true;
}

// Call after CreateTransaction unless you want to abort
bool __wx__::CommitTransaction(__wx__Tx& wtxNew, CReserveKey& reservekey)
{
  {
      LOCK2(cs_main, cs_wallet);
      printf("CommitTransaction:\n%s", wtxNew.ToString().c_str());
      {
	  // This is only to keep the database open to defeat the auto-flush for the
	  // duration of this scope.  This is the only place where this optimization
	  // maybe makes sense; please don't do it anywhere else.
	  __wx__DB* pwalletdb = fFileBacked ? new __wx__DB(strWalletFile,"r") : NULL;

	  // Take key pair from key pool so it won't be used again
	  reservekey.KeepKey();

          __xfa(wtxNew.vout);

	  // Add tx to wallet, because if it has change it's also ours,
	  // otherwise just for transaction history.
	  AddToWallet(wtxNew);

	  // Mark old coins as spent
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
	      delete pwalletdb;
      }

      // Track how many getdata requests our transaction gets
      mapRequestCount[wtxNew.GetHash()] = 0;

      // Broadcast
      if (!wtxNew.AcceptToMemoryPool())
      {
	  // This must not fail. The transaction has already been signed and recorded.
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
	  strError = strprintf(_("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds  "), FormatMoney(nFeeRequired).c_str());
      else
	  strError = _("Error: Transaction creation failed  ");
      printf("SendMoney() : %s", strError.c_str());
      return strError;
  }

  if (fAskFee && !uiInterface.ThreadSafeAskFee(nFeeRequired, _("Sending...")))
      return "ABORTED";

  if (!CommitTransaction__(wtxNew, reservekey))
      return _("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");

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
	  strError = strprintf(_("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds  "), FormatMoney(nFeeRequired).c_str());
      else
	  strError = _("Error: Transaction creation failed  ");
      printf("SendMoney() : %s", strError.c_str());
      return strError;
  }

  if (fAskFee && !uiInterface.ThreadSafeAskFee(nFeeRequired, _("Sending...")))
      return "ABORTED";

  if (!CommitTransaction(wtxNew, reservekey))
      return _("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");

  return "";
}



string __wx__::SendMoneyToDestination(const CTxDestination& address, int64_t nValue, __wx__Tx& wtxNew, bool fAskFee, std::string strTxInfo)
{
  // Check amount
  if (nValue <= 0)
      return _("Invalid amount");
  if (nValue + S_MIN_TX_FEE > GetBalance())
      return _("Insufficient funds");

  // Parse Bitcoin address
  CScript scriptPubKey;
  scriptPubKey.SetDestination(address);


  return SendMoney(scriptPubKey, nValue, wtxNew, fAskFee, strTxInfo);
}




DBErrors __wx__::LoadWallet(bool& fFirstRunRet)
{
  if (!fFileBacked)
      return DB_LOAD_OK;
  fFirstRunRet = false;
  DBErrors nLoadWalletRet = __wx__DB(strWalletFile,"cr+").LoadWallet(this);
  if (nLoadWalletRet == DB_NEED_REWRITE)
  {
      if (CDB::Rewrite(strWalletFile, "\x04pool"))
      {
	  LOCK(cs_wallet);
	  setKeyPool.clear();
	  // Note: can't top-up keypool here, because wallet is locked.
	  // User will be prompted to unlock wallet the next operation
	  // the requires a new key.
      }
  }

  if (nLoadWalletRet != DB_LOAD_OK)
      return nLoadWalletRet;
  fFirstRunRet = !vchDefaultKey.IsValid();

  NewThread(ThreadFlushWalletDB, &strWalletFile);
  return DB_LOAD_OK;
}


bool __wx__::SetAddressBookName(const CTxDestination& address, const string& aliasStr)
{
  bool fUpdated = false;
  {
      LOCK(cs_wallet); // mapAddressBook
      std::map<CTxDestination, std::string>::iterator mi = mapAddressBook.find(address);
      fUpdated = mi != mapAddressBook.end();
      mapAddressBook[address] = aliasStr;
  }
  NotifyAddressBookChanged(this, address, aliasStr, ::IsMine(*this, address),
			   (fUpdated ? CT_UPDATED : CT_NEW) );
  if (!fFileBacked)
      return false;
  return __wx__DB(strWalletFile).WriteName(cba(address).ToString(), aliasStr);
}

bool __wx__::DelAddressBookName(const CTxDestination& address)
{
  {
      LOCK(cs_wallet); // mapAddressBook

      mapAddressBook.erase(address);
  }

  NotifyAddressBookChanged(this, address, "", ::IsMine(*this, address), CT_DELETED);

  if (!fFileBacked)
      return false;
  return __wx__DB(strWalletFile).EraseName(cba(address).ToString());
}


void __wx__::PrintWallet(const CBlock& block)
{
  {
      LOCK(cs_wallet);
      if (block.IsProofOfWork() && mapWallet.count(block.vtx[0].GetHash()))
      {
	  __wx__Tx& wtx = mapWallet[block.vtx[0].GetHash()];
	  printf("    mine:  %d  %d  %"PRId64"", wtx.GetDepthInMainChain(), wtx.GetBlocksToMaturity(), wtx.GetCredit());
      }
      if (block.IsProofOfStake() && mapWallet.count(block.vtx[1].GetHash()))
      {
	  __wx__Tx& wtx = mapWallet[block.vtx[1].GetHash()];
	  printf("    stake: %d  %d  %"PRId64"", wtx.GetDepthInMainChain(), wtx.GetBlocksToMaturity(), wtx.GetCredit());
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
	  return false;
  }
  vchDefaultKey = vchPubKey;
  return true;
}

bool GetWalletFile(__wx__* pwallet, string &strWalletFileOut)
{
  if (!pwallet->fFileBacked)
      return false;
  strWalletFileOut = pwallet->strWalletFile;
  return true;
}

//
// Mark old keypool keys as used,
// and generate all new keys
//
bool __wx__::NewKeyPool()
{
  {
      LOCK(cs_wallet);
      __wx__DB walletdb(strWalletFile);
      BOOST_FOREACH(int64_t nIndex, setKeyPool)
	  walletdb.ErasePool(nIndex);
      setKeyPool.clear();

      if (as())
	  return false;

      int64_t nKeys = max(GetArg("-keypool", 100), (int64_t)0);
      for (int i = 0; i < nKeys; i++)
      {
	  int64_t nIndex = i+1;
	  walletdb.WritePool(nIndex, CKeyPool(GenerateNewKey()));
	  setKeyPool.insert(nIndex);
      }
      printf("__wx__::NewKeyPool wrote %"PRId64" new keys\n", nKeys);
  }
  return true;
}

bool __wx__::TopUpKeyPool(unsigned int nSize)
{
  {
      LOCK(cs_wallet);

      if (as())
	  return false;

      __wx__DB walletdb(strWalletFile);

      // Top up key pool
      unsigned int nTargetSize;
      if (nSize > 0)
	  nTargetSize = nSize;
      else
	  nTargetSize = max(GetArg("-keypool", 100), (int64_t)0);

      while (setKeyPool.size() < (nTargetSize + 1))
      {
	  int64_t nEnd = 1;
	  if (!setKeyPool.empty())
	      nEnd = *(--setKeyPool.end()) + 1;
	  if (!walletdb.WritePool(nEnd, CKeyPool(GenerateNewKey())))
	      throw runtime_error("TopUpKeyPool() : writing generated key failed");
	  setKeyPool.insert(nEnd);
	  printf("keypool added key %"PRId64", size=%"PRIszu"\n", nEnd, setKeyPool.size());
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
	  TopUpKeyPool();

      // Get the oldest key
      if(setKeyPool.empty())
	  return;

      __wx__DB walletdb(strWalletFile);

      nIndex = *(setKeyPool.begin());
      setKeyPool.erase(setKeyPool.begin());
      if (!walletdb.ReadPool(nIndex, keypool))
	  throw runtime_error("ReserveKeyFromKeyPool() : read failed");
      if (!HaveKey(keypool.vchPubKey.GetID()))
	  throw runtime_error("ReserveKeyFromKeyPool() : unknown key in key pool");
      assert(keypool.vchPubKey.IsValid());
      if (fDebug && GetBoolArg("-printkeypool"))
	  printf("keypool reserve %"PRId64"\n", nIndex);
  }
}

int64_t __wx__::AddReserveKey(const CKeyPool& keypool)
{
  {
      LOCK2(cs_main, cs_wallet);
      __wx__DB walletdb(strWalletFile);

      int64_t nIndex = 1 + *(--setKeyPool.end());
      if (!walletdb.WritePool(nIndex, keypool))
	  throw runtime_error("AddReserveKey() : writing added key failed");
      setKeyPool.insert(nIndex);
      return nIndex;
  }
  return -1;
}

void __wx__::KeepKey(int64_t nIndex)
{
  // Remove from key pool
  if (fFileBacked)
  {
      __wx__DB walletdb(strWalletFile);
      walletdb.ErasePool(nIndex);
  }
  if(fDebug)
      printf("keypool keep %"PRId64"\n", nIndex);
}

void __wx__::ReturnKey(int64_t nIndex)
{
  // Return to key pool
  {
      LOCK(cs_wallet);
      setKeyPool.insert(nIndex);
  }
  if(fDebug)
      printf("keypool return %"PRId64"\n", nIndex);
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
	  if (as()) return false;
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
	  if (as()) return false;
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
      throw runtime_error("update vtx");
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
	  if (as()) return false;
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
      return GetTime();
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
	      continue;

	  if ((pcoin->IsCoinBase() || pcoin->IsCoinStake()) && pcoin->GetBlocksToMaturity() > 0)
	      continue;

	  int nDepth = pcoin->GetDepthInMainChain();
	  if (nDepth < (pcoin->IsFromMe() ? 0 : 1))
	      continue;

	  for (unsigned int i = 0; i < pcoin->vout.size(); i++)
	  {
	      CTxDestination addr;
	      if (!IsMine(pcoin->vout[i]))
		  continue;
	      if(!ExtractDestination(pcoin->vout[i].scriptPubKey, addr))
		  continue;

	      int64_t n = pcoin->IsSpent(i) ? 0 : pcoin->vout[i].nValue;

	      if (!balances.count(addr))
		  balances[addr] = 0;
	      balances[addr] += n;
	  }
      }
  }

  return balances;
}

set< set<CTxDestination> > __wx__::GetAddressGroupings()
{
  AssertLockHeld(cs_wallet); // mapWallet
  set< set<CTxDestination> > groupings;
  set<CTxDestination> grouping;

  BOOST_FOREACH(PAIRTYPE(uint256, __wx__Tx) walletEntry, mapWallet)
  {
      __wx__Tx *pcoin = &walletEntry.second;

      if (pcoin->vin.size() > 0 && IsMine(pcoin->vin[0]))
      {
	  // group all input addresses with each other
	  BOOST_FOREACH(CTxIn txin, pcoin->vin)
	  {
	      CTxDestination address;
	      if(!ExtractDestination(mapWallet[txin.prevout.hash].vout[txin.prevout.n].scriptPubKey, address))
		  continue;
	      grouping.insert(address);
	  }

	  // group change with input addresses
	  BOOST_FOREACH(CTxOut txout, pcoin->vout)
	      if (IsChange(txout))
	      {
		  __wx__Tx tx = mapWallet[pcoin->vin[0].prevout.hash];
		  CTxDestination txoutAddr;
		  if(!ExtractDestination(txout.scriptPubKey, txoutAddr))
		      continue;
		  grouping.insert(txoutAddr);
	      }
	  groupings.insert(grouping);
	  grouping.clear();
      }

      // group lone addrs by themselves
      for (unsigned int i = 0; i < pcoin->vout.size(); i++)
	  if (IsMine(pcoin->vout[i]))
	  {
	      CTxDestination address;
	      if(!ExtractDestination(pcoin->vout[i].scriptPubKey, address))
		  continue;
	      grouping.insert(address);
	      groupings.insert(grouping);
	      grouping.clear();
	  }
  }

  set< set<CTxDestination>* > uniqueGroupings; // a set of pointers to groups of addresses
  map< CTxDestination, set<CTxDestination>* > setmap;  // map addresses to the unique group containing it
  BOOST_FOREACH(set<CTxDestination> grouping, groupings)
  {
      // make a set of all the groups hit by this new group
      set< set<CTxDestination>* > hits;
      map< CTxDestination, set<CTxDestination>* >::iterator it;
      BOOST_FOREACH(CTxDestination address, grouping)
	  if ((it = setmap.find(address)) != setmap.end())
	      hits.insert((*it).second);

      // merge all hit groups into a new single group and delete old groups
      set<CTxDestination>* merged = new set<CTxDestination>(grouping);
      BOOST_FOREACH(set<CTxDestination>* hit, hits)
      {
	  merged->insert(hit->begin(), hit->end());
	  uniqueGroupings.erase(hit);
	  delete hit;
      }
      uniqueGroupings.insert(merged);

      // update setmap
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

// ppcoin: check 'spent' consistency between wallet and txindex
// ppcoin: fix wallet spent state according to txindex
void __wx__::FixSpentCoins(int& nMismatchFound, int64_t& nBalanceInQuestion, bool fCheckOnly)
{
  nMismatchFound = 0;
  nBalanceInQuestion = 0;

  LOCK(cs_wallet);
  vector<__wx__Tx*> vCoins;
  vCoins.reserve(mapWallet.size());
  for (map<uint256, __wx__Tx>::iterator it = mapWallet.begin(); it != mapWallet.end(); ++it)
      vCoins.push_back(&(*it).second);

  CTxDB txdb("r");
  BOOST_FOREACH(__wx__Tx* pcoin, vCoins)
  {
      // Find the corresponding transaction index
      CTxIndex txindex;
      if (!txdb.ReadTxIndex(pcoin->GetHash(), txindex))
	  continue;
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

// ppcoin: disable transaction (only for coinstake)
void __wx__::DisableTransaction(const CTransaction &tx)
{
  if (!tx.IsCoinStake() || !IsFromMe(tx))
      return; // only disconnecting coinstake requires marking input unspent

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
	  vchPubKey = keypool.vchPubKey;
      else {
	  if (pwallet->vchDefaultKey.IsValid()) {
	      printf("CReserveKey::GetReservedKey(): Warning: Using default key instead of a new key, top up your keypool!");
	      vchPubKey = pwallet->vchDefaultKey;
	  } else
	      return false;
      }
  }
  assert(vchPubKey.IsValid());
  pubkey = vchPubKey;
  return true;
}

void CReserveKey::KeepKey()
{
  if (nIndex != -1)
      pwallet->KeepKey(nIndex);
  nIndex = -1;
  vchPubKey = CPubKey();
}

void CReserveKey::ReturnKey()
{
  if (nIndex != -1)
      pwallet->ReturnKey(nIndex);
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
	  throw runtime_error("GetAllReserveKeyHashes() : read failed");
      assert(keypool.vchPubKey.IsValid());
      CKeyID keyID = keypool.vchPubKey.GetID();
      if (!HaveKey(keyID))
	  throw runtime_error("GetAllReserveKeyHashes() : unknown key in key pool");
      setAddress.insert(keyID);
  }
}

void __wx__::UpdatedTransaction(const uint256 &hashTx)
{
  {
      LOCK(cs_wallet);
      // Only notify UI if this transaction is in this wallet
      map<uint256, __wx__Tx>::const_iterator mi = mapWallet.find(hashTx);
      if (mi != mapWallet.end())
	  NotifyTransactionChanged(this, hashTx, CT_UPDATED);
  }
}

void __wx__::kt(std::map<CKeyID, int64_t> &mapKeyBirth) const {
  AssertLockHeld(cs_wallet); // kd
  mapKeyBirth.clear();

  // get birth times for keys with metadata
  for (std::map<CKeyID, CKeyMetadata>::const_iterator it = kd.begin(); it != kd.end(); it++)
      if (it->second.nCreateTime)
	  mapKeyBirth[it->first] = it->second.nCreateTime;

  // map in which we'll infer heights of other keys
  CBlockIndex *pindexMax = FindBlockByHeight(std::max(0, nBestHeight - 144)); // the tip can be reorganised; use a 144-block safety margin
  std::map<CKeyID, CBlockIndex*> mapKeyFirstBlock;
  std::set<CKeyID> setKeys;
  GetKeys(setKeys);
  BOOST_FOREACH(const CKeyID &keyid, setKeys) {
      if (mapKeyBirth.count(keyid) == 0)
	  mapKeyFirstBlock[keyid] = pindexMax;
  }
  setKeys.clear();

  // if there are no such keys, we're done
  if (mapKeyFirstBlock.empty())
      return;

  // find first block that affects those keys, if there are any left
  std::vector<CKeyID> vAffected;
  for (std::map<uint256, __wx__Tx>::const_iterator it = mapWallet.begin(); it != mapWallet.end(); it++) {
      // iterate over all wallet transactions...
      const __wx__Tx &wtx = (*it).second;
      std::map<uint256, CBlockIndex*>::const_iterator blit = mapBlockIndex.find(wtx.hashBlock);
      if (blit != mapBlockIndex.end() && blit->second->IsInMainChain()) {
	  // ... which are already in a block
	  int nHeight = blit->second->nHeight;
	  BOOST_FOREACH(const CTxOut &txout, wtx.vout) {
	      // iterate over all their outputs
	      ::ExtractAffectedKeys(*this, txout.scriptPubKey, vAffected);
	      BOOST_FOREACH(const CKeyID &keyid, vAffected) {
		  // ... and all their affected keys
		  std::map<CKeyID, CBlockIndex*>::iterator rit = mapKeyFirstBlock.find(keyid);
		  if (rit != mapKeyFirstBlock.end() && nHeight < rit->second->nHeight)
		      rit->second = blit->second;
	      }
	      vAffected.clear();
	  }
      }
  }

  // Extract block timestamps for those keys
  for (std::map<CKeyID, CBlockIndex*>::const_iterator it = mapKeyFirstBlock.begin(); it != mapKeyFirstBlock.end(); it++)
      mapKeyBirth[it->first] = it->second->nTime - 7200; // block times can be 2h off
}
bool
__wx__Tx::GetEncryptedMessageUpdate (int& nOut, vchType& nm, vchType& r, vchType& val, vchType& iv, vchType& s) const
{
if (nVersion != CTransaction::DION_TX_VERSION)
  return false;

//if (!pkTxDecoded)
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
      pkTxDecodeSuccess = false;
  }

if (!pkTxDecodeSuccess)
  return false;

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
  return false;

//if (!pkTxDecoded)
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
      pkTxDecodeSuccess = false;
  }

if (!pkTxDecodeSuccess)
  return false;

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
  return false;

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
      pkTxDecodeSuccess = false;
  }

if (!pkTxDecodeSuccess)
  return false;

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
    return false;

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
    return false;

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
    return false;

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
         s__ = true;
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
      return 0;

  // Find the block it claims to be in
  map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
  if (mi == mapBlockIndex.end())
      return 0;
  CBlockIndex* pindex = (*mi).second;
  if (!pindex || !pindex->IsInMainChain())
      return 0;

  // Make sure the merkle branch connects to this block
  if (!fMerkleVerified)
  {
      if (CBlock::CheckMerkleBranch(GetHash(), vMerkleBranch, nIndex) != pindex->hashMerkleRoot)
	  return 0;
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
            if(intersect) return true; 
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
            if(t.size() == 0) continue;
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
              if(tmp1.size() == 0) continue;
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
                  throw std::runtime_error("Key");

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
    uint160 i  = pwalletMain->kd[ck].z;
    if(i == 0)
      continue;

    bool found=false;
    CPubKey k  = pwalletMain->kd[ck].k;
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
        if(found) break;
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
            throw std::runtime_error("Key");
        }
      }
    }
  }
  return true;
}

DBErrors __wx__::ZapWalletTx()
{
    if (!fFileBacked)
        return DB_LOAD_OK;
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
      return nZapWalletTxRet;

    return DB_LOAD_OK;
}
