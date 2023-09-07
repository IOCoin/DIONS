
#include "wallet.h"
#include "walletdb.h"
#include "bitcoin.h"
#include "init.h"
#include "base58.h"
#include "dions.h"
using namespace json_spirit;
using namespace std;
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
extern int scaleMonitor();
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
Value getinfo(const Array& params, bool fHelp)
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
Value getnewpubkey(const Array& params, bool fHelp)
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
Value getnewaddress(const Array& params, bool fHelp)
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
Value sectionlog(const Array& params, bool fHelp)
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
Value shade(const Array& params, bool fHelp)
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
Value sr71(const Array& params, bool fHelp)
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
Value center__base__0(const Array& params, bool fHelp)
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

  if(!IsMine(*pwalletMainId, u.Get()))
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid");
  }

  cba v(r);

  if(!v.IsValid())
  {
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address");
  }

  if(!IsMine(*pwalletMainId, v.Get()))
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
Value getaccountaddress(const Array& params, bool fHelp)
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
Value setaccount(const Array& params, bool fHelp)
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
Value sa(const Array& params, bool fHelp)
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
Value getaccount(const Array& params, bool fHelp)
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
Value getaddressesbyaccount(const Array& params, bool fHelp)
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
Value addresstodion(const Array& params, bool fHelp)
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
Value sendtodion(const Array& params, bool fHelp)
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
Value sendtoaddress(const Array& params, bool fHelp)
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
Value listaddressgroupings(const Array& params, bool fHelp)
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
Value signmessage(const Array& params, bool fHelp)
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
Value verifymessage(const Array& params, bool fHelp)
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

  if (!IsMine(*pwalletMainId,scriptPubKey))
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
Value xtu_url(const Array& params, bool fHelp)
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

  if (!IsMine(*pwalletMainId,scriptPubKey))
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
Value getreceivedbyaddress(const Array& params, bool fHelp)
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

  if (!IsMine(*pwalletMainId,scriptPubKey))
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
Value gra(const Array& params, bool fHelp)
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

  if (!IsMine(*pwalletMainId,scriptPubKey))
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
Value getreceivedbyaccount(const Array& params, bool fHelp)
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

      if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*pwalletMainId, address) && setAddress.count(address))
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
Value pending(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 2)
    throw runtime_error(
      "pending [account] [minconf=1]\n"
      "If [account] is not specified, returns the server's total available balance.\n"
      "If [account] is specified, returns the balance in the account.");

  return ValueFromAmount(pwalletMainId->GetUnconfirmedBalance());
}
Value getbalance(const Array& params, bool fHelp)
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
Value movecmd(const Array& params, bool fHelp)
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
Value sendfrom(const Array& params, bool fHelp)
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
Value sendmany(const Array& params, bool fHelp)
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
Value addmultisigaddress(const Array& params, bool fHelp)
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
Value addredeemscript(const Array& params, bool fHelp)
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

      if (!ExtractDestination(txout.scriptPubKey, address) || !IsMine(*pwalletMainId, address))
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
Value listreceivedbyaddress(const Array& params, bool fHelp)
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
Value listreceivedbyaccount(const Array& params, bool fHelp)
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
Value listtransactions__(const Array& params, bool fHelp)
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
Value listtransactions(const Array& params, bool fHelp)
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
Value listaccounts(const Array& params, bool fHelp)
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
    if (IsMine(*pwalletMainId, entry.first))
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
Value listsinceblock(const Array& params, bool fHelp)
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
Value gettransaction(const Array& params, bool fHelp)
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

    if (GetTransaction(hash, tx, hashBlock))
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
Value backupwallet(const Array& params, bool fHelp)
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
Value keypoolrefill(const Array& params, bool fHelp)
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
Value walletpassphrase(const Array& params, bool fHelp)
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
Value walletpassphrasechange(const Array& params, bool fHelp)
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
Value walletlockstatus(const Array& params, bool fHelp)
{
  Object result;
  result.push_back(Pair("isEncrypted", pwalletMainId->IsCrypted()));
  result.push_back(Pair("isLocked", pwalletMainId->as()));
  return result;
}
Value walletlock(const Array& params, bool fHelp)
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
Value getencryptionstatus(const Array& params, bool fHelp)
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
Value encryptwallet(const Array& params, bool fHelp)
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
Value validateaddress(const Array& params, bool fHelp)
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
    bool fMine = IsMine(*pwalletMainId, dest);
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
Value validatepubkey(const Array& params, bool fHelp)
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
    bool fMine = IsMine(*pwalletMainId, dest);
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
Value reservebalance(const Array& params, bool fHelp)
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
Value checkwallet(const Array& params, bool fHelp)
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
Value repairwallet(const Array& params, bool fHelp)
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
Value resendtx(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "resendtx\n"
      "Re-send unconfirmed transactions.\n"
    );

  ResendWalletTransactions(true);
  return Value::null;
}
Value makekeypair(const Array& params, bool fHelp)
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
Value rmtx(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 1)
    throw runtime_error(
      "rmtx txid\n"
      "Removes transaction id from memory pool.");

  uint256 hash;
  hash.SetHex(params[0].get_str());
  CTransaction tx;
  uint256 hashBlock = 0;

  if (!GetTransaction(hash, tx, hashBlock))
  {
    throw JSONRPCError(-5, "No information available about transaction in mempool");
  }

  mempool.remove(tx);
  return tx.GetHash().GetHex();
}
Value shadesend(const Array& params, bool fHelp)
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
Value __vtx_s(const Array& params, bool fHelp)
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
Value sublimateYdwi(const Array& params, bool fHelp)
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
Value shadeK(const Array& params, bool fHelp)
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
