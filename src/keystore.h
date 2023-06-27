#ifndef BITCOIN_KEYSTORE_H
#define BITCOIN_KEYSTORE_H 

#include "crypter.h"
#include "sync.h"
#include <boost/signals2/signal.hpp>

class CScript;
class cba;


class CKeyStore
{
protected:
  mutable CCriticalSection cs_KeyStore;

public:
  std::map<uint160, std::vector<unsigned char> > mapPubKeys;
  virtual ~CKeyStore() {}


  virtual bool ak(const CKey& key) =0;
  mutable CCriticalSection cs_mapKeys;


  virtual bool HaveKey(const CKeyID &address) const =0;
  virtual bool GetKey(const CKeyID &address, CKey& keyOut) const =0;
  virtual void GetKeys(std::set<CKeyID> &setAddress) const =0;
  virtual bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;


  virtual bool AddCScript(const CScript& redeemScript) =0;
  virtual bool HaveCScript(const CScriptID &hash) const =0;
  virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const =0;

  virtual bool GetSecret(const CKeyID &address, CSecret& vchSecret, bool &fCompressed) const
  {
    CKey key;
    if (!GetKey(address, key))
    {
      return false;
    }
    vchSecret = key.GetSecret(fCompressed);
    return true;
  }
};

typedef std::map<CKeyID, std::pair<CSecret, bool> > KeyMap;
typedef std::map<CScriptID, CScript > ScriptMap;


class CBasicKeyStore : public CKeyStore
{
protected:
  KeyMap mapKeys;
  ScriptMap mapScripts;

public:
  bool ak(const CKey& key);
  bool HaveKey(const CKeyID &address) const
  {
    bool result;
    {
      LOCK(cs_KeyStore);
      result = (mapKeys.count(address) > 0);
    }
    return result;
  }
  void GetKeys(std::set<CKeyID> &setAddress) const
  {
    setAddress.clear();
    {
      LOCK(cs_KeyStore);
      KeyMap::const_iterator mi = mapKeys.begin();
      while (mi != mapKeys.end())
      {
        setAddress.insert((*mi).first);
        mi++;
      }
    }
  }
  bool GetKey(const CKeyID &address, CKey &keyOut) const
  {
    {
      LOCK(cs_KeyStore);
      KeyMap::const_iterator mi = mapKeys.find(address);
      if (mi != mapKeys.end())
      {
        keyOut.Reset();
        keyOut.SetSecret((*mi).second.first, (*mi).second.second);
        return true;
      }
    }
    return false;
  }
  virtual bool AddCScript(const CScript& redeemScript);
  virtual bool HaveCScript(const CScriptID &hash) const;
  virtual bool GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const;
};

typedef std::map<CKeyID, std::pair<CPubKey, std::vector<unsigned char> > > CryptedKeyMap;




class CCryptoKeyStore : public CBasicKeyStore
{
private:
  CryptedKeyMap mapCryptedKeys;

  CKeyingMaterial vMasterKey;



  bool fUseCrypto;

protected:
  bool SetCrypted();


  bool EncryptKeys(CKeyingMaterial& vMasterKeyIn);

  bool Unlock(const CKeyingMaterial& vMasterKeyIn);

public:
  CCryptoKeyStore() : fUseCrypto(false)
  {
  }

  bool IsCrypted() const
  {
    return fUseCrypto;
  }


  bool as() const
  {
    if (!IsCrypted())
    {
      return false;
    }
    bool result;
    {
      LOCK(cs_KeyStore);
      result = vMasterKey.empty();
    }
    return result;
  }

  bool Lock();

  virtual bool sync(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret);
  bool ak(const CKey& key);
  bool HaveKey(const CKeyID &address) const
  {
    {
      LOCK(cs_KeyStore);
      if (!IsCrypted())
      {
        return CBasicKeyStore::HaveKey(address);
      }

      return mapCryptedKeys.count(address) > 0;
    }
    return false;
  }
  bool GetKey(const CKeyID &address, CKey& keyOut) const;
  bool GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;
  void GetKeys(std::set<CKeyID> &setAddress) const
  {
    if (!IsCrypted())
    {
      CBasicKeyStore::GetKeys(setAddress);
      return;
    }
    setAddress.clear();
    CryptedKeyMap::const_iterator mi = mapCryptedKeys.begin();
    while (mi != mapCryptedKeys.end())
    {
      setAddress.insert((*mi).first);
      mi++;
    }
  }




  boost::signals2::signal<void (CCryptoKeyStore* wallet)> NotifyStatusChanged;
};

#endif
