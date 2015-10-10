// Copyright (c) 2010-2011 Vincent Durham
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.
//
#include "db.h"
#include "txdb-leveldb.h"
#include "keystore.h"
#include "wallet.h"
#include "init.h"
#include "dions.h"

#include "bitcoinrpc.h"

#include "main.h"

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#include <boost/xpressive/xpressive_dynamic.hpp>

using namespace std;
using namespace json_spirit;

extern Object JSONRPCError(int code, const string& message);
template<typename T> void ConvertTo(Value& value, bool fAllowNull=false);

std::map<vchType, uint256> mapMyMessages;
std::map<vchType, uint256> mapMyNames;
std::map<vchType, set<uint256> > mapNamePending;
std::set<vchType> setNewHashes;

#ifdef GUI
extern std::map<uint160, vchType> mapMyNameHashes;
#endif

extern uint256 SignatureHash(CScript scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

// forward decls
extern bool Solver(const CKeyStore& keystore, const CScript& scriptPubKey, uint256 hash, int nHashType, CScript& scriptSigRet, txnouttype& type);
extern bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn, int nHashType);
extern void rescanfornames();
extern Value sendtoaddress(const Array& params, bool fHelp);

class CDionsHooks : public CHooks
{
public:
    virtual bool ConnectInputs( map<uint256, CTxIndex>& mapTestPool,
           const CTransaction& tx,
           vector<CTransaction>& vTxPrev,
           vector<CTxIndex>& vTxindex,
           CBlockIndex* pindexBlock,
           CDiskTxPos& txPos,
           bool fBlock,
           bool fMiner);
    virtual bool AcceptToMemoryPool(const CTransaction& tx);
    virtual void RemoveFromMemoryPool(const CTransaction& tx);
    virtual bool IsMine(const CTransaction& tx);
    virtual bool IsMine(const CTransaction& tx, const CTxOut& txout, bool ignore_name_new = false);
};

vchType vchFromValue (const Value& value)
{
    const std::string str = value.get_str ();
    return vchFromString (str);
}

vchType vchFromString (const std::string& str)
{
    const unsigned char* strbeg;
    strbeg = reinterpret_cast<const unsigned char*> (str.c_str ());
    return vchType(strbeg, strbeg + str.size ());
}

string stringFromVch(const vector<unsigned char> &vch)
{
    string res;
    vector<unsigned char>::const_iterator vi = vch.begin();
    while (vi != vch.end()) {
      res += (char)(*vi);
      vi++;
    }
    return res;
}

int GetExpirationDepth(int nHeight)
{
    return 210000;
}

// For display purposes, pass the name height.
int GetDisplayExpirationDepth()
{
    return 210000;
}

int GetTxPosHeight(CNameIndex& txPos)
{
    return txPos.nHeight;
}

int GetTxPosHeight(CDiskTxPos& txPos)
{
    // Read block header
    CBlock block;
    if (!block.ReadFromDisk(txPos.nFile, txPos.nBlockPos, false))
        return 0;

    // Find the block in the index
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(block.GetHash());
    if (mi == mapBlockIndex.end())
        return 0;

    CBlockIndex* pindex = (*mi).second;
    if (!pindex || !pindex->IsInMainChain())
        return 0;

    return pindex->nHeight;
}

int GetNameHeight(vector<unsigned char> vchName)
{
    vector<CNameIndex> vtxPos;
    CNameDB nameDb("r");

    if (nameDb.ExistsName (vchName))
    {
        if (!nameDb.ReadName (vchName, vtxPos))
            return error("GetNameHeight() : failed to read from name DB");

        if (vtxPos.empty ())
            return -1;

        CNameIndex& txPos = vtxPos.back ();
        return GetTxPosHeight (txPos);
    }

    return -1;
}

CScript RemoveNameScriptPrefix(const CScript& scriptIn)
{
    int op;
    vector<vector<unsigned char> > vvch;
    CScript::const_iterator pc = scriptIn.begin();

    if (!DecodeNameScript(scriptIn, op, vvch,  pc))
        throw runtime_error("RemoveNameScriptPrefix() : could not decode name script");

    return CScript(pc, scriptIn.end());
}

bool IsMyName(const CTransaction& tx, const CTxOut& txout)
{
    const CScript& scriptPubKey = RemoveNameScriptPrefix(txout.scriptPubKey);
    CScript scriptSig;
    txnouttype whichType;

    if (!Solver(*pwalletMain, scriptPubKey, 0, 0, scriptSig, whichType))
        return false;

    return true;
}

bool CreateTransactionWithInputTx(const vector<pair<CScript, int64_t> >& vecSend, const CWalletTx& wtxIn, int nTxOut, CWalletTx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet)
{
    int64_t nValue = 0;
    BOOST_FOREACH(const PAIRTYPE(CScript, int64_t)& s, vecSend)
    {
        if (nValue < 0)
            return false;
        nValue += s.second;
    }

    if (vecSend.empty() || nValue < 0)
        return false;

    wtxNew.pwallet = pwalletMain;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        // txdb must be opened before the mapWallet lock
        CTxDB txdb("r");
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            nFeeRet = nTransactionFee;
            for(;;)
            {
                wtxNew.vin.clear();
                wtxNew.vout.clear();
                wtxNew.fFromMe = true;

                int64_t nTotalValue = nValue + nFeeRet;
                printf("CreateTransactionWithInputTx: total value = %d\n", nTotalValue);
                // vouts to the payees
                BOOST_FOREACH(const PAIRTYPE(CScript, int64_t)& s, vecSend)
                    wtxNew.vout.push_back(CTxOut(s.second, s.first));

                int64_t nWtxinCredit = wtxIn.vout[nTxOut].nValue;

                // Choose coins to use
                set<pair<const CWalletTx*, unsigned int> > setCoins;
                int64_t nValueIn = 0;
                printf("CreateTransactionWithInputTx: SelectCoins(%s), nTotalValue = %s, nWtxinCredit = %s\n", FormatMoney(nTotalValue - nWtxinCredit).c_str(), FormatMoney(nTotalValue).c_str(), FormatMoney(nWtxinCredit).c_str());
                if (nTotalValue - nWtxinCredit > 0)
                {
                    if (!pwalletMain->SelectCoins(nTotalValue - nWtxinCredit, wtxIn.nTime, setCoins, nValueIn))
                        return false;
                }

                printf("CreateTransactionWithInputTx: selected %d tx outs, nValueIn = %s\n", setCoins.size(), FormatMoney(nValueIn).c_str());

                vector<pair<const CWalletTx*, unsigned int> >
                    vecCoins(setCoins.begin(), setCoins.end());

                // Input tx always at first position
                vecCoins.insert(vecCoins.begin(), make_pair(&wtxIn, nTxOut));

                nValueIn += nWtxinCredit;

                // Fill a vout back to self with any change
                int64_t nChange = nValueIn - nTotalValue;
                if (nChange >= CENT)
                {
                    // Note: We use a new key here to keep it from being obvious which side is the change.
                    //  The drawback is that by not reusing a previous key, the change may be lost if a
                    //  backup is restored, if the backup doesn't have the new private key for the change.
                    //  If we reused the old key, it would be possible to add code to look for and
                    //  rediscover unknown transactions that were written with keys of ours to recover
                    //  post-backup change.

                    // Reserve a new key pair from key pool
                    CPubKey pubkey;
                    //vector<unsigned char> vchPubKey = reservekey.GetReservedKey();
                    if(!reservekey.GetReservedKey(pubkey))
                    {
                      return false;
                    }

                    // -------------- Fill a vout to ourself, using same address type as the payment
                    // Now sending always to hash160 (GetBitcoinAddressHash160 will return hash160, even if pubkey is used)
                    CScript scriptChange;
                    scriptChange.SetDestination(pubkey.GetID());

                    // Insert change txn at random position:
                    vector<CTxOut>::iterator position = wtxNew.vout.begin()+GetRandInt(wtxNew.vout.size());
                    wtxNew.vout.insert(position, CTxOut(nChange, scriptChange));
                }
                else
                    reservekey.ReturnKey();

                // Fill vin
                BOOST_FOREACH(PAIRTYPE(const CWalletTx*, unsigned int)& coin, vecCoins)
                    wtxNew.vin.push_back(CTxIn(coin.first->GetHash(), coin.second));

                // Sign
                int nIn = 0;
                BOOST_FOREACH(PAIRTYPE(const CWalletTx*, unsigned int)& coin, vecCoins)
                {
                    if (!SignSignature(*pwalletMain, *coin.first, wtxNew, nIn++))
                        return false;
                }

                // Limit size
                unsigned int nBytes = ::GetSerializeSize(*(CTransaction*)&wtxNew, SER_NETWORK);
                if (nBytes >= MAX_BLOCK_SIZE_GEN/5)
                    return false;

                // Check that enough fee is included
                int64_t nPayFee = nTransactionFee * (1 + (int64_t)nBytes / 1000);
                int64_t nMinFee = wtxNew.GetMinFee(1, GMF_SEND, nBytes);

                if (nFeeRet < max(nPayFee, nMinFee))
                {
                    nFeeRet = max(nPayFee, nMinFee);
                    printf("CreateTransactionWithInputTx: re-iterating (nFreeRet = %s)\n", FormatMoney(nFeeRet).c_str());
                    continue;
                }

                // Fill vtxPrev by copying from previous transactions vtxPrev
                wtxNew.AddSupportingTransactions(txdb);
                wtxNew.fTimeReceivedIsTxTime = true;

                break;
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    printf("CreateTransactionWithInputTx succeeded:\n%s", wtxNew.ToString().c_str());
    return true;
}

string SendMoneyWithInputTx(const CScript& scriptPubKey, int64_t nValue, const CWalletTx& wtxIn, CWalletTx& wtxNew, bool fAskFee)
{
    int nTxOut = IndexOfNameOutput(wtxIn);
    CReserveKey reservekey(pwalletMain);
    int64_t nFeeRequired;
    vector< pair<CScript, int64_t> > vecSend;
    vecSend.push_back(make_pair(scriptPubKey, nValue));

    if (!CreateTransactionWithInputTx(vecSend, wtxIn, nTxOut, wtxNew, reservekey, nFeeRequired))
    {
        string strError;
        if (nValue + nFeeRequired > pwalletMain->GetBalance())
            strError = strprintf(_("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds "), FormatMoney(nFeeRequired).c_str());
        else
            strError = _("Error: Transaction creation failed  ");
        printf("SendMoney() : %s", strError.c_str());
        return strError;
    }

#ifdef GUI
    if (fAskFee && !uiInterface.ThreadSafeAskFee(nFeeRequired))
        return "ABORTED";
#endif

    if (!pwalletMain->CommitTransaction(wtxNew, reservekey))
        return _("Error: The transaction was rejected.  This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");

    return "";
}

bool GetValueOfTxPos(CNameIndex& txPos, vector<unsigned char>& vchValue, uint256& hash, int& nHeight)
{
    nHeight = GetTxPosHeight(txPos);
    vchValue = txPos.vValue;
    CTransaction tx;

    if (!tx.ReadFromDisk(txPos.txPos))
        return error("GetValueOfTxPos() : could not read tx from disk");

    hash = tx.GetHash();
    return true;
}

bool GetValueOfTxPos(CDiskTxPos& txPos, vector<unsigned char>& vchValue, uint256& hash, int& nHeight)
{
    nHeight = GetTxPosHeight(txPos);
    CTransaction tx;

    if (!tx.ReadFromDisk(txPos))
        return error("GetValueOfTxPos() : could not read tx from disk");

    if (!GetValueOfNameTx(tx, vchValue))
        return error("GetValueOfTxPos() : could not decode value from tx");

    hash = tx.GetHash();
    return true;
}

bool GetTxOfName(CNameDB& dbName, const vector<unsigned char> &vchName, CTransaction& tx)
{
    //vector<CDiskTxPos> vtxPos;
    vector<CNameIndex> vtxPos;
    if (!dbName.ReadName(vchName, vtxPos) || vtxPos.empty())
        return false;

    //CDiskTxPos& txPos = vtxPos.back();
    CNameIndex& txPos = vtxPos.back();
    //int nHeight = GetTxPosHeight(txPos);
    int nHeight = txPos.nHeight;
    if (nHeight + GetExpirationDepth(pindexBest->nHeight) < pindexBest->nHeight)
    {
        string name = stringFromVch(vchName);
        printf("GetTxOfName(%s) : expired", name.c_str());
        return false;
    }

    if (!tx.ReadFromDisk(txPos.txPos))
        return error("GetTxOfName() : could not read tx from disk");
    return true;
}

bool GetNameAddress(const CTransaction& tx, std::string& strAddress)
{
    int op;
    int nOut;
    vector<vector<unsigned char> > vvch;

    if (!DecodeNameTx(tx, op, nOut, vvch))
        return false;

    const CTxOut& txout = tx.vout[nOut];
    const CScript& scriptPubKey = RemoveNameScriptPrefix(txout.scriptPubKey);
    strAddress = scriptPubKey.GetBitcoinAddress();
    return true;
}

bool GetNameAddress(const CDiskTxPos& txPos, std::string& strAddress)
{
    CTransaction tx;
    if (!tx.ReadFromDisk(txPos))
        return error("GetNameAddress() : could not read tx from disk");

    return GetNameAddress(tx, strAddress);
}

Value public_key_list(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
                "public_key_list [<name>]\n"
                "list imported public keys "
                );

    vchType vchNameUniq;
    if (params.size () == 1)
      vchNameUniq = vchFromValue (params[0]);

    std::map<vchType, int> vNamesI;
    std::map<vchType, Object> vNamesO;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                          pwalletMain->mapWallet)
            {
                const CWalletTx& tx = item.second;

                vchType vchSender, vchKey, vchSig;
                int nOut;
                if (!tx.GetPublicKeyUpdate (nOut, vchSender, vchKey, vchSig))
                  continue;

                const int nHeight = tx.GetHeightInMainChain ();
                if (nHeight == -1)
                  continue;
                assert (nHeight >= 0);

                Object oName;
                oName.push_back(Pair("sender", stringFromVch(vchSender)));
                oName.push_back(Pair("key", stringFromVch(vchKey)));
                oName.push_back(Pair("signature", stringFromVch(vchSig)));

                vNamesI[vchSender] = nHeight;
                vNamesO[vchSender] = oName;
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, vNamesO)
        oRes.push_back(item.second);

    return oRes;
}

Value decrypted_message_list(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
                "name_list [<name>]\n"
                "list my own names"
                );

    vchType vchNameUniq;
    if (params.size () == 1)
        vchNameUniq = vchFromValue (params[0]);

    std::map<vchType, int> vNamesI;
    std::map<vchType, Object> vNamesO;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                          pwalletMain->mapWallet)
            {
                const CWalletTx& tx = item.second;

                vchType vchSender, vchRecipient, vchEncryptedMessage, vchSig;
                int nOut;
                if (!tx.GetEncryptedMessageUpdate (nOut, vchSender, vchRecipient, vchEncryptedMessage, vchSig))
                    continue;

                const int nHeight = tx.GetHeightInMainChain ();
                if (nHeight == -1)
                    continue;

                assert (nHeight >= 0);

                Object oName;
                oName.push_back(Pair("sender", stringFromVch(vchSender)));
                oName.push_back(Pair("recipient", stringFromVch(vchRecipient)));
                oName.push_back(Pair("encrypted message", stringFromVch(vchEncryptedMessage)));
                string rsaPrivKey;
                CBitcoinAddress r(stringFromVch(vchRecipient));
                if (!r.IsValid())
                  throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

                CKeyID keyID;
                if (!r.GetKeyID(keyID))
                  throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

                CKey key;
                if (!pwalletMain->GetKey(keyID, key))
                  throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

                CPubKey pubKey = key.GetPubKey();
                if(pwalletMain->GetRSAPrivateKeyMetadata(pubKey, rsaPrivKey) == false)
                {
                  printf("rsaPrivKey %s\n", rsaPrivKey.c_str());
                }

                string decrypted;
                DecryptMessage(rsaPrivKey, stringFromVch(vchEncryptedMessage), decrypted);
                oName.push_back(Pair("plain text", decrypted));
                oName.push_back(Pair("signature", stringFromVch(vchSig)));

                vNamesI[vchSender] = nHeight;
                vNamesO[vchSender] = oName;
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, vNamesO)
        oRes.push_back(item.second);

    return oRes;
}

Value message_list_plain(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
                "name_list [<name>]\n"
                "list my own names"
                );

    vchType vchNameUniq;
    if (params.size () == 1)
        vchNameUniq = vchFromValue (params[0]);

    std::map<vchType, int> vNamesI;
    std::map<vchType, Object> vNamesO;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                          pwalletMain->mapWallet)
            {
                const CWalletTx& tx = item.second;

                vchType vchSender, vchRecipient, vchMessage, vchSig;
                int nOut;
                if (!tx.GetMessageUpdate (nOut, vchSender, vchRecipient, vchMessage, vchSig))
                  continue;

                const int nHeight = tx.GetHeightInMainChain ();
                if (nHeight == -1)
                  continue;
                assert (nHeight >= 0);

                Object oName;
                oName.push_back(Pair("sender", stringFromVch(vchSender)));
                oName.push_back(Pair("recipient", stringFromVch(vchRecipient)));
                oName.push_back(Pair("message", stringFromVch(vchMessage)));
                oName.push_back(Pair("signature", stringFromVch(vchSig)));

                vNamesI[vchSender] = nHeight;
                vNamesO[vchSender] = oName;
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, vNamesO)
        oRes.push_back(item.second);

    return oRes;
}

Value get_name_record(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
                "get_name_record <name>\n"
                "get data associated with <name>"
                );

    vchType vchNameUniq;
    if (params.size () == 1)
      vchNameUniq = vchFromValue (params[0]);

    std::map<vchType, int> vNamesI;
    std::map<vchType, Object> vNamesO;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                          pwalletMain->mapWallet)
            {
                const CWalletTx& tx = item.second;

                vchType vchName, vchValue;
                int nOut;
                if (!tx.GetNameUpdate (nOut, vchName, vchValue))
                    continue;

                if(!vchNameUniq.empty () && vchNameUniq != vchName)
                    continue;

                const int nHeight = tx.GetHeightInMainChain ();
                if (nHeight == -1)
                    continue;
                assert (nHeight >= 0);

                // get last active name only
                if (vNamesI.find (vchName) != vNamesI.end ()
                    && vNamesI[vchName] > nHeight)
                    continue;

                if(vchNameUniq == vchName)
                {
                    Object oName;
                    oName.push_back(Pair("name", stringFromVch(vchName)));
                    oName.push_back(Pair("value", stringFromVch(vchValue)));
                    if (!hook->IsMine (tx))
                        oName.push_back(Pair("transferred", 1));
                    string strAddress = "";
                    GetNameAddress(tx, strAddress);
                    oName.push_back(Pair("address", strAddress));

                    const int expiresIn = nHeight + GetDisplayExpirationDepth() - pindexBest->nHeight;
                    oName.push_back (Pair("expires_in", expiresIn));
                    if (expiresIn <= 0)
                      oName.push_back (Pair("expired", 1));

                    vNamesI[vchName] = nHeight;
                    vNamesO[vchName] = oName;
                }
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, vNamesO)
        oRes.push_back(item.second);

    return oRes;
}

Value name_list(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
                "name_list [<name>]\n"
                "list my own names"
                );

    vchType vchNameUniq;
    if (params.size () == 1)
        vchNameUniq = vchFromValue (params[0]);

    std::map<vchType, int> vNamesI;
    std::map<vchType, Object> vNamesO;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                          pwalletMain->mapWallet)
            {
                const CWalletTx& tx = item.second;

                vchType vchName, vchValue;
                int nOut;
                if (!tx.GetNameUpdate (nOut, vchName, vchValue))
                  continue;

                if(!vchNameUniq.empty () && vchNameUniq != vchName)
                  continue;

                const int nHeight = tx.GetHeightInMainChain ();
                if (nHeight == -1)
                  continue;
                assert (nHeight >= 0);

                // get last active name only
                if (vNamesI.find (vchName) != vNamesI.end ()
                    && vNamesI[vchName] > nHeight)
                  continue;

                Object oName;
                oName.push_back(Pair("name", stringFromVch(vchName)));
                oName.push_back(Pair("value", stringFromVch(vchValue)));
                if (!hook->IsMine (tx))
                    oName.push_back(Pair("transferred", 1));
                string strAddress = "";
                GetNameAddress(tx, strAddress);
                oName.push_back(Pair("address", strAddress));

                const int expiresIn = nHeight + GetDisplayExpirationDepth() - pindexBest->nHeight;
                oName.push_back (Pair("expires_in", expiresIn));
                if (expiresIn <= 0)
                  oName.push_back (Pair("expired", 1));

                vNamesI[vchName] = nHeight;
                vNamesO[vchName] = oName;
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    Array oRes;
    BOOST_FOREACH(const PAIRTYPE(vector<unsigned char>, Object)& item, vNamesO)
        oRes.push_back(item.second);

    return oRes;
}

Value name_debug(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 0)
        throw runtime_error(
            "name_debug\n"
            "Dump pending transactions id in the debug file.\n");

    printf("Pending:\n----------------------------\n");
    pair<vector<unsigned char>, set<uint256> > pairPending;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        BOOST_FOREACH(pairPending, mapNamePending)
        {
            string name = stringFromVch(pairPending.first);
            printf("%s :\n", name.c_str());
            uint256 hash;
            BOOST_FOREACH(hash, pairPending.second)
            {
                printf("    ");
                if (!pwalletMain->mapWallet.count(hash))
                    printf("foreign ");
                printf("    %s\n", hash.GetHex().c_str());
            }
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    printf("----------------------------\n");
    return true;
}

Value name_debug1(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1)
        throw runtime_error(
            "name_debug1 <name>\n"
            "Dump name blocks number and transactions id in the debug file.\n");

    vector<unsigned char> vchName = vchFromValue(params[0]);
    printf("Dump name:\n");
    ENTER_CRITICAL_SECTION(cs_main)
    {
        //vector<CDiskTxPos> vtxPos;
        vector<CNameIndex> vtxPos;
        CNameDB dbName("r");
        if (!dbName.ReadName(vchName, vtxPos))
        {
            error("failed to read from name DB");
            return false;
        }
        //CDiskTxPos txPos;
        CNameIndex txPos;
        BOOST_FOREACH(txPos, vtxPos)
        {
            CTransaction tx;
            if (!tx.ReadFromDisk(txPos.txPos))
            {
                error("could not read txpos %s", txPos.txPos.ToString().c_str());
                continue;
            }
            printf("@%d %s\n", GetTxPosHeight(txPos), tx.GetHash().GetHex().c_str());
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    printf("-------------------------\n");
    return true;
}

Value name_firstupdate(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 5)
        throw runtime_error(
                "name_firstupdate <name> <rand> [<tx>] <value> [<toaddress>]\n"
                "Perform a first update after a name_new reservation.\n"
                "Note that the first update will go into a block 12 blocks after the name_new, at the soonest."
                + HelpRequiringPassphrase());

    const vchType vchName = vchFromValue (params[0]);
    const vchType vchRand = ParseHex (params[1].get_str ());

    vchType vchValue;
    if (params.size () == 3)
        vchValue = vchFromValue (params[2]);
    else
        vchValue = vchFromValue (params[3]);

    if (vchValue.size () > UI_MAX_VALUE_LENGTH)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "the value is too long");

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        if (mapNamePending.count(vchName) && mapNamePending[vchName].size())
        {
            error("name_firstupdate() : there are %d pending operations on that name, including %s",
                    mapNamePending[vchName].size(),
                    mapNamePending[vchName].begin()->GetHex().c_str());
            throw runtime_error("there are pending operations on that name");
        }
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    {
        CNameDB dbName("r");
        CTransaction tx;
        if (GetTxOfName(dbName, vchName, tx))
        {
            error("name_firstupdate() : this name is already active with tx %s",
                    tx.GetHash().GetHex().c_str());
            throw runtime_error("this name is already active");
        }
    }

    CScript scriptPubKeyOrig;
    if (params.size () == 5)
    {
        const std::string strAddress = params[4].get_str ();
        uint160 hash160;
        bool isValid = AddressToHash160(strAddress, hash160);
        if (!isValid)
            throw JSONRPCError (RPC_INVALID_ADDRESS_OR_KEY,
                                "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress (strAddress);
    }
    else
    {
        vector<unsigned char> vchPubKey = pwalletMain->GetKeyFromKeyPool ();
        scriptPubKeyOrig.SetBitcoinAddress (vchPubKey);
    }

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        // Make sure there is a previous NAME_NEW tx on this name
        // and that the random value matches
        uint256 wtxInHash;
        if (params.size() == 3)
        {
            if (!mapMyNames.count(vchName))
            {
                throw runtime_error("could not find a coin with this name, try specifying the name_new transaction id");
            }
            wtxInHash = mapMyNames[vchName];
        }
        else
        {
            wtxInHash.SetHex(params[2].get_str());
        }

        if (!pwalletMain->mapWallet.count(wtxInHash))
        {
            throw runtime_error("previous transaction is not in the wallet");
        }

        CScript scriptPubKey;
        scriptPubKey << OP_NAME_FIRSTUPDATE << vchName << vchRand << vchValue << OP_2DROP << OP_2DROP;
        scriptPubKey += scriptPubKeyOrig;

        CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
        vector<unsigned char> vchHash;
        bool found = false;
        BOOST_FOREACH(CTxOut& out, wtxIn.vout)
        {
            vector<vector<unsigned char> > vvch;
            int op;
            if (DecodeNameScript(out.scriptPubKey, op, vvch)) {
                if (op != OP_NAME_NEW)
                    throw runtime_error("previous transaction wasn't a name_new");
                vchHash = vvch[0];
                found = true;
            }
        }

        if (!found)
        {
            throw runtime_error("previous tx on this name is not a name tx");
        }

        vector<unsigned char> vchToHash(vchRand);
        vchToHash.insert(vchToHash.end(), vchName.begin(), vchName.end());
        uint160 hash =  Hash160(vchToHash);
        if (uint160(vchHash) != hash)
        {
            throw runtime_error("previous tx used a different random value");
        }

        string strError = SendMoneyWithInputTx(scriptPubKey, MIN_AMOUNT, wtxIn, wtx, false);
       //string strError = pwalletMain->SendMoney(scriptPubKey, MIN_AMOUNT, wtx, false);
        if (strError != "")
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    LEAVE_CRITICAL_SECTION(cs_main)


    return wtx.GetHash().GetHex();
}

Value name_update(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
                "name_update <name> <value> [<toaddress>]\nUpdate and possibly transfer a name"
                + HelpRequiringPassphrase());

    const vchType vchName = vchFromValue (params[0]);
    const vchType vchValue = vchFromValue (params[1]);

    if (vchValue.size () > UI_MAX_VALUE_LENGTH)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "the value is too long");

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;
    CScript scriptPubKeyOrig;

    if (params.size() == 3)
    {
        string strAddress = params[2].get_str();
        uint160 hash160;
        bool isValid = AddressToHash160(strAddress, hash160);
        if (!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress(strAddress);
    }
    else
    {
        vector<unsigned char> vchPubKey = pwalletMain->GetKeyFromKeyPool();
        scriptPubKeyOrig.SetBitcoinAddress(vchPubKey);
    }

    CScript scriptPubKey;
    scriptPubKey << OP_NAME_UPDATE << vchName << vchValue << OP_2DROP << OP_DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            if (mapNamePending.count(vchName) && mapNamePending[vchName].size())
            {
                error("name_update() : there are %d pending operations on that name, including %s",
                        mapNamePending[vchName].size(),
                        mapNamePending[vchName].begin()->GetHex().c_str());
                throw runtime_error("there are pending operations on that name");
            }

            EnsureWalletIsUnlocked();

            CNameDB dbName("r");
            CTransaction tx;
            if (!GetTxOfName(dbName, vchName, tx))
            {
                throw runtime_error("could not find a coin with this name");
            }

            uint256 wtxInHash = tx.GetHash();

            if (!pwalletMain->mapWallet.count(wtxInHash))
            {
                error("name_update() : this coin is not in your wallet %s",
                        wtxInHash.GetHex().c_str());
                throw runtime_error("this coin is not in your wallet");
            }

            CWalletTx& wtxIn = pwalletMain->mapWallet[wtxInHash];
            string strError = SendMoneyWithInputTx(scriptPubKey, MIN_AMOUNT, wtxIn, wtx, false);
            if (strError != "")
                throw JSONRPCError(RPC_WALLET_ERROR, strError);
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)
    return wtx.GetHash().GetHex();
}

Value new_public_key(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
                "new_public_key <public_address>"
                + HelpRequiringPassphrase());

    //Generate the signature
    EnsureWalletIsUnlocked();

    CWalletDB walletdb(pwalletMain->strWalletFile, "r+");

    string strSenderAddress = params[0].get_str();

    CBitcoinAddress addr(strSenderAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CPubKey pubKey = key.GetPubKey();

    string testKey;

    if (!pwalletMain->SetRSAMetadata(pubKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

    if (!walletdb.UpdateKey(pubKey, pwalletMain->mapKeyMetadata[pubKey.GetID()]))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to write meta data for key");

    if (!pwalletMain->GetRSAPublicKeyMetadata(pubKey, testKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

    string pKey;
    if (!pwalletMain->GetRSAPrivateKeyMetadata(pubKey, pKey))
        throw JSONRPCError(RPC_TYPE_ERROR, "Failed to load meta data for key");

    vector<Value> res;
    res.push_back(strSenderAddress);
    res.push_back(pKey);
    res.push_back(testKey);
    return res;
}

Value public_key_send(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
                "public_key_send <sender> <recipient>"
                + HelpRequiringPassphrase());

    //Generate the signature
    EnsureWalletIsUnlocked();

    string strSenderAddress = params[0].get_str();
    string strRecipientAddress = params[1].get_str();

    CBitcoinAddress addr(strSenderAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");


    string rsaPubKeyStr = "";
    if (!pwalletMain->GetRSAPublicKeyMetadata(key.GetPubKey(), rsaPubKeyStr))
        throw JSONRPCError(RPC_WALLET_ERROR, "no rsa key available for address");

    CDataStream ss(SER_GETHASH, 0);
    ss << rsaPubKeyStr;

    vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    //Send the public key to the recipient
    const vchType vchSender = vchFromValue(strSenderAddress);

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;
    //if (params.size() == 2)
    //{
        uint160 hash160;
        bool isValid = AddressToHash160(strRecipientAddress, hash160);
        if (!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress(strRecipientAddress);
    //}
    //else
    //{
    //    vector<unsigned char> vchPubKey = pwalletMain->GetKeyFromKeyPool();
    //    scriptPubKeyOrig.SetBitcoinAddress(vchPubKey);
    //}
    const vchType vchKey = vchFromValue(rsaPubKeyStr);
    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    scriptPubKey << OP_PUBLIC_KEY << vchSender << vchKey << vchFromString(sigBase64) << OP_2DROP << OP_2DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        string strError = pwalletMain->SendMoney(scriptPubKey, MIN_AMOUNT, wtx, false);

        printf("  scriptPubKey %s\n", scriptPubKey.ToString().c_str());
        if (strError != "")
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        //mapMyImportedKeys[vchName] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    printf("public_key_send : tx=%s\n", wtx.GetHash().GetHex().c_str());

    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    res.push_back(sigBase64);
    return res;
}

Value message_send_plain(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
                "message_send <sender_address> <message> <recipient_address>"
                + HelpRequiringPassphrase());
    //const vchType vchName = vchFromValue(params[0]);
    const string strSenderAddress = params[0].get_str();
    const string strMessage = params[1].get_str();
    const string strRecipientAddress = params[2].get_str();

    CBitcoinAddress senderAddr(strSenderAddress);
    if (!senderAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address");

    CKeyID keyID;
    if (!senderAddr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "senderAddr does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CBitcoinAddress recipientAddr(strRecipientAddress);
    if (!recipientAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid recipient address");

    CKeyID rkeyID;
    if (!recipientAddr.GetKeyID(rkeyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "recipientAddr does not refer to key");

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessage;

    vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;
    //if (params.size() == 2)
    //{
        uint160 hash160;
        bool isValid = AddressToHash160(strRecipientAddress, hash160);
        if (!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress(strRecipientAddress);
    //}
    //else
    //{
    //    vector<unsigned char> vchPubKey = pwalletMain->GetKeyFromKeyPool();
    //    scriptPubKeyOrig.SetBitcoinAddress(vchPubKey);
    //}

    vchType vchMessage = vchFromString(strMessage);
    scriptPubKey << OP_MESSAGE << vchFromString(strSenderAddress) << vchFromString(strRecipientAddress) << vchMessage << vchFromString(sigBase64) << OP_2DROP << OP_2DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        string strError = pwalletMain->SendMoney(scriptPubKey, MIN_AMOUNT, wtx, false);

        printf("  scriptPubKey %s\n", scriptPubKey.ToString().c_str());
        if (strError != "")
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        mapMyMessages[vchMessage] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    printf("name_new : name=%s, tx=%s\n", stringFromVch(vchMessage).c_str(), wtx.GetHash().GetHex().c_str());

    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    return res;
}

Value message_send(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
                "message_send <sender_address> <message> <recipient_address>"
                + HelpRequiringPassphrase());
    //const vchType vchName = vchFromValue(params[0]);
    const string strSenderAddress = params[0].get_str();
    const string strMessage = params[1].get_str();
    const string strRecipientAddress = params[2].get_str();

    CBitcoinAddress senderAddr(strSenderAddress);
    if (!senderAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid sender address");

    CKeyID keyID;
    if (!senderAddr.GetKeyID(keyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "senderAddr does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CBitcoinAddress recipientAddr(strRecipientAddress);
    if (!recipientAddr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid recipient address");

    CKeyID rkeyID;
    if (!recipientAddr.GetKeyID(rkeyID))
        throw JSONRPCError(RPC_TYPE_ERROR, "recipientAddr does not refer to key");

    //Get the recipient's imported public rsa key
    bool found=false;
    vchType recipientAddress = vchFromString(strRecipientAddress);
    vchType publicKeyVch;
    ENTER_CRITICAL_SECTION(cs_main)
    {
        ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
        {
            BOOST_FOREACH(PAIRTYPE(const uint256, CWalletTx)& item,
                          pwalletMain->mapWallet)
            {
                const CWalletTx& tx = item.second;

                vchType vchSender, vchKey, vchSig;
                int nOut;
                if (!tx.GetPublicKeyUpdate (nOut, vchSender, vchKey, vchSig))
                    continue;

                const int nHeight = tx.GetHeightInMainChain ();
                if (nHeight == -1)
                    continue;
                assert (nHeight >= 0);

                Object oName;
                oName.push_back(Pair("sender", stringFromVch(vchSender)));
                oName.push_back(Pair("key", stringFromVch(vchKey)));
                oName.push_back(Pair("signature", stringFromVch(vchSig)));
                if(recipientAddress == vchSender)
                {
                    publicKeyVch = vchKey;
                    found=true;
                    break;
                }
            }
        }
        LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    if(!found)
        throw JSONRPCError(RPC_TYPE_ERROR, "No public key for recipient");

    //Encrypt the message using the public key
    string encrypted;
    const string publicKeyStr = stringFromVch(publicKeyVch);
    EncryptMessage(publicKeyStr, strMessage, encrypted);

    CDataStream ss(SER_GETHASH, 0);
    ss << encrypted;

    vector<unsigned char> vchSig;
    if (!key.SignCompact(Hash(ss.begin(), ss.end()), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    string sigBase64 = EncodeBase64(&vchSig[0], vchSig.size());

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    CScript scriptPubKeyOrig;
    CScript scriptPubKey;
    //if (params.size() == 2)
    //{
        uint160 hash160;
        bool isValid = AddressToHash160(strRecipientAddress, hash160);
        if (!isValid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid dions address");
        scriptPubKeyOrig.SetBitcoinAddress(strRecipientAddress);
    //}
    //else
    //{
    //    vector<unsigned char> vchPubKey = pwalletMain->GetKeyFromKeyPool();
    //    scriptPubKeyOrig.SetBitcoinAddress(vchPubKey);
    //}

    vchType vchEncryptedMessage = vchFromString(encrypted);
    scriptPubKey << OP_ENCRYPTED_MESSAGE << vchFromString(strSenderAddress) << vchFromString(strRecipientAddress) << vchEncryptedMessage << vchFromString(sigBase64) << OP_2DROP << OP_2DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        string strError = pwalletMain->SendMoney(scriptPubKey, MIN_AMOUNT, wtx, false);

        printf("  scriptPubKey %s\n", scriptPubKey.ToString().c_str());
        if (strError != "")
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        mapMyMessages[vchEncryptedMessage] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    printf("name_new : name=%s, tx=%s\n", stringFromVch(vchEncryptedMessage).c_str(), wtx.GetHash().GetHex().c_str());

    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    return res;
}

Value name_new(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
                "name_new <name>"
                + HelpRequiringPassphrase());

    const vchType vchName = vchFromValue(params[0]);

    CWalletTx wtx;
    wtx.nVersion = CTransaction::DION_TX_VERSION;

    const uint64_t rand = GetRand((uint64_t)-1);
    const vchType vchRand = CBigNum(rand).getvch();
    vchType vchToHash(vchRand);
    vchToHash.insert(vchToHash.end(), vchName.begin(), vchName.end());
    const uint160 hash =  Hash160(vchToHash);

    const vchType vchPubKey = pwalletMain->GetKeyFromKeyPool();
    CScript scriptPubKeyOrig;
    scriptPubKeyOrig.SetBitcoinAddress(vchPubKey);
    CScript scriptPubKey;
    scriptPubKey << OP_NAME_NEW << hash << OP_2DROP;
    scriptPubKey += scriptPubKeyOrig;

    ENTER_CRITICAL_SECTION(cs_main)
    {
        EnsureWalletIsUnlocked();

        string strError = pwalletMain->SendMoney(scriptPubKey, MIN_AMOUNT, wtx, false);

        printf("  scriptPubKey %s\n", scriptPubKey.ToString().c_str());
        if (strError != "")
            throw JSONRPCError(RPC_WALLET_ERROR, strError);
        mapMyNames[vchName] = wtx.GetHash();
    }
    LEAVE_CRITICAL_SECTION(cs_main)

    printf("name_new : name=%s, rand=%s, tx=%s\n", stringFromVch(vchName).c_str(), HexStr(vchRand).c_str(), wtx.GetHash().GetHex().c_str());

    vector<Value> res;
    res.push_back(wtx.GetHash().GetHex());
    res.push_back(HexStr(vchRand));
    return res;
}

void rescanfornames()
{
    printf("Scanning blockchain for names to create fast index...\n");

    /* The database should already be created although empty.  */

    CNameDB dbName("r+");
    dbName.ReconstructNameIndex();
}

bool CheckNameTxPos(const vector<CNameIndex> &vtxPos, const CDiskTxPos& txPos)
{
    if (vtxPos.empty())
        return false;

    return vtxPos.back().txPos == txPos;
}

bool CNameDB::ReconstructNameIndex()
{
    CTxDB txdb("r");
    CTxIndex txindex;
    CBlockIndex* pindex = pindexGenesisBlock;
    ENTER_CRITICAL_SECTION(pwalletMain->cs_wallet)
    {
        while (pindex)
        {
            TxnBegin();
            CBlock block;
            block.ReadFromDisk(pindex);
            int nHeight = pindex->nHeight;

            BOOST_FOREACH(CTransaction& tx, block.vtx)
            {
                if (tx.nVersion != CTransaction::DION_TX_VERSION)
                    continue;

                vector<vector<unsigned char> > vvchArgs;
                int op;
                int nOut;

                if (!DecodeNameTx(tx, op, nOut, vvchArgs))
                    continue;

                if (op == OP_NAME_NEW)
                    continue;
                const vector<unsigned char> &vchName = vvchArgs[0];
                const vector<unsigned char> &vchValue = vvchArgs[op == OP_NAME_FIRSTUPDATE ? 2 : 1];

                if(!txdb.ReadDiskTx(tx.GetHash(), tx, txindex))
                    continue;

                CDiskTxPos prevTxPos;

                vector<CNameIndex> vtxPos;
                if (ExistsName(vchName))
                {
                    if (!ReadName(vchName, vtxPos))
                        return error("Rescanfornames() : failed to read from name DB");
                }

                CNameIndex txPos2;
                txPos2.nHeight = nHeight;
                txPos2.vValue = vchValue;
                txPos2.txPos = txindex.pos;
                vtxPos.push_back(txPos2);
                if (!WriteName(vchName, vtxPos))
                {
                    return error("Rescanfornames() : failed to write to name DB");
                }

                //if (AddToWalletIfInvolvingMe(tx, &block, fUpdate))
                //    ret++;
            }
            pindex = pindex->pnext;
            TxnCommit();
        }
    }
    LEAVE_CRITICAL_SECTION(pwalletMain->cs_wallet)
}

CHooks* InitHook()
{
    return new CDionsHooks();
}

bool DecodeNameScript(const CScript& script, int& op, vector<vector<unsigned char> > &vvch)
{
  CScript::const_iterator pc = script.begin();
  return DecodeNameScript(script, op, vvch, pc);
}

bool DecodeNameScript(const CScript& script, int& op, vector<vector<unsigned char> > &vvch, CScript::const_iterator& pc)
{
    opcodetype opcode;
    if (!script.GetOp(pc, opcode))
        return false;
    if (opcode < OP_1 || opcode > OP_16)
        return false;

    op = opcode - OP_1 + 1;

    for (;;) {
        vector<unsigned char> vch;
        if (!script.GetOp(pc, opcode, vch))
            return false;
        if (opcode == OP_DROP || opcode == OP_2DROP || opcode == OP_NOP)
            break;
        if (!(opcode >= 0 && opcode <= OP_PUSHDATA4))
            return false;
        vvch.push_back(vch);
    }

    // move the pc to after any DROP or NOP
    while (opcode == OP_DROP || opcode == OP_2DROP || opcode == OP_NOP)
    {
        if (!script.GetOp(pc, opcode))
            break;
    }

    pc--;

    if ((op == OP_NAME_NEW && vvch.size() == 1) ||
            (op == OP_NAME_FIRSTUPDATE && vvch.size() == 3) ||
            (op == OP_MESSAGE) ||
            (op == OP_ENCRYPTED_MESSAGE) ||
            (op == OP_PUBLIC_KEY) ||
            (op == OP_NAME_UPDATE && vvch.size() == 2))
        return true;
    return error("invalid number of arguments for name op");
}

bool DecodeMessageTx(const CTransaction& tx, int& op, int& nOut, vector<vector<unsigned char> >& vvch )
{
    bool found = false;

    for (int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& out = tx.vout[i];

        vector<vector<unsigned char> > vvchRead;

        if (DecodeNameScript(out.scriptPubKey, op, vvchRead))
        {
            // If more than one name op, fail
            if (found)
            {
                vvch.clear();
                return false;
            }
            nOut = i;
            found = true;
            vvch = vvchRead;
        }
    }

    if (!found)
        vvch.clear();

    return found;
}

bool DecodeNameTx(const CTransaction& tx, int& op, int& nOut, vector<vector<unsigned char> >& vvch )
{
    bool found = false;

    for (int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& out = tx.vout[i];

        vector<vector<unsigned char> > vvchRead;

        if (DecodeNameScript(out.scriptPubKey, op, vvchRead))
        {
            // If more than one name op, fail
            if (found)
            {
                vvch.clear();
                return false;
            }
            nOut = i;
            found = true;
            vvch = vvchRead;
        }
    }

    if (!found)
        vvch.clear();

    return found;
}

bool GetValueOfNameTx(const CTransaction& tx, vector<unsigned char>& value)
{
    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    if (!DecodeNameTx(tx, op, nOut, vvch))
        return false;

    switch (op)
    {
    case OP_NAME_NEW:
        return false;
    case OP_NAME_FIRSTUPDATE:
        value = vvch[2];
        return true;
    case OP_NAME_UPDATE:
        value = vvch[1];
        return true;
    default:
        return false;
    }
}

int IndexOfNameOutput(const CTransaction& tx)
{
    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    bool good = DecodeNameTx(tx, op, nOut, vvch);

    if (!good)
        throw runtime_error("IndexOfNameOutput() : name output not found");
    return nOut;
}

bool CDionsHooks::IsMine(const CTransaction& tx)
{
    if (tx.nVersion != CTransaction::DION_TX_VERSION)
        return false;

    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    // We do the check under the correct rule set (post-hardfork)
    bool good = DecodeNameTx(tx, op, nOut, vvch);

    if (!good)
    {
        error("IsMine() hook : no output out script in name tx %s\n", tx.ToString().c_str());
        return false;
    }

    const CTxOut& txout = tx.vout[nOut];
    if (IsMyName(tx, txout))
    {
        //printf("IsMine() hook : found my transaction %s nout %d\n", tx.GetHash().GetHex().c_str(), nOut);
        return true;
    }
    return false;
}

bool CDionsHooks::IsMine(const CTransaction& tx, const CTxOut& txout, bool ignore_name_new /* = false*/)
{
    if (tx.nVersion != CTransaction::DION_TX_VERSION)
        return false;

    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    if (!DecodeNameScript(txout.scriptPubKey, op, vvch))
        return false;

    if (ignore_name_new && op == OP_NAME_NEW)
        return false;

    if (IsMyName(tx, txout))
    {
        //printf("IsMine() hook : found my transaction %s value %ld\n", tx.GetHash().GetHex().c_str(), txout.nValue);
        return true;
    }
    return false;
}

bool CDionsHooks::AcceptToMemoryPool(const CTransaction& tx)
{
    if (tx.nVersion != CTransaction::DION_TX_VERSION)
        return true;

    if (tx.vout.size() < 1)
      return error ("AcceptToMemoryPool: no output in name tx %s\n",
                    tx.GetHash ().ToString ().c_str ());

    std::vector<vchType> vvch;

    int op;
    int nOut;

    bool good = DecodeNameTx(tx, op, nOut, vvch);

    if (!good)
      return error ("AcceptToMemoryPool: no output out script in name tx %s",
                    tx.GetHash ().ToString ().c_str ());

    if (op == OP_NAME_NEW)
    {
        const vchType& hash = vvch[0];
        if (setNewHashes.count (hash) > 0)
          return error ("AcceptToMemoryPool: duplicate name_new hash in tx %s",
                        tx.GetHash ().ToString ().c_str ());
        setNewHashes.insert (hash);
    }
    else
    {
        ENTER_CRITICAL_SECTION(cs_main)
        {
            mapNamePending[vvch[0]].insert (tx.GetHash ());
        }
        LEAVE_CRITICAL_SECTION(cs_main)
    }

    return true;
}

void CDionsHooks::RemoveFromMemoryPool(const CTransaction& tx)
{
    if (tx.nVersion != CTransaction::DION_TX_VERSION)
        return;

    if (tx.vout.size() < 1)
        return;

    vector<vector<unsigned char> > vvch;

    int op;
    int nOut;

    if (!DecodeNameTx(tx, op, nOut, vvch))
        return;

    if (op != OP_NAME_NEW)
    {
        ENTER_CRITICAL_SECTION(cs_main)
        {
            std::map<std::vector<unsigned char>, std::set<uint256> >::iterator mi = mapNamePending.find(vvch[0]);
            if (mi != mapNamePending.end())
                mi->second.erase(tx.GetHash());
        }
        LEAVE_CRITICAL_SECTION(cs_main)
    }
}

int CheckTransactionAtRelativeDepth(CBlockIndex* pindexBlock, CTxIndex& txindex, int maxDepth)
{
    for (CBlockIndex* pindex = pindexBlock; pindex && pindexBlock->nHeight - pindex->nHeight < maxDepth; pindex = pindex->pprev)
        if (pindex->nBlockPos == txindex.pos.nBlockPos && pindex->nFile == txindex.pos.nFile)
            return pindexBlock->nHeight - pindex->nHeight;
    return -1;
}

bool CDionsHooks::ConnectInputs (map<uint256, CTxIndex>& mapTestPool,
                              const CTransaction& tx,
                               vector<CTransaction>& vTxPrev,
                               vector<CTxIndex>& vTxindex,
                               CBlockIndex* pindexBlock, CDiskTxPos& txPos,
                               bool fBlock, bool fMiner)
{
    CNameDB nameDb("r+");
    int nInput;
    bool found = false;

    int prevOp;
    std::vector<vchType> vvchPrevArgs;

    // Strict check - bug disallowed
    for (int i = 0; i < tx.vin.size(); i++)
    {
        const CTxOut& out = vTxPrev[i].vout[tx.vin[i].prevout.n];
        std::vector<vchType> vvchPrevArgsRead;

        if (DecodeNameScript(out.scriptPubKey, prevOp, vvchPrevArgsRead))
        {
            if (found)
                return error("ConnectInputHook() : multiple previous name transactions");
            found = true;
            nInput = i;

          vvchPrevArgs = vvchPrevArgsRead;
        }
    }
    if (tx.nVersion != CTransaction::DION_TX_VERSION)
    {
        /* See if there are any name outputs.  If they are, disallow
           for mempool or after the corresponding soft fork point.  Note
           that we can't just use 'DecodeNameTx', since that would also
           report "false" if we have *multiple* name outputs.  This should
           also be detected, though.  */
        bool foundOuts = false;
        for (int i = 0; i < tx.vout.size(); i++)
        {
            const CTxOut& out = tx.vout[i];

            std::vector<vchType> vvchRead;
            int opRead;

            if (DecodeNameScript(out.scriptPubKey, opRead, vvchRead))
                foundOuts = true;
        }

        // Make sure name-op outputs are not spent by a regular transaction, or the name
        // would be lost
        if (found)
            return error("ConnectInputHook() : a non-dions transaction with a dions input");
        return true;
    }

    std::vector<vchType> vvchArgs;
    int op;
    int nOut;

    bool good = DecodeNameTx(tx, op, nOut, vvchArgs);
    if (!good)
        return error("ConnectInputsHook() : could not decode a dions tx");

    int nPrevHeight;
    int nDepth;

    /* Enforce locked name coin amount if we are beyond the fork point.  Also
       miners (generating new blocks) and checks for inclusion into the mempool
       enforce the fee, even before the fork point.  */
    if (tx.vout[nOut].nValue < MIN_AMOUNT)
    {
        if (!fBlock )
            return error ("ConnectInputsHook: not enough locked amount");
        printf ("WARNING: not enough locked amount, ignoring for now\n");
    }

    // HACK: The following two checks are redundant after hard-fork at block 150000, because it is performed
    // in CheckTransaction. However, before that, we do not know height during CheckTransaction
    // and cannot apply the right set of rules
    if (vvchArgs[0].size() > MAX_NAME_LENGTH)
        return error("name transaction with name too long");

    switch (op)
    {
    case OP_PUBLIC_KEY:
        printf("OP_PUBLIC_KEY");
        const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
        const std::string pkey(vvchArgs[1].begin(), vvchArgs[1].end());
        const std::string sig(stringFromVch(vvchArgs[2]));
        printf("sender : %s\n", sender.c_str());
        printf("key : %s\n", pkey.c_str());
        printf("signature : %s\n", sig.c_str());

        CBitcoinAddress addr(sender);

        if (!addr.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

        CKeyID keyID;
        if (!addr.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

        bool fInvalid = false;
        vector<unsigned char> vchSig = DecodeBase64(sig.c_str(), &fInvalid);

        if (fInvalid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

        CDataStream ss(SER_GETHASH, 0);
        ss << pkey;

        CKey key;
        if (!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
            return false;

        if(key.GetPubKey().GetID() != keyID)
            return error("public key tx verification failed");

        break;

    case OP_ENCRYPTED_MESSAGE:
        printf("OP_ENCRYPTED_MESSAGE");
        const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
        const std::string recipient(vvchArgs[1].begin(), vvchArgs[1].end());
        const std::string encrypted(vvchArgs[2].begin(), vvchArgs[2].end());
        const std::string sig(stringFromVch(vvchArgs[3]));
        printf("sender : %s\n", sender.c_str());
        printf("recipient: %s\n", recipient.c_str());
        printf("encrypted message : %s\n", encrypted.c_str());
        printf("signature : %s\n", sig.c_str());

        CBitcoinAddress addr(sender);

        if (!addr.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

        CKeyID keyID;
        if (!addr.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

        bool fInvalid = false;
        vector<unsigned char> vchSig = DecodeBase64(sig.c_str(), &fInvalid);

        if (fInvalid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

        CDataStream ss(SER_GETHASH, 0);
        ss << encrypted;

        CKey key;
        if (!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
            return false;

        if(key.GetPubKey().GetID() != keyID)
            return error("encrypted message tx verification failed");

        break;

    case OP_MESSAGE:
        printf("OP_MESSAGE");
        const std::string sender(vvchArgs[0].begin(), vvchArgs[0].end());
        const std::string recipient(vvchArgs[1].begin(), vvchArgs[1].end());
        const std::string message(vvchArgs[2].begin(), vvchArgs[2].end());
        const std::string sig(stringFromVch(vvchArgs[3]));
        printf("sender : %s\n", sender.c_str());
        printf("recipient: %s\n", recipient.c_str());
        printf("message : %s\n", message.c_str());
        printf("signature : %s\n", sig.c_str());

        CBitcoinAddress addr(sender);

        if (!addr.IsValid())
            throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

        CKeyID keyID;
        if (!addr.GetKeyID(keyID))
            throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

        bool fInvalid = false;
        vector<unsigned char> vchSig = DecodeBase64(sig.c_str(), &fInvalid);

        if (fInvalid)
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");

        CDataStream ss(SER_GETHASH, 0);
        ss << message;

        CKey key;
        if (!key.SetCompactSignature(Hash(ss.begin(), ss.end()), vchSig))
            return false;

        if (key.GetPubKey().GetID() != keyID)
            return error("encrypted message tx verification failed");

        break;

    case OP_NAME_NEW:
        if (found)
            return error("ConnectInputsHook() : name_new tx pointing to previous dions tx");

        // HACK: The following check is redundant after hard-fork at block 150000, because it is performed
        // in CheckTransaction. However, before that, we do not know height during CheckTransaction
        // and cannot apply the right set of rules
        if (vvchArgs[0].size() != 20)
            return error("name_new tx with incorrect hash length");

        break;

    case OP_NAME_FIRSTUPDATE:
        if (!found || prevOp != OP_NAME_NEW)
            return error("ConnectInputsHook() : name_firstupdate tx without previous name_new tx");

        // HACK: The following two checks are redundant after hard-fork at block 150000, because it is performed
        // in CheckTransaction. However, before that, we do not know height during CheckTransaction
        // and cannot apply the right set of rules
        if (vvchArgs[1].size() > 20)
            return error("name_firstupdate tx with rand too big");
        if (vvchArgs[2].size() > MAX_VALUE_LENGTH)
            return error("name_firstupdate tx with value too long");

        {
            // Check hash
            const vchType& vchHash = vvchPrevArgs[0];
            const vchType& vchName = vvchArgs[0];
            const vchType& vchRand = vvchArgs[1];
            vchType vchToHash(vchRand);
            vchToHash.insert(vchToHash.end(), vchName.begin(), vchName.end());
            uint160 hash = Hash160(vchToHash);
            if (uint160(vchHash) != hash)
            {
                    return error("ConnectInputsHook() : name_firstupdate hash mismatch");
            }
        }

        nPrevHeight = GetNameHeight (vvchArgs[0]);
        if (nPrevHeight >= 0 && pindexBlock->nHeight - nPrevHeight < GetExpirationDepth(pindexBlock->nHeight))
            return error("ConnectInputsHook() : name_firstupdate on an unexpired name");

        nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], MIN_FIRSTUPDATE_DEPTH);
        // Do not accept if in chain and not mature
        if ((fBlock || fMiner) && nDepth >= 0 && nDepth < MIN_FIRSTUPDATE_DEPTH)
            return false;

        // Do not mine if previous name_new is not visible.  This is if
        // name_new expired or not yet in a block
        if (fMiner)
        {
            // TODO CPU intensive
            nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], GetExpirationDepth(pindexBlock->nHeight));
            if (nDepth == -1)
                return error("ConnectInputsHook() : name_firstupdate cannot be mined if name_new is not already in chain and unexpired");
            // Check that no other pending txs on this name are already in the block to be mined
            set<uint256>& setPending = mapNamePending[vvchArgs[0]];
            BOOST_FOREACH(const PAIRTYPE(uint256, CTxIndex)& s, mapTestPool)
            {
                if (setPending.count(s.first))
                {
                    printf("ConnectInputsHook() : will not mine %s because it clashes with %s",
                            tx.GetHash().GetHex().c_str(),
                            s.first.GetHex().c_str());
                    return false;
                }
            }
        }
        break;

    case OP_NAME_UPDATE:
        if (!found || (prevOp != OP_NAME_FIRSTUPDATE && prevOp != OP_NAME_UPDATE))
            return error("name_update tx without previous update tx");

        // HACK: The following check is redundant after hard-fork at block 150000, because it is performed
        // in CheckTransaction. However, before that, we do not know height during CheckTransaction
        // and cannot apply the right set of rules
        if (vvchArgs[1].size() > MAX_VALUE_LENGTH)
            return error("name_update tx with value too long");

        // Check name
        if (vvchPrevArgs[0] != vvchArgs[0])
        {
                return error("ConnectInputsHook() : name_update name mismatch");
        }

        // TODO CPU intensive
        nDepth = CheckTransactionAtRelativeDepth(pindexBlock, vTxindex[nInput], GetExpirationDepth(pindexBlock->nHeight));
        if ((fBlock || fMiner) && nDepth < 0)
            return error("ConnectInputsHook() : name_update on an expired name, or there is a pending transaction on the name");
        break;

    default:
        return error("ConnectInputsHook() : name transaction has unknown op");
    }

    if (!fBlock && op == OP_NAME_UPDATE)
    {
        vector<CNameIndex> vtxPos;
        if (nameDb.ExistsName (vvchArgs[0])
            && !nameDb.ReadName (vvchArgs[0], vtxPos))
          return error("ConnectInputsHook() : failed to read from name DB");
        // Valid tx on top of buggy tx: if not in block, reject
        if (!CheckNameTxPos(vtxPos, vTxindex[nInput].pos))
            return error("ConnectInputsHook() : Name bug workaround: tx %s rejected, since previous tx (%s) is not in the name DB\n", tx.GetHash().ToString().c_str(), vTxPrev[nInput].GetHash().ToString().c_str());
    }

    if (fBlock)
    {
        if (op == OP_NAME_FIRSTUPDATE || op == OP_NAME_UPDATE)
        {
            //vector<CDiskTxPos> vtxPos;
            vector<CNameIndex> vtxPos;
            if (nameDb.ExistsName (vvchArgs[0])
                && !nameDb.ReadName (vvchArgs[0], vtxPos))
              return error("ConnectInputsHook() : failed to read from name DB");

            if (op == OP_NAME_UPDATE && !CheckNameTxPos(vtxPos, vTxindex[nInput].pos))
            {
                printf("ConnectInputsHook() : Name bug workaround: tx %s rejected, since previous tx (%s) is not in the name DB\n", tx.GetHash().ToString().c_str(), vTxPrev[nInput].GetHash().ToString().c_str());
            }

            vector<unsigned char> vchValue; // add
            int nHeight;
            uint256 hash;
            GetValueOfTxPos(txPos, vchValue, hash, nHeight);
            CNameIndex txPos2;
            txPos2.nHeight = pindexBlock->nHeight;
            txPos2.vValue = vchValue;
            txPos2.txPos = txPos;
            vtxPos.push_back(txPos2); // fin add
            if (!nameDb.WriteName (vvchArgs[0], vtxPos))
              return error("ConnectInputsHook() : failed to write to name DB");
        }

        if (op != OP_NAME_NEW)
        {
            ENTER_CRITICAL_SECTION(cs_main)
            {
                std::map<std::vector<unsigned char>, std::set<uint256> >::iterator mi = mapNamePending.find(vvchArgs[0]);
                if (mi != mapNamePending.end())
                    mi->second.erase(tx.GetHash());
            }
            LEAVE_CRITICAL_SECTION(cs_main)
         }
    }

    return true;
}

unsigned char GetAddressVersion() { return ((unsigned char)(fTestNet ? 111 : 103)); }

Value name_rescan(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
                "name_rescan\n"
                "rescan for names"
                );

    rescanfornames();

    Array oRes;
    return oRes;
}
