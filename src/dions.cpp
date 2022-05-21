




// Copyright (c) 2018 The I/O Coin developers
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
#include <ctime>
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
extern Object JSONRPCError(int code, const string& message);
extern Value xtu_url__(const string& url);
template<typename T> void ConvertTo(Value& value, bool fAllowNull=false);

std::map<vchType, uint256> mapMyMessages;
std::map<vchType, uint256> mapLocator;
std::map<vchType, set<uint256> > mapState;
std::map<vchType, set<uint256> > k1Export;

void xsc(CBlockIndex*);
static bool vclose(string&,string&);

CScript aliasStrip(const CScript& scriptIn);
#ifdef GUI
extern std::map<uint160, vchType> mapLocatorHashes;
#endif

#ifdef LL_RELAY_BASE__
#define mul_mod(a,b,m) (( (long long) (a) * (long long) (b) ) % (m))
#else
#define mul_mod(a,b,m) fmod( (double) a * (double) b, m)
#endif


const int __AV__[] = {
    0xcf, 0xfe, 0xac, 0xbf, 0xde, 0xfa, 0xcc, 0xab,
    0xef, 0x26, 0x10, 0xca, 0xbd, 0x45, 0x37, 0xff,
    0xce, 0x06, 0x34, 0xac, 0xdc, 0x29, 0x26, 0xca,
    0x2c, 0x19, 0xc2, 0xff, 0xef, 0x08, 0xdc, 0x2f,
    0xff, 0x42, 0xcd, 0xaf, 0x2d, 0x31, 0xcf, 0xff
};
int relay_inv(int x, int y)
{
    int q, u, v, a, c, t;

    u = x;
    v = y;
    c = 1;
    a = 0;
    do
    {
        q = v / u;

        t = c;
        c = a - q * c;
        a = t;

        t = u;
        u = v - q * u;
        v = t;
    } while (u != 0);
    a = a % y;
    if (a < 0)
        a = y + a;
    return a;
}


extern int INTERN_REF0__;
extern int EXTERN_REF0__;

extern uint256 SignatureHash(CScript scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

extern bool Solver(const CKeyStore& keystore, const CScript& scriptPubKey, uint256 hash, int nHashType, CScript& scriptSigRet, txnouttype& type);
extern bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn, int nHashType);
extern Value sendtoaddress(const Array& params, bool fHelp);

//Ext frame
bool relaySigFrame(int,vchType&);
bool frlRelay(int&);

//VX
Value alias(const Array& params, bool fHelp);
Value updateEncrypt(const Array& params, bool fHelp);
Value statusList(const Array& params, bool fHelp);
Value downloadDecrypt(const Array& params, bool fHelp);
Value downloadDecryptEPID(const Array& params, bool fHelp);

extern unsigned int LR_SHIFT__[LR_R];

bool collisionReference(string& s, uint256& wtxInHash);
bool searchPathEncrypted2(string l, uint256& wtxInHash);
bool getImportedPubKey(string senderAddress, string recipientAddress, vchType& recipientPubKeyVch, vchType& aesKeyBase64EncryptedVch, bool& thresholdCount);
bool getImportedPubKey(string senderAddress, string recipientAddress, vchType& recipientPubKeyVch, vchType& aesKeyBase64EncryptedVch);
bool getImportedPubKey(string recipientAddress, vchType& recipientPubKeyVch);
bool internalReference__(string recipientAddress, vchType& recipientPubKeyVch);
bool pk(string senderAddress, string recipientAddress, vchType& recipientPubKeyVch, vchType& aesKeyBase64EncryptedVch);

static bool xs(string&);

bool tunnelSwitch__(int r);

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
bool channelPredicate(string ext, string& tor)
{
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchS, vchR, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.GetPublicKeyUpdate(nOut, vchS, vchR, vchKey, vchAes, vchSig))
                    continue;

                string k1 = stringFromVch(vchR);
                if(k1 != ext)
                    continue;

                string k2 = stringFromVch(vchS);
                cba r(k2);
                CKeyID keyID;
                r.GetKeyID(keyID);
                CKey key;
                if(!pwalletMain->GetKey(keyID, key))
                {
                    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                    LEAVE_CRITICAL_SECTION(cs_main)

                    return false;
                }
                else
                {
                    tor = k2;
                }
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return true;
}
Value gw1(const Array& params, bool fHelp)
{
    Array oRes;
    LocatorNodeDB ln1Db("r");

    Dbc* cursorp;
    try
    {
        cursorp = ln1Db.GetCursor();

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
                Object o;
                printf("  k1 %s\n", k1.c_str());
                vchType k2;
                ssKey >> k2;
                string a = stringFromVch(k2);
                printf("  k2 %s\n", a.c_str());
                o.push_back(Pair("alias", a));

                vector<PathIndex> vtxPos;
                CDataStream ssValue((char*)data.get_data(), (char*)data.get_data() + data.get_size(), SER_DISK, CLIENT_VERSION);
                ssValue >> vtxPos;

                PathIndex i = vtxPos.back();
                string i_address = i.vAddress;
                o.push_back(Pair("address", i_address));
                o.push_back(Pair("h", (int)i.nHeight));
                oRes.push_back(o);
            }
        }
        if (ret != DB_NOTFOUND)
        {
            // ret should be DB_NOTFOUND upon exiting the loop.
            // Dbc::get() will by default throw an exception if any
            // significant errors occur, so by default this if block
            // can never be reached.
        }
    }
    catch(DbException &e)
    {
        //ln1Db.err(e.get_errno(), "Error!");
    }
    catch(std::exception &e)
    {
        //ln1Db.errx("Error! %s", e.what());
    }

    if (cursorp != NULL)
        cursorp->close();

    printf("XXXX xsc scanning for current dions\n");
    LocatorNodeDB l("cr+");
    CTxDB txdb("r");

    CBlockIndex* p = pindexGenesisBlock;
    for(; p; p=p->pnext)
    {
        if(p->nHeight < 1625000)
            continue;

        CBlock block;
        CDiskTxPos txPos;
        block.ReadFromDisk(p);
        uint256 h;

        BOOST_FOREACH(CTransaction& tx, block.vtx)
        {
            if (tx.nVersion != CTransaction::DION_TX_VERSION)
                continue;

            vector<vector<unsigned char> > vvchArgs;
            int op, nOut;

            aliasTx(tx, op, nOut, vvchArgs);
            if (op != OP_ALIAS_SET)
                continue;

            const vector<unsigned char>& v = vvchArgs[0];
            string a = stringFromVch(v);

            if (!GetTransaction(tx.GetHash(), tx, h))
                continue;

            printf("XXXX ALIAS  %s\n", a.c_str());
            const CTxOut& txout = tx.vout[nOut];
            const CScript& scriptPubKey = aliasStrip(txout.scriptPubKey);
            string s = scriptPubKey.GetBitcoinAddress();
            printf("XXXX ADDRESS %s\n", s.c_str());
            printf("XXXX HEIGHT %d\n", p->nHeight);
            printf("XXXX TX     %s\n", tx.GetHash().ToString().c_str());
            CTxIndex txI;
            if(!txdb.ReadTxIndex(tx.GetHash(), txI))
                continue;

            //printf("XXXX read txI\n");
            //linkSet(vvchArgs, p, txI.pos, s, l);
        }
    }

    return oRes;
}
bool channel(string l, string f, string& k, bool& black)
{
    vchType rVch;
    vchType alphaVch;
    bool transient=false;
    if((!getImportedPubKey(l, f, rVch, alphaVch, transient)) || transient)
    {
        CKeyID keyID;
        cba keyAddress(l);
        keyAddress.GetKeyID(keyID);
        CPubKey vchPubKey;
        pwalletMain->GetPubKey(keyID, vchPubKey);
        if(pwalletMain->aes_(vchPubKey, f, k))
        {
            black=false;
            return true;
        }

        return false;
    }

    k = stringFromVch(alphaVch);

    return true;
}
unsigned int scaleMonitor()
{
    if(!fTestNet)
        return 210000;

    return 210000;
}
int GetTxPosHeight(PathIndex& txPos)
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
aliasHeight(vector<unsigned char> vchPath)
{
    vector<PathIndex> vtxPos;
    LocatorNodeDB ln1Db("r");
    if(ln1Db.lKey(vchPath))
    {
        if(!ln1Db.lGet(vchPath, vtxPos))
            return error("aliasHeight() : failed to read from alias DB");
        if(vtxPos.empty())
            return -1;

        PathIndex& txPos = vtxPos.back();
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
bool txPost(const vector<pair<CScript, int64_t> >& vecSend, const __wx__Tx& wtxIn, int nTxOut, __wx__Tx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet)
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

                set<pair<const __wx__Tx*, unsigned int> > setCoins;
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


                vector<pair<const __wx__Tx*, unsigned int> >
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

                BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int)& coin, vecCoins)
                {
                    wtxNew.vin.push_back(CTxIn(coin.first->GetHash(), coin.second));
                }

                int nIn = 0;
                BOOST_FOREACH(PAIRTYPE(const __wx__Tx*, unsigned int)& coin, vecCoins)
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
int atod(const std::string& addr, std::string& d)
{
    cba address__(addr);
    if(!address__.IsValid())
        return -2;

    vector<PathIndex> vtxPos_;
    string a("a");
    if (ln1Db->lGet(vchFromString(a), vtxPos_))
    {
        PathIndex i = vtxPos_.back();
    }

    Dbc* cursorp;
    try
    {
        cursorp = ln1Db->GetCursor();

        Dbt key, data;
        int ret;

        while ((ret = cursorp->get(&key, &data, DB_NEXT)) == 0)
        {
            CDataStream ssKey(SER_DISK, CLIENT_VERSION);
            ssKey.write((char*)key.get_data(), key.get_size());

            string k1;
            ssKey >> k1;
            if(k1 == "alias_")
            {
                vchType k2;
                ssKey >> k2;
                string a = stringFromVch(k2);

                vector<PathIndex> vtxPos;
                CDataStream ssValue((char*)data.get_data(), (char*)data.get_data() + data.get_size(), SER_DISK, CLIENT_VERSION);
                ssValue >> vtxPos;

                BOOST_FOREACH(PathIndex& i, vtxPos)
                {
                    int k = i.nHeight + scaleMonitor() - pindexBest->nHeight;
                    if(k<=0)
                    {
                        continue;
                    }
                    string i_address = (i.vAddress).c_str();
                    if(i_address == addr)
                    {
                        d = a;
                        break;
                    }
                }
            }
        }
    }
    catch(DbException &e)
    {

    }
    catch(std::exception &e)
    {
    }

    if (cursorp != NULL)
        cursorp->close();

    if(d == "")
        return -1;

    return 0;
}
bool txRelayPre__(const CScript& scriptPubKey, const __wx__Tx& wtxIn, __wx__Tx& wtxNew, int64_t& t, string& e)
{
    int nTxOut = aliasOutIndex(wtxIn);
    if(nTxOut == -1) return false;
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
string txRelay_no_commit(const CScript& scriptPubKey, int64_t nValue, const __wx__Tx& wtxIn, __wx__Tx& wtxNew, bool fAskFee)
{
    int nTxOut = aliasOutIndex(wtxIn);
    if(nTxOut == -1) return "error out index";
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

    //if(!pwalletMain->CommitTransaction(wtxNew, reservekey))
    //    return _("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");

    return "";
}
string txRelay(const CScript& scriptPubKey, int64_t nValue, const __wx__Tx& wtxIn, __wx__Tx& wtxNew, bool fAskFee)
{
    int nTxOut = aliasOutIndex(wtxIn);
    if(nTxOut == -1) return "error out index";
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
bool txTrace(PathIndex& txPos, vector<unsigned char>& vchValue, uint256& hash, int& nHeight)
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
bool aliasTx(LocatorNodeDB& aliasCacheDB, const vector<unsigned char> &vchPath, CTransaction& tx)
{

    vector<PathIndex> vtxPos;
    if(!aliasCacheDB.lGet(vchPath, vtxPos) || vtxPos.empty())
        return false;

    PathIndex& txPos = vtxPos.back();

    unsigned int nHeight = txPos.nHeight;
    if(nHeight + scaleMonitor() <= pindexBest->nHeight)
    {
        string alias = stringFromVch(vchPath);
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
    BOOST_FOREACH(const PAIRTYPE(cba, string)& item, pwalletMain->mapAddressBook)
    {
        const cba& a = item.first;
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
    BOOST_FOREACH(const PAIRTYPE(cba, string)& item, pwalletMain->mapAddressBook)
    {

        const cba& a = item.first;
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

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

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

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchS, vchR, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.GetPublicKeyUpdate(nOut, vchS, vchR, vchKey, vchAes, vchSig))
                    continue;

                string keySenderAddr = stringFromVch(vchS);
                cba r(keySenderAddr);
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

bool hk(string addrStr)
{
    cba r(addrStr);
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

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchSender, vchRecipient, vchEncryptedMessage, ivVch, vchSig;
                int nOut;
                if(!tx.GetEncryptedMessageUpdate(nOut, vchSender, vchRecipient, vchEncryptedMessage, ivVch, vchSig))
                {
                    continue;
                }

                const int nHeight = tx.GetHeightInMainChain();

                string myAddr;
                string fKey;
                if(hk(stringFromVch(vchSender)))
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

                cba r(myAddr);
                if(!r.IsValid())
                {
                    continue;
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

                mapPathVchInt[vchSender] = nHeight;
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

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;





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
Value externFrame__(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
            "externFrame__ [<node opt>]\n"
        );

    string k1;
    vchType vchNodeLocator;
    k1=(params[0]).get_str();
    vchNodeLocator = vchFromString(k1);

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;
    CScript scriptPubKey;

    scriptPubKey << OP_ALIAS_RELAY << vchNodeLocator;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchNodeLocator) && mapState[vchNodeLocator].size())
            {
                error("externFrame__() : %lu pending operations , including %s",
                      mapState[vchNodeLocator].size(),
                      mapState[vchNodeLocator].begin()->GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("pending ops ");
            }

            EnsureWalletIsUnlocked();

            LocatorNodeDB ln1Db("r");
            CTransaction tx;
            if(!aliasTx(ln1Db, vchNodeLocator, tx))
            {
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("could not find a coin with this alias 2");
            }

            uint256 wtxInHash = tx.GetHash();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("externFrame__() : not present %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("not present");
            }
            string strAddress = "";
            aliasAddress(tx, strAddress);
            if(strAddress == "")
                throw runtime_error("no associated address");

            uint160 hash160;
            bool isValid = AddressToHash160(strAddress, hash160);
            if(!isValid)
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
            scriptPubKeyOrig.SetBitcoinAddress(strAddress);

            const __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            int op__;
            int nOut;
            vchType vX;
            wtxIn.aliasSet(op__, nOut, vchNodeLocator, vX);

            scriptPubKey << vX << OP_2DROP << OP_DROP;
            scriptPubKey += scriptPubKeyOrig;

            string locatorStr = stringFromVch(vchNodeLocator);
            string vXStr = stringFromVch(vX);
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

Value internFrame__(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
            "internFrame__ [<node opt>]\n"
        );

    string k1;
    vchType vchNodeLocator;
    k1=(params[0]).get_str();

    vchNodeLocator = vchFromString(k1);

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchNodeLocator) && mapState[vchNodeLocator].size())
        {
            error("updateEncryptedPath() : %lu pending operations %s",
                  mapState[vchNodeLocator].size(),
                  mapState[vchNodeLocator].begin()->GetHex().c_str());
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("pending operations ");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB ln1Db("r");
        CTransaction tx;
        if(aliasTx(ln1Db, vchNodeLocator, tx))
        {
            error("internFrame__() : already active with tx %s",
                  tx.GetHash().GetHex().c_str());
            throw runtime_error("already active");
        }
    }

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchPathEncrypted2(stringFromVch(vchNodeLocator), wtxInHash))
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("not found");
        }


        if(!pwalletMain->mapWallet.count(wtxInHash))
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("not in the wallet");
        }

        CScript scriptPubKeyOrig;

        CScript scriptPubKey;

        __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if(aliasScript(out.scriptPubKey, op, vvch)) {
                if(op != OP_ALIAS_ENCRYPTED)
                    throw runtime_error("previous transaction was not an OP_ALIAS_ENCRYPTED");


                string iv = stringFromVch(vvch[6]);
                State hydr(stringFromVch(vvch[7]));
                string gen;
                string reference = "_";
                vchType xref_;
                if(iv != "_")
                {
                    if(hydr() == State::ION)
                    {
                        string csKStr;
                        string f = stringFromVch(vvch[7]);
                        string l = stringFromVch(vvch[2]);
                        bool black_0=true;
                        if(channel(l, f, csKStr, black_0))
                        {
                            string alpha;
                            if(black_0)
                            {
                                cba p(l);
                                if(!p.IsValid())
                                    throw JSONRPCError(RPC_TYPE_ERROR, "address");
                                CKeyID k;
                                if(!p.GetKeyID(k))
                                    throw JSONRPCError(RPC_TYPE_ERROR, "key");

                                CKey k_;
                                if(!pwalletMain->GetKey(k, k_))
                                    throw JSONRPCError(RPC_TYPE_ERROR, "not found");

                                CPubKey pk = k_.GetPubKey();
                                string q;
                                pwalletMain->envCP0(pk, q);
                                DecryptMessage(q, csKStr, alpha);
                                csKStr=alpha;
                            }

                            bool fInvalid = false;
                            vector<unsigned char> csK = DecodeBase64(csKStr.c_str(), &fInvalid);
                            string decrypted;
                            DecryptMessageAES(stringFromVch(vvch[4]),
                                              decrypted,
                                              csK,
                                              iv);

                            string value = decrypted;
                            Relay r;
                            if(!pwalletMain->relay_(vvch[2], r))
                            {
                                vchType kAlpha;
                                GenerateAESKey(kAlpha);

                                string alpha = EncodeBase64(&kAlpha[0], kAlpha.size());

                                r.ctrl(alpha);

                                pwalletMain->relay(vvch[2], r);

                                __wx__DB walletdb(pwalletMain->strWalletFile, "r+");

                                pwalletMain->LoadRelay(vchNodeLocator, r);
                                if(!walletdb.UpdateKey(vchNodeLocator, pwalletMain->lCache[vchNodeLocator]))
                                    throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write data for key");
                            }

                            string idx = r.ctrl_();
                            fInvalid = false;
                            vector<unsigned char> v = DecodeBase64(idx.c_str(), &fInvalid);

                            EncryptMessageAES(decrypted, gen, v, reference);
                            if(gen.size() > MAX_XUNIT_LENGTH)
                                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                        }
                        else
                        {
                            throw JSONRPCError(RPC_TYPE_ERROR, "non ionic state");
                        }
                    }

                    xref_ = vchFromString(State::ATOMIC);
                }
                else
                    xref_ = vchFromString(State::GROUND);

                string encrypted = stringFromVch(vvch[0]);
                uint160 hash = uint160(vvch[3]);
                string vXStr;
                if(iv == "_" || hydr() == State::ATOMIC)
                    vXStr = stringFromVch(vvch[4]);
                else
                    vXStr = gen;

                string ionExcitation = stringFromVch(vvch[5]);
                CDataStream ss(SER_GETHASH, 0);
                ss << encrypted;
                ss << hash.ToString();
                ss << vXStr;
                ss << ionExcitation;

                CScript script;
                script.SetBitcoinAddress(stringFromVch(vvch[2]));

                cba ownerAddr = script.GetBitcoinAddress();
                if(!ownerAddr.IsValid())
                    throw JSONRPCError(RPC_TYPE_ERROR, "Invalid owner address");

                CKeyID keyID;
                if(!ownerAddr.GetKeyID(keyID))
                    throw JSONRPCError(RPC_TYPE_ERROR, "does not refer to key");

                CKey key;
                if(!pwalletMain->GetKey(keyID, key))
                    throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

                CPubKey vchPubKey;
                pwalletMain->GetPubKey(keyID, vchPubKey);

                vector<unsigned char> vchSig;
                if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

                string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

                scriptPubKey << OP_ALIAS_ENCRYPTED << vvch[0] << vchFromString(sigBase64) << vvch[2] << vvch[3] << vchFromString(vXStr) << vvch[5] << vchFromString(reference) << xref_ << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
                scriptPubKeyOrig.SetBitcoinAddress(stringFromVch(vvch[2]));
                scriptPubKey += scriptPubKeyOrig;
                found = true;
            }
        }

        if(!found)
        {
            throw runtime_error("previous is not an alias tx");
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

    return false;
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
    fs::path p = out__;
    boost::filesystem::path ve = p.parent_path();
    if(!fs::exists(ve))
        throw runtime_error("Invalid out put path");

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    string value;
    bool found=false;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
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
                    cba r(strAddress);
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
                    if(mapPathVchInt.find(vchFromString(decrypted)) != mapPathVchInt.end() && mapPathVchInt[vchFromString(decrypted)] > nHeight)
                    {
                        continue;
                    }
                    mapPathVchInt[vchFromString(decrypted)] = nHeight;

                    DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);
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
                    if(mapPathVchInt.find(vchPath) != mapPathVchInt.end() && mapPathVchInt[vchPath] > nHeight)
                    {
                        continue;
                    }
                    mapPathVchInt[vchPath] = nHeight;

                    if(k1 != stringFromVch(vchPath))
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
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
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
                    cba r(strAddress);
                    if(!r.IsValid())
                        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

                    CKeyID keyID;
                    if(!r.GetKeyID(keyID))
                        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

                    CKey key;
                    if(!pwalletMain->GetKey(keyID, key))
                    {
                        aliasObj.push_back(Pair("alias", stringFromVch(vchPath)));
                    }
                    else
                    {
                        CPubKey pubKey = key.GetPubKey();
                        if(pwalletMain->envCP0(pubKey, rsaPrivKey) == false)
                        {
                            continue;
                        }

                        DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);

                        aliasObj.push_back(Pair("alias", decrypted));
                    }

                    aliasObj.push_back(Pair("encrypted", "true"));
                }
                else
                {
                    if(k1 != stringFromVch(vchPath)) continue;

                    aliasObj.push_back(Pair("alias", stringFromVch(vchPath)));
                    aliasObj.push_back(Pair("encrypted", "false"));
                }

                aliasObj.push_back(Pair("value", value));

                if(!IsMinePost(tx))
                    aliasObj.push_back(Pair("transferred", 1));
                aliasObj.push_back(Pair("address", strAddress));
                aliasObj.push_back(Pair("nHeigt", nHeight));


                cba keyAddress(strAddress);
                CKeyID keyID;
                keyAddress.GetKeyID(keyID);
                CPubKey vchPubKey;
                pwalletMain->GetPubKey(keyID, vchPubKey);
                vchType vchRand;

                const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
                aliasObj.push_back(Pair("expires_in", ex));
                if(ex <= 0)
                    aliasObj.push_back(Pair("expired", 1));

                if(tunnelSwitch__(ex))
                    aliasObj.push_back(Pair("tunnel_switch", ex));

                if(mapState.count(vchPath) && mapState[vchPath].size())
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

    cba address;
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
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
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
                    cba r(strAddress);
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

                    DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);
                    if(k1 != decrypted)
                    {
                        continue;
                    }

                    aliasObj.push_back(Pair("alias", decrypted));

                    aliasObj.push_back(Pair("encrypted", "true"));
                }
                else
                {
                    if(k1 != stringFromVch(vchPath)) continue;

                    aliasObj.push_back(Pair("alias", stringFromVch(vchPath)));
                    aliasObj.push_back(Pair("encrypted", "false"));
                }

                aliasObj.push_back(Pair("value", value));

                if(!IsMinePost(tx))
                    aliasObj.push_back(Pair("transferred", 1));
                aliasObj.push_back(Pair("address", strAddress));
                aliasObj.push_back(Pair("nHeigt", nHeight));


                cba keyAddress(strAddress);
                CKeyID keyID;
                keyAddress.GetKeyID(keyID);
                CPubKey vchPubKey;
                pwalletMain->GetPubKey(keyID, vchPubKey);
                vchType vchRand;

                const int expiresIn = nHeight + scaleMonitor() - pindexBest->nHeight;
                aliasObj.push_back(Pair("expires_in", expiresIn));
                if(expiresIn <= 0)
                    aliasObj.push_back(Pair("expired", 1));

                if(mapState.count(vchPath) && mapState[vchPath].size())
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


    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
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
                    cba r(strAddress);
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

                    DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);
                    if(k1 != decrypted)
                    {
                        continue;
                    }

                    if(mapPathVchInt.find(vchFromString(decrypted)) != mapPathVchInt.end() && mapPathVchInt[vchFromString(decrypted)] > nHeight)
                    {
                        continue;
                    }
                    aliasObj.push_back(Pair("alias", decrypted));

                    aliasObj.push_back(Pair("encrypted", "true"));
                    mapPathVchInt[vchFromString(decrypted)] = nHeight;
                }
                else
                {
                    if(mapPathVchInt.find(vchPath) != mapPathVchInt.end() && mapPathVchInt[vchPath] > nHeight)
                    {
                        continue;
                    }

                    if(k1 != stringFromVch(vchPath)) continue;

                    aliasObj.push_back(Pair("alias", stringFromVch(vchPath)));
                    aliasObj.push_back(Pair("encrypted", "false"));
                    mapPathVchInt[vchPath] = nHeight;
                }

                aliasObj.push_back(Pair("value", value));

                if(!IsMinePost(tx))
                    aliasObj.push_back(Pair("transferred", 1));
                aliasObj.push_back(Pair("address", strAddress));
                aliasObj.push_back(Pair("nHeigt", nHeight));


                cba keyAddress(strAddress);
                CKeyID keyID;
                keyAddress.GetKeyID(keyID);
                CPubKey vchPubKey;
                pwalletMain->GetPubKey(keyID, vchPubKey);
                vchType vchRand;

                const int expiresIn = nHeight + scaleMonitor() - pindexBest->nHeight;
                aliasObj.push_back(Pair("expires_in", expiresIn));
                if(expiresIn <= 0)
                    aliasObj.push_back(Pair("expired", 1));

                if(mapState.count(vchPath) && mapState[vchPath].size())
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
                    aliasMapVchObj[vchPath] = aliasObj;
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

bool collisionReference(const string& descriptor_alias, uint256& wtxInHash)
{

    bool found=false;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
                    continue;

                if(tx.IsSpent(nOut))
                {
                    continue;
                }

                const int nHeight = tx.GetHeightInMainChain();

                if(nHeight == -1)
                {
                    continue;
                }

                assert(nHeight >= 0);

                const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
                if(ex <= 0)
                {
                    continue;
                }

                string strAddress = "";
                aliasAddress(tx, strAddress);
                string alias = stringFromVch(vchPath);

                int pos=0;
                if(alias == descriptor_alias)
                {
                    found = true;
                    wtxInHash=tx.GetHash();
                    break;
                }
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return found;
}

bool searchPathEncrypted2(string alias, uint256& wtxInHash)
{
    bool found=false;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
                    continue;

                if(tx.IsSpent(nOut))
                    continue;

                const int nHeight = tx.GetHeightInMainChain();

                if(nHeight == -1)
                    continue;

                assert(nHeight >= 0);

                const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
                if(ex <= 0)
                    continue;

                string strAddress = "";
                aliasAddress(tx, strAddress);
                string decrypted;
                if(op__ == OP_ALIAS_ENCRYPTED)
                {
                    string rsaPrivKey;
                    cba r(strAddress);
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

                    DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);
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

bool searchPathEncrypted(string alias, uint256& wtxInHash)
{
    bool found=false;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
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
                    cba r(strAddress);
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

                    DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);
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

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;

    if(pwalletMain->as())
        return oRes;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
                    continue;

                Object aliasObj;


                if(!vchNodeLocator.empty() && vchNodeLocator != vchPath)
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
                    cba r(strAddress);
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
                            DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);
                            aliasObj.push_back(Pair("alias", decrypted));
                        }
                    }
                    else
                    {
                        for(unsigned int i=0; i < tx.vin.size(); i++)
                        {
                            COutPoint prevout = tx.vin[i].prevout;
                            __wx__Tx& txPrev = pwalletMain->mapWallet[prevout.hash];

                            CTxOut& out = txPrev.vout[prevout.n];

                            std::vector<vchType> vvchPrevArgsRead;
                            int prevOp;
                            if(aliasScript(out.scriptPubKey, prevOp, vvchPrevArgsRead))
                            {
                                string a__ = "";
                                aliasAddress(txPrev, a__);

                                cba r0(a__);
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

                    if(mapPathVchInt.find(vchFromString(decrypted)) != mapPathVchInt.end() && mapPathVchInt[vchFromString(decrypted)] > nHeight)
                    {
                        continue;
                    }

                    aliasObj.push_back(Pair("encrypted", "true"));
                    mapPathVchInt[vchFromString(decrypted)] = nHeight;
                }
                else
                {
                    if(mapPathVchInt.find(vchPath) != mapPathVchInt.end() && mapPathVchInt[vchPath] > nHeight)
                    {
                        continue;
                    }
                    string s = stringFromVch(vchPath);
                    aliasObj.push_back(Pair("alias", s));
                    aliasObj.push_back(Pair("encrypted", "false"));
                    Value v = xtu_url__(s);
                    aliasObj.push_back(Pair("xtu", v.get_real()));
                    mapPathVchInt[vchPath] = nHeight;
                    if(xs(s))
                    {
                        aliasObj.push_back(Pair("xstat", "true"));
                    }
                }

                if(!IsMinePost(tx))
                    aliasObj.push_back(Pair("transferred", 1));
                aliasObj.push_back(Pair("address", strAddress));
                aliasObj.push_back(Pair("nHeigt", nHeight));

                cba keyAddress(strAddress);
                CKeyID keyID;
                keyAddress.GetKeyID(keyID);
                CPubKey vchPubKey;
                pwalletMain->GetPubKey(keyID, vchPubKey);
                vchType vchRand;

                const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
                aliasObj.push_back(Pair("expires_in", ex));
                if(ex <= 0)
                    aliasObj.push_back(Pair("expired", 1));
                if(tunnelSwitch__(ex))
                    aliasObj.push_back(Pair("tunnel_switch", ex));

                if(mapState.count(vchPath) && mapState[vchPath].size())
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

                string bitSig = stringFromVch(vchValue);
                if(bitSig != "")
                    aliasObj.push_back(Pair("xtuVector", "transform"));

                if(op__ != OP_ALIAS_ENCRYPTED)
                    aliasMapVchObj[vchPath] = aliasObj;
                else
                    aliasMapVchObj[vchFromString(decrypted)] = aliasObj;

            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, aliasMapVchObj)
    oRes.push_back(item.second);

    return oRes;
}
Value lookupStoragePath(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
            "aliasList [<alias>]\n"
        );

    uint256 wtx__;
    const string storagePath = "NOT FOUND";
    collisionReference(storagePath, wtx__);

    Array oRes;
    oRes.push_back(storagePath);
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

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                __wx__Tx& tx = item.second;

                vchType vchPath, vchValue;
                int nOut;
                int op__=-1;
                if(!tx.aliasSet(op__, nOut, vchPath, vchValue))
                    continue;

                Object aliasObj;

                if(!vchNodeLocator.empty() && vchNodeLocator != vchPath)
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
                    cba r(strAddress);
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
                            DecryptMessage(rsaPrivKey, stringFromVch(vchPath), decrypted);
                            aliasObj.push_back(Pair("alias", decrypted));
                        }
                    }
                    else
                    {
                        for(unsigned int i=0; i < tx.vin.size(); i++)
                        {
                            COutPoint prevout = tx.vin[i].prevout;
                            __wx__Tx& txPrev = pwalletMain->mapWallet[prevout.hash];

                            CTxOut& out = txPrev.vout[prevout.n];

                            std::vector<vchType> vvchPrevArgsRead;
                            int prevOp;
                            if(aliasScript(out.scriptPubKey, prevOp, vvchPrevArgsRead))
                            {
                                string a__ = "";
                                aliasAddress(txPrev, a__);

                                cba r0(a__);
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

                    if(mapPathVchInt.find(vchFromString(decrypted)) != mapPathVchInt.end() && mapPathVchInt[vchFromString(decrypted)] > nHeight)
                    {
                        continue;
                    }

                    aliasObj.push_back(Pair("encrypted", "true"));
                    mapPathVchInt[vchFromString(decrypted)] = nHeight;
                }
                else
                {
                    if(mapPathVchInt.find(vchPath) != mapPathVchInt.end() && mapPathVchInt[vchPath] > nHeight)
                    {
                        continue;
                    }
                    aliasObj.push_back(Pair("alias", stringFromVch(vchPath)));
                    aliasObj.push_back(Pair("encrypted", "false"));
                    string s = stringFromVch(vchPath);
                    if(xs(s))
                    {
                        aliasObj.push_back(Pair("xstat", "true"));
                    }

                    mapPathVchInt[vchPath] = nHeight;
                }


                aliasObj.push_back(Pair("value", value));

                if(!IsMinePost(tx))
                    aliasObj.push_back(Pair("transferred", 1));
                aliasObj.push_back(Pair("address", strAddress));
                aliasObj.push_back(Pair("nHeigt", nHeight));


                cba keyAddress(strAddress);
                CKeyID keyID;
                keyAddress.GetKeyID(keyID);
                CPubKey vchPubKey;
                pwalletMain->GetPubKey(keyID, vchPubKey);
                vchType vchRand;

                const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
                aliasObj.push_back(Pair("expires_in", ex));
                if(ex <= 0)
                    aliasObj.push_back(Pair("expired", 1));
                if(tunnelSwitch__(ex))
                    aliasObj.push_back(Pair("tunnel_switch", ex));

                if(mapState.count(vchPath) && mapState[vchPath].size())
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
                    aliasMapVchObj[vchPath] = aliasObj;
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

    vector<unsigned char> vchPath = vchFromValue(params[0]);
    ENTER_CRITICAL_SECTION(cs_main)
    {

        vector<PathIndex> vtxPos;
        LocatorNodeDB aliasCacheDB("r");
        if(!aliasCacheDB.lGet(vchPath, vtxPos))
        {
            error("failed to read from alias DB");
            return false;
        }

        PathIndex txPos;
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
Value primaryCXValidate(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 3)
        throw runtime_error(
            "primaryCXValidate <alias> <predicate> <l1_internal>"
            + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();
    vchType vchLocator = vchFromString(locatorStr);

    cba predicate((params[1]).get_str());

    if(!predicate.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "predicate");

    CKeyID pid;
    if(!predicate.GetKeyID(pid))
        throw JSONRPCError(RPC_TYPE_ERROR, "pid");

    CKey pid_;
    if(!pwalletMain->GetKey(pid, pid_))
        throw JSONRPCError(RPC_WALLET_ERROR, "pid_");


    string extPredicate = (params[2]).get_str();
    cba externPredicate(extPredicate);
    if(!externPredicate.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPredicate = vchFromString(extPredicate);
        if (ln1Db.lKey (vchPredicate))
        {
            if (!ln1Db.lGet (vchPredicate, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            externPredicate.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin predicate");
        }
    }

    CKeyID epid;
    if(!externPredicate.GetKeyID(epid))
        throw JSONRPCError(RPC_TYPE_ERROR, "epid");

    string l0Str;
    string ext = externPredicate.ToString();
    if(!channelPredicate(ext, l0Str))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "local predicate");
    }

    cba localPredicate(l0Str);

    CKeyID l0id;
    if(!localPredicate.GetKeyID(l0id))
        throw JSONRPCError(RPC_TYPE_ERROR, "l0id");

    CKey l0id_;
    if(!pwalletMain->GetKey(l0id, l0id_))
        throw JSONRPCError(RPC_WALLET_ERROR, "l0id_");

    vchType vchRand;
    string r_;
    if(!pwalletMain->GetRandomKeyMetadata(pid_.GetPubKey(), vchRand, r_))
        throw JSONRPCError(RPC_WALLET_ERROR, "random key");

    CPubKey vchPubKey = pid_.GetPubKey();

    vchType rVch = vchFromString(externPredicate.ToString());

    vchType extVch;
    vchType cskVch;
    vector<unsigned char> aesRawVector;
    vector<unsigned char> cskVec;
    string f = externPredicate.ToString();
    if(!getImportedPubKey(localPredicate.ToString(), f, extVch, cskVch))
    {
        if(!internalReference__(f, extVch))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "external");

        string cskStr;
        if(pwalletMain->aes_(vchPubKey, f, cskStr))
        {
            bool fInvalid = false;
            cskVec = DecodeBase64(cskStr.c_str(), &fInvalid);
        }
        else
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "void internal relay");
    }
    else
    {
        string channelList = stringFromVch(cskVch);
        if(channelList == "I")
        {
            if(pwalletMain->aes_(l0id_.GetPubKey(), f, channelList))
            {
                bool fInvalid = false;
                cskVec = DecodeBase64(channelList.c_str(), &fInvalid);
            }
        }
        else
        {
            string p0;
            if(!pwalletMain->envCP0(l0id_.GetPubKey(), p0))
                throw JSONRPCError(RPC_TYPE_ERROR, "intrinsic");
            string openChannelStream;
            DecryptMessage(p0, channelList, openChannelStream);
            bool fl = false;
            cskVec = DecodeBase64(openChannelStream.c_str(), &fl);

        }
    }

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    scriptPubKeyOrig.SetBitcoinAddress(externPredicate.ToString());

    CScript scriptPubKey;

    Object ret;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        uint256 wtxInHash;
        if(!searchPathEncrypted2(stringFromVch(vchLocator), wtxInHash))
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("could not find this alias");
        }

        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchLocator) && mapState[vchLocator].size())
            {
                error("xfer encrypted: there are %lu pending operations on that alias, including %s",
                      mapState[vchLocator].size(),
                      mapState[vchLocator].begin()->GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("there are pending operations on that alias");
            }

            EnsureWalletIsUnlocked();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }


            string encryptedRandForRecipient;
            string randBase64 = EncodeBase64(&vchRand[0], vchRand.size());
            EncryptMessage(stringFromVch(extVch), randBase64, encryptedRandForRecipient);
            EncryptMessage(stringFromVch(extVch), randBase64, encryptedRandForRecipient);
            string encryptedPathForRecipient;
            EncryptMessage(stringFromVch(extVch), locatorStr, encryptedPathForRecipient);

            const __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            bool found = false;
            BOOST_FOREACH(const CTxOut& out, wtxIn.vout)
            {
                vector<vector<unsigned char> > vvch;
                int op;
                if(aliasScript(out.scriptPubKey, op, vvch))
                {
                    if(op != OP_ALIAS_ENCRYPTED)
                        throw runtime_error("previous was not OP_ALIAS_ENCRYPTED");

                    string gen;
                    string reference;
                    string iv128 = stringFromVch(vvch[6]);
                    string r = stringFromVch(vvch[5]);
                    State s__(stringFromVch(vvch[7]));
                    if(iv128 != "_")
                    {
                        if(s__() != State::ATOMIC)
                        {
                            string csK;
                            string l = stringFromVch(vvch[2]);
                            bool black_0=true;
                            if(!channel(l, f, csK, black_0))
                                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "invalid channel");
                            bool fInvalid = false;
                            vector<unsigned char> cskRaw = DecodeBase64(csK.c_str(), &fInvalid);
                            string decrypted;
                            DecryptMessageAES(stringFromVch(vvch[4]),
                                              decrypted,
                                              cskRaw,
                                              iv128);

                            EncryptMessageAES(decrypted, gen, cskVec, reference);
                            if(gen.size() > MAX_XUNIT_LENGTH)
                                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                        }
                        else
                        {
                            string csK;
                            Relay r;
                            if(pwalletMain->relay_(vchLocator, r))
                            {
                                string ctrl_ = r.ctrl_();
                                bool fInvalid = false;
                                vector<unsigned char> asK = DecodeBase64(ctrl_.c_str(), &fInvalid);
                                string decrypted;
                                DecryptMessageAES(stringFromVch(vvch[4]),
                                                  decrypted,
                                                  asK,
                                                  iv128);

                                EncryptMessageAES(decrypted, gen, cskVec, reference);
                                if(gen.size() > MAX_XUNIT_LENGTH)
                                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                            }
                        }
                    }

                    uint160 hash = uint160(vvch[3]);

                    CDataStream ss(SER_GETHASH, 0);
                    ss << encryptedPathForRecipient;
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
                        iv128 = reference;
                    }

                    ss << encryptedRandForRecipient;

                    vchType fs = vchFromString(localPredicate.ToString());

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
                        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

                    vector<unsigned char> vchSig;
                    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

                    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
                    scriptPubKey << OP_ALIAS_ENCRYPTED << vchFromString(encryptedPathForRecipient) << vchFromString(sigBase64) << rVch << vvch[3] << q1 << vchFromString(encryptedRandForRecipient) << vchFromString(iv128) << fs << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;

                    scriptPubKey += scriptPubKeyOrig;
                    found = true;
                    break;
                }
            }

            if(!found)
            {
                throw runtime_error("previous tx type is not alias");
            }

            int64_t t;
            string strError;
            bool s = txRelayPre__(scriptPubKey, wtxIn, wtx, t, strError);
            if(!s)
            {
                LEAVE_CRITICAL_SECTION(cs_main)
                ret.push_back(Pair("status", "error"));
                ret.push_back(Pair("message", strError));
            }
            ret.push_back(Pair("status", "ok"));
            ret.push_back(Pair("fee", ValueFromAmount(t)));

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
    return ret;
}
Value vEPID(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "vEPID [<node opt>]\n"
        );


    //VX series
    vchType vchNodeLocator;
    string k =(params[0]).get_str();
    cba k1(k);
    if(!k1.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");
    const char* out__ = (params[1].get_str()).c_str();

    fs::path p = out__;
    boost::filesystem::path ve = p.parent_path();
    if(!fs::exists(ve))
        throw runtime_error("Invalid out put path");

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    vector< vector<unsigned char> > vvch;
    bool found=false;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

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
                    cba r(strAddress);
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
                    if(mapPathVchInt.find(vchFromString(decrypted)) != mapPathVchInt.end() && mapPathVchInt[vchFromString(decrypted)] > nHeight)
                    {
                        continue;
                    }
                    mapPathVchInt[vchFromString(decrypted)] = nHeight;

                    DecryptMessage(rsaPrivKey, stringFromVch(vv[0]), decrypted);
                    if(k1.ToString() != r.ToString())
                    {
                        continue;
                    }
                    else
                    {
                        printf(">>>> decrypted %s\n", decrypted.c_str());
                        vchNodeLocator = vchFromString(decrypted);
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
                vector<unsigned char> asK = DecodeBase64(ctrl_.c_str(), &fInvalid);
                string decrypted;
                DecryptMessageAES(stringFromVch(vvch[4]),
                                  decrypted,
                                  asK,
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
            string csKStr;
            string localAddr = stringFromVch(vchNodeLocator);
            string f = stringFromVch(vvch[7]);
            string l = stringFromVch(vvch[2]);
            bool black_0=true;
            if(channel(l, f, csKStr, black_0))
            {
                string alpha;
                if(black_0)
                {
                    cba p(l);
                    if(!p.IsValid())
                        throw JSONRPCError(RPC_TYPE_ERROR, "address");
                    CKeyID k;
                    if(!p.GetKeyID(k))
                        throw JSONRPCError(RPC_TYPE_ERROR, "key");

                    CKey k_;
                    if(!pwalletMain->GetKey(k, k_))
                        throw JSONRPCError(RPC_TYPE_ERROR, "not found");

                    CPubKey pk = k_.GetPubKey();
                    string q;
                    pwalletMain->envCP0(pk, q);
                    DecryptMessage(q, csKStr, alpha);
                    csKStr=alpha;
                }

                bool fInvalid = false;
                vector<unsigned char> csK = DecodeBase64(csKStr.c_str(), &fInvalid);
                string decrypted;
                DecryptMessageAES(stringFromVch(vvch[4]),
                                  decrypted,
                                  csK,
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
Value validate(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "validate [<node opt>]\n"
        );

    string k1;
    vchType vchNodeLocator;
    k1 =(params[0]).get_str();
    vchNodeLocator = vchFromValue(params[0]);
    const char* out__ = (params[1].get_str()).c_str();

    fs::path p = out__;
    boost::filesystem::path ve = p.parent_path();
    if(!fs::exists(ve))
        throw runtime_error("Invalid out put path");

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    vector< vector<unsigned char> > vvch;
    bool found=false;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

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
                    cba r(strAddress);
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
                    if(mapPathVchInt.find(vchFromString(decrypted)) != mapPathVchInt.end() && mapPathVchInt[vchFromString(decrypted)] > nHeight)
                    {
                        continue;
                    }
                    mapPathVchInt[vchFromString(decrypted)] = nHeight;

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
                vector<unsigned char> asK = DecodeBase64(ctrl_.c_str(), &fInvalid);
                string decrypted;
                DecryptMessageAES(stringFromVch(vvch[4]),
                                  decrypted,
                                  asK,
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
            string csKStr;
            string localAddr = stringFromVch(vchNodeLocator);
            string f = stringFromVch(vvch[7]);
            string l = stringFromVch(vvch[2]);
            bool black_0=true;
            if(channel(l, f, csKStr, black_0))
            {
                string alpha;
                if(black_0)
                {
                    cba p(l);
                    if(!p.IsValid())
                        throw JSONRPCError(RPC_TYPE_ERROR, "address");
                    CKeyID k;
                    if(!p.GetKeyID(k))
                        throw JSONRPCError(RPC_TYPE_ERROR, "key");

                    CKey k_;
                    if(!pwalletMain->GetKey(k, k_))
                        throw JSONRPCError(RPC_TYPE_ERROR, "not found");

                    CPubKey pk = k_.GetPubKey();
                    string q;
                    pwalletMain->envCP0(pk, q);
                    DecryptMessage(q, csKStr, alpha);
                    csKStr=alpha;
                }

                bool fInvalid = false;
                vector<unsigned char> csK = DecodeBase64(csKStr.c_str(), &fInvalid);
                string decrypted;
                DecryptMessageAES(stringFromVch(vvch[4]),
                                  decrypted,
                                  csK,
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

    const vchType vchPath = vchFromString(locatorStr);

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


    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchPath) && mapState[vchPath].size())
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
        if(aliasTx(aliasCacheDB, vchPath, tx))
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
        if(!searchPathEncrypted2(stringFromVch(vchPath), wtxInHash))
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


        __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
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

                cba ownerAddr = script.GetBitcoinAddress();
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

Value updateEncryptedPathFile(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "updateEncryptedPath <locator> <file>"
            + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();

    const vchType vchPath = vchFromString(locatorStr);

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

    int CRC__KEY = relay_inv(INTERN_REF0__, EXTERN_REF0__);

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchPath) && mapState[vchPath].size())
        {
            error("updateEncryptedPath() : there are %lu pending operations on that alias, including %s",
                  mapState[vchPath].size(),
                  mapState[vchPath].begin()->GetHex().c_str());
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchPath, tx))
        {
            error("updateEncryptedPath() : this alias is already active with tx %s",
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

        CScript scriptPubKey;



        __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
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

                cba ownerAddr = script.GetBitcoinAddress();
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
Value updateEncryptedPath(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 3)
        throw runtime_error(
            "updateEncryptedPath <alias> <value> <address>"
            + HelpRequiringPassphrase());

    const vchType vchPath = vchFromString(params[0].get_str());
    vchType vchValue = vchFromString(params[1].get_str());

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchPath) && mapState[vchPath].size())
        {
            error("updateEncryptedPath() : there are %lu pending operations on that alias, including %s",
                  mapState[vchPath].size(),
                  mapState[vchPath].begin()->GetHex().c_str());
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchPath, tx))
        {
            error("updateEncryptedPath() : this alias is already active with tx %s",
                  tx.GetHash().GetHex().c_str());
            throw runtime_error("this alias is already active");
        }
    }

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        uint256 wtxInHash;
        if(!searchPathEncrypted(stringFromVch(vchPath), wtxInHash))
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

        CScript scriptPubKey;


        __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
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

                cba ownerAddr = script.GetBitcoinAddress();
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
bool decrypt_ra(const string& alias, const string& address, __wx__Tx& ra_tx, __wx__Tx& decrypt_ra_tx)
{
    string locatorStr = alias;
    const vchType vchPath = vchFromValue(locatorStr);
    const std::string addressOfOwner = address;

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


    //__wx__Tx wtx;
    decrypt_ra_tx.nVersion = CTransaction::DION_TX_VERSION;

    //ENTER_CRITICAL_SECTION(cs_main)
    //{
    //  if(mapState.count(vchPath) && mapState[vchPath].size())
    // {
    //    error("decryptPath() : there are %lu pending operations on that alias, including %s",
    //           mapState[vchPath].size(),
    //          mapState[vchPath].begin()->GetHex().c_str());
    // LEAVE_CRITICAL_SECTION(cs_main)
    //        throw runtime_error("there are pending operations on that alias");
    //   }
    //}
    //LEAVE_CRITICAL_SECTION(cs_main)

    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchPath, tx))
        {
            error("decryptPath() : this alias is already active with tx %s",
                  tx.GetHash().GetHex().c_str());
            throw runtime_error("this alias is already active");
        }
    }


    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        //   uint256 wtxInHash;
        //if(!searchPathEncrypted2(stringFromsch(vchPath), wtxInHash))
        //{
        //LEAVE_CRITICAL_SECTION(cs_main)
        //     throw runtime_error("could not find a coin with this alias, try specifying the registerPath transaction id");
        //  }

//        if(!pwalletMain->mapWallet.count(wtxInHash))
//       {
        //  LEAVE_CRITICAL_SECTION(cs_main)
        //         throw runtime_error("previous transaction is not in the wallet");
        //    }

        CScript scriptPubKeyOrig;
        scriptPubKeyOrig.SetBitcoinAddress(addressOfOwner);

        CScript scriptPubKey;


        //  __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        //   const int nHeight = wtxIn.GetHeightInMainChain();
        //  const int ex = nHeight + scaleMonitor() - pindexBest->nHeight;
        // if(ex <= 0)
        //   {
        //    LEAVE_CRITICAL_SECTION(cs_main)
        //   throw runtime_error("this encrypted alias is expired. You must create a new one of that name to decrypt it.");
        // }

        vector<unsigned char> vchPrevSig;
        bool found = false;
        BOOST_FOREACH(CTxOut& out, ra_tx.vout)
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

            }
        }

        string strError = txRelay_no_commit(scriptPubKey, CTRL__, ra_tx, decrypt_ra_tx, false);
        if(strError != "")
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return true;
}

Value decryptPath(const Array& params, bool fHelp)
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
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchPath) && mapState[vchPath].size())
        {
            error("decryptPath() : there are %lu pending operations on that alias, including %s",
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
            error("decryptPath() : this alias is already active with tx %s",
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

Value transferEncryptedExtPredicate(const Array& params, bool fHelp)
{
    if(fHelp || params.size() < 3 || params.size() > 4)
        throw runtime_error(
            "transferEncryptedExtPredicate <alias> <predicate> <l0> <l1>"
            + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();
    vchType vchLocator = vchFromString(locatorStr);

    cba predicate((params[1]).get_str());

    if(!predicate.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "predicate");

    CKeyID pid;
    if(!predicate.GetKeyID(pid))
        throw JSONRPCError(RPC_TYPE_ERROR, "pid");

    CKey pid_;
    if(!pwalletMain->GetKey(pid, pid_))
        throw JSONRPCError(RPC_WALLET_ERROR, "pid_");

    cba localPredicate((params[2]).get_str());
    if(!localPredicate.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "local predicate");

    CKeyID l0id;
    if(!localPredicate.GetKeyID(l0id))
        throw JSONRPCError(RPC_TYPE_ERROR, "l0id");

    CKey l0id_;
    if(!pwalletMain->GetKey(l0id, l0id_))
        throw JSONRPCError(RPC_WALLET_ERROR, "l0id_");

    cba externPredicate((params[3]).get_str());
    if(!externPredicate.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "externPredicate");

    CKeyID epid;
    if(!externPredicate.GetKeyID(epid))
        throw JSONRPCError(RPC_TYPE_ERROR, "epid");

    vchType vchRand;
    string r_;
    if(!pwalletMain->GetRandomKeyMetadata(pid_.GetPubKey(), vchRand, r_))
        throw JSONRPCError(RPC_WALLET_ERROR, "random key");

    CPubKey vchPubKey = pid_.GetPubKey();

    vchType rVch = vchFromString(externPredicate.ToString());




    vchType extVch;
    vchType cskVch;
    vector<unsigned char> aesRawVector;
    vector<unsigned char> cskVec;
    string f = externPredicate.ToString();
    if(!getImportedPubKey(localPredicate.ToString(), f, extVch, cskVch))
    {
        if(!internalReference__(f, extVch))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "external");

        string cskStr;
        if(pwalletMain->aes_(vchPubKey, f, cskStr))
        {
            bool fInvalid = false;
            cskVec = DecodeBase64(cskStr.c_str(), &fInvalid);
        }
        else
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "void internal relay");
    }
    else
    {
        string channelList = stringFromVch(cskVch);
        if(channelList == "I")
        {
            if(pwalletMain->aes_(l0id_.GetPubKey(), f, channelList))
            {
                bool fInvalid = false;
                cskVec = DecodeBase64(channelList.c_str(), &fInvalid);
            }
        }
        else
        {
            string p0;
            if(!pwalletMain->envCP0(l0id_.GetPubKey(), p0))
                throw JSONRPCError(RPC_TYPE_ERROR, "intrinsic");
            string openChannelStream;
            DecryptMessage(p0, channelList, openChannelStream);
            bool fl = false;
            cskVec = DecodeBase64(openChannelStream.c_str(), &fl);

        }
    }

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    scriptPubKeyOrig.SetBitcoinAddress(externPredicate.ToString());

    CScript scriptPubKey;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        uint256 wtxInHash;
        if(!searchPathEncrypted(stringFromVch(vchLocator), wtxInHash))
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("could not find this alias");
        }

        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchLocator) && mapState[vchLocator].size())
            {
                error("xfer encrypted: there are %lu pending operations on that alias, including %s",
                      mapState[vchLocator].size(),
                      mapState[vchLocator].begin()->GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("there are pending operations on that alias");
            }

            EnsureWalletIsUnlocked();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }


            string encryptedRandForRecipient;
            string randBase64 = EncodeBase64(&vchRand[0], vchRand.size());
            EncryptMessage(stringFromVch(extVch), randBase64, encryptedRandForRecipient);
            EncryptMessage(stringFromVch(extVch), randBase64, encryptedRandForRecipient);
            string encryptedPathForRecipient;
            EncryptMessage(stringFromVch(extVch), locatorStr, encryptedPathForRecipient);

            const __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            bool found = false;
            BOOST_FOREACH(const CTxOut& out, wtxIn.vout)
            {
                vector<vector<unsigned char> > vvch;
                int op;
                if(aliasScript(out.scriptPubKey, op, vvch))
                {
                    if(op != OP_ALIAS_ENCRYPTED)
                        throw runtime_error("previous was not OP_ALIAS_ENCRYPTED");

                    string gen;
                    string reference;
                    string iv128 = stringFromVch(vvch[6]);
                    string r = stringFromVch(vvch[5]);
                    State s__(stringFromVch(vvch[7]));
                    if(iv128 != "_")
                    {
                        if(s__() != State::ATOMIC)
                        {
                            string csK;
                            string l = stringFromVch(vvch[2]);
                            bool black_0=true;
                            if(!channel(l, f, csK, black_0))
                                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "invalid channel");
                            bool fInvalid = false;
                            vector<unsigned char> cskRaw = DecodeBase64(csK.c_str(), &fInvalid);
                            string decrypted;
                            DecryptMessageAES(stringFromVch(vvch[4]),
                                              decrypted,
                                              cskRaw,
                                              iv128);

                            EncryptMessageAES(decrypted, gen, cskVec, reference);
                            if(gen.size() > MAX_XUNIT_LENGTH)
                                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                        }
                        else
                        {
                            string csK;
                            Relay r;
                            if(pwalletMain->relay_(vchLocator, r))
                            {
                                string ctrl_ = r.ctrl_();
                                bool fInvalid = false;
                                vector<unsigned char> asK = DecodeBase64(ctrl_.c_str(), &fInvalid);
                                string decrypted;
                                DecryptMessageAES(stringFromVch(vvch[4]),
                                                  decrypted,
                                                  asK,
                                                  iv128);

                                EncryptMessageAES(decrypted, gen, cskVec, reference);
                                if(gen.size() > MAX_XUNIT_LENGTH)
                                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                            }
                        }
                    }

                    uint160 hash = uint160(vvch[3]);

                    CDataStream ss(SER_GETHASH, 0);
                    ss << encryptedPathForRecipient;
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
                        iv128 = reference;
                    }

                    ss << encryptedRandForRecipient;

                    vchType fs = vchFromString(localPredicate.ToString());

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
                        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

                    vector<unsigned char> vchSig;
                    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

                    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
                    scriptPubKey << OP_ALIAS_ENCRYPTED << vchFromString(encryptedPathForRecipient) << vchFromString(sigBase64) << rVch << vvch[3] << q1 << vchFromString(encryptedRandForRecipient) << vchFromString(iv128) << fs << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;

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
Value transferEncryptedPath(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 3)
        throw runtime_error(
            "transferEncryptedPath <alias> <predicate> <l1_internal>"
            + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();
    vchType vchLocator = vchFromString(locatorStr);

    cba predicate((params[1]).get_str());

    if(!predicate.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "predicate");

    CKeyID pid;
    if(!predicate.GetKeyID(pid))
        throw JSONRPCError(RPC_TYPE_ERROR, "pid");

    CKey pid_;
    if(!pwalletMain->GetKey(pid, pid_))
        throw JSONRPCError(RPC_WALLET_ERROR, "pid_");


    string extPredicate = (params[2]).get_str();
    cba externPredicate(extPredicate);
    if(!externPredicate.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPredicate = vchFromString(extPredicate);
        if (ln1Db.lKey (vchPredicate))
        {
            if (!ln1Db.lGet (vchPredicate, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            externPredicate.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin predicate");
        }
    }

    CKeyID epid;
    if(!externPredicate.GetKeyID(epid))
        throw JSONRPCError(RPC_TYPE_ERROR, "epid");

    string l0Str;
    string ext = externPredicate.ToString();
    if(!channelPredicate(ext, l0Str))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "local predicate");
    }

    cba localPredicate(l0Str);

    CKeyID l0id;
    if(!localPredicate.GetKeyID(l0id))
        throw JSONRPCError(RPC_TYPE_ERROR, "l0id");

    CKey l0id_;
    if(!pwalletMain->GetKey(l0id, l0id_))
        throw JSONRPCError(RPC_WALLET_ERROR, "l0id_");

    vchType vchRand;
    string r_;
    if(!pwalletMain->GetRandomKeyMetadata(pid_.GetPubKey(), vchRand, r_))
        throw JSONRPCError(RPC_WALLET_ERROR, "random key");

    CPubKey vchPubKey = pid_.GetPubKey();

    vchType rVch = vchFromString(externPredicate.ToString());

    vchType extVch;
    vchType cskVch;
    vector<unsigned char> aesRawVector;
    vector<unsigned char> cskVec;
    string f = externPredicate.ToString();
    if(!getImportedPubKey(localPredicate.ToString(), f, extVch, cskVch))
    {
        if(!internalReference__(f, extVch))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "external");

        string cskStr;
        if(pwalletMain->aes_(vchPubKey, f, cskStr))
        {
            bool fInvalid = false;
            cskVec = DecodeBase64(cskStr.c_str(), &fInvalid);
        }
        else
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "void internal relay");
    }
    else
    {
        string channelList = stringFromVch(cskVch);
        if(channelList == "I")
        {
            if(pwalletMain->aes_(l0id_.GetPubKey(), f, channelList))
            {
                bool fInvalid = false;
                cskVec = DecodeBase64(channelList.c_str(), &fInvalid);
            }
        }
        else
        {
            string p0;
            if(!pwalletMain->envCP0(l0id_.GetPubKey(), p0))
                throw JSONRPCError(RPC_TYPE_ERROR, "intrinsic");
            string openChannelStream;
            DecryptMessage(p0, channelList, openChannelStream);
            bool fl = false;
            cskVec = DecodeBase64(openChannelStream.c_str(), &fl);

        }
    }

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    scriptPubKeyOrig.SetBitcoinAddress(externPredicate.ToString());

    CScript scriptPubKey;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        uint256 wtxInHash;
        if(!searchPathEncrypted2(stringFromVch(vchLocator), wtxInHash))
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("could not find this alias");
        }

        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchLocator) && mapState[vchLocator].size())
            {
                error("xfer encrypted: there are %lu pending operations on that alias, including %s",
                      mapState[vchLocator].size(),
                      mapState[vchLocator].begin()->GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("there are pending operations on that alias");
            }

            EnsureWalletIsUnlocked();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }


            string encryptedRandForRecipient;
            string randBase64 = EncodeBase64(&vchRand[0], vchRand.size());
            EncryptMessage(stringFromVch(extVch), randBase64, encryptedRandForRecipient);
            EncryptMessage(stringFromVch(extVch), randBase64, encryptedRandForRecipient);
            string encryptedPathForRecipient;
            EncryptMessage(stringFromVch(extVch), locatorStr, encryptedPathForRecipient);

            const __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            bool found = false;
            BOOST_FOREACH(const CTxOut& out, wtxIn.vout)
            {
                vector<vector<unsigned char> > vvch;
                int op;
                if(aliasScript(out.scriptPubKey, op, vvch))
                {
                    if(op != OP_ALIAS_ENCRYPTED)
                        throw runtime_error("previous was not OP_ALIAS_ENCRYPTED");

                    string gen;
                    string reference;
                    string iv128 = stringFromVch(vvch[6]);
                    string r = stringFromVch(vvch[5]);
                    State s__(stringFromVch(vvch[7]));
                    if(iv128 != "_")
                    {
                        if(s__() != State::ATOMIC)
                        {
                            string csK;
                            string l = stringFromVch(vvch[2]);
                            bool black_0=true;
                            if(!channel(l, f, csK, black_0))
                                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "invalid channel");
                            bool fInvalid = false;
                            vector<unsigned char> cskRaw = DecodeBase64(csK.c_str(), &fInvalid);
                            string decrypted;
                            DecryptMessageAES(stringFromVch(vvch[4]),
                                              decrypted,
                                              cskRaw,
                                              iv128);

                            EncryptMessageAES(decrypted, gen, cskVec, reference);
                            if(gen.size() > MAX_XUNIT_LENGTH)
                                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                        }
                        else
                        {
                            string csK;
                            Relay r;
                            if(pwalletMain->relay_(vchLocator, r))
                            {
                                string ctrl_ = r.ctrl_();
                                bool fInvalid = false;
                                vector<unsigned char> asK = DecodeBase64(ctrl_.c_str(), &fInvalid);
                                string decrypted;
                                DecryptMessageAES(stringFromVch(vvch[4]),
                                                  decrypted,
                                                  asK,
                                                  iv128);

                                EncryptMessageAES(decrypted, gen, cskVec, reference);
                                if(gen.size() > MAX_XUNIT_LENGTH)
                                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "xunit size exceeded");
                            }
                        }
                    }

                    uint160 hash = uint160(vvch[3]);

                    CDataStream ss(SER_GETHASH, 0);
                    ss << encryptedPathForRecipient;
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
                        iv128 = reference;
                    }

                    ss << encryptedRandForRecipient;

                    vchType fs = vchFromString(localPredicate.ToString());

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
                        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

                    vector<unsigned char> vchSig;
                    if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
                        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

                    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
                    scriptPubKey << OP_ALIAS_ENCRYPTED << vchFromString(encryptedPathForRecipient) << vchFromString(sigBase64) << rVch << vvch[3] << q1 << vchFromString(encryptedRandForRecipient) << vchFromString(iv128) << fs << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;

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
Value transferPath(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "transferPath <alias> <toaddress>"
            + HelpRequiringPassphrase());

    vchType vchPath = vchFromValue(params[0]);
    const vchType vchAddress = vchFromValue(params[1]);

    string locatorStr = stringFromVch(vchPath);
    string addressStr = stringFromVch(vchAddress);


    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    string strAddress = params[1].get_str();
    cba address(strAddress);
    if(!address.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(strAddress);
        if (ln1Db.lKey (vchPath))
        {
            if (!ln1Db.lGet (vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            address.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias 1");
        }
    }


    scriptPubKeyOrig.SetBitcoinAddress(address.ToString());

    CScript scriptPubKey;

    scriptPubKey << OP_ALIAS_RELAY << vchPath ;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updateEncryptedPath() : there are %lu pending operations on that alias, including %s",
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

            uint256 wtxInHash = tx.GetHash();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("updateEncryptedPath() : this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }

            const __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            int op__;
            int nOut;
            vchType vchValue;
            wtxIn.aliasSet(op__, nOut, vchPath, vchValue);

            scriptPubKey << vchValue << OP_2DROP << OP_DROP;
            scriptPubKey += scriptPubKeyOrig;

            string locatorStr = stringFromVch(vchPath);
            string indexStr = stringFromVch(vchValue);
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
Value uC(const Array& params, bool fHelp) {
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "uC <url> <value>"
            + HelpRequiringPassphrase());
    string locatorStr = params[0].get_str();

    const vchType vchPath = vchFromValue(locatorStr);
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

        __wx__DB walletdb(pwalletMain->strWalletFile, "r+");

        pwalletMain->LoadRelay(vchPath, r);
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

    const vchType vchValue = vchFromString(gen);

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        if(mapState.count(vchPath) && mapState[vchPath].size())
        {
            error("updateEncryptedPath() : there are %lu pending operations on that alias, including %s",
                  mapState[vchPath].size(),
                  mapState[vchPath].begin()->GetHex().c_str());
            LEAVE_CRITICAL_SECTION(cs_main)
            throw runtime_error("there are pending operations on that alias");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    string ownerAddrStr;
    {
        LocatorNodeDB aliasCacheDB("r");
        CTransaction tx;
        if(aliasTx(aliasCacheDB, vchPath, tx))
        {
            error("updateEncryptedPath() : this alias is already active with tx %s",
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

        CScript scriptPubKey;


        __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
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

                cba ownerAddr = script.GetBitcoinAddress();
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

    const vchType vchPath = vchFromValue(locatorStr);
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


    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    CScript scriptPubKey;
    scriptPubKey << OP_ALIAS_RELAY << vchPath << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
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
            if(!aliasTx(aliasCacheDB, vchPath, tx))
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

            __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
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
Value updatePathFile(const Array& params, bool fHelp)
{
    if(fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "updatePath <alias> <value> [<toaddress>] update or transfer"
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
    if(s.size() > MAX_XUNIT_LENGTH)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Locator file too large");

    const vchType vchValue = vchFromValue(s);


    __wx__Tx wtx;
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
    scriptPubKey << OP_ALIAS_RELAY << vchPath << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath() : there are %lu pending operations on that alias, including %s",
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
                throw runtime_error("could not find a coin with this alias 4");
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
                error("updatePath() : this coin is not in your wallet %s",
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

__wx__Tx createRelayDescriptor(const string& origin)
{
    uint256 wtx__;
    string locatorStr = "descriptor_";
    locatorStr += origin;
    const vchType vchPath = vchFromValue(locatorStr);

    std::time_t t = std::time(NULL);
    ostringstream oss;
    oss << t;
    const vchType vchValue = vchFromValue(oss.str());

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    CScript scriptPubKey;
    scriptPubKey << OP_ALIAS_RELAY << vchPath << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath() : there are %lu pending operations on that alias, including %s",
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
                throw runtime_error("could not find a coin with this alias 5 " + stringFromVch(vchPath));
            }

            uint256 wtxInHash = tx.GetHash();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("updatePath() : this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }

            //if(params.size() == 2)
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

            __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            scriptPubKey += scriptPubKeyOrig;
            string strError = txRelay_no_commit(scriptPubKey, CTRL__, wtxIn, wtx, false);
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
    return wtx;
}

bool validate_serial_n(const string& origin, const string& data, __wx__Tx& serial_n)
{
    string locatorStr = "descriptor_" + origin;
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

    serial_n.nVersion = CTransaction::DION_TX_VERSION;

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

        string strError = pwalletMain->generateSM__(scriptPubKey, CTRL__, serial_n, false);

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

bool generate_ra_plain(const string& origin, __wx__Tx& ra)
{
    string locatorStr = "descriptor_" + origin;

    uint256 wtxInHash__;

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

    //__wx__Tx wtx;
    //wtx.nVersion = CTransaction::DION_TX_VERSION;
    ra.nVersion = CTransaction::DION_TX_VERSION;

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

        string strError = pwalletMain->generateSM__(scriptPubKey, CTRL__, ra, false);

        if(strError != "")
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        mapLocator[vchPath] = ra.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return true;
}

bool generate_ra_tested_encrypted(const string& origin, __wx__Tx& ra)
{
    string locatorStr = "descriptor_" + origin;

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

    //__wx__Tx wtx;
    //wtx.nVersion = CTransaction::DION_TX_VERSION;
    ra.nVersion = CTransaction::DION_TX_VERSION;

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

        string strError = pwalletMain->generateSM__(scriptPubKey, CTRL__, ra, false);

        if(strError != "")
        {
            LEAVE_CRITICAL_SECTION(cs_main)
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        mapLocator[vchPath] = ra.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    return true;
}

__wx__Tx generateUpdate(const string& origin)
{
    uint256 wtx__;
    string locatorStr = "descriptor_";
    locatorStr += origin;
    const vchType vchPath = vchFromValue(locatorStr);

    std::time_t t = std::time(NULL);
    ostringstream oss;
    oss << t;
    const vchType vchValue = vchFromValue(oss.str());

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    CScript scriptPubKey;
    scriptPubKey << OP_ALIAS_RELAY << vchPath << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath() : there are %lu pending operations on that alias, including %s",
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
                throw runtime_error("could not find a coin with this alias 5 " + stringFromVch(vchPath));
            }

            uint256 wtxInHash = tx.GetHash();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("updatePath() : this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }

            //if(params.size() == 2)
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

            __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            scriptPubKey += scriptPubKeyOrig;
            string strError = txRelay_no_commit(scriptPubKey, CTRL__, wtxIn, wtx, false);
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
    return wtx;
}

Value updatePath(const Array& params, bool fHelp)
{
    if(fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "updatePath <alias> <value> [<toaddress>]\nUpdate and possibly transfer a alias"
            + HelpRequiringPassphrase());
    string locatorStr = params[0].get_str();
    const vchType vchPath = vchFromValue(locatorStr);
    const vchType vchValue = vchFromValue(params[1]);

    __wx__Tx wtx;
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
    scriptPubKey << OP_ALIAS_RELAY << vchPath << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath() : there are %lu pending operations on that alias, including %s",
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
                throw runtime_error("could not find a coin with this alias 6");
            }

            uint256 wtxInHash = tx.GetHash();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("updatePath() : this coin is not in your wallet %s",
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


Value publicKey(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
            "publicKey <public_address>"
            + HelpRequiringPassphrase());

    EnsureWalletIsUnlocked();

    __wx__DB walletdb(pwalletMain->strWalletFile, "r+");

    string myAddress = params[0].get_str();

    cba addr(myAddress);
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
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed set");

    if(!walletdb.UpdateKey(pubKey, pwalletMain->kd[pubKey.GetID()]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

    if(!pwalletMain->envCP1(pubKey, testKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed load alpha");

    string pKey;
    if(!pwalletMain->envCP0(pubKey, pKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed load beta");

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

    cba addr;
    int r = checkAddress(myAddress, addr);
    if(r<0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if(!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    cba aRecipient;
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

    __wx__Tx wtx;
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
bool tunnelSwitch__(int r)
{
    if(!fTestNet)
        return r < INTERN_REF0__;

    return r < EXTERN_REF0__;
}

bool internalReference__(string ref__, vchType& recipientPubKeyVch)
{
    bool s__=false;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            cba a(ref__) ;
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
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

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
bool getImportedPubKey(string myAddress, string fKey, vchType& recipientPubKeyVch, vchType& aesKeyBase64EncryptedVch, bool& transientThreshold)
{
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchSender, vchRecipient, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.GetPublicKeyUpdate(nOut, vchSender, vchRecipient, vchKey, vchAes, vchSig))
                    continue;

                string keyRecipientAddr = stringFromVch(vchRecipient);
                string ext = stringFromVch(vchSender);
                if(keyRecipientAddr == myAddress && ext == fKey )
                {
                    recipientPubKeyVch = vchKey;
                    aesKeyBase64EncryptedVch = vchAes;

                    string a = stringFromVch(vchAes);
                    if(a == "I") transientThreshold = true;
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
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchSender, vchRecipient, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.GetPublicKeyUpdate(nOut, vchSender, vchRecipient, vchKey, vchAes, vchSig))
                    continue;

                string keyRecipientAddr = stringFromVch(vchRecipient);
                string ext = stringFromVch(vchSender);
                if(keyRecipientAddr == myAddress && ext == fKey )
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

int checkAddress(string addr, cba& a)
{
    cba address(addr);
    if(!address.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(addr);
        if(ln1Db.lKey(vchPath))
        {
            if(!ln1Db.lGet(vchPath, vtxPos))
                return -2;
            if(vtxPos.empty())
                return -3;

            PathIndex& txPos = vtxPos.back();
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

    cba addr;
    int r = checkAddress(myAddress, addr);
    if(r < 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    cba aRecipient;
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

    __wx__Tx wtx;
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

        __wx__DB walletdb(pwalletMain->strWalletFile, "r+");
        if(!pwalletMain->aes(vchPubKey, f, aesKeyStr))
            throw JSONRPCError(RPC_TYPE_ERROR, "Failed to set meta data for key");

        if(!walletdb.UpdateKey(vchPubKey, pwalletMain->kd[vchPubKey.GetID()]))
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

    cba senderAddr(myAddress);
    if(!senderAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address");

    CKeyID keyID;
    if(!senderAddr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "senderAddr does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    cba recipientAddr(f);
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

    __wx__Tx wtx;
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

    cba recipientAddr(f);
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
        cba senderAddr(myAddress);
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

    __wx__Tx wtx;
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
    cba ownerAddr(address);
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

    cba addr(address);
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
bool read_serial_n(const string& origin, const string& data, __wx__Tx& serial_n)
{
    uint256 wtx__;
    string locatorStr = "descriptor_";
    locatorStr += origin;
    const vchType vchPath = vchFromValue(locatorStr);

    const vchType vchValue = vchFromValue(data);

    serial_n.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    CScript scriptPubKey;
    scriptPubKey << OP_BASE_RELAY << vchPath << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath() : there are %lu pending operations on that alias, including %s",
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
                throw runtime_error("could not find a coin with this alias 5 " + stringFromVch(vchPath));
            }

            uint256 wtxInHash = tx.GetHash();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("updatePath() : this coin is not in your wallet %s",
                      wtxInHash.GetHex().c_str());
                LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
                LEAVE_CRITICAL_SECTION(cs_main)
                throw runtime_error("this coin is not in your wallet");
            }

            //if(params.size() == 2)
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

            __wx__Tx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            scriptPubKey += scriptPubKeyOrig;
            string strError = txRelay_no_commit(scriptPubKey, CTRL__, wtxIn, serial_n, false);
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
    return true;
}
Value updateDataNode(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "createDataNode <alias> <data>"
            + HelpRequiringPassphrase());
    string locatorStr = params[0].get_str();
    const vchType vchPath = vchFromValue(locatorStr);
    const vchType vchValue = vchFromValue(params[1]);

    __wx__Tx wtx;
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
    scriptPubKey << OP_BASE_RELAY << vchPath << vchValue << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath() : there are %lu pending operations on that alias, including %s",
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
                throw runtime_error("could not find a coin with this alias 6");
            }

            uint256 wtxInHash = tx.GetHash();

            if(!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("updatePath() : this coin is not in your wallet %s",
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
Value createDataNode(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "createDataNode <alias> <data>"
            + HelpRequiringPassphrase());

    string locatorStr = params[0].get_str();
    string indexStr = params[1].get_str();

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

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

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
Value registerPathGenerate(const Array& params, bool fHelp)
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
    wtx.nVersion = CTransaction::DION_TX_VERSION;

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
Value registerPath(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "registerPath <alias> <address>"
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

    const vchType vchPath = vchFromValue(locatorStr);
    vchToHash.insert(vchToHash.end(), vchPath.begin(), vchPath.end());
    const uint160 hash = Hash160(vchToHash);

    uint160 hash160;
    bool isValid = AddressToHash160(strAddress, hash160);
    if(!isValid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                           "Invalid dions address");

    Encode__N graphN;
    int h = 0;
    const char* l = locatorStr.c_str();
    while (*l)
        h = h << 1 ^ *l++;

    if(!graphN.conformal(h))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                           "external file index");


    cba keyAddress(strAddress);
    CKeyID keyID;
    keyAddress.GetKeyID(keyID);
    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);
    string pub_k;
    if(!pwalletMain->envCP1(vchPubKey, pub_k))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address has no associated RSA keys");

    if(!pwalletMain->SetRandomKeyMetadata(vchPubKey, vchRand))
        throw JSONRPCError(RPC_WALLET_ERROR, "Failed to set meta data for key");

    __wx__DB walletdb(pwalletMain->strWalletFile, "r+");

    if(!walletdb.UpdateKey(vchPubKey, pwalletMain->kd[vchPubKey.GetID()]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");


    string encrypted;
    EncryptMessage(pub_k, locatorStr, encrypted);


    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

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
    scriptPubKey << OP_ALIAS_ENCRYPTED << vchEncryptedPath << vchFromString(sigBase64) << vchFromString(strAddress) << hash << vchValue << vchFromString("0") << vchFromString("_") << vchFromString(State::GROUND) << OP_2DROP << OP_2DROP << OP_2DROP << OP_2DROP << OP_DROP;
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


    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    return res;
}
bool aliasTxPos(const vector<PathIndex> &vtxPos, const CDiskTxPos& txPos)
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
    printf("aliasScript 1\n");
    opcodetype opcode;
    if(!script.GetOp(pc, opcode))
    {
        printf("aliasScript 1 1\n");
        return false;
    }
    if(opcode < OP_1 || opcode > OP_16)
    {
        printf("aliasScript 1 2\n");
        if(opcode == OP_BASE_SET)
            printf("aliasScript 1 3\n");

        return false;
    }

    op = opcode - OP_1 + 1;

    printf("aliasScript 2\n");
    for(;;) {
        vector<unsigned char> vch;
        if(!script.GetOp(pc, opcode, vch))
            return false;
        if(opcode == OP_DROP || opcode == OP_2DROP || opcode == OP_NOP)
            break;
        if(!(opcode >= 0 && opcode <= OP_PUSHDATA4))
        {
            printf("aliasScript 2 2\n");
            return false;
        }
        vvch.push_back(vch);
    }

    printf("aliasScript 3\n");
    while(opcode == OP_DROP || opcode == OP_2DROP || opcode == OP_NOP)
    {
        if(!script.GetOp(pc, opcode))
            break;
    }

    pc--;

    printf("aliasScript 4\n");
    if((op == OP_ALIAS_ENCRYPTED && vvch.size() == 8) ||
            (op == OP_ALIAS_SET && vvch.size() == 5) ||
            (op == OP_MESSAGE) ||
            (op == OP_BASE_SET) ||
            (op == OP_BASE_RELAY) ||
            (op == OP_ENCRYPTED_MESSAGE) ||
            (op == OP_MAP_PROJECT) ||
            (op == OP_PUBLIC_KEY) ||
            (op == OP_VERTEX) ||
            (op == OP_ALIAS_RELAY && vvch.size() == 2))
    {
        printf("aliasScript 5\n");
        return true;
    }
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
    printf("aliasTx 1\n");
    bool found = false;
    for(unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& out = tx.vout[i];

        vector<vector<unsigned char> > vvchRead;

        printf("aliasTx 2\n");
        if(aliasScript(out.scriptPubKey, op, vvchRead))
        {
            if(found)
            {
                vvch.clear();
                printf("aliasTx 3\n");
                return false;
            }
            nOut = i;
            found = true;
            vvch = vvchRead;
            printf("aliasTx 4\n");
        }
    }

    printf("aliasTx 5\n");
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

    printf("aliasTxVal 1\n");
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
    case OP_BASE_SET:
        value = vvch[1];
        return true;
    case OP_BASE_RELAY:
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
        return -1;
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
    cba addr(strAddress);
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
    cba addr(strAddress);
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
bool IsMinePost(const CTransaction& tx, const CTxOut& txout, bool ignore_registerPath )
{
    if(tx.nVersion != CTransaction::DION_TX_VERSION)
        return false;

    vector<vector<unsigned char> > vvch;

    int op;

    if(!aliasScript(txout.scriptPubKey, op, vvch))
        return false;

    if(ignore_registerPath && op == OP_ALIAS_ENCRYPTED)
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

    if(op == OP_ALIAS_SET)
    {
        if(vvch[0].size() > MAX_LOCATOR_LENGTH)
            return error("locator too long");

        int nPrevHeight = aliasHeight(vvch[0]);
        if(nPrevHeight >= 0 && nBestHeight - nPrevHeight < scaleMonitor())
            return false;
    }
    if(op == OP_BASE_SET)
    {
        if(vvch[0].size() > MAX_LOCATOR_LENGTH)
            return error("locator too long");

        int nPrevHeight = aliasHeight(vvch[0]);
        if(nPrevHeight >= 0 && nBestHeight - nPrevHeight < scaleMonitor())
            return false;
    }

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

    if(op == OP_PUBLIC_KEY || op == OP_VERTEX)
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
    printf("conectinputspost 1\n");
    if(!(tx.nVersion == CTransaction::DION_TX_VERSION || tx.nVersion == CTransaction::CYCLE_TX_VERSION))
    {
        bool found= false;
        for(unsigned int i = 0; i < tx.vout.size(); i++)
        {
            const CTxOut& out = tx.vout[i];

            std::vector<vchType> vvchRead;
            int opRead;

            if(aliasScript(out.scriptPubKey, opRead, vvchRead))
                found=true;
        }

        if(found)
            printf("encountered non-dions transaction with a dions input");

        return true;
    }
    LocatorNodeDB ln1Db("r+");
    int nInput;
    bool found = false;

    printf("conectinputspost 2\n");
    int prevOp;
    std::vector<vchType> vvchPrevArgs;
    printf("CIP tx %s\n",
           tx.GetHash().GetHex().c_str());

    for(int i = 0; i < tx.vin.size(); i++)
    {
        const CTxOut& out = vTxPrev[i].vout[tx.vin[i].prevout.n];
        std::vector<vchType> vvchPrevArgsRead;

        printf("conectinputspost 3\n");
        if(aliasScript(out.scriptPubKey, prevOp, vvchPrevArgsRead))
        {
            if(found)
                return error("ConnectInputsPost() : multiple previous alias transactions");

            found = true;
            nInput = i;

            vvchPrevArgs = vvchPrevArgsRead;
        }
    }

    std::vector<vchType> vvchArgs;
    int op;
    int nOut;

    printf("conectinputspost 4\n");
    if(!aliasTx(tx, op, nOut, vvchArgs))
        return error("ConnectInputsPost() : could not decode a dions tx");

    printf("conectinputspost 5\n");
    CScript s1 = tx.vout[nOut].scriptPubKey;
    const CScript& s1_ = aliasStrip(s1);
    string a1 = s1_.GetBitcoinAddress();

    int nDepth;

    if(vvchArgs[0].size() > MAX_LOCATOR_LENGTH)
        return error("alias transaction with alias too long");

    printf("conectinputspost 6\n");
    switch(op)
    {
    case OP_PUBLIC_KEY:
    case OP_VERTEX:
    {
        const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
        const std::string recipient(vvchArgs[1].begin(), vvchArgs[1].end());
        const std::string pkey(vvchArgs[2].begin(), vvchArgs[2].end());
        const std::string aesEncrypted(stringFromVch(vvchArgs[3]));
        const std::string sig(stringFromVch(vvchArgs[4]));

        cba addr(sender);

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
    case OP_MAP_PROJECT:
    {
        const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
        const std::string recipient(vvchArgs[1].begin(), vvchArgs[1].end());
        const std::string encrypted(vvchArgs[2].begin(), vvchArgs[2].end());
        const std::string iv128Base64(stringFromVch(vvchArgs[3]));
        const std::string sig(stringFromVch(vvchArgs[4]));

        cba addr(sender);

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

        cba addr(sender);

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
            return error("registerPath tx with incorrect hash length");
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
            return error("Dions::ConnectInputsPost: failed to verify signature for registerPath tx %s",
                         tx.GetHash().ToString().c_str());
        }


        if(r != "0" && IsMinePost(tx))
        {
            CScript script;
            script.SetBitcoinAddress(stringFromVch(vvchArgs[2]));

            cba ownerAddr = script.GetBitcoinAddress();
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
            __wx__DB walletdb(pwalletMain->strWalletFile, "r+");
            if(!walletdb.UpdateKey(vchPubKey, pwalletMain->kd[vchPubKey.GetID()]))
                throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");
        }

    }
    break;
    case OP_ALIAS_SET:
    {
        if(!found || prevOp != OP_ALIAS_ENCRYPTED)
        {
            return error("ConnectInputsPost() : decryptPath tx without previous registerPath tx");
        }

        CScript script;
        if(vvchPrevArgs.size() != 0)
            script.SetBitcoinAddress(stringFromVch(vvchPrevArgs[2]));
        else
            script.SetBitcoinAddress(stringFromVch(vvchArgs[2]));

        if(!verifymessage(script.GetBitcoinAddress(), stringFromVch(vvchArgs[1]), stringFromVch(vvchArgs[0])))
            return error("Dions::ConnectInputsPost: failed to verify signature for decryptPath tx %s",
                         tx.GetHash().ToString().c_str());



        if(vvchArgs[2] != vvchPrevArgs[2])
        {
            return error("mismatch");
        }

        if(vvchArgs[3].size() > 20)
            return error("tx with field offset too long");

        if(vvchArgs[2].size() > MAX_XUNIT_LENGTH)
            return error("tx with field entry too long");

        {
            const vchType& vchHash = vvchPrevArgs[3];
            const vchType& vchPath = vvchArgs[0];
            const vchType& vchRand = vvchArgs[3];
            vchType vchToHash(vchRand);
            vchToHash.insert(vchToHash.end(), vchPath.begin(), vchPath.end());
            uint160 hash = Hash160(vchToHash);
            if(uint160(vchHash) != hash)
            {
                return error("hash mismatch");
            }
        }

        nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], MIN_SET_DEPTH);

        if((fBlock || fMiner) && nDepth >= 0 && nDepth < MIN_SET_DEPTH)
            return false;

        if(fMiner)
        {
            nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], scaleMonitor());
            if(nDepth == -1)
                return error("cannot be mined if not already in chain and unexpired");

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
            return error("updatePath tx without previous update tx");

        if(vvchArgs[1].size() > MAX_XUNIT_LENGTH)
            return error("updatePath tx with value too long");

        if(vvchPrevArgs[0] != vvchArgs[0])
        {
            return error("alias mismatch");
        }

        nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], scaleMonitor());
        if((fBlock || fMiner) && nDepth < 0)
            return error("expired alias, or there is a pending transaction on the alias");
        break;
    case OP_BASE_RELAY:
        if(!found ||(prevOp != OP_BASE_SET && prevOp != OP_BASE_RELAY))
            return error("updatePath tx without previous update tx");

        if(vvchArgs[1].size() > MAX_XUNIT_LENGTH)
            return error("updatePath tx with value too long");

        if(vvchPrevArgs[0] != vvchArgs[0])
        {
            return error("alias mismatch");
        }

        nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], scaleMonitor());
        if((fBlock || fMiner) && nDepth < 0)
            return error("expired alias, or there is a pending transaction on the alias");
        break;
    case OP_BASE_SET:
        printf("conectinputspost 7\n");
        if(vvchArgs[1].size() > MAX_XUNIT_LENGTH)
            return error("createDataNode tx with value too long");
        //CScript script;
        //if(vvchPrevArgs.size() != 0)
        //  script.SetBitcoinAddress(stringFromVch(vvchPrevArgs[2]));
        //else
        //  script.SetBitcoinAddress(stringFromVch(vvchArgs[2]));

        break;
    default:
        return error("alias transaction has unknown op");
    }
    if(!fBlock && (op == OP_ALIAS_RELAY || op == OP_BASE_RELAY))
    {
        vector<PathIndex> vtxPos;
        if(ln1Db.lKey(vvchArgs[0])
                && !ln1Db.lGet(vvchArgs[0], vtxPos))
            return error("ConnectInputsPost() : failed to read from alias DB");
        if(!aliasTxPos(vtxPos, vTxindex[nInput].pos))
            return error("ConnectInputsPost() : tx %s rejected, since previous tx(%s) is not in the alias DB\n", tx.GetHash().ToString().c_str(), vTxPrev[nInput].GetHash().ToString().c_str());
    }
    //if(!fBlock && op == OP_BASE_SET)
    //{
    //printf("conectinputspost 8\n");
    //    vector<PathIndex> vtxPos;
    //    if(ln1Db.lKey(vvchArgs[0])
    //        && !ln1Db.lGet(vvchArgs[0], vtxPos))
    //      return error("ConnectInputsPost() : failed to read from alias DB");
    //printf("conectinputspost 9\n");
    //}
    if(fBlock)
    {
        if(op == OP_ALIAS_SET || op == OP_ALIAS_RELAY || op == OP_BASE_SET || op == OP_BASE_RELAY)
        {

            string locatorStr = stringFromVch(vvchArgs[0]);

            if(op == OP_ALIAS_SET || OP_BASE_SET)
            {
                CTransaction tx;
                if(aliasTx(ln1Db, vchFromString(locatorStr), tx))
                {
                    printf("%s flagged active with tx %s\n", locatorStr.c_str(),
                           tx.GetHash().GetHex().c_str());
                }
            }
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

    printf("conectinputspost 10\n");
    return true;
}

void xsc(CBlockIndex* p)
{
    for(; p; p=p->pnext)
    {
        if(!fTestNet && p->nHeight < 1625000)
            continue;

        ln1Db->filter(p);
    }

    return;
}

unsigned char GetAddressVersion()
{
    return((unsigned char)(fTestNet ? 111 : 103));
}


Value alias(const Array& params, bool fHelp)
{
    return registerPathGenerate(params, fHelp);
}
Value statusList(const Array& params, bool fHelp)
{
    return aliasList__(params, fHelp);
}
Value updateEncrypt(const Array& params, bool fHelp)
{
    return uC(params, fHelp);
}
Value downloadDecrypt(const Array& params, bool fHelp)
{
    return validate(params, fHelp);
}

Value downloadDecryptEPID(const Array& params, bool fHelp)
{
    return vEPID(params, fHelp);
}
Value ioget(const Array& params, bool fHelp)
{
    return psimplex(params, fHelp);
}

bool relaySigFrame(int i, vchType& s)
{
    return LR_SHIFT__[i] == s[0];
}
bool frlRelay(int& i)
{
    return true;
}

Value vtx(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
            "vtx <addr> "
        );

    string l = params[0].get_str();
    CKeyID keyID;
    cba keyAddress(l);
    if(!keyAddress.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(l);
        if (ln1Db.lKey(vchPath))
        {
            if (!ln1Db.lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            keyAddress.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_TYPE_ERROR, "invalid reference");
        }
    }

    keyAddress.GetKeyID(keyID);
    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);

    vchType kAlpha;
    GenerateAESKey(kAlpha);
    string s = EncodeBase64(&kAlpha[0], kAlpha.size());
    __wx__DB walletdb(pwalletMain->strWalletFile, "r+");

    if(!pwalletMain->vtx(vchPubKey, s))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to set meta data for key");

    if(!walletdb.UpdateKey(vchPubKey, pwalletMain->kd[vchPubKey.GetID()]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

    return true;
}
Value mapVertex(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 2)
        throw runtime_error(
            "mapVertex <addr> <addr>"
        );

    EnsureWalletIsUnlocked();

    cba addr(params[0].get_str());
    if(!addr.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(params[0].get_str());
        if (ln1Db.lKey(vchPath))
        {
            if (!ln1Db.lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            addr.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_TYPE_ERROR, "invalid reference");
        }
    }

    cba aRecipient(params[1].get_str());

    if(!aRecipient.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(params[1].get_str());
        if (ln1Db.lKey(vchPath))
        {
            if (!ln1Db.lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            aRecipient.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_TYPE_ERROR, "invalid reference");
        }
    }

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
    const vchType vchSender = vchFromValue(addr.ToString());
    const vchType vchRecipient = vchFromValue(aRecipient.ToString());
    vector<unsigned char> vchSig;
    CDataStream ss(SER_GETHASH, 0);

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;

    vchType recipientPubKeyVch;
    vchType recipientAESKeyVch;
    vchType aes256Key;

    string sigBase64;
    string encrypted;
    if(pk(addr.ToString(), aRecipient.ToString(), recipientPubKeyVch, recipientAESKeyVch))
    {
        string s;
        if(!pwalletMain->vtx_(vchPubKey, s))
            throw JSONRPCError(RPC_TYPE_ERROR, "key trace");

        const string publicKeyStr = stringFromVch(recipientPubKeyVch);
        EncryptMessage(publicKeyStr, s, encrypted);


        __wx__DB walletdb(pwalletMain->strWalletFile, "r+");

        ss <<(rsaPubKeyStr + encrypted);
        if(!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");
        sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());
        scriptPubKey << OP_VERTEX << vchSender << vchRecipient << vchKey
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
        scriptPubKey << OP_VERTEX << vchSender << vchRecipient << vchKey
                     << vchFromString("I")
                     << vchFromString(sigBase64)
                     << OP_2DROP << OP_2DROP << OP_2DROP;
    }


    scriptPubKeyOrig.SetBitcoinAddress(aRecipient.ToString());

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
bool pk(string myAddress, string fKey, vchType& recipientPubKeyVch, vchType& aesKeyBase64EncryptedVch)
{
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchSender, vchRecipient, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.vtx(nOut, vchSender, vchRecipient, vchKey, vchAes, vchSig))
                    continue;

                string keyRecipientAddr = stringFromVch(vchRecipient);
                string ext = stringFromVch(vchSender);
                if(keyRecipientAddr == myAddress && ext == fKey )
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

Value vtxtrace(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
            "vtxtrace [<alias>]\n"
        );
    vchType vchNodeLocator;
    if(params.size() == 1)
        vchNodeLocator = vchFromValue(params[0]);

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchS, vchR, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.vtx(nOut, vchS, vchR, vchKey, vchAes, vchSig))
                    continue;

                string keySenderAddr = stringFromVch(vchS);
                cba r(keySenderAddr);
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
                string tr;
                if(atod(stringFromVch(vchR), tr) == 0)
                    aliasObj.push_back(Pair("trans", tr));
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
Value mapProject(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 3)
        throw runtime_error(
            "mapProject <addr> <project> <addr>"
        );
    string strMessage = params[1].get_str();

    cba prj(params[2].get_str());
    if(!prj.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(params[2].get_str());
        if (ln1Db.lKey(vchPath))
        {
            if (!ln1Db.lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            prj.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_TYPE_ERROR, "invalid reference");
        }
    }

    CKeyID rkeyID;
    if(!prj.GetKeyID(rkeyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "prj does not refer to key");

    vchType recipientAddressVch = vchFromString(prj.ToString());
    vchType recipientPubKeyVch;
    vector<unsigned char> aesRawVector;

    CKey key;
    cba node(params[0].get_str());
    if(!node.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(params[0].get_str());
        if (ln1Db.lKey(vchPath))
        {
            if (!ln1Db.lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            node.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_TYPE_ERROR, "invalid reference");
        }
    }
    if(!node.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address");

    CKeyID keyID;
    if(!node.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "node does not refer to key");

    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);

    string aesBase64Plain;
    if(pwalletMain->vtx_(vchPubKey, aesBase64Plain))
    {
        bool fInvalid = false;
        aesRawVector = DecodeBase64(aesBase64Plain.c_str(), &fInvalid);
    }
    else
    {
        vchType aesKeyBase64EncryptedVch;
        if(pk(node.ToString(), prj.ToString(), recipientPubKeyVch, aesKeyBase64EncryptedVch))
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
            throw JSONRPCError(RPC_WALLET_ERROR, "No local symmetric key and no imported symmetric key");
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

    __wx__Tx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;

    uint160 hash160;
    bool isValid = AddressToHash160(prj.ToString(), hash160);
    if(!isValid)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
    scriptPubKeyOrig.SetBitcoinAddress(prj.ToString());

    vchType vchEncryptedMessage = vchFromString(encrypted);
    vchType iv128Base64Vch = vchFromString(iv128Base64);
    scriptPubKey << OP_MAP_PROJECT << vchFromString(node.ToString()) << vchFromString(prj.ToString()) << vchEncryptedMessage << iv128Base64Vch << vchFromString(sigBase64) << OP_2DROP << OP_2DROP << OP_DROP;
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
Value projection(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 1)
        throw runtime_error(
            "projection [<alias>]\n"
        );
    vchType vchNodeLocator;
    string ref;
    if(params.size() == 1)
        ref = params[0].get_str();

    cba alpha(ref);
    if(!alpha.IsValid())
    {
        vector<PathIndex> vtxPos;
        LocatorNodeDB ln1Db("r");
        vchType vchPath = vchFromString(ref);
        if (ln1Db.lKey(vchPath))
        {
            if (!ln1Db.lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            alpha.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_TYPE_ERROR, "invalid reference");
        }
    }

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchV0, vchV1, vchEncryptedMessage, ivVch, vchSig;
                int nOut;
                if(!tx.proj(nOut, vchV0, vchV1, vchEncryptedMessage, ivVch, vchSig))
                {
                    continue;
                }

                const int nHeight = tx.GetHeightInMainChain();

                string trans;
                string fKey;
                string v0=stringFromVch(vchV0);
                string v1=stringFromVch(vchV1);

                if(!(v0 == alpha.ToString() || v1 == alpha.ToString()))
                    continue;

                if(hk(v0))
                {
                    trans = stringFromVch(vchV0);
                    fKey = stringFromVch(vchV1);
                }
                else if(hk(v1))
                {
                    trans = stringFromVch(vchV1);
                    fKey = stringFromVch(vchV0);
                }
                else
                {
                    string w;
                    if(vclose(v0,w))
                    {
                        trans=w;
                        fKey=v0;
                    }
                    else if(vclose(v1,w))
                    {
                        trans=w;
                        fKey=v1;
                    }
                }

                Object aliasObj;

                aliasObj.push_back(Pair("sender", stringFromVch(vchV0)));
                aliasObj.push_back(Pair("recipient", stringFromVch(vchV1)));
                aliasObj.push_back(Pair("encrypted_message", stringFromVch(vchEncryptedMessage)));
                string t = DateTimeStrFormat(tx.GetTxTime());
                aliasObj.push_back(Pair("time", t));


                string rsaPrivKey;

                cba r(trans);
                if(!r.IsValid())
                {
                    continue;
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
                if(pwalletMain->vtx_(pubKey, aesBase64Plain))
                {
                    bool fInvalid = false;
                    aesRawVector = DecodeBase64(aesBase64Plain.c_str(), &fInvalid);
                }
                else
                {
                    vchType aesKeyBase64EncryptedVch;
                    vchType pub_key = pubKey.Raw();
                    if(pk(trans, fKey, pub_key, aesKeyBase64EncryptedVch))
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


                string a;
                if(atod(stringFromVch(vchV0), a) == 0)
                    aliasObj.push_back(Pair("alias", a));
                string tr;
                if(atod(stringFromVch(vchV1), tr) == 0)
                    aliasObj.push_back(Pair("trans", tr));
                else
                    aliasObj.push_back(Pair("trans", "0"));
                oRes.push_back(aliasObj);

                mapPathVchInt[vchV0] = nHeight;
                aliasMapVchObj[vchV0] = aliasObj;
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    return oRes;
}

bool collx(const CTransaction& t)
{
    if (t.nVersion != CTransaction::DION_TX_VERSION)
        return false;

    std::vector<vchType> vvchArgs;
    int op;
    int nOut;

    if(!aliasTx(t, op, nOut, vvchArgs))
        return false;

    if(op != OP_MAP_PROJECT)
        return false;

    std::string s(vvchArgs[0].begin(), vvchArgs[0].end());
    std::string r(vvchArgs[1].begin(), vvchArgs[1].end());
    std::string e(vvchArgs[2].begin(), vvchArgs[2].end());
    std::string iv128Base64(stringFromVch(vvchArgs[3]));
    std::string sig(stringFromVch(vvchArgs[4]));


    string w;
    if(vclose(s, w) || vclose(r, w))
        return true;

    return false;
}

bool vclose(string& v, string& w)
{
    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    Array oRes;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchS, vchR, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.vtx(nOut, vchS, vchR, vchKey, vchAes, vchSig))
                    continue;

                string keySenderAddr = stringFromVch(vchS);
                cba r(keySenderAddr);
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
                if(s == v)
                {
                    w=r;
                    return true;
                }
                else if(r == v)
                {
                    w=s;
                    return true;
                }
            }
        }
    }

    return false;
}

Value xstat(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
            "xstat <addr> "
        );

    string l = params[0].get_str();
    CKeyID keyID;
    cba keyAddress(l);
    if(!keyAddress.IsValid())
    {
        vector<PathIndex> vtxPos;
        vchType vchPath = vchFromString(l);
        if (ln1Db->lKey(vchPath))
        {
            if (!ln1Db->lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            if(txPos.nHeight + scaleMonitor() <= nBestHeight)
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "extern alias");
            keyAddress.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias 2");
        }
    }

    if(!keyAddress.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);

    Array oRes;
    string r;
    Object o;
    if(pwalletMain->vtx_(vchPubKey, r))
        o.push_back(Pair("xstat", "true"));
    else
        o.push_back(Pair("xstat", "false"));

    oRes.push_back(o);

    return oRes;
}

static bool xs(string& s)
{
    CKeyID keyID;
    cba keyAddress(s);
    if(!keyAddress.IsValid())
    {
        vector<PathIndex> vtxPos;
        vchType vchPath = vchFromString(s);
        if (ln1Db->lKey(vchPath))
        {
            if (!ln1Db->lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            keyAddress.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias 3");
        }
    }

    if(!keyAddress.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CPubKey vchPubKey;
    pwalletMain->GetPubKey(keyID, vchPubKey);

    string r;
    return pwalletMain->vtx_(vchPubKey, r);
}

Value svtx(const Array& params, bool fHelp)
{
    if(fHelp || params.size() != 1)
        throw runtime_error(
            "svtx[<alias>]\n"
        );
    string ref = params[0].get_str();

    Array oRes;
    if(xs(ref)) return oRes;

    cba k(ref);
    if(!k.IsValid())
    {
        vector<PathIndex> vtxPos;
        vchType vchPath = vchFromString(ref);
        if (ln1Db->lKey(vchPath))
        {
            if (!ln1Db->lGet(vchPath, vtxPos))
                return error("aliasHeight() : failed to read from name DB");
            if (vtxPos.empty ())
                return -1;

            PathIndex& txPos = vtxPos.back ();
            if(txPos.nHeight + scaleMonitor() <= nBestHeight)
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "extern alias");
            k.SetString(txPos.vAddress);
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias 4");
        }
    }
    CKeyID keyID;
    if(!k.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if(!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_TYPE_ERROR, "external");

    std::map<vchType, int> mapPathVchInt;
    std::map<vchType, Object> aliasMapVchObj;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, __wx__Tx)& item,
                          pwalletMain->mapWallet)
            {
                const __wx__Tx& tx = item.second;

                vchType vchS, vchR, vchKey, vchAes, vchSig;
                int nOut;
                if(!tx.vtx(nOut, vchS, vchR, vchKey, vchAes, vchSig))
                    continue;

                string keySenderAddr = stringFromVch(vchS);
                cba r(keySenderAddr);
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

    Array res_;
    for(unsigned int i=0; i<oRes.size(); i++)
    {
        Object& o = oRes[i].get_obj();

        string s = o[0].value_.get_str();
        string r = o[1].value_.get_str();
        for(unsigned int j=i+1; j<oRes.size(); j++)
        {
            Object& o1 = oRes[j].get_obj();
            if(s == o1[1].value_.get_str() && r == o1[0].value_.get_str())
            {
                o[3].value_ = "true";
                o1[3].value_ = "true";
                if(o[7].value_ == ref || o1[7].value_ == ref)
                {
                    if(o[2].value_.get_str() == "imported")
                    {
                        res_.push_back(o[7].value_);
                    }
                    else
                    {
                        res_.push_back(o1[7].value_);
                    }
                }
            }
        }
    }
    return res_;
}
Value simplexU(const Array& params, bool fHelp) {
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
    scriptPubKey << OP_ALIAS_RELAY << vchPath << vchFromString(r) << OP_2DROP << OP_DROP;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if(mapState.count(vchPath) && mapState[vchPath].size())
            {
                error("updatePath() : there are %lu pending operations on that alias, including %s",
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
                throw runtime_error("could not find a coin with this alias 1");
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
                error("updatePath() : this coin is not in your wallet %s",
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
Value psimplex(const Array& params, bool fHelp) {
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
