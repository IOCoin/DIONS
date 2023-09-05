
#include <iostream>
#include <fstream>
#include "init.h"
#include "bitcoinrpc.h"
#include "ui_interface.h"
#include "base58.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/variant/get.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#define printf OutputDebugStringF
using namespace json_spirit;
using namespace std;
void EnsureWalletIsUnlocked();
namespace bt = boost::posix_time;
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
Value importprivkey(const Array& params, bool fHelp)
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
Value importwalletRT(const Array& params, bool fHelp)
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
Value dumpprivkey(const Array& params, bool fHelp)
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
Value dumpwalletRT(const Array& params, bool fHelp)
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
Value ydwiWhldw_base_diff(const Array& params, bool fHelp)
{
  Array a;
  std::map<CKeyID, int64_t> mk;
  pwalletMainId->kt(mk);
  std::set<CKeyID> setKeyPool;
  pwalletMainId->GetAllReserveKeys(setKeyPool);
  bool set=false;

  for(map< CKeyID, int64_t >::const_iterator it = mk.begin(); it != mk.end(); it++)
  {
    cba a_(it->first);
    CKeyID k;
    a_.GetKeyID(k);
    string delta = a_.ToString();
    Object o;

    if(pwalletMainId->mapAddressBook.count(it->first))
    {
      string t0 = EncodeDumpString(pwalletMainId->mapAddressBook[it->first]);
      o.push_back(Pair(t0, delta));
      a.push_back(o);

      if(t0 == "sparechange")
      {
        set = true;
      }
    }
    else
    {
      CScript s;
      s.SetDestination(a_.Get());
      bool f=false;

      for (map<uint256, __wx__Tx>::iterator it = pwalletMainId->mapWallet.begin(); it != pwalletMainId->mapWallet.end(); ++it)
      {
        if(!setKeyPool.count(k))
        {
          continue;
        }

        const __wx__Tx& wtx = (*it).second;

        if (wtx.IsCoinBase() || wtx.IsCoinStake() || !IsFinalTx(wtx))
        {
          continue;
        }

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
          if (txout.scriptPubKey == s)
          {
            if (wtx.GetDepthInMainChain() >= 10)
            {
              o.push_back(Pair("", delta));
              a.push_back(o);
              f=true;
              break;
            }
          }
        }

        if(f)
        {
          break;
        }
      }
    }
  }

  if(!set)
  {
    Object o;

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
      if(item.second == "sparechange")
      {
        pwalletMainId->SetAddressBookName(item.first.Get(), "");
      }
    }
    pwalletMainId->SetAddressBookName(keyID, "sparechange");
    o.push_back(Pair("sparechange", cba(keyID).ToString()));
    a.push_back(o);
  }

  return a;
}
Value importwallet(const Array& params, bool fHelp)
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
Value dumpwallet(const Array& params, bool fHelp)
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
Value trc(const Array& params, bool fHelp)
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
Value trcbase(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 1 || params.size() > 2)
    throw runtime_error(
      "trcbase <base>");

  return Value::null;
}
