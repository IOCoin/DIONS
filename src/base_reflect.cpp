




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

