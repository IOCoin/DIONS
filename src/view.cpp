#include "view.h"
#include "script.h"

bool CViewKeyStore::ak(const CKeyID& ckid)
{
  {
    LOCK(cs_KeyStore);
    mapKeys.insert(ckid);
  }
  return true;
}

bool CViewKeyStore::AddCScript(const CScript& redeemScript)
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

bool CViewKeyStore::HaveCScript(const CScriptID& hash) const
{
  bool result;
  {
    LOCK(cs_KeyStore);
    result = (mapScripts.count(hash) > 0);
  }
  return result;
}


bool CViewKeyStore::GetCScript(const CScriptID &hash, CScript& redeemScriptOut) const
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
