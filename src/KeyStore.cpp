
#include "KeyStore.h"
#include "Script.h"
bool CKeyStore::GetPubKey(const CKeyID &address, CPubKey &vchPubKeyOut) const
{
  CKey key;

  if (!GetKey(address, key))
  {
    return false;
  }

  vchPubKeyOut = key.GetPubKey();
  return true;
}
bool CBasicKeyStore::ak(const CKey& key)
{
  bool fCompressed = false;
  CSecret secret = key.GetSecret(fCompressed);
  {
    LOCK(cs_KeyStore);
    mapKeys[key.GetPubKey().GetID()] = make_pair(secret, fCompressed);
    mapPubKeys[Hash160(key.GetPubKey().Raw())] = key.GetPubKey().Raw();
  }
  return true;
}
bool CBasicKeyStore::AddCScript(const CScript& redeemScript)
{
  if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
  {
    return error("CBasicKeyStore::AddCScript() : redeemScripts > %i bytes are invalid", MAX_SCRIPT_ELEMENT_SIZE);
  }

  {
    LOCK(cs_KeyStore);
    mapScripts[redeemScript.GetID()] = redeemScript;
  }

  return true;
}
bool CBasicKeyStore::HaveCScript(const CScriptID& hash) const
{
  bool result;
  {
    LOCK(cs_KeyStore);
    result = (mapScripts.count(hash) > 0);
  }
  return result;
}
bool CBasicKeyStore::GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const
{
  {
    LOCK(cs_KeyStore);
    ScriptMap::const_iterator mi = mapScripts.find(hash);

    if (mi != mapScripts.end())
    {
      redeemScriptOut = (*mi).second;
      return true;
    }
  }
  return false;
}
bool CCryptoKeyStore::SetCrypted()
{
  {
    LOCK(cs_KeyStore);

    if (fUseCrypto)
    {
      return true;
    }

    if (!mapKeys.empty())
    {
      return false;
    }

    fUseCrypto = true;
  }
  return true;
}
bool CCryptoKeyStore::Lock()
{
  if (!SetCrypted())
  {
    return false;
  }

  {
    LOCK(cs_KeyStore);
    vMasterKey.clear();
  }

  NotifyStatusChanged(this);
  return true;
}
bool CCryptoKeyStore::Unlock(const CKeyingMaterial& vMasterKeyIn)
{
  {
    LOCK(cs_KeyStore);

    if (!SetCrypted())
    {
      return false;
    }

    CryptedKeyMap::const_iterator mi = mapCryptedKeys.begin();

    for (; mi != mapCryptedKeys.end(); ++mi)
    {
      const CPubKey &vchPubKey = (*mi).second.first;
      const std::vector<unsigned char> &vchCryptedSecret = (*mi).second.second;
      CSecret vchSecret;

      if(!DecryptSecret(vMasterKeyIn, vchCryptedSecret, vchPubKey.GetHash(), vchSecret))
      {
        return false;
      }

      if (vchSecret.size() != 32)
      {
        return false;
      }

      CKey key;
      key.SetPubKey(vchPubKey);
      key.SetSecret(vchSecret);

      if (key.GetPubKey() == vchPubKey)
      {
        break;
      }

      return false;
    }

    vMasterKey = vMasterKeyIn;
  }
  NotifyStatusChanged(this);
  return true;
}
bool CCryptoKeyStore::ak(const CKey& key)
{
  {
    LOCK(cs_KeyStore);

    if (!IsCrypted())
    {
      return CBasicKeyStore::ak(key);
    }

    if (as())
    {
      return false;
    }

    std::vector<unsigned char> vchCryptedSecret;
    CPubKey vchPubKey = key.GetPubKey();
    bool fCompressed;

    if (!EncryptSecret(vMasterKey, key.GetSecret(fCompressed), vchPubKey.GetHash(), vchCryptedSecret))
    {
      return false;
    }

    if (!sync(key.GetPubKey(), vchCryptedSecret))
    {
      return false;
    }
  }
  return true;
}
bool CCryptoKeyStore::sync(const CPubKey &vchPubKey, const std::vector<unsigned char> &vchCryptedSecret)
{
  {
    LOCK(cs_KeyStore);

    if (!SetCrypted())
    {
      return false;
    }

    mapCryptedKeys[vchPubKey.GetID()] = make_pair(vchPubKey, vchCryptedSecret);
  }
  return true;
}
bool CCryptoKeyStore::GetKey(const CKeyID &address, CKey& keyOut) const
{
  {
    LOCK(cs_KeyStore);

    if (!IsCrypted())
    {
      return CBasicKeyStore::GetKey(address, keyOut);
    }

    CryptedKeyMap::const_iterator mi = mapCryptedKeys.find(address);

    if (mi != mapCryptedKeys.end())
    {
      const CPubKey &vchPubKey = (*mi).second.first;
      const std::vector<unsigned char> &vchCryptedSecret = (*mi).second.second;
      CSecret vchSecret;

      if (!DecryptSecret(vMasterKey, vchCryptedSecret, vchPubKey.GetHash(), vchSecret))
      {
        return false;
      }

      if (vchSecret.size() != 32)
      {
        return false;
      }

      keyOut.SetPubKey(vchPubKey);
      keyOut.SetSecret(vchSecret);
      return true;
    }
  }
  return false;
}
bool CCryptoKeyStore::GetPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const
{
  {
    LOCK(cs_KeyStore);

    if (!IsCrypted())
    {
      return CKeyStore::GetPubKey(address, vchPubKeyOut);
    }

    CryptedKeyMap::const_iterator mi = mapCryptedKeys.find(address);

    if (mi != mapCryptedKeys.end())
    {
      vchPubKeyOut = (*mi).second.first;
      return true;
    }
  }
  return false;
}
bool CCryptoKeyStore::EncryptKeys(CKeyingMaterial& vMasterKeyIn)
{
  {
    LOCK(cs_KeyStore);

    if (!mapCryptedKeys.empty() || IsCrypted())
    {
      return false;
    }

    fUseCrypto = true;
    BOOST_FOREACH(KeyMap::value_type& mKey, mapKeys)
    {
      CKey key;

      if (!key.SetSecret(mKey.second.first, mKey.second.second))
      {
        return false;
      }

      const CPubKey vchPubKey = key.GetPubKey();
      std::vector<unsigned char> vchCryptedSecret;
      bool fCompressed;

      if (!EncryptSecret(vMasterKeyIn, key.GetSecret(fCompressed), vchPubKey.GetHash(), vchCryptedSecret))
      {
        return false;
      }

      if (!sync(vchPubKey, vchCryptedSecret))
      {
        return false;
      }
    }
    mapKeys.clear();
  }
  return true;
}
