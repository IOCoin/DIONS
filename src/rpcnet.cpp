

#include "net.h"
#include "bitcoinrpc.h"
#include "alert.h"
#include "wallet.h"
#include "db.h"
#include "walletdb.h"
using namespace json_spirit;
using namespace std;
Value getconnectioncount(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 0)
    throw runtime_error(
      "getconnectioncount\n"
      "Returns the number of connections to other nodes.");

  LOCK(cs_vNodes);
  return (int)vNodes.size();
}
static void CopyNodeStats(std::vector<CNodeStats>& vstats)
{
  vstats.clear();
  LOCK(cs_vNodes);
  vstats.reserve(vNodes.size());
  BOOST_FOREACH(CNode* pnode, vNodes)
  {
    CNodeStats stats;
    pnode->copyStats(stats);
    vstats.push_back(stats);
  }
}
Value getpeerinfo(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 0)
    throw runtime_error(
      "getpeerinfo\n"
      "Returns data about each connected network node.");

  vector<CNodeStats> vstats;
  CopyNodeStats(vstats);
  Array ret;
  BOOST_FOREACH(const CNodeStats& stats, vstats)
  {
    Object obj;
    obj.push_back(Pair("addr", stats.addrName));
    obj.push_back(Pair("services", strprintf("%08" PRIx64, stats.nServices)));
    obj.push_back(Pair("lastsend", (int64_t)stats.nLastSend));
    obj.push_back(Pair("lastrecv", (int64_t)stats.nLastRecv));
    obj.push_back(Pair("conntime", (int64_t)stats.nTimeConnected));
    obj.push_back(Pair("version", stats.nVersion));
    obj.push_back(Pair("subver", stats.strSubVer));
    obj.push_back(Pair("inbound", stats.fInbound));
    obj.push_back(Pair("startingheight", stats.nStartingHeight));
    obj.push_back(Pair("banscore", stats.nMisbehavior));
    ret.push_back(obj);
  }
  return ret;
}
Value sendalert(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 6)
    throw runtime_error(
      "sendalert <message> <privatekey> <minver> <maxver> <priority> <id> [cancelupto] [expiredelta]\n"
      "<message> is the alert text message\n"
      "<privatekey> is hex string of alert master private key\n"
      "<minver> is the minimum applicable internal client version\n"
      "<maxver> is the maximum applicable internal client version\n"
      "<priority> is integer priority number\n"
      "<id> is the alert id\n"
      "[cancelupto] cancels all alert id's up to this number\n"
      "[expiredelta] after this many seconds, the alert will expire\n"
      "Returns true or false.");

  CAlert alert;
  CKey key;
  alert.strStatusBar = params[0].get_str();
  alert.nMinVer = params[2].get_int();
  alert.nMaxVer = params[3].get_int();
  alert.nPriority = params[4].get_int();
  alert.nID = params[5].get_int();

  if (params.size() > 6)
  {
    alert.nCancel = params[6].get_int();
  }

  alert.nVersion = PROTOCOL_VERSION;
  alert.nRelayUntil = GetAdjustedTime() + 365*24*60*60;
  int64_t expireDelta = 365*24*60*60;

  if (params.size() > 7)
  {
    expireDelta = params[7].get_int();
  }

  alert.nExpiration = GetAdjustedTime() + expireDelta;
  CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
  sMsg << (CUnsignedAlert)alert;
  alert.vchMsg = vector<unsigned char>(sMsg.begin(), sMsg.end());
  vector<unsigned char> vchPrivKey = ParseHex(params[1].get_str());
  key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()));

  if (!key.Sign(Hash(alert.vchMsg.begin(), alert.vchMsg.end()), alert.vchSig))
    throw runtime_error(
      "Unable to sign alert, check private key?\n");

  if(!alert.ProcessAlert())
    throw runtime_error(
      "Failed to process alert.\n");

  {
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    alert.RelayTo(pnode);
  }
  Object result;
  result.push_back(Pair("strStatusBar", alert.strStatusBar));
  result.push_back(Pair("nVersion", alert.nVersion));
  result.push_back(Pair("nMinVer", alert.nMinVer));
  result.push_back(Pair("nMaxVer", alert.nMaxVer));
  result.push_back(Pair("nPriority", alert.nPriority));
  result.push_back(Pair("nID", alert.nID));

  if (alert.nCancel > 0)
  {
    result.push_back(Pair("nCancel", alert.nCancel));
  }

  return result;
}
