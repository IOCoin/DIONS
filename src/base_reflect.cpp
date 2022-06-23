




// Copyright (c) 2018 The I/O Coin developers
//
//
#include "db.h"
#include "txdb-leveldb.h"
#include "keystore.h"
#include "wallet.h"
#include "init.h"
#include "base_reflect.h"

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

extern LocatorNodeDB* ln1Db;
extern bool aliasTx(LocatorNodeDB& aliasCacheDB, const vector<unsigned char> &vchPath, CTransaction& tx);
extern unsigned int scaleMonitor();
extern string txRelay(const CScript& scriptPubKey, int64_t nValue, const __wx__Tx& wtxIn, __wx__Tx& wtxNew, bool fAskFee);
extern bool aliasAddress(const CTransaction& tx, std::string& strAddress);
extern Object JSONRPCError(int code, const string& message);
extern Value xtu_url__(const string& url);
template<typename T> void ConvertTo(Value& value, bool fAllowNull=false);

std::map<vchType, uint256> mapMyMessages_cycle;
std::map<vchType, uint256> mapLocator_cycle;
std::map<vchType, set<uint256> > mapState_cycle;
std::map<vchType, set<uint256> > k1Export_cycle;

#ifdef GUI
extern std::map<uint160, vchType> mapLocatorHashes;
#endif

#ifdef LL_RELAY_BASE__
#define mul_mod(a,b,m) (( (long long) (a) * (long long) (b) ) % (m))
#else
#define mul_mod(a,b,m) fmod( (double) a * (double) b, m)
#endif


//Ext frame

//VX

extern vchType vchFromValue(const Value& value);
extern vchType vchFromString(const std::string& str);
extern string stringFromVch(const vector<unsigned char> &vch);

bool searchPathEncrypted2(string l, uint256& wtxInHash);

extern int GetTxPosHeight(PathIndex& txPos);
extern int GetTxPosHeight(CDiskTxPos& txPos);
int aliasHeight(vector<unsigned char> vchPath);

bool vertex_serial_n_cycle(const string& origin, const string& data, __wx__Tx& serial_n)
{
    string locatorStr = "vertex_" + origin;
    string indexStr = data;

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

    uint256 wtxInHash__;

    vector<Value> res;
    LocatorNodeDB aliasCacheDB("r");
    CTransaction tx;
    if(aliasTx(aliasCacheDB, vchFromString(locatorStr), tx))
    {
        string err = "Attempt to register alias : " + locatorStr + ", this alias is already active with tx " + tx.GetHash().GetHex();

        throw JSONRPCError(RPC_WALLET_ERROR, err);
    }

    CPubKey vchPubKey;
    CReserveKey reservekey(pwalletMain);
    if(!reservekey.GetReservedKey(vchPubKey))
    {
        return false;
    }

    reservekey.KeepKey();

    cba keyAddress(vchPubKey.GetID());
    CKeyID keyID;
    keyAddress.GetKeyID(keyID);
    pwalletMain->SetAddressBookName(keyID, "");

    __wx__DB walletdb(pwalletMain->strWalletFile, "r+");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    serial_n.nVersion = CTransaction::CYCLE_TX_VERSION;

    CScript scriptPubKeyOrig;
    scriptPubKeyOrig.SetBitcoinAddress(vchPubKey.Raw());
    CScript scriptPubKey;
    vchType vchPath = vchFromString(locatorStr);
    vchType vchValue = vchFromString(indexStr);

    scriptPubKey << OP_BASE_SET << vchPath << vchValue << OP_2DROP << OP_DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, serial_n, false);

        if(strError != "")
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        mapLocator[vchPath] = serial_n.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return true;
}

Value decryptPath_cycle(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 3)
        throw runtime_error(
            "registerPathGenerate <alias> <t> <i>"
            + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();
    string tStr = params[1].get_str();
    string iStr = params[2].get_str();

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
    __wx__Tx vertex;
    vertex_serial_n_cycle(locatorStr, tStr + ":" + iStr, vertex);
    vector<Value> res;
    res.push_back(vertex.GetHash().GetHex());
    return res;
}

Value decryptPath_cycle__(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2 )
        throw runtime_error(
            "decryptPath <alias> <address specified by owner>\n"
            + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();
    const vchType vchPath = vchFromValue(locatorStr);
    const std::string addressOfOwner = params[1].get_str();

    cba ownerAddr(addressOfOwner);
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


    __wx__Tx wtx;
    wtx.nVersion = CTransaction::CYCLE_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchPath) && mapState[vchPath].size())
        {
            error("decryptPath_cycle() : there are %lu pending operations on that alias, including %s",
                  mapState[vchPath].size(),
                  mapState[vchPath].begin()->GetHex().c_str());
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchPath, tx))
        {
            error("decryptPath_cycle() : this alias is already active with tx %s",
                  tx.GetHash().GetHex().c_str());
            throw runtime_error("this alias is already active");
        }
    }


    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchPathEncrypted2(stringFromVch(vchPath), wtxInHash))
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("could not find a coin with this alias, try specifying the registerPath transaction id");
        }


        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("previous transaction is not in the wallet");
        }

        CScript scriptPubKeyOrig;
        scriptPubKeyOrig.SetBitcoinAddress(addressOfOwner);

        CScript scriptPubKey;


        __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        const int nHeight = wtxIn.GetHeightInMainChain();
        const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
        if(ex <= 0)
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("this encrypted alias is expired. You must create a new one of that name to decrypt it.");
        }

        vector<unsigned char> vchPrevSig;
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) {
                if(op != OP_ALIAS_ENCRYPTED)
                    throw runtime_error("previous transaction wasn't a registerPath");
                CDataStream ss(SER_GETHASH, 0);
                ss << locatorStr;
                CScript script;
                script.SetBitcoinAddress(stringFromVch(vvch[2]));

                cba ownerAddr = script.GetBitcoinAddress();
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


                scriptPubKey << OP_ALIAS_SET << vchPath << vchFromString(sigBase64) << vchFromString(addressOfOwner) << vchRand << vvch[4] << OP_2DROP << OP_2DROP << OP_2DROP;
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



Value registerPathGenerate_cycle(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
            "registerPathGenerate <alias>"
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

    uint256 wtxInHash__;
    if(searchPathEncrypted2(locatorStr, wtxInHash__) == true)
    {
        __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash__];
        const int nHeight = wtxIn.GetHeightInMainChain();
        const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
        if(ex > 0)
        {
            string err = "Attempt to register alias : " + locatorStr + ", this alias is already registered as encrypted with tx " + wtxInHash__.GetHex();

            throw JSONRPCError(RPC_WALLET_ERROR, err);
        }
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

    const vchType vchPath = vchFromValue(locatorStr);
    vchToHash.insert(vchToHash.end(), vchPath.begin(), vchPath.end());
    const uint160 hash = Hash160(vchToHash);

    CPubKey vchPubKey;
    CReserveKey reservekey(pwalletMain);
    if(!reservekey.GetReservedKey(vchPubKey))
    {
        return false;
    }

    reservekey.KeepKey();

    cba keyAddress(vchPubKey.GetID());
    CKeyID keyID;
    keyAddress.GetKeyID(keyID);
    pwalletMain->SetAddressBookName(keyID, "");

    __wx__DB walletdb(pwalletMain->strWalletFile, "r+");
    if(!pwalletMain->SetRSAMetadata(vchPubKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

    if(!walletdb.UpdateKey(vchPubKey, pwalletMain->kd[vchPubKey.GetID()]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

    string pKey;
    if(!pwalletMain->envCP0(vchPubKey, pKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load alpha");

    string pub_k;
    if(!pwalletMain->envCP1(vchPubKey, pub_k))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address has no associated RSA keys");

    if(!pwalletMain->SetRandomKeyMetadata(vchPubKey, vchRand))
        throw JSONRPCError(RPC_WALLET_ERROR, "Failed to set meta data for key");

    if(!walletdb.UpdateKey(vchPubKey, pwalletMain->kd[vchPubKey.GetID()]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    string encrypted;
    EncryptMessage(pub_k, locatorStr, encrypted);

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::CYCLE_TX_VERSION;

    CScript scriptPubKeyOrig;
    scriptPubKeyOrig.SetBitcoinAddress(vchPubKey.Raw());
    CScript scriptPubKey;
    vchType vchEncryptedPath = vchFromString(encrypted);
    string tmp = stringFromVch(vchEncryptedPath);
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

    scriptPubKey << OP_ALIAS_ENCRYPTED << vchEncryptedPath << vchFromString(sigBase64) << vchFromString(keyAddress.ToString()) << hash << vchValue << vchFromString("0") << vchFromString("_") << vchFromString(State::GROUND) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
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
        mapLocator[vchPath] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    res.push_back(wtx.GetHash().GetHex());
    return res;
}

Value ioget_cycle(const Array& params, bool fHelp)
{
    return psimplex_cycle(params, fHelp);
}

Value simplexU_cycle(const Array& params, bool fHelp) {
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "simplexU <base> <forward>"
            + HelpRequiringPassphrase());
    string locatorStr = params[0].get_str();

    const vchType vchPath = vchFromValue(locatorStr);
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
    vchType sv = vchFromString(s);
    string r = EncodeBase64(&sv[0], sv.size());

    if(r.size() > MAX_XUNIT_LENGTH)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");


    __wx__Tx wtx;
    wtx.nVersion = CTransaction::CYCLE_TX_VERSION;
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
    scriptPubKey << OP_ALIAS_RELAY << vchPath << vchFromString(r) << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath_cycle() : there are %lu pending operations on that alias, including %s",
                      mapState[vchPath].size(),
                      mapState[vchPath].begin()->GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("there are pending operations on that alias");
            }

            EnsureWalletIsUnlocked();

            LocatorNodeDB aliasCacheDB("r");
            CTransaction tx;
            if(!aliasTx(aliasCacheDB, vchPath, tx))
            {
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("could not find a coin with this alias 5");
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
                error("updatePath_cycle() : this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }

            __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
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
Value psimplex_cycle(const Array& params, bool fHelp) {
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "psimplex <base> <is>"
            + HelpRequiringPassphrase());
    string k1;
    vchType vchNodeLocator;
    k1 =(params[0]).get_str();
    vchNodeLocator = vchFromValue(params[0]);
    const char* out__ = (params[1].get_str()).c_str();

    fs::path p = out__;
    boost::filesystem::path ve = p.parent_path();
    if(!fs::exists(ve))
        throw runtime_error("Invalid out put path");


    ln1Db->filter();
    string alias = params[0].get_str();
    string address = "address not found";
    vchType value;

    vector<PathIndex> vtxPos;
    vchType vchPath = vchFromString(alias);
    if(ln1Db->lKey(vchPath))
    {
        if(!ln1Db->lGet(vchPath, vtxPos))
            return error("aliasHeight() : failed to read from name DB");
        if(vtxPos.empty())
            return -1;

        PathIndex& txPos = vtxPos.back();
        address = txPos.vAddress;
        value = txPos.vValue;
    }
    else
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "DION does not exist.");
    }

    string val = stringFromVch(value);

    bool fInvalid = false;
    vector<unsigned char> asK = DecodeBase64(val.c_str(), &fInvalid);

    string v = stringFromVch(asK);
    try
    {
        stringstream is(v, ios_base::in | ios_base::binary);
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
