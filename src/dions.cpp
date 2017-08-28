// Copyright (c) 2017 IODigital foundation developers 
//
//
#include "db.h"
#include "txdb-leveldb.h"
#include "keystore.h"
#include "wallet.h"
#include "init.h"
#include "dions.h"

#include "bitcoinrpc.h"
#include "main.h"
#include "state.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#include <boost/xpressive/xpressive_dynamic.hpp>
#include <boost/filesystem.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
using namespace std;
using namespace json_spirit;
using namespace boost::iostreams;

namespace fs = boost::filesystem;

extern Object JSONRPCError(int code, const string& message);
extern Value xtu_url__(const string& url);
template<typename T> void ConvertTo(Value& value, bool fAllowNull=false);

std::map<vchType, uint256> mapMyMessages;
std::map<vchType, uint256> mapLocator;
std::map<vchType, set<uint256> > mapState;
std::map<vchType, set<uint256> > k1Export;

#ifdef GUI
extern std::map<uint160, vchType> mapLocatorHashes;
#endif

extern uint256 SignatureHash(CScript scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

extern bool Solver(const CKeyStore& keystore, const CScript& scriptPubKey, uint256 hash, int nHashType, CScript& scriptSigRet, txnouttype& type);
extern bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn, int nHashType);
extern Value sendtoaddress(const Array& params, bool fHelp);

bool getImportedPubKey(string senderAddress, string recipientAddress, vchType& recipientPubKeyVch, vchType& aesKeyBase64EncryptedVch);
bool getImportedPubKey(string recipientAddress, vchType& recipientPubKeyVch);
bool internalReference__(string recipientAddress, vchType& recipientPubKeyVch);

vchType vchFromValue(const Value& value)
{
  const std::string str = value.get_str();
  return vchFromString(str);
}

vchType vchFromString(const std::string& str)
{
  const unsigned char* strbeg;
  strbeg = reinterpret_cast<const unsigned char*>(str.c_str());
  return vchType(strbeg, strbeg + str.size());
}

string stringFromVch(const vector<unsigned char> &vch)
{
  string res;
  vector<unsigned char>::const_iterator vi = vch.begin();
  while(vi != vch.end()) {
    res +=(char)(*vi);
    vi++;
  }
  return res;
}
bool channel(string l, string f, string k)
{
  vchType rVch;
  vchType alphaVch;
  if(!getImportedPubKey(l, f, rVch, alphaVch))
  {
    CKeyID keyID;
    CBitcoinAddress keyAddress(l);
    keyAddress.GetKeyID(keyID);
    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);
    if(pwalletMain->aes_(vchPubKey, f, k))
    {
      return true;
    }

    return false;
  }

  k = stringFromVch(alphaVch);

  return true;
}
int scaleMonitor()
{
  return 210000;
}
int GetTxPosHeight(AliasIndex& txPos)
{
    return txPos.nHeight;
}
int GetTxPosHeight(CDiskTxPos& txPos)
{
    CBlock block;
    if(!block.ReadFromDisk(txPos.nFile, txPos.nBlockPos, false))
        return 0;
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(block.GetHash());
    if(mi == mapBlockIndex.end())
        return 0;
    CBlockIndex* pindex =(*mi).second;
    if(!pindex || !pindex->IsInMainChain())
        return 0;
    return pindex->nHeight;
}
int
aliasHeight(vector<unsigned char> vchAlias)
{
  vector<AliasIndex> vtxPos;
  LocatorNodeDB ln1Db("r");
  if(ln1Db.lKey(vchAlias))
    {
      if(!ln1Db.lGet(vchAlias, vtxPos))
        return error("aliasHeight() : failed to read from alias DB");
      if(vtxPos.empty())
        return -1;

      AliasIndex& txPos = vtxPos.back();
      return GetTxPosHeight(txPos);
    }

  return -1;
}
CScript aliasStrip(const CScript& scriptIn)
{
    int op;
    vector<vector<unsigned char> > vvch;
    CScript::const_iterator pc = scriptIn.begin();

    if(!aliasScript(scriptIn, op, vvch, pc))
        throw runtime_error("aliasStrip() : could not decode alias script");
    return CScript(pc, scriptIn.end());
}
bool IsLocator(const CTransaction& tx, const CTxOut& txout)
{
    const CScript& scriptPubKey = aliasStrip(txout.scriptPubKey);
    CScript scriptSig;
    txnouttype whichType;
    if(!Solver(*pwalletMain, scriptPubKey, 0, 0, scriptSig, whichType))
        return false;
    return true;
}
bool txPost(const vector<pair<CScript, int64_t> >& vecSend, const CWalletTx& wtxIn, int nTxOut, CWalletTx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet)
{
    int64_t nValue = 0;
    BOOST_FOREACH(const PAIRTYPE(CScript, int64_t)& s, vecSend)
    {
        if(nValue < 0)
            return false;
        nValue += s.second;
    }
    if(vecSend.empty() || nValue < 0)
        return false;

    wtxNew.pwallet = pwalletMain;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        CTxDB txdb("r");
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
          nFeeRet = CENT;
          for(;;)
          {
            wtxNew.vin.clear();
            wtxNew.vout.clear();
            wtxNew.fFromMe = true;

            int64_t nTotalValue = nValue + nFeeRet;
            BOOST_FOREACH(const PAIRTYPE(CScript, int64_t)& s, vecSend)
              wtxNew.vout.push_back(CTxOut(s.second, s.first));

            int64_t nWtxinCredit = 0;

            set<pair<const CWalletTx*, unsigned int> > setCoins;
            int64_t nValueIn = 0;
            if(nTotalValue - nWtxinCredit > 0)
            {
              if(!pwalletMain->SelectCoins(nTotalValue - nWtxinCredit, wtxNew.nTime, setCoins, nValueIn))
              {
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                return false;
              }
            }


            vector<pair<const CWalletTx*, unsigned int> >
              vecCoins(setCoins.begin(), setCoins.end());

              vecCoins.insert(vecCoins.begin(), make_pair(&wtxIn, nTxOut));

            nValueIn += nWtxinCredit;

            int64_t nChange = nValueIn - nTotalValue;
            if(nChange >= CENT)
            {
              CPubKey pubkey;
              if(!reservekey.GetReservedKey(pubkey))
              {
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                return false;
              }

              CScript scriptChange;
              scriptChange.SetDestination(pubkey.GetID());

              vector<CTxOut>::iterator position = wtxNew.vout.begin()+GetRandInt(wtxNew.vout.size());
              wtxNew.vout.insert(position, CTxOut(nChange, scriptChange));
            }
            else
              reservekey.ReturnKey();
  
            BOOST_FOREACH(PAIRTYPE(const CWalletTx*, unsigned int)& coin, vecCoins)
            {
              wtxNew.vin.push_back(CTxIn(coin.first->GetHash(), coin.second));
            }

            int nIn = 0;
            BOOST_FOREACH(PAIRTYPE(const CWalletTx*, unsigned int)& coin, vecCoins)
            {
              if(!SignSignature(*pwalletMain, *coin.first, wtxNew, nIn++))
              {
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                return false;
              }
            }

            unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK);

            if(nBytes >= MAX_BLOCK_SIZE)
            {
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
              return false;
            }

            int64_t nPayFee = CENT * (1 +(int64_t)nBytes / 1024);
            int64_t nMinFee = CENT; 

            if(nFeeRet < max(nPayFee, nMinFee))
            {
              nFeeRet = max(nPayFee, nMinFee);
              continue;
            }

            wtxNew.AddSupportingTransactions(txdb);
            wtxNew.fTimeReceivedIsTxTime = true;
            break;
          }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    return true;
}
bool txRelayPre__(const CScript& scriptPubKey, const CWalletTx& wtxIn, CWalletTx& wtxNew, int64_t& t, string& e)
{
    int nTxOut = aliasOutIndex(wtxIn);
    CReserveKey reservekey(pwalletMain);
    vector< pair<CScript, int64_t> > vecSend;
    vecSend.push_back(make_pair(scriptPubKey, CTRL__));

    if(!txPost(vecSend, wtxIn, nTxOut, wtxNew, reservekey, t))
    {
        if(CTRL__ + t > pwalletMain->GetBalance())
            e = strprintf(_("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds "), FormatMoney(t).c_str());
        else
            e = _("Error: Transaction creation failed  ");
        return false;
    }

    return true;
}
string txRelay(const CScript& scriptPubKey, int64_t nValue, const CWalletTx& wtxIn, CWalletTx& wtxNew, bool fAskFee)
{
    int nTxOut = aliasOutIndex(wtxIn);
    CReserveKey reservekey(pwalletMain);
    int64_t nFeeRequired;
    vector< pair<CScript, int64_t> > vecSend;
    vecSend.push_back(make_pair(scriptPubKey, nValue));

    if(!txPost(vecSend, wtxIn, nTxOut, wtxNew, reservekey, nFeeRequired))
    {
        string strError;
        if(nValue + nFeeRequired > pwalletMain->GetBalance())
            strError = strprintf(_("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds "), FormatMoney(nFeeRequired).c_str());
        else
            strError = _("Error: Transaction creation failed  ");
        return strError;
    }

    if(!pwalletMain->CommitTransaction(wtxNew, reservekey))
        return _("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");

    return "";
}
bool txTrace(AliasIndex& txPos, vector<unsigned char>& vchValue, uint256& hash, int& nHeight)
{
    nHeight = GetTxPosHeight(txPos);
    vchValue = txPos.vValue;
    CTransaction tx;
    if(!tx.ReadFromDisk(txPos.txPos))
        return error("txTrace() : could not read tx from disk");
    hash = tx.GetHash();
    return true;
}
bool txTrace(CDiskTxPos& txPos, vector<unsigned char>& vchValue, uint256& hash, int& nHeight)
{
    nHeight = GetTxPosHeight(txPos);
    CTransaction tx;
    if(!tx.ReadFromDisk(txPos))
        return error("txTrace() : could not read tx from disk");
    if(!aliasTxValue(tx, vchValue))
        return error("txTrace() : could not decode value from tx");
    hash = tx.GetHash();
    return true;
}
bool aliasTx(LocatorNodeDB& aliasCacheDB, const vector<unsigned char> &vchAlias, CTransaction& tx)
{

    vector<AliasIndex> vtxPos;
    if(!aliasCacheDB.lGet(vchAlias, vtxPos) || vtxPos.empty())
        return false;

    AliasIndex& txPos = vtxPos.back();

    int nHeight = txPos.nHeight;
    if(nHeight + scaleMonitor() < pindexBest->nHeight)
    {
        string alias = stringFromVch(vchAlias);
        return false;
    }

    if(!tx.ReadFromDisk(txPos.txPos))
        return error("aliasTx() : could not read tx from disk");
    return true;
}
bool aliasAddress(const CTransaction& tx, std::string& strAddress)
{
    int op;
    int nOut;
    vector<vector<unsigned char> > vvch;
    if(!aliasTx(tx, op, nOut, vvch))
        return false;
    const CTxOut& txout = tx.vout[nOut];
    const CScript& scriptPubKey = aliasStrip(txout.scriptPubKey);
    strAddress = scriptPubKey.GetBitcoinAddress();
    return true;
}
bool aliasAddress(const CDiskTxPos& txPos, std::string& strAddress)
{
    CTransaction tx;
    if(!tx.ReadFromDisk(txPos))
        return error("aliasAddress() : could not read tx from disk");

    return aliasAddress(tx, strAddress);
}

Value myRSAKeys(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "myRSAKeys\n"
                "list my rsa keys "
                );

  Array jsonAddressRSAList;
  BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, string)& item, pwalletMain->mapAddressBook)
  {

    const CBitcoinAddress& a = item.first;
    const string& aliasStr = item.second;
    Object oAddressInfo;
    oAddressInfo.push_back(Pair("address", a.ToString()));

    string d = "";
    int r = atod(a.ToString(), d);


    if(r == 0)
      oAddressInfo.push_back(Pair("alias", d));
    else
      oAddressInfo.push_back(Pair("alias", "NONE"));

    CKeyID keyID;
    if(!a.GetKeyID(keyID))
      throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
    {
      return jsonAddressRSAList;
    }

    CPubKey pubKey = key.GetPubKey();

    string rsaPrivKey;
    bool found = pwalletMain->envCP0(pubKey, rsaPrivKey);
    if(found == false)
      continue;

    jsonAddressRSAList.push_back(oAddressInfo);

  }

  return jsonAddressRSAList;
}

Value myRSAKeys__(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "myRSAKeys\n"
                "list my rsa keys "
                );

  Array jsonAddressRSAList;
  BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, string)& item, pwalletMain->mapAddressBook)
  {

    const CBitcoinAddress& a = item.first;
    const string& aliasStr = item.second;
    Object oAddressInfo;
    oAddressInfo.push_back(Pair("address", a.ToString()));

    string d = "";
    int r = atod(a.ToString(), d);

    if(r == 0)
      oAddressInfo.push_back(Pair("alias", d));
    else
      oAddressInfo.push_back(Pair("alias", "NONE"));

    CKeyID keyID;
    if(!a.GetKeyID(keyID))
      throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
    {
      return jsonAddressRSAList;
    }

    CPubKey pubKey = key.GetPubKey();

    string rsaPrivKey;
    bool found = pwalletMain->envCP0(pubKey, rsaPrivKey);
    if(found == false)
      continue;

    jsonAddressRSAList.push_back(oAddressInfo);

  }

  return jsonAddressRSAList;
}
Value publicKeyExports(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "publicKeyExports [<alias>]\n"
                "list exported public keys "
                );

    vchType vchNodeLocator;
    if(params.size() == 1)
      vchNodeLocator = vchFromValue(params[0]);

    std::map<vchType, int> mapAliasVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
          {
            const CWalletTx& tx = item.second;

            vchType vchSender, vchRecipient, vchKey, vchAes, vchSig;
            int nOut;
            if(!tx.GetPublicKeyUpdate(nOut, vchSender, vchRecipient, vchKey, vchAes, vchSig))
              continue;

            if(!IsMinePost(tx))
              continue;

            const int nHeight = tx.GetHeightInMainChain();
            if(nHeight == -1)
              continue;
            assert(nHeight >= 0);

            Object aliasObj;
            aliasObj.push_back(Pair("exported_to", stringFromVch(vchSender)));
            aliasObj.push_back(Pair("key", stringFromVch(vchKey)));
            aliasObj.push_back(Pair("aes256_encrypted", stringFromVch(vchAes)));
            aliasObj.push_back(Pair("signature", stringFromVch(vchSig)));

            oRes.push_back(aliasObj);


        }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, aliasMapVchObj)
        oRes.push_back(item.second);

    return oRes;
}

Value publicKeys(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "publicKeys [<alias>]\n"
                );
    vchType vchNodeLocator;
    if(params.size() == 1)
      vchNodeLocator = vchFromValue(params[0]);

    std::map<vchType, int> mapAliasVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
          {
            const CWalletTx& tx = item.second;

            vchType vchS, vchR, vchKey, vchAes, vchSig;
            int nOut;
            if(!tx.GetPublicKeyUpdate(nOut, vchS, vchR, vchKey, vchAes, vchSig))
              continue;

            string keySenderAddr = stringFromVch(vchS);
            CBitcoinAddress r(keySenderAddr);
            CKeyID keyID;
            r.GetKeyID(keyID);
            CKey key;
            bool imported=false;
            if(!pwalletMain->GetKey(keyID, key))
            {
              imported=true;
            }

            const int nHeight = tx.GetHeightInMainChain();
            if(nHeight == -1)
              continue;
            assert(nHeight >= 0);

            string recipient = stringFromVch(vchR);

            Object aliasObj;
            aliasObj.push_back(Pair("sender", stringFromVch(vchS)));

            
            aliasObj.push_back(Pair("recipient", recipient));
            if(imported)
              aliasObj.push_back(Pair("status", "imported"));
            else
              aliasObj.push_back(Pair("status", "exported"));

            vchType k;
            k.reserve(vchS.size() + vchR.size());
            k.insert(k.end(), vchR.begin(), vchR.end());
            k.insert(k.end(), vchS.begin(), vchS.end());
            if(k1Export.count(k) && k1Export[k].size())
            {
              aliasObj.push_back(Pair("pending", "true"));
            }
            
            aliasObj.push_back(Pair("confirmed", "false"));

            aliasObj.push_back(Pair("key", stringFromVch(vchKey)));
            aliasObj.push_back(Pair("aes256_encrypted", stringFromVch(vchAes)));
            aliasObj.push_back(Pair("signature", stringFromVch(vchSig)));
            string a;
            if(atod(stringFromVch(vchS), a) == 0)
              aliasObj.push_back(Pair("alias", a));
            oRes.push_back(aliasObj);
        }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    for(unsigned int i=0; i<oRes.size(); i++)
    {
       Object& o = oRes[i].get_obj();

       string s = o[0].value_.get_str();
       string r = o[1].value_.get_str();
       string status = o[2].value_.get_str();
       for(unsigned int j=i+1; j<oRes.size(); j++)
       {
         Object& o1 = oRes[j].get_obj();
         if(s == o1[1].value_.get_str() && r == o1[0].value_.get_str())
         {
           o[3].value_ = "true";
           o1[3].value_ = "true";
         }
       }
    }

    return oRes;
}

bool isMyAddress(string addrStr)
{
  CBitcoinAddress r(addrStr);
  if(!r.IsValid())
  {
    return false;
  }

  CKeyID keyID;
  if(!r.GetKeyID(keyID))
    return false;

  CKey key;
  if(!pwalletMain->GetKey(keyID, key))
    return false;

  return true;
}

Value decryptedMessageList(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "decryptedMessageList [<alias>]\n"
                );
    vchType vchNodeLocator;
    if(params.size() == 1)
      vchNodeLocator = vchFromValue(params[0]);

    std::map<vchType, int> mapAliasVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
        {
            const CWalletTx& tx = item.second;

            vchType vchSender, vchRecipient, vchEncryptedMessage, ivVch, vchSig;
            int nOut;
            if(!tx.GetEncryptedMessageUpdate(nOut, vchSender, vchRecipient, vchEncryptedMessage, ivVch, vchSig))
            {
              continue;
            }

            const int nHeight = tx.GetHeightInMainChain();

            string myAddr;
            string fKey;
            if(isMyAddress(stringFromVch(vchSender)))
            {
              myAddr = stringFromVch(vchSender);
              fKey = stringFromVch(vchRecipient);
            }
            else
            {
              myAddr = stringFromVch(vchRecipient);
              fKey = stringFromVch(vchSender);
            }

            Object aliasObj;
            aliasObj.push_back(Pair("sender", stringFromVch(vchSender)));
            aliasObj.push_back(Pair("recipient", stringFromVch(vchRecipient)));
            aliasObj.push_back(Pair("encrypted_message", stringFromVch(vchEncryptedMessage)));
            string t = DateTimeStrFormat(tx.GetTxTime());
            aliasObj.push_back(Pair("time", t));


            string rsaPrivKey;
            string recipient = stringFromVch(vchRecipient);

            CBitcoinAddress r(myAddr);
            if(!r.IsValid())
            {
              continue;
              LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
              LEAVE_CRITICAL_SECTION(cs_main)
              throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");
            }

            CKeyID keyID;
            if(!r.GetKeyID(keyID))
              throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");
            CKey key;
            if(!pwalletMain->GetKey(keyID, key))
            {
              continue;
            }

            CPubKey pubKey = key.GetPubKey();

            string aesBase64Plain;
            vector<unsigned char> aesRawVector;
            if(pwalletMain->aes_(pubKey, fKey, aesBase64Plain))
            {
              bool fInvalid = false;
              aesRawVector = DecodeBase64(aesBase64Plain.c_str(), &fInvalid);
            }
            else
            {
              vchType aesKeyBase64EncryptedVch;
              vchType pub_key = pubKey.Raw();
              if(getImportedPubKey(myAddr, fKey, pub_key, aesKeyBase64EncryptedVch))
              {
                string aesKeyBase64Encrypted = stringFromVch(aesKeyBase64EncryptedVch);

                string privRSAKey;
                if(!pwalletMain->envCP0(pubKey, privRSAKey))
                  throw JSONRPCError(RPC_TYPE_ERROR, "Failed to retrieve private RSA key");

                string decryptedAESKeyBase64;
                DecryptMessage(privRSAKey, aesKeyBase64Encrypted, decryptedAESKeyBase64);
                bool fInvalid = false;
                aesRawVector = DecodeBase64(decryptedAESKeyBase64.c_str(), &fInvalid);
              }
              else
              {
                throw JSONRPCError(RPC_WALLET_ERROR, "No local symmetric key and no imported symmetric key found for recipient");
              }
            }

            string decrypted;
            string iv128Base64 = stringFromVch(ivVch);
            DecryptMessageAES(stringFromVch(vchEncryptedMessage),
                              decrypted,
                              aesRawVector,
                              iv128Base64);

            aliasObj.push_back(Pair("plain_text", decrypted));
            aliasObj.push_back(Pair("iv128Base64", stringFromVch(ivVch)));
            aliasObj.push_back(Pair("signature", stringFromVch(vchSig)));

            oRes.push_back(aliasObj);

            mapAliasVchInt[vchSender] = nHeight;
            aliasMapVchObj[vchSender] = aliasObj;
        }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    return oRes;
}
Value plainTextMessageList(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "plainTextMessageList [<alias>]\n"
                );

    vchType vchNodeLocator;
    if(params.size() == 1)
      vchNodeLocator = vchFromValue(params[0]);

    std::map<vchType, int> mapAliasVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
        {
            const CWalletTx& tx = item.second;





            vchType vchSender, vchRecipient, vchEncryptedMessage, ivVch, vchSig;
            int nOut;
            if(!tx.GetEncryptedMessageUpdate(nOut, vchSender, vchRecipient, vchEncryptedMessage, ivVch, vchSig))
              continue;

            Object aliasObj;
            aliasObj.push_back(Pair("sender", stringFromVch(vchSender)));
            aliasObj.push_back(Pair("recipient", stringFromVch(vchRecipient)));
            aliasObj.push_back(Pair("message", stringFromVch(vchEncryptedMessage)));
            aliasObj.push_back(Pair("iv128Base64", stringFromVch(ivVch)));
            aliasObj.push_back(Pair("signature", stringFromVch(vchSig)));
            oRes.push_back(aliasObj);
        }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return oRes;
}

Value aliasOut(const Array& params, bool fHelp)
{
  if(fHelp || params.size() != 2)
      throw runtime_error(
              "aliasOut [<node opt>]\n"
              );

  string k1;
  vchType vchNodeLocator;
  k1 =(params[0]).get_str();
  vchNodeLocator = vchFromValue(params[0]);
  const char* out__ = (params[1].get_str()).c_str();

  std::map<vchType, int> mapAliasVchInt;
  std::map<vchType, Object> aliasMapVchObj;

  string value;
  bool found=false;
  ENTER_CRITICAL_SECTION(cs_main)
  {
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
      BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                  pwalletMain->mapWallet)
      {
        const CWalletTx& tx = item.second;

        vchType vchAlias, vchValue;
        int nOut;
        int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
          continue;
        assert(nHeight >= 0);

        string decrypted = "";

        string strAddress = "";
        aliasAddress(tx, strAddress);
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(!pwalletMain->GetKey(keyID, key))
          {
            continue;
          }

          CPubKey pubKey = key.GetPubKey();
          if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
          {
            continue;
          }
        if(mapAliasVchInt.find(vchFromString(decrypted)) != mapAliasVchInt.end() && mapAliasVchInt[vchFromString(decrypted)] > nHeight)
        {
          continue;
        }
          mapAliasVchInt[vchFromString(decrypted)] = nHeight;

          DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);
          if(k1 != decrypted) 
          {
            continue;
          }
          else
          {
            value = stringFromVch(vchValue);
            found=true;
          }
        }
        else
        {
          if(mapAliasVchInt.find(vchAlias) != mapAliasVchInt.end() && mapAliasVchInt[vchAlias] > nHeight)
          {
            continue;
          }
          mapAliasVchInt[vchAlias] = nHeight;

          if(k1 != stringFromVch(vchAlias)) 
          {
            continue;
          }
          else
          {
            value = stringFromVch(vchValue);
            found = true;
          }
        }
      }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  }
  LEAVE_CRITICAL_SECTION(cs_main)

  if(found == true)
  {
    stringstream is(value, ios_base::in | ios_base::binary);   
    filtering_streambuf<input> in__;
    in__.push(gzip_decompressor());
    in__.push(is);
    ofstream file__(out__, ios_base::out | ios_base::binary);
    boost::iostreams::copy(in__, file__);
    return true;
  }

  return false;
}

Value nodeValidate(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "nodeValidate [<node opt>]\n"
                );

    string k1;
    vchType vchNodeLocator;
    if(params.size() == 1)
    {
      k1 =(params[0]).get_str();
      vchNodeLocator = vchFromValue(params[0]);
    }

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
          {
            const CWalletTx& tx = item.second;

            vchType vchAlias, vchValue;
            int nOut;
            int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        Object aliasObj;


        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
          continue;
        assert(nHeight >= 0);

        string decrypted = "";
        string value = stringFromVch(vchValue);

        string strAddress = "";
        aliasAddress(tx, strAddress);
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(!pwalletMain->GetKey(keyID, key))
          {
            aliasObj.push_back(Pair("alias", stringFromVch(vchAlias)));
          }
          else
          {
            CPubKey pubKey = key.GetPubKey();
            if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
            {
              continue;
            }

            DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);

            aliasObj.push_back(Pair("alias", decrypted));
          }

          aliasObj.push_back(Pair("encrypted", "true"));
        }
        else
        {
          if(k1 != stringFromVch(vchAlias)) continue;

          aliasObj.push_back(Pair("alias", stringFromVch(vchAlias)));
          aliasObj.push_back(Pair("encrypted", "false"));
        }

        aliasObj.push_back(Pair("value", value));

        if(!IsMinePost(tx))
          aliasObj.push_back(Pair("transferred", 1));
        aliasObj.push_back(Pair("address", strAddress));
        aliasObj.push_back(Pair("nHeigt", nHeight));


        CBitcoinAddress keyAddress(strAddress);
        CKeyID keyID;
        keyAddress.GetKeyID(keyID);
        CPubKey vchPubKey;
        pwalletMain->GetPubKey(keyID, vchPubKey);
        vchType vchRand;

        const int expiresIn = nHeight + scaleMonitor() - pindexBest->nHeight;
        aliasObj.push_back(Pair("expires_in", expiresIn));
        if(expiresIn <= 0)
          aliasObj.push_back(Pair("expired", 1));

        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            aliasObj.push_back(Pair("status", "pending_update"));
        }

        if(decrypted != "")
        {
          vchType d1 = vchFromString(decrypted);
          if(mapState.count(d1) && mapState[d1].size())
          {
              aliasObj.push_back(Pair("status", "pending_update"));
          }

        }
        oRes.push_back(aliasObj);
      }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    return oRes;
}
Value validateLocator(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "validateLocator <locator>\n"
            "Return information about <locator>.");

    CBitcoinAddress address;
    int r = checkAddress(params[0].get_str(), address);

    Object ret;
    if(r == 0)
    {
      ret.push_back(Pair("isvalid", true));
      CTxDestination dest = address.Get();
      bool mine = IsMine(*pwalletMain, dest);
      ret.push_back(Pair("ismine", mine));
      vchType rConvert__;
      if(getImportedPubKey(address.ToString(), rConvert__) || internalReference__(address.ToString(), rConvert__))
      {
        ret.push_back(Pair("k__status", true));
      }
      else
        ret.push_back(Pair("k__status", false));
    }
    else
      ret.push_back(Pair("isvalid", false));



    return ret;
}

Value nodeRetrieve(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "nodeRetrieve [<node opt>]\n"
                );

    string k1;
    vchType vchNodeLocator;
    if(params.size() == 1)
    {
      k1 =(params[0]).get_str();
      vchNodeLocator = vchFromValue(params[0]);
    }

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
          {
            const CWalletTx& tx = item.second;

            vchType vchAlias, vchValue;
            int nOut;
            int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        Object aliasObj;


        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
          continue;
        assert(nHeight >= 0);

        string decrypted = "";
        string value = stringFromVch(vchValue);

        string strAddress = "";
        aliasAddress(tx, strAddress);
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(!pwalletMain->GetKey(keyID, key))
          {
            continue;
          }

          CPubKey pubKey = key.GetPubKey();
          if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
          {
            continue;
          }

          DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);
          if(k1 != decrypted) 
          {
            continue;
          }

          aliasObj.push_back(Pair("alias", decrypted));

          aliasObj.push_back(Pair("encrypted", "true"));
        }
        else
        {
          if(k1 != stringFromVch(vchAlias)) continue;

          aliasObj.push_back(Pair("alias", stringFromVch(vchAlias)));
          aliasObj.push_back(Pair("encrypted", "false"));
        }

        aliasObj.push_back(Pair("value", value));

        if(!IsMinePost(tx))
          aliasObj.push_back(Pair("transferred", 1));
        aliasObj.push_back(Pair("address", strAddress));
        aliasObj.push_back(Pair("nHeigt", nHeight));


        CBitcoinAddress keyAddress(strAddress);
        CKeyID keyID;
        keyAddress.GetKeyID(keyID);
        CPubKey vchPubKey;
        pwalletMain->GetPubKey(keyID, vchPubKey);
        vchType vchRand;

        const int expiresIn = nHeight + scaleMonitor() - pindexBest->nHeight;
        aliasObj.push_back(Pair("expires_in", expiresIn));
        if(expiresIn <= 0)
          aliasObj.push_back(Pair("expired", 1));

        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            aliasObj.push_back(Pair("status", "pending_update"));
        }

        if(decrypted != "")
        {
          vchType d1 = vchFromString(decrypted);
          if(mapState.count(d1) && mapState[d1].size())
          {
              aliasObj.push_back(Pair("status", "pending_update"));
          }

        }
        oRes.push_back(aliasObj);
      }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    //Array oRes;
    //BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, aliasMapVchObj)
    //    oRes.push_back(item.second);

    return oRes;
}

Value getNodeRecord(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "getNodeRecord [<node opt>]\n"
                );

    string k1;
    vchType vchNodeLocator;
    if(params.size() == 1)
    {
      k1 =(params[0]).get_str();
      vchNodeLocator = vchFromValue(params[0]);
    }


    std::map<vchType, int> mapAliasVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
          {
            const CWalletTx& tx = item.second;

            vchType vchAlias, vchValue;
            int nOut;
            int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        Object aliasObj;


        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
          continue;
        assert(nHeight >= 0);

        string decrypted = "";
        string value = stringFromVch(vchValue);

        string strAddress = "";
        aliasAddress(tx, strAddress);
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(!pwalletMain->GetKey(keyID, key))
          {
            continue;
          }

          CPubKey pubKey = key.GetPubKey();
          if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
          {
            continue;
          }

          DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);
          if(k1 != decrypted) 
          {
            continue;
          }

        if(mapAliasVchInt.find(vchFromString(decrypted)) != mapAliasVchInt.end() && mapAliasVchInt[vchFromString(decrypted)] > nHeight)
        {
          continue;
        }
          aliasObj.push_back(Pair("alias", decrypted));

          aliasObj.push_back(Pair("encrypted", "true"));
          mapAliasVchInt[vchFromString(decrypted)] = nHeight;
        }
        else
        {
        if(mapAliasVchInt.find(vchAlias) != mapAliasVchInt.end() && mapAliasVchInt[vchAlias] > nHeight)
        {
          continue;
        }

          if(k1 != stringFromVch(vchAlias)) continue;

          aliasObj.push_back(Pair("alias", stringFromVch(vchAlias)));
          aliasObj.push_back(Pair("encrypted", "false"));
          mapAliasVchInt[vchAlias] = nHeight;
        }

        aliasObj.push_back(Pair("value", value));

        if(!IsMinePost(tx))
          aliasObj.push_back(Pair("transferred", 1));
        aliasObj.push_back(Pair("address", strAddress));
        aliasObj.push_back(Pair("nHeigt", nHeight));


        CBitcoinAddress keyAddress(strAddress);
        CKeyID keyID;
        keyAddress.GetKeyID(keyID);
        CPubKey vchPubKey;
        pwalletMain->GetPubKey(keyID, vchPubKey);
        vchType vchRand;

        const int expiresIn = nHeight + scaleMonitor() - pindexBest->nHeight;
        aliasObj.push_back(Pair("expires_in", expiresIn));
        if(expiresIn <= 0)
          aliasObj.push_back(Pair("expired", 1));

        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            aliasObj.push_back(Pair("status", "pending_update"));
        }

        if(decrypted != "")
        {
          vchType d1 = vchFromString(decrypted);
          if(mapState.count(d1) && mapState[d1].size())
          {
              aliasObj.push_back(Pair("status", "pending_update"));
          }

        }


        if(op__ != OP_ALIAS_ENCRYPTED)
          aliasMapVchObj[vchAlias] = aliasObj;
        else
          aliasMapVchObj[vchFromString(decrypted)] = aliasObj;

      }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, aliasMapVchObj)
        oRes.push_back(item.second);

    return oRes;
}

bool searchAliasEncrypted2(string alias, uint256& wtxInHash)
{
  std::transform(alias.begin(), alias.end(), alias.begin(), ::tolower);
  bool found=false;
  ENTER_CRITICAL_SECTION(cs_main)
  {
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
      BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                    pwalletMain->mapWallet)
      {
        const CWalletTx& tx = item.second;

        vchType vchAlias, vchValue;
        int nOut;
        int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        if(tx.IsSpent(nOut))
          continue;

        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
              continue;
        assert(nHeight >= 0);

        string strAddress = "";
        aliasAddress(tx, strAddress);
        string decrypted;
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(!pwalletMain->GetKey(keyID, key))
          {
            continue;
          }

          CPubKey pubKey = key.GetPubKey();
          if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
          {
            throw JSONRPCError(RPC_WALLET_ERROR, "error p0");
          }

          DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);
          std::transform(decrypted.begin(), decrypted.end(), decrypted.begin(), ::tolower);
          if(decrypted == alias)
          {
            found=true;
            wtxInHash=tx.GetHash();
            break;
          }
        }
      }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  }
  LEAVE_CRITICAL_SECTION(cs_main)

  return found;
}

bool searchAliasEncrypted(string alias, uint256& wtxInHash)
{
  bool found=false;
  ENTER_CRITICAL_SECTION(cs_main)
  {
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
      BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                    pwalletMain->mapWallet)
      {
        const CWalletTx& tx = item.second;

        vchType vchAlias, vchValue;
        int nOut;
        int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        if(tx.IsSpent(nOut))
          continue;

        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
              continue;
        assert(nHeight >= 0);

        string strAddress = "";
        aliasAddress(tx, strAddress);
        string decrypted;
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
          {
            continue;
          }

          CKey key;
          if(!pwalletMain->GetKey(keyID, key))
            throw JSONRPCError(RPC_WALLET_ERROR, "sae : Private key not available");

          CPubKey pubKey = key.GetPubKey();
          if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
          {
            throw JSONRPCError(RPC_WALLET_ERROR, "error p0");
          }

          DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);
          if(decrypted == alias)
          {
            found=true;
            wtxInHash=tx.GetHash();
            break;
          }
        }
      }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  }
  LEAVE_CRITICAL_SECTION(cs_main)

  return found;
}

Value aliasList__(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "aliasList__ [<alias>]\n"
                );

    vchType vchNodeLocator;
    if(params.size() == 1)
      vchNodeLocator = vchFromValue(params[0]);

    std::map<vchType, int> mapAliasVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
          {
            const CWalletTx& tx = item.second;

            vchType vchAlias, vchValue;
            int nOut;
            int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        Object aliasObj;


        if(!vchNodeLocator.empty() && vchNodeLocator != vchAlias)
          continue;

        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
          continue;
        assert(nHeight >= 0);

        string decrypted = "";

        string strAddress = "";
        aliasAddress(tx, strAddress);
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(pwalletMain->GetKey(keyID, key))
          {
            CPubKey pubKey = key.GetPubKey();
            if(pwalletMain->envCP0(pubKey, rsaPrivKey) == true)
            {
              DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);
              aliasObj.push_back(Pair("alias", decrypted));
            }
          }
          else
          {
            for(int i=0; i < tx.vin.size(); i++)
            {
              COutPoint prevout = tx.vin[i].prevout;
              CWalletTx& txPrev = pwalletMain->mapWallet[prevout.hash];

              CTxOut& out = txPrev.vout[prevout.n];

              std::vector<vchType> vvchPrevArgsRead;
              int prevOp;
              if(aliasScript(out.scriptPubKey, prevOp, vvchPrevArgsRead))
              {
                string a__ = "";
                aliasAddress(txPrev, a__);

                CBitcoinAddress r0(a__);
                if(!r0.IsValid())
                  throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

                CKeyID keyID_0;
                if(!r0.GetKeyID(keyID_0))
                  throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

                CKey key0;
                if(pwalletMain->GetKey(keyID_0, key0))
                {
                  CPubKey pubKey = key0.GetPubKey();
                  if(pwalletMain->envCP0(pubKey, rsaPrivKey) == true)
                  {
                    DecryptMessage(rsaPrivKey, stringFromVch(vvchPrevArgsRead[0]), decrypted);
                    aliasObj.push_back(Pair("alias", decrypted));
                  }
                }

                break;     
              }
            }
          }

        if(mapAliasVchInt.find(vchFromString(decrypted)) != mapAliasVchInt.end() && mapAliasVchInt[vchFromString(decrypted)] > nHeight)
        {
          continue;
        }

          aliasObj.push_back(Pair("encrypted", "true"));
          mapAliasVchInt[vchFromString(decrypted)] = nHeight;
        }
        else
        {
          if(mapAliasVchInt.find(vchAlias) != mapAliasVchInt.end() && mapAliasVchInt[vchAlias] > nHeight)
          {
            continue;
          }
          string s = stringFromVch(vchAlias);
          aliasObj.push_back(Pair("alias", s));
          aliasObj.push_back(Pair("encrypted", "false"));
          Value v = xtu_url__(s); 
          aliasObj.push_back(Pair("xtu", v.get_real()));
          mapAliasVchInt[vchAlias] = nHeight;
        }

        if(!IsMinePost(tx))
          aliasObj.push_back(Pair("transferred", 1));
        aliasObj.push_back(Pair("address", strAddress));
        aliasObj.push_back(Pair("nHeigt", nHeight));

        CBitcoinAddress keyAddress(strAddress);
        CKeyID keyID;
        keyAddress.GetKeyID(keyID);
        CPubKey vchPubKey;
        pwalletMain->GetPubKey(keyID, vchPubKey);
        vchType vchRand;

        const int expiresIn = nHeight + scaleMonitor() - pindexBest->nHeight;
        aliasObj.push_back(Pair("expires_in", expiresIn));
        if(expiresIn <= 0)
          aliasObj.push_back(Pair("expired", 1));

        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            aliasObj.push_back(Pair("status", "pending_update"));
        }

        if(decrypted != "")
        {
          vchType d1 = vchFromString(decrypted);
          if(mapState.count(d1) && mapState[d1].size())
          {
              aliasObj.push_back(Pair("status", "pending_update"));
          }

        }


        if(op__ != OP_ALIAS_ENCRYPTED)
          aliasMapVchObj[vchAlias] = aliasObj;
        else
          aliasMapVchObj[vchFromString(decrypted)] = aliasObj;

      }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, aliasMapVchObj)
        oRes.push_back(item.second);

    return oRes;
}

Value aliasList(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
                "aliasList [<alias>]\n"
                );

    vchType vchNodeLocator;
    if(params.size() == 1)
      vchNodeLocator = vchFromValue(params[0]);

    std::map<vchType, int> mapAliasVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
          {
            CWalletTx& tx = item.second;

            vchType vchAlias, vchValue;
            int nOut;
            int op__=-1;
        if(!tx.aliasSet(op__, nOut, vchAlias, vchValue))
          continue;

        Object aliasObj;

        if(!vchNodeLocator.empty() && vchNodeLocator != vchAlias)
          continue;

        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
          continue;
        assert(nHeight >= 0);

        string decrypted = "";
        string value = stringFromVch(vchValue);

        string strAddress = "";
        aliasAddress(tx, strAddress);
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          string rsaPrivKey;
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(pwalletMain->GetKey(keyID, key))
          {
            CPubKey pubKey = key.GetPubKey();
            if(pwalletMain->envCP0(pubKey, rsaPrivKey) == true)
            {
              DecryptMessage(rsaPrivKey, stringFromVch(vchAlias), decrypted);
              aliasObj.push_back(Pair("alias", decrypted));
            }
          }
          else
          {
            for(int i=0; i < tx.vin.size(); i++)
            {
              COutPoint prevout = tx.vin[i].prevout;
              CWalletTx& txPrev = pwalletMain->mapWallet[prevout.hash];

              CTxOut& out = txPrev.vout[prevout.n];

              std::vector<vchType> vvchPrevArgsRead;
              int prevOp;
              if(aliasScript(out.scriptPubKey, prevOp, vvchPrevArgsRead))
              {
                string a__ = "";
                aliasAddress(txPrev, a__);

                CBitcoinAddress r0(a__);
                if(!r0.IsValid())
                  throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

                CKeyID keyID_0;
                if(!r0.GetKeyID(keyID_0))
                  throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

                CKey key0;
                if(pwalletMain->GetKey(keyID_0, key0))
                {
                  CPubKey pubKey = key0.GetPubKey();
                  if(pwalletMain->envCP0(pubKey, rsaPrivKey) == true)
                  {
                    DecryptMessage(rsaPrivKey, stringFromVch(vvchPrevArgsRead[0]), decrypted);
                    aliasObj.push_back(Pair("alias", decrypted));
                  }
                }

                break;     
              }
            }
          }

          if(mapAliasVchInt.find(vchFromString(decrypted)) != mapAliasVchInt.end() && mapAliasVchInt[vchFromString(decrypted)] > nHeight)
          {
            continue;
          }

          aliasObj.push_back(Pair("encrypted", "true"));
          mapAliasVchInt[vchFromString(decrypted)] = nHeight;
        }
        else
        {
          if(mapAliasVchInt.find(vchAlias) != mapAliasVchInt.end() && mapAliasVchInt[vchAlias] > nHeight)
          {
            continue;
          }
          aliasObj.push_back(Pair("alias", stringFromVch(vchAlias)));
          aliasObj.push_back(Pair("encrypted", "false"));
          mapAliasVchInt[vchAlias] = nHeight;
        }


        aliasObj.push_back(Pair("value", value));

        if(!IsMinePost(tx))
          aliasObj.push_back(Pair("transferred", 1));
        aliasObj.push_back(Pair("address", strAddress));
        aliasObj.push_back(Pair("nHeigt", nHeight));


        CBitcoinAddress keyAddress(strAddress);
        CKeyID keyID;
        keyAddress.GetKeyID(keyID);
        CPubKey vchPubKey;
        pwalletMain->GetPubKey(keyID, vchPubKey);
        vchType vchRand;

        const int expiresIn = nHeight + scaleMonitor() - pindexBest->nHeight;
        aliasObj.push_back(Pair("expires_in", expiresIn));
        if(expiresIn <= 0)
          aliasObj.push_back(Pair("expired", 1));

        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            aliasObj.push_back(Pair("status", "pending_update"));
        }

        if(decrypted != "")
        {
          vchType d1 = vchFromString(decrypted);
          if(mapState.count(d1) && mapState[d1].size())
          {
              aliasObj.push_back(Pair("status", "pending_update"));
          }

        }


        if(op__ != OP_ALIAS_ENCRYPTED)
          aliasMapVchObj[vchAlias] = aliasObj;
        else
          aliasMapVchObj[vchFromString(decrypted)] = aliasObj;

      }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, aliasMapVchObj)
        oRes.push_back(item.second);

    return oRes;
}
Value nodeDebug(const Array& params, bool fHelp)
{
    if(fHelp)
        throw runtime_error(
            "nodeDebug\n"
            "Dump pending transactions id in the debug file.\n");

    printf("Pending:\n----------------------------\n");
    pair<vector<unsigned char>, set<uint256> > pairPending;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        BOOST_FOREACH(pairPending, mapState)
        {
            string alias = stringFromVch(pairPending.first);
            uint256 hash;
            BOOST_FOREACH(hash, pairPending.second)
            {
                if(!pwalletMain->mapWallet.count(hash))
                    printf("foreign ");
                printf("    %s\n", hash.GetHex().c_str());
            }
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    printf("----------------------------\n");
    return true;
}
Value nodeDebug1(const Array& params, bool fHelp)
{
    if(fHelp || params.size() < 1)
        throw runtime_error(
            "nodeDebug1 <alias>\n"
            "Dump alias blocks number and transactions id in the debug file.\n");

    vector<unsigned char> vchAlias = vchFromValue(params[0]);
    ENTER_CRITICAL_SECTION(cs_main)
    {

        vector<AliasIndex> vtxPos;
        LocatorNodeDB aliasCacheDB("r");
        if(!aliasCacheDB.lGet(vchAlias, vtxPos))
        {
            error("failed to read from alias DB");
            return false;
        }

        AliasIndex txPos;
        BOOST_FOREACH(txPos, vtxPos)
        {
            CTransaction tx;
            if(!tx.ReadFromDisk(txPos.txPos))
            {
                error("could not read txpos %s", txPos.txPos.ToString().c_str());
                continue;
            }
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    printf("-------------------------\n");
    return true;
}
Value transform(const Array& params, bool fHelp)
{
  if(fHelp || params.size() != 2)
      throw runtime_error(
              "aliasOut [<node opt>]\n"
              );
  string locatorStr = params[0].get_str();

  std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
  const vchType vchAlias = vchFromValue(locatorStr);
  const char* locatorFile = (params[1].get_str()).c_str();

  fs::path p = locatorFile;
  if(!fs::exists(p))
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Locator file does not exist");

  ifstream file(locatorFile, ios_base::in | ios_base::binary);
  filtering_streambuf<input> in;
  in.push(gzip_compressor());
  in.push(file);

  stringstream ss;
  boost::iostreams::copy(in, ss);
  string s = ss.str();

  vchType kAlpha;
  GenerateAESKey(kAlpha);

  string alpha = EncodeBase64(&kAlpha[0], kAlpha.size());

  string gen;
  string reference;
    
  bool fInvalid = false;
  vector<unsigned char> v = DecodeBase64(alpha.c_str(), &fInvalid);

  EncryptMessageAES(s, gen, v, reference);


  vector<unsigned char> aesRawVector = DecodeBase64(alpha.c_str(), &fInvalid);
  string decrypted;
  DecryptMessageAES(gen,
    decrypted,
    aesRawVector,
    reference);

  stringstream is(decrypted, ios_base::in | ios_base::binary);   
  filtering_streambuf<input> in__;
  in__.push(gzip_decompressor());
  in__.push(is);
  ofstream file__("T", ios_base::binary);
  boost::iostreams::copy(in__, file__);

  return true;
}
Value validate(const Array& params, bool fHelp)
{
  if(fHelp || params.size() != 2)
      throw runtime_error(
              "aliasOut [<node opt>]\n"
              );

  string k1;
  vchType vchNodeLocator;
  k1 =(params[0]).get_str();
  vchNodeLocator = vchFromValue(params[0]);
  const char* out__ = (params[1].get_str()).c_str();

  std::map<vchType, int> mapAliasVchInt;
  std::map<vchType, Object> aliasMapVchObj;

  vector< vector<unsigned char> > vvch;
  bool found=false;
  ENTER_CRITICAL_SECTION(cs_main)
  {
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
      BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                  pwalletMain->mapWallet)
      {
        const CWalletTx& tx = item.second;

        vector< vector<unsigned char> > vv;
        int nOut;
        int op__=-1;
        if(!tx.aliasSet(op__, nOut, vv))
        {
          continue;
        }

        if(op__ != OP_ALIAS_ENCRYPTED)
        {
          continue;
        }

        const int nHeight = tx.GetHeightInMainChain();
        if(nHeight == -1)
          continue;
        assert(nHeight >= 0);

        string decrypted = "";

        string strAddress = "";
        aliasAddress(tx, strAddress);
        if(op__ == OP_ALIAS_ENCRYPTED)
        {
          CBitcoinAddress r(strAddress);
          if(!r.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

          CKeyID keyID;
          if(!r.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

          CKey key;
          if(!pwalletMain->GetKey(keyID, key))
          {
            continue;
          }

          CPubKey pubKey = key.GetPubKey();
          string rsaPrivKey;
          if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
          {
            continue;
          }
        if(mapAliasVchInt.find(vchFromString(decrypted)) != mapAliasVchInt.end() && mapAliasVchInt[vchFromString(decrypted)] > nHeight)
        {
          continue;
        }
          mapAliasVchInt[vchFromString(decrypted)] = nHeight;

          DecryptMessage(rsaPrivKey, stringFromVch(vv[0]), decrypted);
          if(k1 != decrypted) 
          {
            continue;
          }
          else
          {
            vvch = vv;
            found=true;
          }
        }
      }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  }
  LEAVE_CRITICAL_SECTION(cs_main)


  if(found == true)
  {
    string value;
    string iv128 = stringFromVch(vvch[6]);
    State hydr(stringFromVch(vvch[7]));
    if(hydr() == State::ATOMIC)
    {
      Relay r;
      if(pwalletMain->relay_(vchNodeLocator, r))
      {
        string ctrl_ = r.ctrl_();
        bool fInvalid = false;
        vector<unsigned char> aesRawVector = DecodeBase64(ctrl_.c_str(), &fInvalid);
        string decrypted;
        DecryptMessageAES(stringFromVch(vvch[4]),
          decrypted,
          aesRawVector,
          iv128);

        value = decrypted;
      }
    }
    else if(hydr() == State::GROUND)
    {
      value = stringFromVch(vvch[4]);
    }
    else
    {
      string aesKeyStr;
      string localAddr = stringFromVch(vchNodeLocator);
      string f = stringFromVch(vvch[7]);
      if(channel(localAddr, f, aesKeyStr))
      {
        bool fInvalid = false;
        vector<unsigned char> aesRawVector = DecodeBase64(aesKeyStr.c_str(), &fInvalid);
        string decrypted;
        DecryptMessageAES(stringFromVch(vvch[4]),
          decrypted,
          aesRawVector,
          iv128);

        value = decrypted;
      }
      else
      {
        throw JSONRPCError(RPC_TYPE_ERROR, "key not foound");
      }
    }

    try
    {
    stringstream is(value, ios_base::in | ios_base::binary);   
    filtering_streambuf<input> in__;
    in__.push(gzip_decompressor());
    in__.push(is);
    ofstream file__(out__, ios_base::binary);
    boost::iostreams::copy(in__, file__);
    }
    catch(std::exception& e)
    {
      std::cerr << e.what() << std::endl; 
    }

    return true;
  }

  return false;
}
Value transientStatus__C(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
                "transientStatus__C <locator> <file>"
                + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();

    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    const vchType vchAlias = vchFromString(locatorStr);

    Object ret;
    const char* locatorFile = (params[1].get_str()).c_str();
    fs::path p = locatorFile;
    if(!fs::exists(p))
    {
      ret.push_back(Pair("status", "error"));
      ret.push_back(Pair("message", "Locator file does not exist"));
      return ret;
    }

    ifstream file(locatorFile, ios_base::in | ios_base::binary);
    filtering_streambuf<input> in;
    in.push(gzip_compressor());
    in.push(file);

    stringstream ss;
    boost::iostreams::copy(in, ss);

    string s = ss.str();
    if(s.size() > MAX_XUNIT_LENGTH) 
    {
      ret.push_back(Pair("status", "error"));
      ret.push_back(Pair("message", "Locator file compressed size too large"));
      return ret;
    }
   
    const vchType vchValue = vchFromValue(s);


    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          ret.push_back(Pair("status", "error"));
          ret.push_back(Pair("message", "pending ops on that alias"));
          return ret;
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchAlias, tx))
        {
          ret.push_back(Pair("status", "error"));
          ret.push_back(Pair("message", "alias is active"));
          return ret;
        }
    }

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchAliasEncrypted2(stringFromVch(vchAlias), wtxInHash))
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          ret.push_back(Pair("status", "error"));
          ret.push_back(Pair("message", "alias not found"));
          return ret;
        }

        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          ret.push_back(Pair("status", "error"));
          ret.push_back(Pair("message", "prev tx not in wallet"));
          return ret;
        }

        CScript scriptPubKeyOrig;

        CScript scriptPubKey;


        CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) {
                if(op != OP_ALIAS_ENCRYPTED)
                {
                  ret.push_back(Pair("status", "error"));
                  ret.push_back(Pair("message", "prev tx was not encrypted"));
                  return ret;
                }

              string encrypted = stringFromVch(vvch[0]);
              uint160 hash = uint160(vvch[3]);
              string value = stringFromVch(vchValue);

              CDataStream ss(SER_GETHASH, 0);
              ss << encrypted;
              ss << hash.ToString();
              ss << value;
              ss << string("0");

              CScript script;
              script.SetBitcoinAddress(stringFromVch(vvch[2]));

              CBitcoinAddress ownerAddr = script.GetBitcoinAddress();
              if(!ownerAddr.IsValid())
              {
                  ret.push_back(Pair("status", "error"));
                  ret.push_back(Pair("message", "intern invalid owner"));
                  return ret;
              }

              CKeyID keyID;
              if(!ownerAddr.GetKeyID(keyID))
              {
                ret.push_back(Pair("status", "error"));
                ret.push_back(Pair("message", "intern: owner key"));
                return ret;
              }

              CKey key;
              if(!pwalletMain->GetKey(keyID, key))
              {
                ret.push_back(Pair("status", "error"));
                ret.push_back(Pair("message", "intern: priv key"));
                return ret;
              }

              CPubKey vchPubKey;
              pwalletMain->GetPubKey(keyID, vchPubKey);

              vector<unsigned char> vchSig;
              if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                  throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

              string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

              scriptPubKey << OP_ALIAS_ENCRYPTED << vvch[0] << vchFromString(sigBase64) << vvch[2] << vvch[3] << vchValue << vchFromString("0") << vchFromString("_") << vchFromString(State::GROUND) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
              scriptPubKeyOrig.SetBitcoinAddress(stringFromVch(vvch[2]));
              scriptPubKey += scriptPubKeyOrig;
              found = true;
            }
        }

        if(!found)
        {
            throw runtime_error("previous tx on this alias is not a alias tx");
        }

        int64_t t;
        string strError;
        bool s = txRelayPre__(scriptPubKey, wtxIn, wtx, t, strError);
        if(!s)
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          ret.push_back(Pair("status", "error"));
          ret.push_back(Pair("message", strError));
          return ret;
        }
        ret.push_back(Pair("status", "ok"));
        ret.push_back(Pair("fee", ValueFromAmount(t)));
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return ret;
}

Value updateEncryptedAliasFile(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
                "updateEncryptedAlias <locator> <file>"
                + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();

    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    const vchType vchAlias = vchFromString(locatorStr);

    const char* locatorFile = (params[1].get_str()).c_str();
    fs::path p = locatorFile;
    if(!fs::exists(p))
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Locator file does not exist");

    ifstream file(locatorFile, ios_base::in | ios_base::binary);
    filtering_streambuf<input> in;
    in.push(gzip_compressor());
    in.push(file);

    stringstream ss;
    boost::iostreams::copy(in, ss);

    string s = ss.str();
    if(s.size() > MAX_XUNIT_LENGTH) 
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Locator file too large");
   
    const vchType vchValue = vchFromValue(s);


    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            error("updateEncryptedAlias() : there are %lu pending operations on that alias, including %s",
                    mapState[vchAlias].size(),
                    mapState[vchAlias].begin()->GetHex().c_str());
          LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchAlias, tx))
        {
            error("updateEncryptedAlias() : this alias is already active with tx %s",
                    tx.GetHash().GetHex().c_str());
            throw runtime_error("this alias is already active");
        }
    }

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchAliasEncrypted2(stringFromVch(vchAlias), wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
          throw runtime_error("could not find a coin with this alias, try specifying the registerAlias transaction id");
        }


        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("previous transaction is not in the wallet");
        }

        CScript scriptPubKeyOrig;

        CScript scriptPubKey;


        CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) {
                if(op != OP_ALIAS_ENCRYPTED)
                  throw runtime_error("previous transaction was not an OP_ALIAS_ENCRYPTED");

              string encrypted = stringFromVch(vvch[0]);
              uint160 hash = uint160(vvch[3]);
              string value = stringFromVch(vchValue);

              CDataStream ss(SER_GETHASH, 0);
              ss << encrypted;
              ss << hash.ToString();
              ss << value;
              ss << string("0");

              CScript script;
              script.SetBitcoinAddress(stringFromVch(vvch[2]));

              CBitcoinAddress ownerAddr = script.GetBitcoinAddress();
              if(!ownerAddr.IsValid())
                throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

              CKeyID keyID;
              if(!ownerAddr.GetKeyID(keyID))
                throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");

              CKey key;
              if(!pwalletMain->GetKey(keyID, key))
                throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

              CPubKey vchPubKey;
              pwalletMain->GetPubKey(keyID, vchPubKey);

              vector<unsigned char> vchSig;
              if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                  throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

              string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

              scriptPubKey << OP_ALIAS_ENCRYPTED << vvch[0] << vchFromString(sigBase64) << vvch[2] << vvch[3] << vchValue << vchFromString("0") << vchFromString("_") << vchFromString(State::GROUND) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
              scriptPubKeyOrig.SetBitcoinAddress(stringFromVch(vvch[2]));
              scriptPubKey += scriptPubKeyOrig;
              found = true;
            }
        }

        if(!found)
        {
            throw runtime_error("previous tx on this alias is not a alias tx");
        }

        string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);
        if(strError != "")
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return wtx.GetHash().GetHex();
}
Value updateEncryptedAlias(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 3)
        throw runtime_error(
                "updateEncryptedAlias <alias> <value> <address>"
                + HelpRequiringPassphrase());

    const vchType vchAlias = vchFromString(params[0].get_str());
    vchType vchValue = vchFromString(params[1].get_str());

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            error("updateEncryptedAlias() : there are %lu pending operations on that alias, including %s",
                    mapState[vchAlias].size(),
                    mapState[vchAlias].begin()->GetHex().c_str());
    LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchAlias, tx))
        {
            error("updateEncryptedAlias() : this alias is already active with tx %s",
                    tx.GetHash().GetHex().c_str());
            throw runtime_error("this alias is already active");
        }
    }

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchAliasEncrypted(stringFromVch(vchAlias), wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
          throw runtime_error("could not find a coin with this alias, try specifying the registerAlias transaction id");
        }


        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("previous transaction is not in the wallet");
        }

        CScript scriptPubKeyOrig;

        CScript scriptPubKey;


        CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) 
            {
                if(op != OP_ALIAS_ENCRYPTED)
                  throw runtime_error("previous transaction was not an OP_ALIAS_ENCRYPTED");

              string encrypted = stringFromVch(vvch[0]);
              uint160 hash = uint160(vvch[3]);
              string value = stringFromVch(vchValue);

              CDataStream ss(SER_GETHASH, 0);
              ss << encrypted;
              ss << hash.ToString();
              ss << value;
              ss << string("0");

              CScript script;
              script.SetBitcoinAddress(stringFromVch(vvch[2]));

              CBitcoinAddress ownerAddr = script.GetBitcoinAddress();
              if(!ownerAddr.IsValid())
                throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

              CKeyID keyID;
              if(!ownerAddr.GetKeyID(keyID))
                throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");


              CKey key;
              if(!pwalletMain->GetKey(keyID, key))
                throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

              CPubKey vchPubKey;
              pwalletMain->GetPubKey(keyID, vchPubKey);

              vector<unsigned char> vchSig;
              if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                  throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

              string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

              scriptPubKey << OP_ALIAS_ENCRYPTED << vvch[0] << vchFromString(sigBase64) << vvch[2] << vvch[3] << vchValue << vchFromString("0") << vchFromString("_") << vchFromString(State::GROUND) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
              scriptPubKeyOrig.SetBitcoinAddress(stringFromVch(vvch[2]));
              scriptPubKey += scriptPubKeyOrig;
              found = true;
            }
        }

        if(!found)
        {
            throw runtime_error("previous tx on this alias is not a alias tx");
        }

        string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);
        if(strError != "")
        {
    LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return wtx.GetHash().GetHex();
}

Value decryptAlias(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2 )
        throw runtime_error(
                "decryptAlias <alias> <address specified by owner>\n"
                + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();
    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    const vchType vchAlias = vchFromValue(locatorStr);
    const std::string addressOfOwner = params[1].get_str();

    CBitcoinAddress ownerAddr(addressOfOwner);
    if(!ownerAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

    CKeyID keyID;
    if(!ownerAddr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "1 Private key not available");

    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);

    string rsaPubKeyStr = "";
    if(!pwalletMain->envCP1(key.GetPubKey(), rsaPubKeyStr))
        throw JSONRPCError(RPC_WALLET_ERROR, "no rsa key available for address");

    vchType vchRand;
    string r_;
    if(!pwalletMain->GetRandomKeyMetadata(key.GetPubKey(), vchRand, r_))
        throw JSONRPCError(RPC_WALLET_ERROR, "no random key available for address");


    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            error("decryptAlias() : there are %lu pending operations on that alias, including %s",
                    mapState[vchAlias].size(),
                    mapState[vchAlias].begin()->GetHex().c_str());
    LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchAlias, tx))
        {
            error("decryptAlias() : this alias is already active with tx %s",
                    tx.GetHash().GetHex().c_str());
            throw runtime_error("this alias is already active");
        }
    }


    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchAliasEncrypted2(stringFromVch(vchAlias), wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
          throw runtime_error("could not find a coin with this alias, try specifying the registerAlias transaction id");
        }


        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("previous transaction is not in the wallet");
        }

        CScript scriptPubKeyOrig;
        scriptPubKeyOrig.SetBitcoinAddress(addressOfOwner);

        CScript scriptPubKey;


        CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        vector<unsigned char> vchPrevSig;
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) {
                if(op != OP_ALIAS_ENCRYPTED)
                    throw runtime_error("previous transaction wasn't a registerAlias");
                CDataStream ss(SER_GETHASH, 0);
                ss << locatorStr;
                CScript script;
                script.SetBitcoinAddress(stringFromVch(vvch[2]));

                CBitcoinAddress ownerAddr = script.GetBitcoinAddress();
                if(!ownerAddr.IsValid())
                  throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

                CKeyID keyID;
                if(!ownerAddr.GetKeyID(keyID))
                  throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");


                CKey key;
                if(!pwalletMain->GetKey(keyID, key))
                  throw JSONRPCError(RPC_WALLET_ERROR, "2 Private key not available");

                CPubKey vchPubKey;
                pwalletMain->GetPubKey(keyID, vchPubKey);

                vector<unsigned char> vchSig;
                if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                  throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

                string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());


                scriptPubKey << OP_ALIAS_SET << vchAlias << vchFromString(sigBase64) << vchFromString(addressOfOwner) << vchRand << vvch[4] << OP_2DROP << OP_2DROP << OP_2DROP;
                scriptPubKey += scriptPubKeyOrig;

                found = true;
            }
        }

        if(!found)
        {
            throw runtime_error("previous tx on this alias is not a alias tx");
        }

        string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);
        if(strError != "")
        {
    LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return wtx.GetHash().GetHex();
}

Value transferEncryptedAlias(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 3)
        throw runtime_error(
          "transferAlias <alias> <localaddress> <toaddress> transfer a alias to a new address"
          + HelpRequiringPassphrase());

    vchType vchAlias = vchFromValue(params[0]);

    CBitcoinAddress localAddr((params[1]).get_str());

    if(!localAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

    CKeyID localkeyID;
    if(!localAddr.GetKeyID(localkeyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");

    CKey localkey;
    if(!pwalletMain->GetKey(localkeyID, localkey))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    vchType vchRand;
    string r_;
    if(!pwalletMain->GetRandomKeyMetadata(localkey.GetPubKey(), vchRand, r_))
        throw JSONRPCError(RPC_WALLET_ERROR, "no random key available for address");

    CPubKey vchPubKey;
    pwalletMain->GetPubKey(localkeyID, vchPubKey);

    string recipientAddrStr=(params[2]).get_str();
    CBitcoinAddress address(recipientAddrStr);
    if(!address.IsValid())
    {
      vector<AliasIndex> vtxPos;
      LocatorNodeDB ln1Db("r");
      vchType vchAlias = vchFromString(recipientAddrStr);
      if (ln1Db.lKey (vchAlias))
      {
        if (!ln1Db.lGet (vchAlias, vtxPos))
          return error("aliasHeight() : failed to read from name DB");
        if (vtxPos.empty ())
          return -1;

        AliasIndex& txPos = vtxPos.back ();
        address.SetString(txPos.vAddress); 
      }
      else
      {
          throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias");
      }
    }

    vchType rVch = vchFromString(address.ToString());

    CKeyID rkeyID;
    if(!address.GetKeyID(rkeyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");

    CPubKey vchRecipientPubKey;
    pwalletMain->GetPubKey(rkeyID, vchRecipientPubKey);

    vchType v1 = vchRecipientPubKey.Raw();
    string v1Str = stringFromVch(v1);

    CBitcoinAddress tmp;
    tmp.Set(rkeyID);
    string a=(tmp).ToString();

    string locatorStr = stringFromVch(vchAlias);

    vchType recipientPubKeyVch;
    vchType aesVch;
    vector<unsigned char> aesRawVector;
    string f = address.ToString();
    if(!getImportedPubKey(localAddr.ToString(), f, recipientPubKeyVch, aesVch))
    {
      if(!internalReference__(f, recipientPubKeyVch))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "transferEncryptedAlias no RSA key for recipient");

      string aesKeyStr;
      if(pwalletMain->aes_(vchPubKey, f, aesKeyStr))
      {
        bool fInvalid = false;
        aesRawVector = DecodeBase64(aesKeyStr.c_str(), &fInvalid);
      }
    }

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    scriptPubKeyOrig.SetBitcoinAddress(address.ToString());

    CScript scriptPubKey;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      uint256 wtxInHash;
      if(!searchAliasEncrypted(stringFromVch(vchAlias), wtxInHash))
      {
        LEAVE_CRITICAL_SECTION(cs_main)
        throw runtime_error("could not find this alias");
      }

      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
          if(mapState.count(vchAlias) && mapState[vchAlias].size())
          {
              error("updateEncryptedAlias() : there are %lu pending operations on that alias, including %s",
                      mapState[vchAlias].size(),
                      mapState[vchAlias].begin()->GetHex().c_str());
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    LEAVE_CRITICAL_SECTION(cs_main)
              throw runtime_error("there are pending operations on that alias");
          }

          EnsureWalletIsUnlocked();

          if(!pwalletMain->mapWallet.count(wtxInHash))
          {
              error("updateEncryptedAlias() : this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    LEAVE_CRITICAL_SECTION(cs_main)
              throw runtime_error("this coin is not in your wallet");
          }

          string encryptedAliasForRecipient;
          EncryptMessage(stringFromVch(recipientPubKeyVch), locatorStr, encryptedAliasForRecipient);

          string randBase64 = EncodeBase64(&vchRand[0], vchRand.size());
          string encryptedRandForRecipient;
          EncryptMessage(stringFromVch(recipientPubKeyVch), randBase64, encryptedRandForRecipient);

          const CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        bool found = false;
        BOOST_FOREACH(const CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) 
            {
                if(op != OP_ALIAS_ENCRYPTED)
                  throw runtime_error("previous transaction was not OP_ALIAS_ENCRYPTED");

                 string gen;
                 string reference;
                 string iv128 = stringFromVch(vvch[6]);
                 if(iv128 != "_")
                 {
                   string r = stringFromVch(vvch[5]); 
                   State s__(stringFromVch(vvch[7])); 
                   if(s__() != State::ATOMIC)
                   {
                     string decrypted;
                     DecryptMessageAES(stringFromVch(vvch[4]),
                              decrypted,
                              aesRawVector,
                              iv128);

                     EncryptMessageAES(decrypted, gen, aesRawVector, reference);
                     if(gen.size() > MAX_XUNIT_LENGTH) 
                       throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                   }
                   else
                   {
                     Relay r;
                     if(pwalletMain->relay_(vchAlias, r))
                     {
                       string ctrl_ = r.ctrl_();
                       bool fInvalid = false;
                       aesRawVector = DecodeBase64(ctrl_.c_str(), &fInvalid);
                       string decrypted;
                       DecryptMessageAES(stringFromVch(vvch[4]),
                              decrypted,
                              aesRawVector,
                              iv128);

                       EncryptMessageAES(decrypted, gen, aesRawVector, reference);
                       if(gen.size() > MAX_XUNIT_LENGTH) 
                         throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                     }
                   }
                 }

                uint160 hash = uint160(vvch[3]);

                CDataStream ss(SER_GETHASH, 0);
                ss << encryptedAliasForRecipient;
                ss << hash.ToString();

                vchType q1;
                if(iv128 == "_")
                {
                  ss << stringFromVch(vvch[4]);
                  q1 = vvch[4]; 
                }
                else
                {
                  ss << gen;
                  q1 = vchFromString(gen); 
                }

                ss << encryptedRandForRecipient;

                vchType fs = vchFromString(localAddr.ToString());

                CScript script;
                script.SetBitcoinAddress(stringFromVch(vvch[2]));

                CBitcoinAddress ownerAddr = script.GetBitcoinAddress();
                if(!ownerAddr.IsValid())
                  throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

                CKeyID keyID;
                if(!ownerAddr.GetKeyID(keyID))
                  throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");

                CKey key;
                if(!pwalletMain->GetKey(keyID, key))
                  throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

                vector<unsigned char> vchSig;
                if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                  throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

                string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
              scriptPubKey << OP_ALIAS_ENCRYPTED << vchFromString(encryptedAliasForRecipient) << vchFromString(sigBase64) << rVch << vvch[3] << q1 << vchFromString(encryptedRandForRecipient) << vchFromString(iv128) << fs << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;

              scriptPubKey += scriptPubKeyOrig;
              found = true;
              break;
            }
        }

          if(!found)
          {
            throw runtime_error("previous tx type is not alias");
          }

          string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);

          if(strError != "")
          {
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
         }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    return wtx.GetHash().GetHex();
}
Value transferAlias(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
          "transferAlias <alias> <toaddress>"
          + HelpRequiringPassphrase());

    vchType vchAlias = vchFromValue(params[0]);
    const vchType vchAddress = vchFromValue(params[1]);

    string locatorStr = stringFromVch(vchAlias);
    string addressStr = stringFromVch(vchAddress);


    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    string strAddress = params[1].get_str();
    CBitcoinAddress address(strAddress);
    if(!address.IsValid())
    {
      vector<AliasIndex> vtxPos;
      LocatorNodeDB ln1Db("r");
      vchType vchAlias = vchFromString(strAddress);
      if (ln1Db.lKey (vchAlias))
      {
        if (!ln1Db.lGet (vchAlias, vtxPos))
          return error("aliasHeight() : failed to read from name DB");
        if (vtxPos.empty ())
          return -1;

        AliasIndex& txPos = vtxPos.back ();
        address.SetString(txPos.vAddress); 
      }
      else
      {
          throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias");
      }
    }


    scriptPubKeyOrig.SetBitcoinAddress(address.ToString());

    CScript scriptPubKey;

    scriptPubKey << OP_ALIAS_RELAY << vchAlias ;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
          if(mapState.count(vchAlias) && mapState[vchAlias].size())
          {
              error("updateEncryptedAlias() : there are %lu pending operations on that alias, including %s",
                      mapState[vchAlias].size(),
                      mapState[vchAlias].begin()->GetHex().c_str());
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    LEAVE_CRITICAL_SECTION(cs_main)
              throw runtime_error("there are pending operations on that alias");
          }

          EnsureWalletIsUnlocked();

          LocatorNodeDB aliasCacheDB("r");
          CTransaction tx;
          if(!aliasTx(aliasCacheDB, vchAlias, tx))
          {
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("could not find a coin with this alias");
          }

          uint256 wtxInHash = tx.GetHash();

          if(!pwalletMain->mapWallet.count(wtxInHash))
          {
              error("updateEncryptedAlias() : this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
              LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
              LEAVE_CRITICAL_SECTION(cs_main)
              throw runtime_error("this coin is not in your wallet");
          }

          const CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
          int op__;
          int nOut;
          vchType vchValue;
          wtxIn.aliasSet(op__, nOut, vchAlias, vchValue);

          scriptPubKey << vchValue << OP_2DROP << OP_DROP;
          scriptPubKey += scriptPubKeyOrig;

          string locatorStr = stringFromVch(vchAlias);
          string dataStr = stringFromVch(vchValue);
          string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);
          if(strError != "")
          {
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
         }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    return wtx.GetHash().GetHex();
}
Value uC(const Array& params, bool fHelp) { if(fHelp || params.size() != 2)
        throw runtime_error(
                "uC <url> <value>"
                + HelpRequiringPassphrase());
    string locatorStr = params[0].get_str();

    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    const vchType vchAlias = vchFromValue(locatorStr);
    const char* locatorFile = (params[1].get_str()).c_str();

    fs::path p = locatorFile;
    if(!fs::exists(p))
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Locator file does not exist");

    vchType l = vchFromString(locatorStr);
    Relay r;
    if(pwalletMain->relay_(l, r))
    {
      string ctrl_ = r.ctrl_();
    }
    else
    {
      vchType kAlpha;
      GenerateAESKey(kAlpha);

      string alpha = EncodeBase64(&kAlpha[0], kAlpha.size());

      r.ctrl(alpha);

      pwalletMain->relay(l, r);

      CWalletDB walletdb(pwalletMain->strWalletFile, "r+");
 
      pwalletMain->LoadRelay(vchAlias, r); 
      if(!walletdb.UpdateKey(l, pwalletMain->lCache[l]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write data for key");
    }

    ifstream file(locatorFile, ios_base::in | ios_base::binary);
    filtering_streambuf<input> in;
    in.push(gzip_compressor());
    in.push(file);

    stringstream ss;
    boost::iostreams::copy(in, ss);
    string s = ss.str();

    string gen;
    string reference;
    
    string idx = r.ctrl_();
    bool fInvalid = false;
    vector<unsigned char> v = DecodeBase64(idx.c_str(), &fInvalid);

    EncryptMessageAES(s, gen, v, reference);
    if(gen.size() > MAX_XUNIT_LENGTH) 
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
   
    const vchType vchValue = vchFromValue(gen);

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
            error("updateEncryptedAlias() : there are %lu pending operations on that alias, including %s",
                    mapState[vchAlias].size(),
                    mapState[vchAlias].begin()->GetHex().c_str());
          LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchAlias, tx))
        {
            error("updateEncryptedAlias() : this alias is already active with tx %s",
                    tx.GetHash().GetHex().c_str());
            throw runtime_error("this alias is already active");
        }
    }

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchAliasEncrypted2(stringFromVch(vchAlias), wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
          throw runtime_error("could not find a coin with this alias, try specifying the registerAlias transaction id");
        }


        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
    LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("previous transaction is not in the wallet");
        }

        CScript scriptPubKeyOrig;

        CScript scriptPubKey;


        CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) {
                if(op != OP_ALIAS_ENCRYPTED)
                  throw runtime_error("previous transaction was not an OP_ALIAS_ENCRYPTED");

              string encrypted = stringFromVch(vvch[0]);
              uint160 hash = uint160(vvch[3]);
              string value = stringFromVch(vchValue);

              CDataStream ss(SER_GETHASH, 0);
              ss << encrypted;
              ss << hash.ToString();
              ss << value;
              ss << string("0");

              CScript script;
              script.SetBitcoinAddress(stringFromVch(vvch[2]));

              CBitcoinAddress ownerAddr = script.GetBitcoinAddress();
              if(!ownerAddr.IsValid())
                throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

              CKeyID keyID;
              if(!ownerAddr.GetKeyID(keyID))
                throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");

              CKey key;
              if(!pwalletMain->GetKey(keyID, key))
                throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

              CPubKey vchPubKey;
              pwalletMain->GetPubKey(keyID, vchPubKey);

              vector<unsigned char> vchSig;
              if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                  throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

              string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

              scriptPubKey << OP_ALIAS_ENCRYPTED << vvch[0] << vchFromString(sigBase64) << vvch[2] << vvch[3] << vchValue << vchFromString("0") << vchFromString(reference) << vchFromString(State::ATOMIC) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
              scriptPubKeyOrig.SetBitcoinAddress(stringFromVch(vvch[2]));
              scriptPubKey += scriptPubKeyOrig;
              found = true;
            }
        }

        if(!found)
        {
            throw runtime_error("previous tx on this alias is not a alias tx");
        }

        string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);
        if(strError != "")
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return wtx.GetHash().GetHex();
}
Value transientStatus__(const Array& params, bool fHelp)
{
    if(fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
                "transientStatus__ <locator> <file>"
                + HelpRequiringPassphrase());
    Object ret;
    string locatorStr = params[0].get_str();

    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    const vchType vchAlias = vchFromValue(locatorStr);
    const char* locatorFile = (params[1].get_str()).c_str();
    fs::path p = locatorFile;
    if(!fs::exists(p))
    {
      ret.push_back(Pair("status", "error"));
      ret.push_back(Pair("message", "Locator file does not exist"));
      return ret;
    }

    ifstream file(locatorFile, ios_base::in | ios_base::binary);
    filtering_streambuf<input> in;
    in.push(gzip_compressor());
    in.push(file);

    stringstream ss;
    boost::iostreams::copy(in, ss);

    string s = ss.str();
    if(s.size() > MAX_XUNIT_LENGTH) 
    {
      ret.push_back(Pair("status", "error"));
      ret.push_back(Pair("message", "Locator file compressed size too large"));
      return ret;
    }
   
    const vchType vchValue = vchFromValue(s);


    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    CScript scriptPubKey;
    scriptPubKey << OP_ALIAS_RELAY << vchAlias << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
          if(mapState.count(vchAlias) && mapState[vchAlias].size())
          {
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            ret.push_back(Pair("status", "error"));
            ret.push_back(Pair("message", "pending ops on that alias"));
            return ret;
          }

          EnsureWalletIsUnlocked();

          LocatorNodeDB aliasCacheDB("r");
          CTransaction tx;
          if(!aliasTx(aliasCacheDB, vchAlias, tx))
          {
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            ret.push_back(Pair("status", "error"));
            ret.push_back(Pair("message", "could not find that alias"));
            return ret;
          }

          if(params.size() == 2)
          {
            string strAddress = "";
            aliasAddress(tx, strAddress);
            if(strAddress == "")
            {
              ret.push_back(Pair("status", "error"));
              ret.push_back(Pair("message", "no associated address"));
              return ret;
            }

            uint160 hash160;
            bool isValid = AddressToHash160(strAddress, hash160);
            if(!isValid)
            {
              ret.push_back(Pair("status", "error"));
              ret.push_back(Pair("message", "invalid dions address"));
              return ret;
            }

            scriptPubKeyOrig.SetBitcoinAddress(strAddress);
          }
    
          uint256 wtxInHash = tx.GetHash();

          if(!pwalletMain->mapWallet.count(wtxInHash))
          {
              ret.push_back(Pair("status", "error"));
              ret.push_back(Pair("message", "coin is not in your wallet"));
              return ret;
          }

          CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
          scriptPubKey += scriptPubKeyOrig;
          int64_t t;
          string strError;
          bool s = txRelayPre__(scriptPubKey, wtxIn, wtx, t, strError);
          if(!s)
          {
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            ret.push_back(Pair("status", "error"));
            ret.push_back(Pair("message", strError));
            return ret;
         }
        ret.push_back(Pair("status", "ok"));
        ret.push_back(Pair("fee", ValueFromAmount(t)));
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return ret;
}
Value updateAliasFile(const Array& params, bool fHelp)
{
    if(fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
                "updateAlias <alias> <value> [<toaddress>] update or transfer"
                + HelpRequiringPassphrase());
    string locatorStr = params[0].get_str();

    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    const vchType vchAlias = vchFromValue(locatorStr);
    const char* locatorFile = (params[1].get_str()).c_str();
    fs::path p = locatorFile;
    if(!fs::exists(p))
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Locator file does not exist");

    ifstream file(locatorFile, ios_base::in | ios_base::binary);
    filtering_streambuf<input> in;
    in.push(gzip_compressor());
    in.push(file);

    stringstream ss;
    boost::iostreams::copy(in, ss);

    string s = ss.str();
    if(s.size() > MAX_XUNIT_LENGTH) 
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Locator file too large");
   
    const vchType vchValue = vchFromValue(s);


    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    if(params.size() == 3)
    {
      string strAddress = params[2].get_str();
      uint160 hash160;
      bool isValid = AddressToHash160(strAddress, hash160);
      if(!isValid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
      scriptPubKeyOrig.SetBitcoinAddress(strAddress);
    }

    CScript scriptPubKey;
    scriptPubKey << OP_ALIAS_RELAY << vchAlias << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
          if(mapState.count(vchAlias) && mapState[vchAlias].size())
          {
            error("updateAlias() : there are %lu pending operations on that alias, including %s",
                      mapState[vchAlias].size(),
                      mapState[vchAlias].begin()->GetHex().c_str());
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
          }

          EnsureWalletIsUnlocked();

          LocatorNodeDB aliasCacheDB("r");
          CTransaction tx;
          if(!aliasTx(aliasCacheDB, vchAlias, tx))
          {
              LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
              LEAVE_CRITICAL_SECTION(cs_main)
              throw runtime_error("could not find a coin with this alias");
          }

          if(params.size() == 2)
          {
            string strAddress = "";
            aliasAddress(tx, strAddress);
            if(strAddress == "")
              throw runtime_error("alias has no associated address");

            uint160 hash160;
            bool isValid = AddressToHash160(strAddress, hash160);
            if(!isValid)
              throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
            scriptPubKeyOrig.SetBitcoinAddress(strAddress);
          }
    
          uint256 wtxInHash = tx.GetHash();

          if(!pwalletMain->mapWallet.count(wtxInHash))
          {
            error("updateAlias() : this coin is not in your wallet %s",
                    wtxInHash.GetHex().c_str());
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
              throw runtime_error("this coin is not in your wallet");
          }

          CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
          scriptPubKey += scriptPubKeyOrig;
          string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);
          if(strError != "")
          {
            LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
            LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
         }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    return wtx.GetHash().GetHex();
}

Value updateAlias(const Array& params, bool fHelp)
{
    if(fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
                "updateAlias <alias> <value> [<toaddress>]\nUpdate and possibly transfer a alias"
                + HelpRequiringPassphrase());
    string locatorStr = params[0].get_str();
    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    const vchType vchAlias = vchFromValue(locatorStr);
    const vchType vchValue = vchFromValue(params[1]);

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    if(params.size() == 3)
    {
        string strAddress = params[2].get_str();
        uint160 hash160;
        bool isValid = AddressToHash160(strAddress, hash160);
        if(!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress(strAddress);
    }

    CScript scriptPubKey;
    scriptPubKey << OP_ALIAS_RELAY << vchAlias << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
      ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
      {
        if(mapState.count(vchAlias) && mapState[vchAlias].size())
        {
          error("updateAlias() : there are %lu pending operations on that alias, including %s",
            mapState[vchAlias].size(),
            mapState[vchAlias].begin()->GetHex().c_str());
          LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
          LEAVE_CRITICAL_SECTION(cs_main)
          throw runtime_error("there are pending operations on that alias");
        }

        EnsureWalletIsUnlocked();

        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(!aliasTx(aliasCacheDB, vchAlias, tx))
        {
          LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
          LEAVE_CRITICAL_SECTION(cs_main)
          throw runtime_error("could not find a coin with this alias");
        }

        uint256 wtxInHash = tx.GetHash();

        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
          error("updateAlias() : this coin is not in your wallet %s",
                  wtxInHash.GetHex().c_str());
          LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
          LEAVE_CRITICAL_SECTION(cs_main)
          throw runtime_error("this coin is not in your wallet");
        }

        if(params.size() == 2)
        {
          string strAddress = "";
          aliasAddress(tx, strAddress);
          if(strAddress == "")
            throw runtime_error("alias has no associated address");

          uint160 hash160;
          bool isValid = AddressToHash160(strAddress, hash160);
          if(!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
          scriptPubKeyOrig.SetBitcoinAddress(strAddress);
        }

        CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        scriptPubKey += scriptPubKeyOrig;
        string strError = txRelay(scriptPubKey, CTRL__, wtxIn, wtx, false);
        if(strError != "")
        {
          LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
          LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
         }
      }
      LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    return wtx.GetHash().GetHex();
}


Value publicKey(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
                "newPlublicKey <public_address>"
                + HelpRequiringPassphrase());

  EnsureWalletIsUnlocked();

  CWalletDB walletdb(pwalletMain->strWalletFile, "r+");

  string myAddress = params[0].get_str();

  CBitcoinAddress addr(myAddress);
  if(!addr.IsValid())
    throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

  CKeyID keyID;
  if(!addr.GetKeyID(keyID))
    throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

  CKey key;
  if(!pwalletMain->GetKey(keyID, key))
    throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

  CPubKey pubKey = key.GetPubKey();

  string testKey;

  if(!pwalletMain->SetRSAMetadata(pubKey))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

  if(!walletdb.UpdateKey(pubKey, pwalletMain->mapKeyMetadata[pubKey.GetID()]))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

  if(!pwalletMain->envCP1(pubKey, testKey))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

  string pKey;
  if(!pwalletMain->envCP0(pubKey, pKey))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

  vector<Value> res;
  res.push_back(myAddress);
  res.push_back(pKey);
  res.push_back(testKey);
  return res;
}

Value sendSymmetric(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
                "sendSymmetric <sender> <recipient>"
                + HelpRequiringPassphrase());

    EnsureWalletIsUnlocked();

    string myAddress = params[0].get_str();
    string f = params[1].get_str();

    CBitcoinAddress addr;
    int r = checkAddress(myAddress, addr);
    if(r<0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if(!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CBitcoinAddress aRecipient;
    r = checkAddress(f, aRecipient);
    if(r < 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid recipient address");

    f = aRecipient.ToString();

    string rsaPubKeyStr = "";
    if(!pwalletMain->envCP1(key.GetPubKey(), rsaPubKeyStr))
        throw JSONRPCError(RPC_WALLET_ERROR, "no rsa key available for address");

    CDataStream ss(SER_GETHASH, 0);
    ss << rsaPubKeyStr;

    vector<unsigned char> vchSig;
    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    const vchType vchSender = vchFromValue(myAddress);
    const vchType vchRecipient = vchFromValue(f);

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;


        uint160 hash160;
        bool isValid = AddressToHash160(f, hash160);
        if(!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress(f);

    const vchType vchKey = vchFromValue(rsaPubKeyStr);
    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    scriptPubKey << OP_PUBLIC_KEY << vchSender << vchRecipient << vchKey << vchFromString(sigBase64) << OP_2DROP << OP_2DROP << OP_DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

       string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, wtx, false);

        if(strError != "")
        {
    LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }

    }
    LEAVE_CRITICAL_SECTION(cs_main)



    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    res.push_back(sigBase64);
    return res;

}
bool internalReference__(string ref__, vchType& recipientPubKeyVch)
{
  bool s__=false;
  ENTER_CRITICAL_SECTION(cs_main)
  {
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
      CBitcoinAddress a(ref__) ;
      CKeyID keyID;
      if(a.GetKeyID(keyID))
      {
        CKey key;
        if(pwalletMain->GetKey(keyID, key))
        {
          CPubKey pubKey = key.GetPubKey();

          string r1__;
          if(pwalletMain->envCP1(pubKey, r1__))
          {
            recipientPubKeyVch = vchFromString(r1__);
            s__=true;
          }
        }
      }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  }
  LEAVE_CRITICAL_SECTION(cs_main)

  return s__;
}
bool getImportedPubKey(string fKey, vchType& recipientPubKeyVch)
{
  ENTER_CRITICAL_SECTION(cs_main)
  {
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
      BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
      {
        const CWalletTx& tx = item.second;

        vchType vchSender, vchRecipient, vchKey, vchAes, vchSig;
        int nOut;
        if(!tx.GetPublicKeyUpdate(nOut, vchSender, vchRecipient, vchKey, vchAes, vchSig))
          continue;

        string senderAddr = stringFromVch(vchSender);
        if(senderAddr == fKey)
        {
          recipientPubKeyVch = vchKey;
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  LEAVE_CRITICAL_SECTION(cs_main)
          return true;
        }
      }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  }
  LEAVE_CRITICAL_SECTION(cs_main)

  return false;
}

bool getImportedPubKey(string myAddress, string fKey, vchType& recipientPubKeyVch, vchType& aesKeyBase64EncryptedVch)
{
  ENTER_CRITICAL_SECTION(cs_main)
  {
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
      BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                      pwalletMain->mapWallet)
      {
        const CWalletTx& tx = item.second;

        vchType vchSender, vchRecipient, vchKey, vchAes, vchSig;
        int nOut;
        if(!tx.GetPublicKeyUpdate(nOut, vchSender, vchRecipient, vchKey, vchAes, vchSig))
          continue;

        string keyRecipientAddr = stringFromVch(vchRecipient);
        if(keyRecipientAddr == myAddress)
        {
          recipientPubKeyVch = vchKey;
          aesKeyBase64EncryptedVch = vchAes;
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  LEAVE_CRITICAL_SECTION(cs_main)
          return true;
        }
      }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
  }
  LEAVE_CRITICAL_SECTION(cs_main)

  return false;
}

int checkAddress(string addr, CBitcoinAddress& a)
{
  CBitcoinAddress address(addr);
  if(!address.IsValid())
  {
    vector<AliasIndex> vtxPos;
    LocatorNodeDB ln1Db("r");
    std::transform(addr.begin(), addr.end(), addr.begin(), ::tolower);
    vchType vchAlias = vchFromString(addr);
    if(ln1Db.lKey(vchAlias))
    {
      if(!ln1Db.lGet(vchAlias, vtxPos))
        return -2;
      if(vtxPos.empty())
        return -3;

      AliasIndex& txPos = vtxPos.back();
      address.SetString(txPos.vAddress); 
      if(!address.IsValid())
        return -4;
    }
    else
    {
      return -1;
    }
  }
  
  a = address;
  return 0;
}

Value sendPublicKey(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
                "sendPublicKey <addr> <addr>"
                + HelpRequiringPassphrase());

    EnsureWalletIsUnlocked();

    string myAddress = params[0].get_str();
    string f = params[1].get_str();

    CBitcoinAddress addr;
    int r = checkAddress(myAddress, addr);
    if(r < 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CBitcoinAddress aRecipient;
    r = checkAddress(f, aRecipient);
    if(r < 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid recipient address");

    f = aRecipient.ToString();

    CKeyID keyID;
    if(!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);

    string rsaPubKeyStr = "";
    if(!pwalletMain->envCP1(key.GetPubKey(), rsaPubKeyStr))
        throw JSONRPCError(RPC_WALLET_ERROR, "no rsa key available for address");

    const vchType vchKey = vchFromValue(rsaPubKeyStr);
    const vchType vchSender = vchFromValue(myAddress);
    const vchType vchRecipient = vchFromValue(f);
    vector<unsigned char> vchSig;
    CDataStream ss(SER_GETHASH, 0);

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;

    uint160 hash160;
    bool isValid = AddressToHash160(f, hash160);
    if(!isValid)
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");

    vchType recipientPubKeyVch;
    vchType recipientAESKeyVch;
    vchType aes256Key;

    string sigBase64;
    string encrypted;
    if(getImportedPubKey(myAddress, f, recipientPubKeyVch, recipientAESKeyVch))
    {
      GenerateAESKey(aes256Key);

      string aesKeyStr = EncodeBase64(&aes256Key[0], aes256Key.size());

      const string publicKeyStr = stringFromVch(recipientPubKeyVch);
      EncryptMessage(publicKeyStr, aesKeyStr, encrypted);

      CWalletDB walletdb(pwalletMain->strWalletFile, "r+");
      if(!pwalletMain->aes(vchPubKey, f, aesKeyStr))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to set meta data for key");

      if(!walletdb.UpdateKey(vchPubKey, pwalletMain->mapKeyMetadata[vchPubKey.GetID()]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

      ss <<(rsaPubKeyStr + encrypted);
      if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");
      sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
      scriptPubKey << OP_PUBLIC_KEY << vchSender << vchRecipient << vchKey
                   << vchFromString(encrypted)
                   << vchFromString(sigBase64)
                   << OP_2DROP << OP_2DROP << OP_2DROP;
    }
    else
    {
      ss << rsaPubKeyStr + "I";
      if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");
      sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
      scriptPubKey << OP_PUBLIC_KEY << vchSender << vchRecipient << vchKey
                   << vchFromString("I")
                   << vchFromString(sigBase64)
                   << OP_2DROP << OP_2DROP << OP_2DROP;
    }


    scriptPubKeyOrig.SetBitcoinAddress(f);

    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

       string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, wtx, false);

        if(strError != "")
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }

    }
    LEAVE_CRITICAL_SECTION(cs_main)



    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    res.push_back(sigBase64);
    return res;

}
Value sendPlainMessage(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 3)
        throw runtime_error(
                "sendPlainMessage <sender_address> <message> <recipient_address>"
                + HelpRequiringPassphrase());

    const string myAddress = params[0].get_str();
    const string strMessage = params[1].get_str();
    const string f = params[2].get_str();

    CBitcoinAddress senderAddr(myAddress);
    if(!senderAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address");

    CKeyID keyID;
    if(!senderAddr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "senderAddr does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CBitcoinAddress recipientAddr(f);
    if(!recipientAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid recipient address");

    CKeyID rkeyID;
    if(!recipientAddr.GetKeyID(rkeyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "recipientAddr does not refer to key");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessage;

    vector<unsigned char> vchSig;
    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;


        uint160 hash160;
        bool isValid = AddressToHash160(f, hash160);
        if(!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress(f);

    vchType vchMessage = vchFromString(strMessage);
    scriptPubKey << OP_MESSAGE << vchFromString(myAddress) << vchFromString(f) << vchMessage << vchFromString(sigBase64) << OP_2DROP << OP_2DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

       string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, wtx, false);

        if(strError != "")
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        mapMyMessages[vchMessage] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)



    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    return res;

}
Value sendMessage(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 3)
        throw runtime_error(
                "sendMessage <addr> <message> <addr>"
                + HelpRequiringPassphrase());

    string myAddress = params[0].get_str();
    string strMessage = params[1].get_str();
    string f = params[2].get_str();

    CBitcoinAddress recipientAddr(f);
    if(!recipientAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid recipient address");

    CKeyID rkeyID;
    if(!recipientAddr.GetKeyID(rkeyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "recipientAddr does not refer to key");

    vchType recipientAddressVch = vchFromString(f);
    vchType recipientPubKeyVch;
    vector<unsigned char> aesRawVector;

    CKey key;
    if(params.size() == 3)
    {
      CBitcoinAddress senderAddr(myAddress);
      if(!senderAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address");

      CKeyID keyID;
      if(!senderAddr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "senderAddr does not refer to key");

      if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

      CPubKey vchPubKey;
      pwalletMain->GetPubKey(keyID, vchPubKey);

      string aesBase64Plain;
      if(pwalletMain->aes_(vchPubKey, f, aesBase64Plain))
      {
        bool fInvalid = false;
        aesRawVector = DecodeBase64(aesBase64Plain.c_str(), &fInvalid);
      }
      else
      {
        vchType aesKeyBase64EncryptedVch;
        if(getImportedPubKey(myAddress, f, recipientPubKeyVch, aesKeyBase64EncryptedVch))
        {
          string aesKeyBase64Encrypted = stringFromVch(aesKeyBase64EncryptedVch);
          string privRSAKey;
          if(!pwalletMain->envCP0(vchPubKey, privRSAKey))
            throw JSONRPCError(RPC_TYPE_ERROR, "Failed to retrieve private RSA key");
          string decryptedAESKeyBase64;
          DecryptMessage(privRSAKey, aesKeyBase64Encrypted, decryptedAESKeyBase64);
          bool fInvalid = false;
          aesRawVector = DecodeBase64(decryptedAESKeyBase64.c_str(), &fInvalid);
        }
        else
        {
          throw JSONRPCError(RPC_WALLET_ERROR, "No local symmetric key and no imported symmetric key found for recipient");
        }
      }
    }

    string encrypted;
    string iv128Base64;
    EncryptMessageAES(strMessage, encrypted, aesRawVector, iv128Base64);

    CDataStream ss(SER_GETHASH, 0);
    ss << encrypted + iv128Base64;

    vector<unsigned char> vchSig;
    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;

    uint160 hash160;
    bool isValid = AddressToHash160(f, hash160);
    if(!isValid)
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
    scriptPubKeyOrig.SetBitcoinAddress(f);

    vchType vchEncryptedMessage = vchFromString(encrypted);
    vchType iv128Base64Vch = vchFromString(iv128Base64);
    scriptPubKey << OP_ENCRYPTED_MESSAGE << vchFromString(myAddress) << vchFromString(f) << vchEncryptedMessage << iv128Base64Vch << vchFromString(sigBase64) << OP_2DROP << OP_2DROP << OP_DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

       string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, wtx, false);

        if(strError != "")
        {
    LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        mapMyMessages[vchEncryptedMessage] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)



    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    return res;

}
bool sign_verifymessage(string address)
{
  string message = "test";
    CBitcoinAddress ownerAddr(address);
    if(!ownerAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

    CKeyID keyID;
    if(!ownerAddr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CDataStream ss(SER_GETHASH, 0);
    ss << message;

    vector<unsigned char> vchSig;
    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    CBitcoinAddress addr(address);
    if(!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID__;
    if(!addr.GetKeyID(keyID__))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    bool fInvalid = false;
    vector<unsigned char> vchSig__ = DecodeBase64(sigBase64.c_str(), &fInvalid);

    if(fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CDataStream ss__(SER_GETHASH, 0);
    ss__ << message;

    CKey key__;
    if(!key__.SetCompactSignature(Hash(ss__.begin(), ss__.end()), vchSig__))
        return false;

    return(key__.GetPubKey().GetID() == keyID__);
}
Value registerAliasGenerate(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
                "registerAliasGenerate <alias>"
                + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();

    locatorStr = stripSpacesAndQuotes(locatorStr);

    if(isOnlyWhiteSpace(locatorStr))
    {
      string err = "Attempt to register alias consisting only of white space";

      throw JSONRPCError(RPC_WALLET_ERROR, err);
    }
    else if(locatorStr.size() > 255)
    {
      string err = "Attempt to register alias more than 255 chars";

      throw JSONRPCError(RPC_WALLET_ERROR, err);
    }

    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    uint256 wtxInHash__;
    if(searchAliasEncrypted2(locatorStr, wtxInHash__) == true)
    {
      string err = "Attempt to register alias : " + locatorStr + ", this alias is already registered as encrypted with tx " + wtxInHash__.GetHex();

      throw JSONRPCError(RPC_WALLET_ERROR, err);
    }

    vector<Value> res;
    LocatorNodeDB aliasCacheDB("r");
    CTransaction tx;
    if(aliasTx(aliasCacheDB, vchFromString(locatorStr), tx))
    {
      if(IsMinePost(tx))
      {
        string err = "Attempt to register alias : " + locatorStr + ", this alias is already active with tx " + tx.GetHash().GetHex();

        throw JSONRPCError(RPC_WALLET_ERROR, err);
      }
      else
      {
        res.push_back("commit");
      }
    }

    const uint64_t rand = GetRand((uint64_t)-1);
    const vchType vchRand = CBigNum(rand).getvch();
    vchType vchToHash(vchRand);

    const vchType vchAlias = vchFromValue(locatorStr);
    vchToHash.insert(vchToHash.end(), vchAlias.begin(), vchAlias.end());
    const uint160 hash = Hash160(vchToHash);

    CPubKey vchPubKey;
    CReserveKey reservekey(pwalletMain);
    if(!reservekey.GetReservedKey(vchPubKey))
    {
      return false;
    }

    reservekey.KeepKey();

    CBitcoinAddress keyAddress(vchPubKey.GetID());
    CKeyID keyID;
    keyAddress.GetKeyID(keyID);
    pwalletMain->SetAddressBookName(keyID, "");

  CWalletDB walletdb(pwalletMain->strWalletFile, "r+");
  if(!pwalletMain->SetRSAMetadata(vchPubKey))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

  if(!walletdb.UpdateKey(vchPubKey, pwalletMain->mapKeyMetadata[vchPubKey.GetID()]))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

  string pKey;
  if(!pwalletMain->envCP0(vchPubKey, pKey))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

  string pub_k;
  if(!pwalletMain->envCP1(vchPubKey, pub_k))
    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address has no associated RSA keys");

  if(!pwalletMain->SetRandomKeyMetadata(vchPubKey, vchRand))
    throw JSONRPCError(RPC_WALLET_ERROR, "Failed to set meta data for key");

  if(!walletdb.UpdateKey(vchPubKey, pwalletMain->mapKeyMetadata[vchPubKey.GetID()]))
    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

  CKey key;
  if(!pwalletMain->GetKey(keyID, key))
    throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

  string encrypted;
  EncryptMessage(pub_k, locatorStr, encrypted);

  CWalletTx wtx;
  wtx.nVersion = CTransaction::DION_TX_VERSION;

  CScript scriptPubKeyOrig;
  scriptPubKeyOrig.SetBitcoinAddress(vchPubKey.Raw());
  CScript scriptPubKey;
  vchType vchEncryptedAlias = vchFromString(encrypted);
  string tmp = stringFromVch(vchEncryptedAlias);
  vchType vchValue = vchFromString("");

  CDataStream ss(SER_GETHASH, 0);
  ss << encrypted;
  ss << hash.ToString();
  ss << stringFromVch(vchValue);
  ss << string("0");

    vector<unsigned char> vchSig;
    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    scriptPubKey << OP_ALIAS_ENCRYPTED << vchEncryptedAlias << vchFromString(sigBase64) << vchFromString(keyAddress.ToString()) << hash << vchValue << vchFromString("0") << vchFromString("_") << vchFromString(State::GROUND) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

       string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, wtx, false);

        if(strError != "")
        {
          LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        mapLocator[vchAlias] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    res.push_back(wtx.GetHash().GetHex());
    return res;
}
Value registerAlias(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
                "registerAlias <alias> <address>"
                + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();

    locatorStr = stripSpacesAndQuotes(locatorStr);
    if(isOnlyWhiteSpace(locatorStr))
    {
      string err = "Attempt to register alias consisting only of white space";

      throw JSONRPCError(RPC_WALLET_ERROR, err);
    }
    else if(locatorStr.size() > 255)
    {
      string err = "Attempt to register alias more than 255 chars";

      throw JSONRPCError(RPC_WALLET_ERROR, err);
    }

    std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
    uint256 wtxInHash__;
    if(searchAliasEncrypted2(locatorStr, wtxInHash__) == true)
    {
      string err = "Attempt to register alias : " + locatorStr + ", this alias is already registered as encrypted with tx " + wtxInHash__.GetHex();

      throw JSONRPCError(RPC_WALLET_ERROR, err);
    }

    LocatorNodeDB aliasCacheDB("r");
    CTransaction tx;
    if(aliasTx(aliasCacheDB, vchFromString(locatorStr), tx))
    {
      string err = "Attempt to register alias : " + locatorStr + ", this alias is already active with tx " + tx.GetHash().GetHex();

      throw JSONRPCError(RPC_WALLET_ERROR, err);
    }

    const std::string strAddress = params[1].get_str();

    const uint64_t rand = GetRand((uint64_t)-1);
    const vchType vchRand = CBigNum(rand).getvch();
    vchType vchToHash(vchRand);

    const vchType vchAlias = vchFromValue(locatorStr);
    vchToHash.insert(vchToHash.end(), vchAlias.begin(), vchAlias.end());
    const uint160 hash = Hash160(vchToHash);

    uint160 hash160;
    bool isValid = AddressToHash160(strAddress, hash160);
    if(!isValid)
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
        "Invalid dions address");

    CBitcoinAddress keyAddress(strAddress);
    CKeyID keyID;
    keyAddress.GetKeyID(keyID);
    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);
    string pub_k;
    if(!pwalletMain->envCP1(vchPubKey, pub_k))
      throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address has no associated RSA keys");

    if(!pwalletMain->SetRandomKeyMetadata(vchPubKey, vchRand))
      throw JSONRPCError(RPC_WALLET_ERROR, "Failed to set meta data for key");

    CWalletDB walletdb(pwalletMain->strWalletFile, "r+");

    if(!walletdb.UpdateKey(vchPubKey, pwalletMain->mapKeyMetadata[vchPubKey.GetID()]))
      throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");


    string encrypted;
    EncryptMessage(pub_k, locatorStr, encrypted);


    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    scriptPubKeyOrig.SetBitcoinAddress(vchPubKey.Raw());
    CScript scriptPubKey;
    vchType vchEncryptedAlias = vchFromString(encrypted);
    string tmp = stringFromVch(vchEncryptedAlias);

    vchType vchValue = vchFromString("");

    CDataStream ss(SER_GETHASH, 0);
    ss << encrypted;
    ss << hash.ToString();
    ss << stringFromVch(vchValue);
    ss << string("0");

    vector<unsigned char> vchSig;
    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
    scriptPubKey << OP_ALIAS_ENCRYPTED << vchEncryptedAlias << vchFromString(sigBase64) << vchFromString(strAddress) << hash << vchValue << vchFromString("0") << vchFromString("_") << vchFromString(State::GROUND) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

       string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, wtx, false);

        if(strError != "")
        {
    LEAVE_CRITICAL_SECTION(cs_main)
          throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        mapLocator[vchAlias] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    return res;
}
bool aliasTxPos(const vector<AliasIndex> &vtxPos, const CDiskTxPos& txPos)
{
    if(vtxPos.empty())
        return false;

    return vtxPos.back().txPos == txPos;
}
bool aliasScript(const CScript& script, int& op, vector<vector<unsigned char> > &vvch)
{
  CScript::const_iterator pc = script.begin();
  return aliasScript(script, op, vvch, pc);
}
bool aliasScript(const CScript& script, int& op, vector<vector<unsigned char> > &vvch, CScript::const_iterator& pc)
{

    opcodetype opcode;
    if(!script.GetOp(pc, opcode))
        return false;
    if(opcode < OP_1 || opcode > OP_16)
        return false;

    op = opcode - OP_1 + 1;

    for(;;) {
        vector<unsigned char> vch;
        if(!script.GetOp(pc, opcode, vch))
            return false;
        if(opcode == OP_DROP || opcode == OP_2DROP || opcode == OP_NOP)
            break;
        if(!(opcode >= 0 && opcode <= OP_PUSHDATA4))
            return false;
        vvch.push_back(vch);
    }

    while(opcode == OP_DROP || opcode == OP_2DROP || opcode == OP_NOP)
    {
        if(!script.GetOp(pc, opcode))
            break;
    }

    pc--;

    if((op == OP_ALIAS_ENCRYPTED && vvch.size() == 8) ||
       (op == OP_ALIAS_SET && vvch.size() == 5) ||
           (op == OP_MESSAGE) ||
           (op == OP_ENCRYPTED_MESSAGE) ||
           (op == OP_PUBLIC_KEY) ||
           (op == OP_ALIAS_RELAY && vvch.size() == 2))
        return true;
    return error("invalid number of arguments for alias op");
}
bool DecodeMessageTx(const CTransaction& tx, int& op, int& nOut, vector<vector<unsigned char> >& vvch )
{
    bool found = false;

        for(unsigned int i = 0; i < tx.vout.size(); i++)
        {
            const CTxOut& out = tx.vout[i];

            vector<vector<unsigned char> > vvchRead;

            if(aliasScript(out.scriptPubKey, op, vvchRead))
            {
                if(found)
                {
                    vvch.clear();
                    return false;
                }
                nOut = i;
                found = true;
                vvch = vvchRead;
            }
        }

        if(!found)
            vvch.clear();

    return found;
}
bool aliasTx(const CTransaction& tx, int& op, int& nOut, vector<vector<unsigned char> >& vvch )
{
    bool found = false;
        for(unsigned int i = 0; i < tx.vout.size(); i++)
        {
            const CTxOut& out = tx.vout[i];

            vector<vector<unsigned char> > vvchRead;

            if(aliasScript(out.scriptPubKey, op, vvchRead))
            {
                if(found)
                {
                    vvch.clear();
                    return false;
                }
                nOut = i;
                found = true;
                vvch = vvchRead;
            }
        }

        if(!found)
            vvch.clear();

    return found;
}
bool aliasTxValue(const CTransaction& tx, vector<unsigned char>& value)
{
    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    if(!aliasTx(tx, op, nOut, vvch))
        return false;

    switch(op)
    {
        case OP_ALIAS_ENCRYPTED:
            return false;
        case OP_ALIAS_SET:
            value = vvch[2];
            return true;
        case OP_ALIAS_RELAY:
            value = vvch[1];
            return true;
        default:
            return false;
    }
}
int aliasOutIndex(const CTransaction& tx)
{
    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    if(!aliasTx(tx, op, nOut, vvch))
        throw runtime_error("aliasOutIndex() : alias output not found");
    return nOut;
}
bool IsMinePost(const CTransaction& tx)
{
    if(tx.nVersion != CTransaction::DION_TX_VERSION)
        return false;

    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    if(!aliasTx(tx, op, nOut, vvch))
    {
        error("IsMinePost() : no output out script in alias tx %s\n", tx.ToString().c_str());
        return false;
    }

    const CTxOut& txout = tx.vout[nOut];
    if(IsLocator(tx, txout))
    {
      return true;
    }
    return false;
}

bool verifymessage(const string& strAddress, const string& strSig, const string& m1, const string& m2, const string& m3, const string& m4)
{
    CBitcoinAddress addr(strAddress);
    if(!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if(!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSig.c_str(), &fInvalid);

    if(fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CDataStream ss(SER_GETHASH, 0);
    ss << m1;
    ss << m2;
    ss << m3;
    ss << m4;

    CKey key;
    if(!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
        return false;

    return(key.GetPubKey().GetID() == keyID);
}
bool verifymessage(const string& strAddress, const string& strSig, const string& strMessage)
{
    CBitcoinAddress addr(strAddress);
    if(!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if(!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSig.c_str(), &fInvalid);

    if(fInvalid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessage;

    CKey key;
    if(!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
        return false;

    return(key.GetPubKey().GetID() == keyID);
}
bool IsMinePost(const CTransaction& tx, const CTxOut& txout, bool ignore_registerAlias )
{
    if(tx.nVersion != CTransaction::DION_TX_VERSION)
        return false;

    vector<vector<unsigned char> > vvch;

    int op;

    if(!aliasScript(txout.scriptPubKey, op, vvch))
        return false;

    if(ignore_registerAlias && op == OP_ALIAS_ENCRYPTED)
        return false;

    if(IsLocator(tx, txout))
    {
        return true;
    }
    return false;
}

bool
AcceptToMemoryPoolPost(const CTransaction& tx)
{
    if(tx.nVersion != CTransaction::DION_TX_VERSION)
        return true;

    if(tx.vout.size() < 1)
      return error("AcceptToMemoryPoolPost: no output in alias tx %s\n",
                    tx.GetHash().ToString().c_str());

    std::vector<vchType> vvch;

    int op;
    int nOut;

    if(!aliasTx(tx, op, nOut, vvch))
      return error("AcceptToMemoryPoolPost: no output out script in alias tx %s",
                    tx.GetHash().ToString().c_str());

    if(op != OP_ALIAS_ENCRYPTED)
    {
      ENTER_CRITICAL_SECTION(cs_main)
      {
        mapState[vvch[0]].insert(tx.GetHash());
      }
      LEAVE_CRITICAL_SECTION(cs_main)
    }

    return true;
}
void RemoveFromMemoryPoolPost(const CTransaction& tx)
{
    if(tx.nVersion != CTransaction::DION_TX_VERSION)
        return;

    if(tx.vout.size() < 1)
        return;

    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    if(!aliasTx(tx, op, nOut, vvch))
        return;

    if(op != OP_ALIAS_ENCRYPTED)
    {
        ENTER_CRITICAL_SECTION(cs_main)
        {
            std::map<std::vector<unsigned char>, std::set<uint256> >::iterator mi = mapState.find(vvch[0]);
            if(mi != mapState.end())
                mi->second.erase(tx.GetHash());
        }
        LEAVE_CRITICAL_SECTION(cs_main)
    }

    if(op == OP_PUBLIC_KEY)
    {
        vchType k;
        k.reserve(vvch[0].size() + vvch[1].size());
        k.insert(k.end(), vvch[0].begin(), vvch[0].end());
        k.insert(k.end(), vvch[1].begin(), vvch[1].end());
        ENTER_CRITICAL_SECTION(cs_main)
        {
            std::map<std::vector<unsigned char>, std::set<uint256> >::iterator mi = k1Export.find(k);
            if(mi != k1Export.end())
                mi->second.erase(tx.GetHash());
        }
        LEAVE_CRITICAL_SECTION(cs_main)
    }
}

int CheckTransactionAtRelativeDepth(CBlockIndex* pindexBlock, CTxIndex& txindex, int maxDepth)
{
    for(CBlockIndex* pindex = pindexBlock; pindex && pindexBlock->nHeight - pindex->nHeight < maxDepth; pindex = pindex->pprev)
        if(pindex->nBlockPos == txindex.pos.nBlockPos && pindex->nFile == txindex.pos.nFile)
            return pindexBlock->nHeight - pindex->nHeight;
    return -1;
}
bool
ConnectInputsPost(map<uint256, CTxIndex>& mapTestPool,
                               const CTransaction& tx,
                               vector<CTransaction>& vTxPrev,
                               vector<CTxIndex>& vTxindex,
                               CBlockIndex* pindexBlock, CDiskTxPos& txPos,
                               bool fBlock, bool fMiner)
{
    LocatorNodeDB ln1Db("r+");
    int nInput;
    bool found = false;

    int prevOp;
    std::vector<vchType> vvchPrevArgs;

    for(int i = 0; i < tx.vin.size(); i++)
    {
      const CTxOut& out = vTxPrev[i].vout[tx.vin[i].prevout.n];
      std::vector<vchType> vvchPrevArgsRead;

      if(aliasScript(out.scriptPubKey, prevOp, vvchPrevArgsRead))
      {
        if(found)
          return error("ConnectInputsPost() : multiple previous alias transactions");

        found = true;
        nInput = i;

        vvchPrevArgs = vvchPrevArgsRead;
      }
    }
    if(tx.nVersion != CTransaction::DION_TX_VERSION)
    {

        bool found= false;
        for(int i = 0; i < tx.vout.size(); i++)
        {
            const CTxOut& out = tx.vout[i];

            std::vector<vchType> vvchRead;
            int opRead;

            if(aliasScript(out.scriptPubKey, opRead, vvchRead))
                found=true;
        }

        if(found)
            return error("ConnectInputsPost() : a non-dions transaction with a dions input");
        return true;
    }

    std::vector<vchType> vvchArgs;
    int op;
    int nOut;

    if(!aliasTx(tx, op, nOut, vvchArgs))
        return error("ConnectInputsPost() : could not decode a dions tx");

    CScript s1 = tx.vout[nOut].scriptPubKey;
    const CScript& s1_ = aliasStrip(s1);
    string a1 = s1_.GetBitcoinAddress();

    int nPrevHeight;
    int nDepth;

    if(vvchArgs[0].size() > MAX_LOCATOR_LENGTH)
        return error("alias transaction with alias too long");

    switch(op)
    {
        case OP_PUBLIC_KEY:
        {
           const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
           const std::string recipient(vvchArgs[1].begin(), vvchArgs[1].end());
           const std::string pkey(vvchArgs[2].begin(), vvchArgs[2].end());
           const std::string aesEncrypted(stringFromVch(vvchArgs[3]));
           const std::string sig(stringFromVch(vvchArgs[4]));

           CBitcoinAddress addr(sender);

           if(!addr.IsValid())
             throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

           CKeyID keyID;
           if(!addr.GetKeyID(keyID))
               throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

           bool fInvalid = false;
          vector<unsigned char> vchSig = DecodeBase64(sig.c_str(), &fInvalid);

           if(fInvalid)
               throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

           CDataStream ss(SER_GETHASH, 0);
           ss << pkey + aesEncrypted;

           CKey key;
           if(!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
               return false;

           if(key.GetPubKey().GetID() != keyID)
           {
                return error("public key plus aes key tx verification failed");
           }

             vchType k1Base;
             k1Base.reserve(vvchArgs[0].size() + vvchArgs[1].size());
             k1Base.insert(k1Base.end(), vvchArgs[0].begin(), vvchArgs[0].end());
             k1Base.insert(k1Base.end(), vvchArgs[1].begin(), vvchArgs[1].end());
             std::map<std::vector<unsigned char>, std::set<uint256> >::iterator mi = k1Export.find(k1Base);
             if(mi == k1Export.end())
             {
               ENTER_CRITICAL_SECTION(cs_main)
               {
                 k1Export[k1Base].insert(tx.GetHash());
               }
               LEAVE_CRITICAL_SECTION(cs_main)
             }
        }
            break;
        case OP_ENCRYPTED_MESSAGE:
        {
           const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
           const std::string recipient(vvchArgs[1].begin(), vvchArgs[1].end());
           const std::string encrypted(vvchArgs[2].begin(), vvchArgs[2].end());
           const std::string iv128Base64(stringFromVch(vvchArgs[3]));
           const std::string sig(stringFromVch(vvchArgs[4]));

           CBitcoinAddress addr(sender);

           if(!addr.IsValid())
             throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

           CKeyID keyID;
           if(!addr.GetKeyID(keyID))
               throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

           bool fInvalid = false;
          vector<unsigned char> vchSig = DecodeBase64(sig.c_str(), &fInvalid);

           if(fInvalid)
               throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

           CDataStream ss(SER_GETHASH, 0);
           ss << encrypted + iv128Base64;

           CKey key;
           if(!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
               return false;

           if(key.GetPubKey().GetID() != keyID)
           {
                return error("encrypted message tx verification failed");
           }
        }
            break;
        case OP_MESSAGE:
        {
           const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
           const std::string recipient(vvchArgs[1].begin(), vvchArgs[1].end());
           const std::string message(vvchArgs[2].begin(), vvchArgs[2].end());
           const std::string sig(stringFromVch(vvchArgs[3]));

           CBitcoinAddress addr(sender);

           if(!addr.IsValid())
             throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

           CKeyID keyID;
           if(!addr.GetKeyID(keyID))
               throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

           bool fInvalid = false;
          vector<unsigned char> vchSig = DecodeBase64(sig.c_str(), &fInvalid);

           if(fInvalid)
               throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

           CDataStream ss(SER_GETHASH, 0);
           ss << message;

           CKey key;
           if(!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
               return false;

           if(key.GetPubKey().GetID() != keyID)
           {
                return error("encrypted message tx verification failed");
           }
        }
            break;
        case OP_ALIAS_ENCRYPTED:
        {
            if(vvchArgs[3].size() != 20)
              return error("registerAlias tx with incorrect hash length");
            CScript script;
            if(vvchPrevArgs.size() != 0)
              script.SetBitcoinAddress(stringFromVch(vvchPrevArgs[2]));
            else
              script.SetBitcoinAddress(stringFromVch(vvchArgs[2]));

              string encrypted = stringFromVch(vvchArgs[0]);
              uint160 hash = uint160(vvchArgs[3]);
              string value = stringFromVch(vvchArgs[4]);
              string r = stringFromVch(vvchArgs[5]);
            if(!verifymessage(script.GetBitcoinAddress(), stringFromVch(vvchArgs[1]), encrypted, hash.ToString(), value, r))
            {
              return error("Dions::ConnectInputsPost: failed to verify signature for registerAlias tx %s",
                      tx.GetHash().ToString().c_str());
            }

           
           if(r != "0" && IsMinePost(tx))
           {
             CScript script;
             script.SetBitcoinAddress(stringFromVch(vvchArgs[2]));

             CBitcoinAddress ownerAddr = script.GetBitcoinAddress();
             if(!ownerAddr.IsValid())
               throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

             CKeyID keyID;
             if(!ownerAddr.GetKeyID(keyID))
               throw JSONRPCError(RPC_TYPE_ERROR, "ownerAddr does not refer to key");


             CKey key;
             if(!pwalletMain->GetKey(keyID, key))
               throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

             CPubKey vchPubKey;
             pwalletMain->GetPubKey(keyID, vchPubKey);

             bool fInvalid;
             string decryptedRand;

             string privRSAKey;
             ENTER_CRITICAL_SECTION(cs_main)
             {
               ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
               {
                 if(!pwalletMain->envCP0(vchPubKey, privRSAKey))
                 {
                   LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                   LEAVE_CRITICAL_SECTION(cs_main)
                   throw JSONRPCError(RPC_TYPE_ERROR, "Failed to retrieve private RSA key");
                 }
               }
               LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
             }
             LEAVE_CRITICAL_SECTION(cs_main)

             DecryptMessage(privRSAKey, r, decryptedRand);

             vchType vchRand = DecodeBase64(decryptedRand.c_str(), &fInvalid);
             if(!pwalletMain->SetRandomKeyMetadata(vchPubKey, vchRand))
               throw JSONRPCError(RPC_WALLET_ERROR, "Failed to set meta data for key");
            CWalletDB walletdb(pwalletMain->strWalletFile, "r+");
            if(!walletdb.UpdateKey(vchPubKey, pwalletMain->mapKeyMetadata[vchPubKey.GetID()]))
              throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");
           }
           
        }
            break;
        case OP_ALIAS_SET:
        {
            if(!found || prevOp != OP_ALIAS_ENCRYPTED)
            {
                return error("ConnectInputsPost() : decryptAlias tx without previous registerAlias tx");
            }

            CScript script;
            if(vvchPrevArgs.size() != 0)
              script.SetBitcoinAddress(stringFromVch(vvchPrevArgs[2]));
            else
              script.SetBitcoinAddress(stringFromVch(vvchArgs[2]));

            if(!verifymessage(script.GetBitcoinAddress(), stringFromVch(vvchArgs[1]), stringFromVch(vvchArgs[0])))
              return error("Dions::ConnectInputsPost: failed to verify signature for decryptAlias tx %s",
                      tx.GetHash().ToString().c_str());



             if(vvchArgs[2] != vvchPrevArgs[2])
             {
               return error("Dions::ConnectInputsPost: OP_ALIAS_SET owner address does not match registerAlias address");
             }

            if(vvchArgs[3].size() > 20)
                return error("decryptAlias tx with rand too big");
            
            if(vvchArgs[2].size() > MAX_XUNIT_LENGTH)
                return error("decryptAlias tx with value too long");

            {
                const vchType& vchHash = vvchPrevArgs[3];
                const vchType& vchAlias = vvchArgs[0];
                const vchType& vchRand = vvchArgs[3];
                vchType vchToHash(vchRand);
                vchToHash.insert(vchToHash.end(), vchAlias.begin(), vchAlias.end());
                uint160 hash = Hash160(vchToHash);
                if(uint160(vchHash) != hash)
                {
                        return error("ConnectInputsPost() : decryptAlias hash mismatch");
                }
            }

            nPrevHeight = aliasHeight(vvchArgs[0]);
            if(nPrevHeight >= 0 && pindexBlock->nHeight - nPrevHeight < scaleMonitor())
                return error("ConnectInputsPost() : decryptAlias on an unexpired alias");
            nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], MIN_SET_DEPTH);

            if((fBlock || fMiner) && nDepth >= 0 && nDepth < MIN_SET_DEPTH)
                return false;



            if(fMiner)
            {
                nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], scaleMonitor());
                if(nDepth == -1)
                    return error("ConnectInputsPost() : decryptAlias cannot be mined if registerAlias is not already in chain and unexpired");

                set<uint256>& setPending = mapState[vvchArgs[0]];
                BOOST_FOREACH(const PAIRTYPE(uint256, CTxIndex)& s, mapTestPool)
                {
                    if(setPending.count(s.first))
                    {
                        return false;
                    }
                }
            }
        }
            break;
        case OP_ALIAS_RELAY:
            if(!found ||(prevOp != OP_ALIAS_SET && prevOp != OP_ALIAS_RELAY))
                return error("updateAlias tx without previous update tx");

            if(vvchArgs[1].size() > MAX_XUNIT_LENGTH)
                return error("updateAlias tx with value too long");

            if(vvchPrevArgs[0] != vvchArgs[0])
            {
                    return error("ConnectInputsPost() : updateAlias alias mismatch");
            }

            nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], scaleMonitor());
            if((fBlock || fMiner) && nDepth < 0)
                return error("ConnectInputsPost() : updateAlias on an expired alias, or there is a pending transaction on the alias");
            break;
        default:
            return error("ConnectInputsPost() : alias transaction has unknown op");
    }
    if(!fBlock && op == OP_ALIAS_RELAY)
    {
        vector<AliasIndex> vtxPos;
        if(ln1Db.lKey(vvchArgs[0])
            && !ln1Db.lGet(vvchArgs[0], vtxPos))
          return error("ConnectInputsPost() : failed to read from alias DB");
        if(!aliasTxPos(vtxPos, vTxindex[nInput].pos))
            return error("ConnectInputsPost() : tx %s rejected, since previous tx(%s) is not in the alias DB\n", tx.GetHash().ToString().c_str(), vTxPrev[nInput].GetHash().ToString().c_str());
    }
    if(fBlock)
    {
        if(op == OP_ALIAS_SET || op == OP_ALIAS_RELAY)
        {

            string locatorStr = stringFromVch(vvchArgs[0]);
            std::transform(locatorStr.begin(), locatorStr.end(), locatorStr.begin(), ::tolower);
            vector<AliasIndex> vtxPos;
            if(ln1Db.lKey(vchFromString(locatorStr))
                && !ln1Db.lGet(vchFromString(locatorStr), vtxPos))
            {
              return error("ConnectInputsPost() : failed to read from alias DB");
            }
            
            if(op == OP_ALIAS_SET)
            {
              CTransaction tx;
              if(aliasTx(ln1Db, vchFromString(locatorStr), tx))
              {
                return error("ConnectInputsPost() : this alias is already active with tx %s",
                tx.GetHash().GetHex().c_str());
              }
            }

            vector<unsigned char> vchValue;
            int nHeight;
            uint256 hash;
            txTrace(txPos, vchValue, hash, nHeight);
            AliasIndex txPos2;
            txPos2.nHeight = pindexBlock->nHeight;
            txPos2.vValue = vchValue;
            txPos2.vAddress = a1;
            txPos2.txPos = txPos;
            vtxPos.push_back(txPos2);
            if(!ln1Db.lPut(vvchArgs[0], vtxPos))
              return error("ConnectInputsPost() : failed to write to alias DB");
        }

        if(op != OP_ALIAS_ENCRYPTED)
        {
            ENTER_CRITICAL_SECTION(cs_main)
            {
                std::map<std::vector<unsigned char>, std::set<uint256> >::iterator mi = mapState.find(vvchArgs[0]);
                if(mi != mapState.end())
                    mi->second.erase(tx.GetHash());

            }
            LEAVE_CRITICAL_SECTION(cs_main)
         }
    }

    return true;
}
unsigned char GetAddressVersion() 
{ 
  return((unsigned char)(fTestNet ? 111 : 103)); 
}
