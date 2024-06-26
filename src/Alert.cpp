
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <map>
#include "Alert.h"
#include "crypto/Key.h"
#include "Net.h"
#include "Sync.h"
#include "UIInterface.h"
using namespace std;
map<uint256, CAlert> mapAlerts;
CCriticalSection cs_mapAlerts;
static const char* pszMainKey = "04d2045ae17f45675e7c0eaa19d47fca3462defec5a713a7eda2bd04b86fafc8a2eece56562025fc131cf03bd96f3501dfb0ac0f2faa5557d69f7a7778711994f1";
static const char* pszTestKey = "0434fba4959f7c6980f91a0ce0ed0dce5d78634da9590abe7182425538986462b7a64b503b48673b828ab6979081da78c7de0a52f565cc296de67a7d02b6e4085b";
void CUnsignedAlert::SetNull()
{
  nVersion = 1;
  nRelayUntil = 0;
  nExpiration = 0;
  nID = 0;
  nCancel = 0;
  setCancel.clear();
  nMinVer = 0;
  nMaxVer = 0;
  setSubVer.clear();
  nPriority = 0;
  strComment.clear();
  strStatusBar.clear();
  strReserved.clear();
}
std::string CUnsignedAlert::ToString() const
{
  std::string strSetCancel;
  BOOST_FOREACH(int n, setCancel)
  strSetCancel += strprintf("%d ", n);
  std::string strSetSubVer;
  BOOST_FOREACH(std::string str, setSubVer)
  strSetSubVer += "\"" + str + "\" ";
  return strprintf(
           "CAlert(\n"
           "    nVersion     = %d\n"
           "    nRelayUntil  = %" PRId64 "\n"
           "    nExpiration  = %" PRId64 "\n"
           "    nID          = %d\n"
           "    nCancel      = %d\n"
           "    setCancel    = %s\n"
           "    nMinVer      = %d\n"
           "    nMaxVer      = %d\n"
           "    setSubVer    = %s\n"
           "    nPriority    = %d\n"
           "    strComment   = \"%s\"\n"
           "    strStatusBar = \"%s\"\n"
           ")\n",
           nVersion,
           nRelayUntil,
           nExpiration,
           nID,
           nCancel,
           strSetCancel.c_str(),
           nMinVer,
           nMaxVer,
           strSetSubVer.c_str(),
           nPriority,
           strComment.c_str(),
           strStatusBar.c_str());
}
void CUnsignedAlert::print() const
{
  printf("%s", ToString().c_str());
}
void CAlert::SetNull()
{
  CUnsignedAlert::SetNull();
  vchMsg.clear();
  vchSig.clear();
}
bool CAlert::IsNull() const
{
  return (nExpiration == 0);
}
uint256 CAlert::GetHash() const
{
  return Hash(this->vchMsg.begin(), this->vchMsg.end());
}
bool CAlert::IsInEffect() const
{
  return (GetAdjustedTime() < nExpiration);
}
bool CAlert::Cancels(const CAlert& alert) const
{
  if (!IsInEffect())
  {
    return false;
  }

  return (alert.nID <= nCancel || setCancel.count(alert.nID));
}
bool CAlert::AppliesTo(int nVersion, std::string strSubVerIn) const
{
  return (IsInEffect() &&
          nMinVer <= nVersion && nVersion <= nMaxVer &&
          (setSubVer.empty() || setSubVer.count(strSubVerIn)));
}
bool CAlert::AppliesToMe() const
{
  return (strStatusBar.size() > 0) && AppliesTo(PROTOCOL_VERSION, FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, std::vector<std::string>()));
}
bool CAlert::RelayTo(CNode* pnode) const
{
  if (!IsInEffect())
  {
    return false;
  }

  if (pnode->setKnown.insert(GetHash()).second)
  {
    if (AppliesTo(pnode->nVersion, pnode->strSubVer) ||
        AppliesToMe() ||
        GetAdjustedTime() < nRelayUntil)
    {
      pnode->PushMessage("alert", *this);
      return true;
    }
  }

  return false;
}
bool CAlert::CheckSignature() const
{
  CKey key;

  if (!key.SetPubKey(ParseHex(fTestNet ? pszTestKey : pszMainKey)))
  {
    return error("CAlert::CheckSignature() : SetPubKey failed");
  }

  if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
  {
    return error("CAlert::CheckSignature() : verify signature failed");
  }

  CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
  sMsg >> *(CUnsignedAlert*)this;
  return true;
}
CAlert CAlert::getAlertByHash(const uint256 &hash)
{
  CAlert retval;
  {
    LOCK(cs_mapAlerts);
    map<uint256, CAlert>::iterator mi = mapAlerts.find(hash);

    if(mi != mapAlerts.end())
    {
      retval = mi->second;
    }
  }
  return retval;
}
bool CAlert::ProcessAlert(bool fThread)
{
  if (!CheckSignature())
  {
    return false;
  }

  if (!IsInEffect())
  {
    return false;
  }

# 201 "alert.cpp"
  int maxInt = std::numeric_limits<int>::max();

  if (nID == maxInt)
  {
    if (!(
          nExpiration == maxInt &&
          nCancel == (maxInt-1) &&
          nMinVer == 0 &&
          nMaxVer == maxInt &&
          setSubVer.empty() &&
          nPriority == maxInt &&
          strStatusBar == "URGENT: Alert key compromised, upgrade required"
        ))
    {
      return false;
    }
  }

  {
    LOCK(cs_mapAlerts);

    for (map<uint256, CAlert>::iterator mi = mapAlerts.begin(); mi != mapAlerts.end();)
    {
      const CAlert& alert = (*mi).second;

      if (Cancels(alert))
      {
        printf("cancelling alert %d\n", alert.nID);
        uiInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
        mapAlerts.erase(mi++);
      }
      else if (!alert.IsInEffect())
      {
        printf("expiring alert %d\n", alert.nID);
        uiInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
        mapAlerts.erase(mi++);
      }
      else
      {
        mi++;
      }
    }

    BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
    {
      const CAlert& alert = item.second;

      if (alert.Cancels(*this))
      {
        printf("alert already cancelled by %d\n", alert.nID);
        return false;
      }
    }
    mapAlerts.insert(make_pair(GetHash(), *this));

    if(AppliesToMe())
    {
      uiInterface.NotifyAlertChanged(GetHash(), CT_NEW);
      std::string strCmd = GetArg("-alertnotify", "");

      if (!strCmd.empty())
      {
        std::string singleQuote("'");
        std::string safeChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 .,;_/:?@");
        std::string safeStatus;

        for (std::string::size_type i = 0; i < strStatusBar.size(); i++)
        {
          if (safeChars.find(strStatusBar[i]) != std::string::npos)
          {
            safeStatus.push_back(strStatusBar[i]);
          }
        }

        safeStatus = singleQuote+safeStatus+singleQuote;
        boost::replace_all(strCmd, "%s", safeStatus);

        if (fThread)
        {
          boost::thread t(runCommand, strCmd);
        }
        else
        {
          runCommand(strCmd);
        }
      }
    }
  }

  printf("accepted alert %d, AppliesToMe()=%d\n", nID, AppliesToMe());
  return true;
}
