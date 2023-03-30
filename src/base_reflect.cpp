// Copyright (c) 2018 The I/O Coin developers
//
//
#include "db.h"
#include "txdb-leveldb.h"
#include "keystore.h"
#include "wallet.h"
#include "init.h"
#include "base_reflect.h"
//#include "intx/intx.hpp"
#include "uint256.h"

#include "bitcoinrpc.h"
#include "main.h"
#include "ptrie/TrieDB.h"
#include "ptrie/StateCacheDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "ptrie/Account.h"
#include "ptrie/DBFactory.h"
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
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <dvmone/dvmone.h>
#include <dvmc/transitional_node.hpp>
#include <dvmc/dvmc.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/loader.h>
#include <dvmc/tooling.hpp>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using namespace std;
using dvmc::operator""_address;
using namespace json_spirit;
using namespace std;
using namespace json_spirit;
using namespace boost::iostreams;

namespace fs = boost::filesystem;

extern LocatorNodeDB* ln1Db;
extern bool aliasTx(LocatorNodeDB& aliasCacheDB, const vector<unsigned char> &vchPath, CTransaction& tx);
extern unsigned int scaleMonitor();
extern string txRelay(const CScript& scriptPubKey, int64_t nValue, const __wx__Tx& wtxIn, __wx__Tx& wtxNew, bool fAskFee);
extern bool getVertex__(const string& target, string& code);
extern void testGen(dvmc::TransitionalNode& acc,string& contractCode);
extern bool aliasAddress(const CTransaction& tx, std::string& strAddress);
extern Object JSONRPCError(int code, const string& message);
extern Value xtu_url__(const string& url);
extern dev::OverlayDB* overlayDB_;
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
typedef struct vx_bytes32
{
    //The 32 bytes.
    uint8_t bytes[32];
} vx_bytes32;

// * The alias for evmc_bytes32 to represent a big-endian 256-bit integer.
typedef struct vx_bytes32 evmc_uint256be;

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
    string locatorStr = origin;
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

bool vertex_serial_aux_cycle(const string& origin, const string& data, __wx__Tx& serial_n)
{
    string locatorStr = origin;
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

        string strError = pwalletMain->transientRelay(scriptPubKey, CTRL__, serial_n, false);

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

__wx__Tx aux_cycle(string& locatorStr, const string& tStr, const string& iStr)
{
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
    return vertex;
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

bool section_vertex_serial_n(const string& origin, const string& data, __wx__Tx& serial_n)
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

    //ENTER_CRITICAL_SECTION(cs_main)
    //{
    //    EnsureWalletIsUnlocked();

    //    string strError = pwalletMain->SendMoney__(scriptPubKey, CTRL__, serial_n, false);
    vector< pair<CScript, int64_t> > vecSend;
    vecSend.push_back(make_pair(scriptPubKey, CTRL__));


    //    if(strError != "")
    //    {
    //        LEAVE_CRITICAL_SECTION(cs_main)
    //        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    //    }
    //    mapLocator[vchPath] = serial_n.GetHash();
    //}
    //LEAVE_CRITICAL_SECTION(cs_main)

    return true;
}
bool sectionVertex(const string& target, string& code)
{
    bool found=false;
    LocatorNodeDB ln1Db("r");

    Dbc* cursorp;
    try
    {
        cursorp = ln1Db.GetCursor();

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
                Object o;
                vchType k2;
                ssKey >> k2;
                string a = stringFromVch(k2);
                if(a == target)
                {
                    vector<PathIndex> vtxPos;
                    CDataStream ssValue((char*)data.get_data(), (char*)data.get_data() + data.get_size(), SER_DISK, CLIENT_VERSION);
                    ssValue >> vtxPos;

                    PathIndex i = vtxPos.back();
                    string i_address = i.vAddress;
                    code = stringFromVch(i.vValue);
                    found=true;

                }
            }
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

    return found;
}
void sectionTest(std::vector<CTransaction>& testTxVector)
{
    for(int index=0; index<testTxVector.size(); index++)
    {
        cba cAddr_;
        CTransaction& tx = testTxVector[index];
        if(tx.nVersion == CTransaction::CYCLE_TX_VERSION)
        {
            string origin;
            for(int i = 0; i < tx.vout.size(); i++)
            {
                const CTxOut& out = tx.vout[i];
                std::vector<vchType> vvchArgs;

                int prevOp;
                if(aliasScript(out.scriptPubKey, prevOp, vvchArgs))
                {
                    origin = stringFromVch(vvchArgs[1]);
                }
            }
            std::vector<string> contract_execution_data;
            boost::char_separator<char> tok(":");
            boost::tokenizer<boost::char_separator<char>> tokens(origin,tok);
            BOOST_FOREACH(const string& s,tokens)
            {
                contract_execution_data.push_back(s);
            }
            string target_contract = contract_execution_data[0];
            {
                vector<PathIndex> vtxPos;
                vchType vchPath = vchFromString(target_contract);
                if (ln1Db->lKey (vchPath))
                {
                    printf("  name exists\n");
                    if (!ln1Db->lGet (vchPath, vtxPos))
		    {
			    std::cout << "error read : name db" << std::endl;
		    }
                    if (vtxPos.empty ())
		    {
			    std::cout << "error vec : empty" << std::endl;
	            }

                    PathIndex& txPos = vtxPos.back ();
                    cAddr_.SetString(txPos.vAddress);
                }
                else
                {
                    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias");
                }
            }

            string contract_input  = contract_execution_data[1];
            string contractCode;

            bool f = sectionVertex(target_contract,contractCode);
            if(f != true)
                continue;

            CKeyID keyID;
            cAddr_.GetKeyID(keyID);
            dev::Address target_addr(keyID.GetHex().c_str());

            dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(overlayDB_);
            state.init();
            string account_str = state.at(target_addr);
            dev::RLP rlp_state(account_str,0);
            auto const targetAccStorageRoot = rlp_state[2].toHash<dev::h256>();
            auto const nonce = rlp_state[0].toInt<dev::u256>();
            auto const balance = rlp_state[1].toInt<dev::u256>();
            auto const codeHash = rlp_state[3].toHash<dev::h256>();
            dev::eth::Account retrievedAcc(nonce,balance,targetAccStorageRoot,codeHash,0,dev::eth::Account::Unchanged);
            string code_hex_str = contractCode;
            uint256 r;
            string val;
            dvmc::TransitionalNode account_recon;
            {
                map<dev::h256, pair<dev::bytes, dev::bytes>> ret_;
                ret_.clear();
                {
                    {
                        dev::SecureTrieDB<dev::h256, dev::OverlayDB> memdb(const_cast<dev::OverlayDB*>(overlayDB_), targetAccStorageRoot);

                        for (auto it = memdb.hashedBegin(); it != memdb.hashedEnd(); ++it)
                        {
                            dev::h256 const hashedKey((*it).first);
                            auto const key = it.key();
                            dev::bytes const value = dev::RLP((*it).second).toBytes();
                            ret_[hashedKey] = make_pair(key, value);
                        }
                    }
                }
                std::unordered_map<dvmc::bytes32, dvmc::storage_value>& storage_recon = account_recon.storage;
                for(auto i_ : ret_)
                {
                    dvmc::bytes32 reconKey;
                    dev::bytes key = i_.second.first;
                    for(int idx=0; idx<32; idx++)
                        reconKey.bytes[idx] = key[idx];
                    dvmc::storage_value sv;
                    for(int idx=0; idx<32; idx++)
                        sv.value.bytes[idx] = i_.second.second[idx];

                    storage_recon[reconKey] = sv;
                }
            }
            {
                testGen(account_recon,code_hex_str);

                dvmc::VertexNode vTrans;
                dvmc_address create_address;
                uint160 h160;
                AddressToHash160(cAddr_.ToString(),h160);
                memcpy(create_address.bytes,h160.pn,20);
                vTrans.accounts[create_address] = account_recon;
                const auto input = dvmc::from_hex(contract_input);
                dvmc_message msg{};
                msg.recipient = create_address;
                msg.track = std::numeric_limits<int64_t>::max();
                msg.input_data = input.data();
                msg.input_size = input.size();
                dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
                dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
                const auto result = vm.retrieve_desc_vx(vTrans, rev, msg, account_recon.code.data(), account_recon.code.size());
                dvmc::TransitionalNode updatedAcc = vTrans.accounts[create_address];

                dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(overlayDB_);
                state.init();
                {
                    dev::u256 nonce = 12345678;
                    dev::u256 balance = 1010101010101;
                    dev::RLPStream s(4);
                    s << nonce << balance;

                    {
                        dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state.db(), retrievedAcc.baseRoot());
                        for(auto pair : updatedAcc.storage)
                        {
                            auto storage_key = pair.first.bytes;
                            dev::bytes key;
                            for(int i=0; i<32; i++)
                                key.push_back(storage_key[i]);

                            dev::h256 key256(key);

                            auto storage_bytes = pair.second.value;
                            dev::bytes val;
                            for(int i=0; i<32; i++)
                                val.push_back(storage_bytes.bytes[i]);
                            storageDB.insert(key256, dev::rlp(val));
                        }
                        s << storageDB.root();
                    }

                    s << retrievedAcc.codeHash();
                    state.insert(target_addr, &s.out());
                    overlayDB_->commit();
                }
            }
        }
    }

}

//create approve transfer from
//erc20 cham token contract
std::string code_hex_str = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200152838038062001528833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6111ef80620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff161415610ece57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610f1a57600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610fa757600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401146111b457fe5b5050505056fea265627a7a723158205d49fa73a530923e8947e52c7716fccdab177cd126ce5642208726148b661e2064736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000000d6368616d2d32393130323032320000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000126368616d656c656f6e2d32393130323032320000000000000000000000000000";
//approve payload 0x095ea7b30000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000032
//transfer_from payload 0x23b872dd0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000023

//For this erc20 code - the require check on  balance plus the _transfer call were removed
string contract_transfer_no_check_no__transfer = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200149438038062001494833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b61115b80620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561087b565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b810190808035906020019092919050505061088e565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610990565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506109a8565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610bbe565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610c5c565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610c73565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610ddb565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b600081600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156108dc57600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156109f657600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610a7f57600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610c545780601f10610c2957610100808354040283529160200191610c54565b820191906000526020600020905b815481529060010190602001808311610c3757829003601f168201915b505050505081565b6000610c69338484610e00565b6001905092915050565b600080849050610c8385856106ec565b15610dd2578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610d61578082015181840152602081019050610d46565b50505050905090810190601f168015610d8e5780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610db057600080fd5b505af1158015610dc4573d6000803e3d6000fd5b505050506001915050610dd4565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff161415610e3a57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610e8657600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f1357600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011461112057fe5b5050505056fea265627a7a723158205cd0c528800733362a414f53d0de9cc3a44cd4d401d8d1a7fe25b00da51626bb64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000004746573740000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000047465737400000000000000000000000000000000000000000000000000000000";
string code_add_require_no_trans = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200151d3803806200151d833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6111e480620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b50610275610904565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610917565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a19565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a31565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c47565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610ce5565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610cfc565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e64565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561096557600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a7f57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b0857600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610cdd5780601f10610cb257610100808354040283529160200191610cdd565b820191906000526020600020905b815481529060010190602001808311610cc057829003601f168201915b505050505081565b6000610cf2338484610e89565b6001905092915050565b600080849050610d0c85856106ec565b15610e5b578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610dea578082015181840152602081019050610dcf565b50505050905090810190601f168015610e175780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e3957600080fd5b505af1158015610e4d573d6000803e3d6000fd5b505050506001915050610e5d565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff161415610ec357600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610f0f57600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f9c57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401146111a957fe5b5050505056fea265627a7a723158200e39f4a625dc933c525c3da67530525b17b1c85d58f8fa3e0958a4051e1b964164736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000077265717569726500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000b6e6f5f7472616e73666572000000000000000000000000000000000000000000";

string erc20__transfer_req1_commented_out = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b50604051620014ee380380620014ee833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6111b580620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610ee057600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f6d57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011461117a57fe5b5050505056fea265627a7a72315820853cd382da1ecfde57c0977c73926f83905fbb09d285ddabf78e0a04fcd713db64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000004726571310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000047265713100000000000000000000000000000000000000000000000000000000";

string erc20__transfer_req12_commented_out = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b50604051620014a2380380620014a2833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b61116980620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f2157600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011461112e57fe5b5050505056fea265627a7a723158202e8725742cad56203573046c92fc7f0cf76e425bf0d49af4aebb089db8bad81c64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000005726571313200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000057265713132000000000000000000000000000000000000000000000000000000";

string erc20__transfer_req123_commented_out = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200141538038062001415833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6110dc80620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401146110a157fe5b5050505056fea265627a7a72315820418ee216505cc5effbca7a293373664a352de7826158b3a4e59ccc6554d1a9db64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000006726571313233000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000067265713132330000000000000000000000000000000000000000000000000000";

string code_hex_08112022_trace = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b5060405162001e7538038062001e75833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b611b3c80620003396000396000f3fe608060405234801561001057600080fd5b50600436106100f55760003560e01c80636eef7a051161009757806395d89b411161006657806395d89b41146105fa578063a9059cbb1461067d578063cae9ca51146106e3578063dd62ed3e146107e0576100f5565b80636eef7a05146103ae57806370a082311461045557806374546363146104ad57806379cc679014610594576100f5565b806323b872dd116100d357806323b872dd14610201578063313ce5671461028757806342966c68146102ab5780635e57966d146102f1576100f5565b806306fdde03146100fa578063095ea7b31461017d57806318160ddd146101e3575b600080fd5b610102610858565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610142578082015181840152602081019050610127565b50505050905090810190601f16801561016f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6101c96004803603604081101561019357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506108f6565b604051808215151515815260200191505060405180910390f35b6101eb6109e8565b6040518082815260200191505060405180910390f35b61026d6004803603606081101561021757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506109ee565b604051808215151515815260200191505060405180910390f35b61028f610b19565b604051808260ff1660ff16815260200191505060405180910390f35b6102d7600480360360208110156102c157600080fd5b8101908080359060200190929190505050610b2c565b604051808215151515815260200191505060405180910390f35b6103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610c2e565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610373578082015181840152602081019050610358565b50505050905090810190601f1680156103a05780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6103da600480360360208110156103c457600080fd5b8101908080359060200190929190505050610eaf565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561041a5780820151818401526020810190506103ff565b50505050905090810190601f1680156104475780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6104976004803603602081101561046b57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611110565b6040518082815260200191505060405180910390f35b610519600480360360608110156104c357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611128565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561055957808201518184015260208101905061053e565b50505050905090810190601f1680156105865780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6105e0600480360360408110156105aa57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611389565b604051808215151515815260200191505060405180910390f35b61060261159f565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610642578082015181840152602081019050610627565b50505050905090810190601f16801561066f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6106c96004803603604081101561069357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff1690602001909291908035906020019092919050505061163d565b604051808215151515815260200191505060405180910390f35b6107c6600480360360608110156106f957600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561074057600080fd5b82018360208201111561075257600080fd5b8035906020019184600183028401116401000000008311171561077457600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050611654565b604051808215151515815260200191505060405180910390f35b610842600480360360408110156107f657600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff1690602001909291905050506117bc565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156108ee5780601f106108c3576101008083540402835291602001916108ee565b820191906000526020600020905b8154815290600101906020018083116108d157829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610a7957600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610b0e8484846117e1565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610b7a57600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060606040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610cbe5781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610cef57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d4c57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b6014811015610ea35782600485600c840160208110610d9c57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610dd457fe5b602001015160f81c60f81b826002830260020181518110610df157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c840160208110610e3557fe5b1a60f81b1660f81c60ff1681518110610e4a57fe5b602001015160f81c60f81b826002830260030181518110610e6757fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610d81565b50809350505050919050565b6060806040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610f205781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610f5157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610fae57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b60148110156111055782600486600c840160208110610ffe57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff168151811061103657fe5b602001015160f81c60f81b82600283026002018151811061105357fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84016020811061109757fe5b1a60f81b1660f81c60ff16815181106110ac57fe5b602001015160f81c60f81b8260028302600301815181106110c957fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610fe3565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b905061113b85610c2e565b61114485610c2e565b61114d83610eaf565b6040516020018082805190602001908083835b602083106111835780518252602082019150602081019050602083039250611160565b6001836020036101000a0380198251168184511680821785525050505050509050019150506040516020818303038152906040526040516020018083805190602001908083835b602083106111ed57805182526020820191506020810190506020830392506111ca565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020182805190602001908083835b602083106112665780518252602082019150602081019050602083039250611243565b6001836020036101000a038019825116818451168082178552505050505050905001925050506040516020818303038152906040526040516020018083805190602001908083835b602083106112d157805182526020820191506020810190506020830392506112ae565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020182805190602001908083835b6020831061134a5780518252602082019150602081019050602083039250611327565b6001836020036101000a038019825116818451168082178552505050505050905001925050506040516020818303038152906040529150509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156113d757600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561146057600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156116355780601f1061160a57610100808354040283529160200191611635565b820191906000526020600020905b81548152906001019060200180831161161857829003601f168201915b505050505081565b600061164a3384846117e1565b6001905092915050565b60008084905061166485856108f6565b156117b3578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015611742578082015181840152602081019050611727565b50505050905090810190601f16801561176f5780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b15801561179157600080fd5b505af11580156117a5573d6000803e3d6000fd5b5050505060019150506117b5565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff16141561181b57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561186757600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540110156118f457600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540114611b0157fe5b5050505056fea265627a7a7231582090a370df1504357cad55837085e9ed659bf0984b9ee6449a3f98e528bd51085664736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";

string code_hex_09112022_trace = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b5060405162001efe38038062001efe833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b611bc580620003396000396000f3fe608060405234801561001057600080fd5b50600436106100f55760003560e01c80636eef7a051161009757806395d89b411161006657806395d89b41146105fa578063a9059cbb1461067d578063cae9ca51146106e3578063dd62ed3e146107e0576100f5565b80636eef7a05146103ae57806370a082311461045557806374546363146104ad57806379cc679014610594576100f5565b806323b872dd116100d357806323b872dd14610201578063313ce5671461028757806342966c68146102ab5780635e57966d146102f1576100f5565b806306fdde03146100fa578063095ea7b31461017d57806318160ddd146101e3575b600080fd5b610102610858565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610142578082015181840152602081019050610127565b50505050905090810190601f16801561016f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6101c96004803603604081101561019357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506108f6565b604051808215151515815260200191505060405180910390f35b6101eb6109e8565b6040518082815260200191505060405180910390f35b61026d6004803603606081101561021757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506109ee565b604051808215151515815260200191505060405180910390f35b61028f610b19565b604051808260ff1660ff16815260200191505060405180910390f35b6102d7600480360360208110156102c157600080fd5b8101908080359060200190929190505050610b2c565b604051808215151515815260200191505060405180910390f35b6103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610c2e565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610373578082015181840152602081019050610358565b50505050905090810190601f1680156103a05780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6103da600480360360208110156103c457600080fd5b8101908080359060200190929190505050610eaf565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561041a5780820151818401526020810190506103ff565b50505050905090810190601f1680156104475780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6104976004803603602081101561046b57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611110565b6040518082815260200191505060405180910390f35b610519600480360360608110156104c357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611128565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561055957808201518184015260208101905061053e565b50505050905090810190601f1680156105865780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6105e0600480360360408110156105aa57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611412565b604051808215151515815260200191505060405180910390f35b610602611628565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610642578082015181840152602081019050610627565b50505050905090810190601f16801561066f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6106c96004803603604081101561069357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506116c6565b604051808215151515815260200191505060405180910390f35b6107c6600480360360608110156106f957600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561074057600080fd5b82018360208201111561075257600080fd5b8035906020019184600183028401116401000000008311171561077457600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f8201169050808301925050505050505091929192905050506116dd565b604051808215151515815260200191505060405180910390f35b610842600480360360408110156107f657600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611845565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156108ee5780601f106108c3576101008083540402835291602001916108ee565b820191906000526020600020905b8154815290600101906020018083116108d157829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610a7957600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610b0e84848461186a565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610b7a57600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060606040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610cbe5781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610cef57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d4c57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b6014811015610ea35782600485600c840160208110610d9c57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610dd457fe5b602001015160f81c60f81b826002830260020181518110610df157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c840160208110610e3557fe5b1a60f81b1660f81c60ff1681518110610e4a57fe5b602001015160f81c60f81b826002830260030181518110610e6757fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610d81565b50809350505050919050565b6060806040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610f205781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610f5157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610fae57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b60148110156111055782600486600c840160208110610ffe57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff168151811061103657fe5b602001015160f81c60f81b82600283026002018151811061105357fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84016020811061109757fe5b1a60f81b1660f81c60ff16815181106110ac57fe5b602001015160f81c60f81b8260028302600301815181106110c957fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610fe3565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b9050606061113d33610c2e565b905061114886610c2e565b61115186610c2e565b61115a84610eaf565b6040516020018082805190602001908083835b60208310611190578051825260208201915060208101905060208303925061116d565b6001836020036101000a038019825116818451168082178552505050505050905001915050604051602081830303815290604052836040516020018084805190602001908083835b602083106111fb57805182526020820191506020810190506020830392506111d8565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020183805190602001908083835b602083106112745780518252602082019150602081019050602083039250611251565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c206d73672e73656e6465722000000000000000000000000000000000000000815250600d0182805190602001908083835b602083106112ed57805182526020820191506020810190506020830392506112ca565b6001836020036101000a03801982511681845116808217855250505050505090500193505050506040516020818303038152906040526040516020018083805190602001908083835b602083106113595780518252602082019150602081019050602083039250611336565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020182805190602001908083835b602083106113d257805182526020820191506020810190506020830392506113af565b6001836020036101000a03801982511681845116808217855250505050505090500192505050604051602081830303815290604052925050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561146057600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548211156114e957600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156116be5780601f10611693576101008083540402835291602001916116be565b820191906000526020600020905b8154815290600101906020018083116116a157829003601f168201915b505050505081565b60006116d333848461186a565b6001905092915050565b6000808490506116ed85856108f6565b1561183c578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b838110156117cb5780820151818401526020810190506117b0565b50505050905090810190601f1680156117f85780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b15801561181a57600080fd5b505af115801561182e573d6000803e3d6000fd5b50505050600191505061183e565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff1614156118a457600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156118f057600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401101561197d57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540114611b8a57fe5b5050505056fea265627a7a72315820af7e8bbbb6d6a9741aba102f7f3596e217365cf91a782527fe8726f22d31901964736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";


//erc20 with balanceOf adjust in approve routine
//

string erc20_22112022_withdraw_with_parameter_subtract_15 = "0x60806040526012600260006101000a81548160ff021916908360ff16021790555060405162002997380380620029978339818101604052810190620000459190620002ba565b600260009054906101000a900460ff1660ff16600a620000669190620004d7565b8362000073919062000528565b600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009081620000d09190620007b4565b508060019081620000e29190620007b4565b505050506200089b565b6000604051905090565b600080fd5b600080fd5b6000819050919050565b620001158162000100565b81146200012157600080fd5b50565b60008151905062000135816200010a565b92915050565b600080fd5b600080fd5b6000601f19601f8301169050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b620001908262000145565b810181811067ffffffffffffffff82111715620001b257620001b162000156565b5b80604052505050565b6000620001c7620000ec565b9050620001d5828262000185565b919050565b600067ffffffffffffffff821115620001f857620001f762000156565b5b620002038262000145565b9050602081019050919050565b60005b838110156200023057808201518184015260208101905062000213565b60008484015250505050565b6000620002536200024d84620001da565b620001bb565b90508281526020810184848401111562000272576200027162000140565b5b6200027f84828562000210565b509392505050565b600082601f8301126200029f576200029e6200013b565b5b8151620002b18482602086016200023c565b91505092915050565b600080600060608486031215620002d657620002d5620000f6565b5b6000620002e68682870162000124565b935050602084015167ffffffffffffffff8111156200030a5762000309620000fb565b5b620003188682870162000287565b925050604084015167ffffffffffffffff8111156200033c576200033b620000fb565b5b6200034a8682870162000287565b9150509250925092565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b60008160011c9050919050565b6000808291508390505b6001851115620003e257808604811115620003ba57620003b962000354565b5b6001851615620003ca5780820291505b8081029050620003da8562000383565b94506200039a565b94509492505050565b600082620003fd5760019050620004d0565b816200040d5760009050620004d0565b8160018114620004265760028114620004315762000467565b6001915050620004d0565b60ff84111562000446576200044562000354565b5b8360020a91508482111562000460576200045f62000354565b5b50620004d0565b5060208310610133831016604e8410600b8410161715620004a15782820a9050838111156200049b576200049a62000354565b5b620004d0565b620004b0848484600162000390565b92509050818404811115620004ca57620004c962000354565b5b81810290505b9392505050565b6000620004e48262000100565b9150620004f18362000100565b9250620005207fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8484620003eb565b905092915050565b6000620005358262000100565b9150620005428362000100565b9250828202620005528162000100565b915082820484148315176200056c576200056b62000354565b5b5092915050565b600081519050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680620005c657607f821691505b602082108103620005dc57620005db6200057e565b5b50919050565b60008190508160005260206000209050919050565b60006020601f8301049050919050565b600082821b905092915050565b600060088302620006467fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8262000607565b62000652868362000607565b95508019841693508086168417925050509392505050565b6000819050919050565b6000620006956200068f620006898462000100565b6200066a565b62000100565b9050919050565b6000819050919050565b620006b18362000674565b620006c9620006c0826200069c565b84845462000614565b825550505050565b600090565b620006e0620006d1565b620006ed818484620006a6565b505050565b5b81811015620007155762000709600082620006d6565b600181019050620006f3565b5050565b601f82111562000764576200072e81620005e2565b6200073984620005f7565b8101602085101562000749578190505b620007616200075885620005f7565b830182620006f2565b50505b505050565b600082821c905092915050565b6000620007896000198460080262000769565b1980831691505092915050565b6000620007a4838362000776565b9150826002028217905092915050565b620007bf8262000573565b67ffffffffffffffff811115620007db57620007da62000156565b5b620007e78254620005ad565b620007f482828562000719565b600060209050601f8311600181146200082c576000841562000817578287015190505b62000823858262000796565b86555062000893565b601f1984166200083c86620005e2565b60005b8281101562000866578489015182556001820191506020850194506020810190506200083f565b8683101562000886578489015162000882601f89168262000776565b8355505b6001600288020188555050505b505050505050565b6120ec80620008ab6000396000f3fe6080604052600436106101095760003560e01c806370a0823111610095578063a9059cbb11610064578063a9059cbb146103cb578063cae9ca5114610408578063d0e30db014610445578063dd62ed3e1461044f578063f665b8d81461048c57610109565b806370a08231146102e9578063745463631461032657806379cc67901461036357806395d89b41146103a057610109565b8063313ce567116100dc578063313ce567146101de57806342966c6814610209578063486556ce146102465780635e57966d1461026f5780636eef7a05146102ac57610109565b806306fdde031461010e578063095ea7b31461013957806318160ddd1461017657806323b872dd146101a1575b600080fd5b34801561011a57600080fd5b506101236104c9565b60405161013091906117fb565b60405180910390f35b34801561014557600080fd5b50610160600480360381019061015b91906118c5565b610557565b60405161016d9190611920565b60405180910390f35b34801561018257600080fd5b5061018b61068d565b604051610198919061194a565b60405180910390f35b3480156101ad57600080fd5b506101c860048036038101906101c39190611965565b610693565b6040516101d59190611920565b60405180910390f35b3480156101ea57600080fd5b506101f36107c7565b60405161020091906119d4565b60405180910390f35b34801561021557600080fd5b50610230600480360381019061022b91906119ef565b6107da565b60405161023d9190611920565b60405180910390f35b34801561025257600080fd5b5061026d600480360381019061026891906119ef565b6108ee565b005b34801561027b57600080fd5b5061029660048036038101906102919190611a1c565b610947565b6040516102a391906117fb565b60405180910390f35b3480156102b857600080fd5b506102d360048036038101906102ce9190611a7f565b610c52565b6040516102e091906117fb565b60405180910390f35b3480156102f557600080fd5b50610310600480360381019061030b9190611a1c565b610f3e565b60405161031d919061194a565b60405180910390f35b34801561033257600080fd5b5061034d60048036038101906103489190611965565b610f56565b60405161035a91906117fb565b60405180910390f35b34801561036f57600080fd5b5061038a600480360381019061038591906118c5565b61100e565b6040516103979190611920565b60405180910390f35b3480156103ac57600080fd5b506103b561123f565b6040516103c291906117fb565b60405180910390f35b3480156103d757600080fd5b506103f260048036038101906103ed91906118c5565b6112cd565b6040516103ff9190611920565b60405180910390f35b34801561041457600080fd5b5061042f600480360381019061042a9190611be1565b6112e4565b60405161043c9190611920565b60405180910390f35b61044d61137d565b005b34801561045b57600080fd5b5061047660048036038101906104719190611c50565b6113d5565b604051610483919061194a565b60405180910390f35b34801561049857600080fd5b506104b360048036038101906104ae9190611a1c565b6113fa565b6040516104c0919061194a565b60405180910390f35b600080546104d690611cbf565b80601f016020809104026020016040519081016040528092919081815260200182805461050290611cbf565b801561054f5780601f106105245761010080835404028352916020019161054f565b820191906000526020600020905b81548152906001019060200180831161053257829003601f168201915b505050505081565b600081600660003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161067b919061194a565b60405180910390a36001905092915050565b60035481565b6000600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561071e57600080fd5b81600660008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546107aa9190611d1f565b925050819055506107bc848484611412565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561082857600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546108779190611d1f565b9250508190555081600360008282546108909190611d1f565b925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040516108dd919061194a565b60405180910390a260019050919050565b600f816108fb9190611d1f565b90503373ffffffffffffffffffffffffffffffffffffffff166108fc829081150290604051600060405180830381858888f19350505050158015610943573d6000803e3d6000fd5b5050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff8111156109be576109bd611ab6565b5b6040519080825280601f01601f1916602001820160405280156109f05781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610a2857610a27611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610a8c57610a8b611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610c465782600485600c84610ad89190611d82565b60208110610ae957610ae8611d53565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610b2857610b27611d53565b5b602001015160f81c60f81b82600283610b419190611db6565b6002610b4d9190611d82565b81518110610b5e57610b5d611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c84610ba19190611d82565b60208110610bb257610bb1611d53565b5b1a60f81b1660f81c60ff1681518110610bce57610bcd611d53565b5b602001015160f81c60f81b82600283610be79190611db6565b6003610bf39190611d82565b81518110610c0457610c03611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610c3e90611df8565b915050610abe565b50809350505050919050565b606060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff811115610cab57610caa611ab6565b5b6040519080825280601f01601f191660200182016040528015610cdd5781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610d1557610d14611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d7957610d78611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610f335782600486600c84610dc59190611d82565b60208110610dd657610dd5611d53565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610e1557610e14611d53565b5b602001015160f81c60f81b82600283610e2e9190611db6565b6002610e3a9190611d82565b81518110610e4b57610e4a611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84610e8e9190611d82565b60208110610e9f57610e9e611d53565b5b1a60f81b1660f81c60ff1681518110610ebb57610eba611d53565b5b602001015160f81c60f81b82600283610ed49190611db6565b6003610ee09190611d82565b81518110610ef157610ef0611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610f2b90611df8565b915050610dab565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b90506000600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205460001b90506000610fb282610c52565b90506000610fbf33610947565b9050610fca88610947565b610fd388610947565b610fdc86610c52565b8385604051602001610ff2959493929190611f60565b6040516020818303038152906040529450505050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561105c57600080fd5b600660008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548211156110e557600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111349190611d1f565b9250508190555081600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111c79190611d1f565b9250508190555081600360008282546111e09190611d1f565b925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca58360405161122d919061194a565b60405180910390a26001905092915050565b6001805461124c90611cbf565b80601f016020809104026020016040519081016040528092919081815260200182805461127890611cbf565b80156112c55780601f1061129a576101008083540402835291602001916112c5565b820191906000526020600020905b8154815290600101906020018083116112a857829003601f168201915b505050505081565b60006112da338484611412565b6001905092915050565b6000808490506112f48585610557565b15611374578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401611338949392919061203b565b600060405180830381600087803b15801561135257600080fd5b505af1158015611366573d6000803e3d6000fd5b505050506001915050611376565b505b9392505050565b34600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546113cc9190611d82565b92505081905550565b6006602052816000526040600020602052806000526040600020600091509150505481565b60056020528060005260406000206000915090505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff160361144b57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561149757600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115229190611d82565b101561152d57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115b99190611d82565b905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825461160a9190611d1f565b9250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546116609190611d82565b925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040516116c4919061194a565b60405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546117579190611d82565b1461176557611764612087565b5b50505050565b600081519050919050565b600082825260208201905092915050565b60005b838110156117a557808201518184015260208101905061178a565b60008484015250505050565b6000601f19601f8301169050919050565b60006117cd8261176b565b6117d78185611776565b93506117e7818560208601611787565b6117f0816117b1565b840191505092915050565b6000602082019050818103600083015261181581846117c2565b905092915050565b6000604051905090565b600080fd5b600080fd5b600073ffffffffffffffffffffffffffffffffffffffff82169050919050565b600061185c82611831565b9050919050565b61186c81611851565b811461187757600080fd5b50565b60008135905061188981611863565b92915050565b6000819050919050565b6118a28161188f565b81146118ad57600080fd5b50565b6000813590506118bf81611899565b92915050565b600080604083850312156118dc576118db611827565b5b60006118ea8582860161187a565b92505060206118fb858286016118b0565b9150509250929050565b60008115159050919050565b61191a81611905565b82525050565b60006020820190506119356000830184611911565b92915050565b6119448161188f565b82525050565b600060208201905061195f600083018461193b565b92915050565b60008060006060848603121561197e5761197d611827565b5b600061198c8682870161187a565b935050602061199d8682870161187a565b92505060406119ae868287016118b0565b9150509250925092565b600060ff82169050919050565b6119ce816119b8565b82525050565b60006020820190506119e960008301846119c5565b92915050565b600060208284031215611a0557611a04611827565b5b6000611a13848285016118b0565b91505092915050565b600060208284031215611a3257611a31611827565b5b6000611a408482850161187a565b91505092915050565b6000819050919050565b611a5c81611a49565b8114611a6757600080fd5b50565b600081359050611a7981611a53565b92915050565b600060208284031215611a9557611a94611827565b5b6000611aa384828501611a6a565b91505092915050565b600080fd5b600080fd5b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b611aee826117b1565b810181811067ffffffffffffffff82111715611b0d57611b0c611ab6565b5b80604052505050565b6000611b2061181d565b9050611b2c8282611ae5565b919050565b600067ffffffffffffffff821115611b4c57611b4b611ab6565b5b611b55826117b1565b9050602081019050919050565b82818337600083830152505050565b6000611b84611b7f84611b31565b611b16565b905082815260208101848484011115611ba057611b9f611ab1565b5b611bab848285611b62565b509392505050565b600082601f830112611bc857611bc7611aac565b5b8135611bd8848260208601611b71565b91505092915050565b600080600060608486031215611bfa57611bf9611827565b5b6000611c088682870161187a565b9350506020611c19868287016118b0565b925050604084013567ffffffffffffffff811115611c3a57611c3961182c565b5b611c4686828701611bb3565b9150509250925092565b60008060408385031215611c6757611c66611827565b5b6000611c758582860161187a565b9250506020611c868582860161187a565b9150509250929050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680611cd757607f821691505b602082108103611cea57611ce9611c90565b5b50919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b6000611d2a8261188f565b9150611d358361188f565b9250828203905081811115611d4d57611d4c611cf0565b5b92915050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052603260045260246000fd5b6000611d8d8261188f565b9150611d988361188f565b9250828201905080821115611db057611daf611cf0565b5b92915050565b6000611dc18261188f565b9150611dcc8361188f565b9250828202611dda8161188f565b91508282048414831517611df157611df0611cf0565b5b5092915050565b6000611e038261188f565b91507fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8203611e3557611e34611cf0565b5b600182019050919050565b600081905092915050565b6000611e568261176b565b611e608185611e40565b9350611e70818560208601611787565b80840191505092915050565b7f2c20000000000000000000000000000000000000000000000000000000000000600082015250565b6000611eb2600283611e40565b9150611ebd82611e7c565b600282019050919050565b7f2c206d73672e73656e6465722000000000000000000000000000000000000000600082015250565b6000611efe600d83611e40565b9150611f0982611ec8565b600d82019050919050565b7f2c2062616c616e63654f665b5f66726f6d5d2000000000000000000000000000600082015250565b6000611f4a601383611e40565b9150611f5582611f14565b601382019050919050565b6000611f6c8288611e4b565b9150611f7782611ea5565b9150611f838287611e4b565b9150611f8e82611ea5565b9150611f9a8286611e4b565b9150611fa582611ef1565b9150611fb18285611e4b565b9150611fbc82611f3d565b9150611fc88284611e4b565b91508190509695505050505050565b611fe081611851565b82525050565b600081519050919050565b600082825260208201905092915050565b600061200d82611fe6565b6120178185611ff1565b9350612027818560208601611787565b612030816117b1565b840191505092915050565b60006080820190506120506000830187611fd7565b61205d602083018661193b565b61206a6040830185611fd7565b818103606083015261207c8184612002565b905095945050505050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052600160045260246000fdfea26469706673582212209df52aa99bc435db58f014a9bc16704c150672cdc4c01c997e89f93a9e9eb6fa64736f6c6343000811003300000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";

Value integratedTest1(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 0)
        throw runtime_error(
            "integratedTest1 no args"
            + HelpRequiringPassphrase());

    vector<Value> res;
    constexpr auto create_address         = 0xc9ea7ed000000000000000000000000000000001_address;
    constexpr auto test_sender_address    = 0x5b38da6a701c568545dcfcb03fcb875f56beddc4_address;
    constexpr auto test_recipient_address = 0x5b38da6a701c568545dcfcb03fcb875f56beddc4_address;
    std::cout << "integratedTest1 : initialised create address " << dvmc::hex(create_address.bytes) << "\n";
    std::cout << "integratedTest1 : initialised from address " << dvmc::hex(test_sender_address.bytes) << "\n";
    std::cout << "integratedTest1 : initialised to address " << dvmc::hex(test_recipient_address.bytes) << "\n";
    const auto code = dvmc::from_hex(erc20_22112022_withdraw_with_parameter_subtract_15);

    dvmc::VertexNode host;
    dvmc::TransitionalNode created_account;
    dvmc::TransitionalNode sender_account;
    sender_account.set_balance(3141);
    dvmc_message msg{};
    msg.track = std::numeric_limits<int64_t>::max();

    dvmc::bytes_view exec_code = code;
    {
        dvmc_message create_msg{};
        create_msg.kind = DVMC_CREATE;
        create_msg.recipient = create_address;
        create_msg.track = std::numeric_limits<int64_t>::max();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());
        if (create_result.status_code != DVMC_SUCCESS)
        {
            std::cout << "integratedTest1 : Contract creation failed: " << create_result.status_code << "\n";
            return create_result.status_code;
        }

        auto& created_account = host.accounts[create_address];
        created_account.set_balance(100000000000000);

        created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);

        msg.recipient = create_address;
        exec_code = created_account.code;
    }
    //totalSupply
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x18160ddd"); //totalSupply
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: totalSupply  " << result.status_code << "\nGas used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //burn
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x42966c68000000000000000000000000000000000000000000000000000000000000029a"); //burn
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: Burn  " << result.status_code << "\nGas used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //totalSupply
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x18160ddd"); //totalSupply
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: totalSupply  " << result.status_code << "\nGas used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest1 : Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //approve
    {
        //std::cout << "\n";
        const auto input = dvmc::from_hex("0x095ea7b30000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000050"); //approve spender,amount
        msg.sender = test_sender_address;
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

    }
    //print allowance
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"); //approve spender,amount
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

    }
    //transfer_from
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x23b872dd0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000037"); //transfer_from


        msg.track = std::numeric_limits<int64_t>::max();
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: transfer_from  " << result.status_code << "\nGas used: " << track_used << "\n";
    }
    //print allowance
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"); //approve spender,amount
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest1 : Output: allowance  " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //withdraw 4 eth
    {
        std::cout << "\n";
        const auto input =   dvmc::from_hex("0x486556ce0000000000000000000000000000000000000000000000000000000000031415"); //approve spender,amount
        msg.sender = test_sender_address;
        msg.recipient = create_address;
        msg.input_data = input.data();
        msg.input_size = input.size();
        dvmc::bytes32 bytes__sender_before = host.get_balance(test_sender_address);
        dvmc::bytes32 bytes__ = host.get_balance(create_address);

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        dvmc::bytes32 bytes__sender_after = host.get_balance(test_sender_address);
        dvmc::bytes32 bytes__contract_after = host.get_balance(create_address);
        unsigned char test_bytes[] = {
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0x03,0x14,0x06
        };
        uint256 test = test_bytes[0];
        unsigned int test2;

        uint256 msg_test_bytes;
        for(int i=0; i<8; i++)
        {
            //28     24    20   16   12    8     4     0
            //-28   -20   -12   -4   +4   +12   +20  +28
            //unsigned int b1 = test_bytes[i*4 - 28 + 8*(7-i)];

            unsigned int b1 = test_bytes[i*4];
            b1<<=24;
            unsigned int b2 = test_bytes[i*4+1];
            b2<<=16;
            unsigned int b3 = test_bytes[i*4+2];
            b3<<=8;
            unsigned int b4 = test_bytes[i*4+3];
            b4<<=0;
            unsigned int p_i = b1 + b2 + b3 + b4;
            msg_test_bytes.pn[7 - i] = p_i;
        }
        msg_test_bytes +=1;

        uint64_t msg_test_bytes64 = msg_test_bytes.pn[0];
        msg_test_bytes64 += (0x00000000fffffffff&msg_test_bytes.pn[1])<<32;

        const auto track_used = msg.track - result.track_left;
    }

    dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(overlayDB_);
    state.init();
    dev::Address tmpAddr("9999999999999996789012345678901234567890");
    dev::eth::Account tmpAcc(0,0);
    {
        dev::u256 nonce = 12345678;
        dev::u256 balance = 1010101010101;
        dev::RLPStream s(4);
        s << nonce << balance;

        {
            dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state.db(), tmpAcc.baseRoot());
            auto& created_account__ = host.accounts[create_address];
            for(auto pair : created_account__.storage)
            {
                auto storage_key = pair.first.bytes;
                dev::bytes key;
                for(int i=0; i<32; i++)
                    key.push_back(storage_key[i]);

                dev::h256 key256(key);

                auto storage_bytes = pair.second.value;
                dev::bytes val;
                std::cout << "preparing insert dev::bytes val in trie, val.size " << val.size() << std::endl;
                for(int i=0; i<32; i++)
                    val.push_back(storage_bytes.bytes[i]);
                std::cout << "insert " << storageDB.root() << std::endl;
                std::cout << "preparing insert convert val toString" << std::endl;
                std::cout << "preparing insert compare toString with orig" << std::endl;
                std::cout << "preparing insert prior to insert val.size " << val.size() << std::endl;
                bool match=true;
                for(int i=0; i<32; i++)
                {
                    if(val[i] != storage_bytes.bytes[i])
                    {
                        match=false;
                        break;
                    }
                }
                std::cout << "preparing insert compare val with orig " << match << std::endl;
                storageDB.insert(key256, dev::rlp(val));
            }
            s << storageDB.root();
        }

        s << tmpAcc.codeHash();
        state.insert(tmpAddr, &s.out());
        overlayDB_->commit();
    }
    auto created_acc = host.accounts[create_address];
    dvmc::TransitionalNode account_recon;
    auto& node_map = host.accounts;
    account_recon.storage = created_acc.storage;
    account_recon.code = created_acc.code;
    account_recon.balance = created_acc.balance;
    node_map[create_address] = account_recon;
    std::cout << "direct copy storage plus code plus balance to recon" << std::endl;
    {
        std::cout << "RECON QUERY ALLOWANCE" << std::endl;
        std::cout << "\n";
        const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"); //approve spender,amount
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : after recon Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest1 : Output: after recon allowance  " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    std::cout << "========================================================================" << std::endl;
    //integratedTest1 Pull the storage data from db and re construct the transitional_host
    {
        //transitional_node
        /// The account storage map.
        //std::unordered_map<bytes32, storage_value> storage;
        //struct storage_value -> bytes32 value
        string account_str = state.at(tmpAddr);
        dev::RLP rlp_state(account_str,0);
        auto const nonce = rlp_state[0].toInt<dev::u256>();
        std::cout << "extracted nonce " << nonce << std::endl;
        auto const balance = rlp_state[1].toInt<dev::u256>();
        std::cout << "extracted balance " << balance << std::endl;
        auto const tmpAccstorageRoot = rlp_state[2].toHash<dev::h256>();
        std::cout << "extracted storageRoot " << tmpAccstorageRoot << std::endl;
        auto const codeHash = rlp_state[3].toHash<dev::h256>();
        std::cout << "integratedTest 1 extracted codeHash " << codeHash << std::endl;

        dev::eth::Account retrievedAcc(nonce,balance,tmpAccstorageRoot,codeHash,0,dev::eth::Account::Unchanged);
        {
            map<dev::h256, pair<dev::u256, dev::u256>> ret;
            //map<dev::h256, pair<dev::bytes, dev::vector_ref<const unsigned char>>> ret_;
            map<dev::h256, pair<dev::bytes, dev::bytes>> ret_;
            ret_.clear();
            std::cout << "created ret_.size " << ret_.size() << std::endl;
            for(auto p : ret_)
                std::cout << "after creation ret_ entry" << std::endl;

            {
                // Pull out all values from trie storage.
                //if (dev::h256 root = retrievedAcc.baseRoot())
                {
                    dev::SecureTrieDB<dev::h256, dev::OverlayDB> memdb(const_cast<dev::OverlayDB*>(overlayDB_), tmpAccstorageRoot);       // promise we won't alter the overlay! :)

                    for (auto it = memdb.hashedBegin(); it != memdb.hashedEnd(); ++it)
                    {
                        dev::h256 const hashedKey((*it).first);
                        //dev::u256 const key = dev::h256(it.key());
                        auto const key = it.key();
                        //std::cout << "integratedTest 1 set value u256 0" << std::endl;
                        //dev::u256 const value = dev::RLP((*it).second).toInt<dev::u256>();
                        //auto const value = (*it).second;
                        dev::bytes const value = dev::RLP((*it).second).toBytes();
                        std::cout << "integratedTest 1 set ret_ hash map key value" << std::endl;
                        ret_[hashedKey] = make_pair(key, value);
                    }
                }
                std::cout << "integratedTest 1 map ret_.size() " << ret_.size() << std::endl;
                std::cout << "after first setting entries ret_.size " << ret_.size() << std::endl;
                for(auto p : ret_)
                    std::cout << "after setting ret_ entry" << std::endl;
                //Check ret_ values match original storage values
                auto created_account__ = host.accounts[create_address];
                std::cout << "created_account__.storage.size " << created_account__.storage.size() << std::endl;
                for(auto pair : created_account__.storage)
                {
                    auto storage_key = pair.first.bytes;
                    for(auto i_ret_ : ret_)
                    {
                        dev::bytes key = i_ret_.second.first;
                        bool match = true;
                        for(int index=0; index<32; index++)
                        {
                            if(storage_key[index] != key[index])
                            {
                                match=false;
                                break;
                            }
                        }
                        if(match == true)
                            std::cout << "found matching key bytes" << std::endl;
                    }
                }
                std::cout << "integratedTest 1 map ret_.size() " << ret_.size() << std::endl;
                std::cout << "prior to val scan entries ret_.size " << ret_.size() << std::endl;
                for(auto p : ret_)
                    std::cout << "prior to val scan ret_ entry" << std::endl;
                std::cout << "================== scan values ================== " << std::endl;
                std::cout << "ret_.size " << ret_.size() << std::endl;
                std::cout << "prior to val scan created_account__.storage.size " << created_account__.storage.size() << std::endl;
                for(auto pair : created_account__.storage)
                {
                    //std::cout << "scan hash map" << std::endl;
                    auto storage_value = pair.second.value;
                    for(auto i_ret_ : ret_)
                    {
                        //dev::vector_ref<const unsigned char> val = i_ret_.second.second;
                        dev::bytes val = i_ret_.second.second;
                        bool match = true;
                        for(int index=0; index<32; index++)
                        {
                            if(storage_value.bytes[index] != val[index])
                            {
                                match=false;
                                break;
                            }
                        }
                        if(match == true)
                            std::cout << "found matching value bytes" << std::endl;
                    }
                }
                std::cout << "================== scan values end ================== " << std::endl;

                // Then merge cached storage over the top.
                //for (auto const& i : retrievedAcc.storageOverlay())
                //{
                //	dev::h256 const key = i.first;
                //	dev::h256 const hashedKey = sha3(key);
                //   if (i.second)
                //      ret[hashedKey] = i;
                // else
                //    ret.erase(hashedKey);
                //}
            }
            //std::cout << "integratedTest 1 pull from trie storage 2" << std::endl;

            //struct storage_value:: bytes32 value

            //std::cout << "ret.size() " << ret.size() << std::endl;
            dvmc::TransitionalNode account_recon;
            std::unordered_map<dvmc::bytes32, dvmc::storage_value>& storage_recon = account_recon.storage;
            for(auto i_ : ret_)
            {
                dvmc::bytes32 reconKey;
                dev::bytes key = i_.second.first;
                for(int idx=0; idx<32; idx++)
                    reconKey.bytes[idx] = key[idx];
                dvmc::storage_value sv;
                for(int idx=0; idx<32; idx++)
                    sv.value.bytes[idx] = i_.second.second[idx];

                storage_recon[reconKey] = sv;
            }

            auto& created_account___ = host.accounts[create_address];
            {
                //Compare recon keys with orig
                for(auto pair : created_account___.storage)
                {
                    for(auto pair_r : account_recon.storage)
                    {
                        bool match=true;
                        for(int idx=0; idx<32; idx++)
                        {
                            if(pair_r.first.bytes[idx] != pair.first.bytes[idx])
                            {
                                match=false;
                                break;
                            }
                        }
                        if(match) std::cout << "key match" << std::endl;
                    }
                }
            }
            {
                for(auto pair : created_account___.storage)
                {
                    dvmc::storage_value sv = account_recon.storage[pair.first];
                    if(sv.value == pair.second.value)
                        std::cout << "payload match" << std::endl;
                    else
                        std::cout << "payload does not match" << std::endl;

                }
            }
            account_recon.nonce = created_account___.nonce;
            account_recon.balance = created_account___.balance;
            account_recon.code = created_account___.code;
            created_account___ = account_recon;
            {
                std::cout << "RECON TEST" << std::endl;
                std::cout << "\n";
                const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4");
                msg.input_data = input.data();
                msg.input_size = input.size();

                dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
                dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
                const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

                const auto track_used = msg.track - result.track_left;
                std::cout << "integratedTest1 : after recon Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

                if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
                    std::cout << "integratedTest1 : Output: after recon allowance  " << dvmc::hex({result.output_data, result.output_size}) << "\n";
            }
        }
    }


    return res;
}

Value integratedTest2(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 0)
        throw runtime_error(
            "integratedTest2 no args"
            + HelpRequiringPassphrase());

    fs::create_directories("/home/argon/data1/testnet/state");
    fs::permissions("/home/argon/data1/testnet/state", fs::owner_all);
    std::unique_ptr<dev::db::DatabaseFace> db = dev::db::DBFactory::create("/home/argon/data1/testnet/state");
    auto overlay_db = dev::OverlayDB(std::move(db));
    dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(&overlay_db);
    std::unordered_map<dev::Address, dev::eth::Account> m_cache;

    state.init();
    std::cout << "state root before test insert " << state.root() << std::endl;
    dev::Address tmpAddr("1234567890123456789012345678901234567890");
    dev::eth::Account tmpAcc(0,0);
    std::cout << "tmpAcc created storageRoot " << tmpAcc.baseRoot() << std::endl;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
    ("key1", po::value<dev::u256>()->value_name("<v1>"), "hljhkljh")
    ("val1", po::value<dev::u256>()->value_name("<v2>"), "hljhkljh");
    po::variables_map m;
    int arg_count = 3;
    char* args[] = {
        "program",
        "--key1=123",
        "--val1=456"
    };

    po::parsed_options parsed = po::parse_command_line(arg_count, args, desc);
    po::store(parsed, m);
    po::notify(m);

    if(m.count("key1")) std::cout << "key1" << std::endl;
    if(m.count("key2")) std::cout << "key2" << std::endl;

    dev::u256 key1 =
        m["key1"].as<dev::u256>();
    dev::u256 val1 =
        m["val1"].as<dev::u256>();
    dev::RLPStream s(4);
    tmpAcc.setStorage(key1, val1);
    std::cout << "tmpAcc storage set - storageRoot " << tmpAcc.baseRoot() << std::endl;
    dev::SpecificTrieDB<dev::FatGenericTrieDB<dev::OverlayDB>, dev::FixedHash<20> >::iterator iter = state.begin();
    {
        dev::u256 nonce = 12345678;
        dev::u256 balance = 1010101010101;
        s << nonce << balance;

        std::cout << nonce << std::endl;
        {
            dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state.db(), tmpAcc.baseRoot());
            for(auto i : tmpAcc.storageOverlay())
            {
                std::cout << "test 2 insert" << std::endl;
                storageDB.insert(i.first, dev::rlp(i.second));
            }
            s << storageDB.root();
            std::cout << "generated storage root " << storageDB.root() << std::endl;
        }

        std::cout << "insert tmpAddr into state" << std::endl;
        s << tmpAcc.codeHash();
        std::cout << "appended codehash " << tmpAcc.codeHash() << std::endl;
        std::cout << "s.out vector unsigned char " << s.out().size() << std::endl;


        state.insert(tmpAddr, &s.out());
        dev::RLP rlp_state_(s.out(),0);
        auto const nonce_ = rlp_state_[0].toInt<dev::u256>();

        auto const balance_ = rlp_state_[1].toInt<dev::u256>();
        auto const storageRoot_ = rlp_state_[2].toHash<dev::h256>();
        auto const codeHash_ = rlp_state_[3].toHash<dev::h256>();
    }

    std::cout << "commit 0" << std::endl;
    overlay_db.commit();
    std::cout << "commit 1" << std::endl;
    std::cout << "state root after test insert " << state.root() << std::endl;
    std::cout << "tmpAcc inserted - storageRoot " << tmpAcc.baseRoot() << std::endl;
    string account_str = state.at(tmpAddr);
    if(account_str.empty())
        std::cout << "account str empty" << std::endl;
    else
        std::cout << "account str not empty" << std::endl;
    dev::h256 storageRoot__("650276fe18bc32afd3f79fc876c678269635c037e52f178402cfaf1a0c6ea911");

    dev::Address tmpAddr2("9299567890123456789012345678901234567890");
    string account_str2 = state.at(tmpAddr2);
    if(account_str2.empty())
        std::cout << "account str2 empty" << std::endl;
    else
        std::cout << "account str2 not empty" << std::endl;
    std::cout << "read back account string from trie " << account_str << std::endl;
    dev::RLP rlp_state(s.out(),0);
    std::cout << "constructed rlp" << std::endl;
    auto const nonce = rlp_state[0].toInt<dev::u256>();
    std::cout << "extracted nonce " << nonce << std::endl;
    auto const balance = rlp_state[1].toInt<dev::u256>();
    std::cout << "extracted balance " << balance << std::endl;
    auto const storageRoot = rlp_state[2].toHash<dev::h256>();
    std::cout << "extracted storageRoot " << storageRoot << std::endl;
    auto const codeHash = rlp_state[3].toHash<dev::h256>();
    std::cout << "extracted codeHash " << codeHash << std::endl;
    fs::path configFile = "/home/argon/rem/CC/testdata/rlp/transaction.rlp";

    dev::u256 t1 = 123456;
    dev::RLPStream rlpStream(25);
    rlpStream << t1;
    rlpStream << t1;
    rlpStream << t1;
    rlpStream << t1;
    rlpStream << t1;
    rlpStream << t1;
    rlpStream << t1;
    std::cout << "test rlp 1" << std::endl;
    dev::RLP config(rlpStream.out());
    std::cout << "test rlp 6" << std::endl;
    std::ostringstream os_;
    os_ << config;
    std::cout << "test rlp " << os_.str() << std::endl;



    //dev::h256 tmp;
    //while(iter != state.end())
    //{
    //}
    vector<Value> res;
    return res;
}
/*
template <class DB>
AddressHash dev::eth::commit(AccountMap const& _cache, SecureTrieDB<Address, DB>& _state)
{
    AddressHash ret;
    for (auto const& i: _cache)
        if (i.second.isDirty())
        {
            if (!i.second.isAlive())
                _state.remove(i.first);
            else
            {
                auto const version = i.second.version();

                // version = 0: [nonce, balance, storageRoot, codeHash]
                // version > 0: [nonce, balance, storageRoot, codeHash, version]
                RLPStream s(version != 0 ? 5 : 4);
                s << i.second.nonce() << i.second.balance();

                if (i.second.storageOverlay().empty())
                {
                    assert(i.second.baseRoot());
                    s.append(i.second.baseRoot());
                }
                else
                {
                    SecureTrieDB<h256, DB> storageDB(_state.db(), i.second.baseRoot());
                    for (auto const& j: i.second.storageOverlay())
                        if (j.second)
                            storageDB.insert(j.first, rlp(j.second));
                        else
                            storageDB.remove(j.first);
                    assert(storageDB.root());
                    s.append(storageDB.root());
                }

                if (i.second.hasNewCode())
                {
                    h256 ch = i.second.codeHash();
                    // Store the size of the code
                    CodeSizeCache::instance().store(ch, i.second.code().size());
                    _state.db()->insert(ch, &i.second.code());
                    s << ch;
                }
                else
                    s << i.second.codeHash();

                if (version != 0)
                    s << i.second.version();

                _state.insert(i.first, &s.out());
            }
            ret.insert(i.first);
        }
    return ret;
}
*/

Value integratedTest3(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 0)
        throw runtime_error(
            "integratedTest3 no args"
            + HelpRequiringPassphrase());

    std::unique_ptr<dev::db::DatabaseFace> db = dev::db::DBFactory::create("/home/argon/data1/testnet/state");
    dev::h256 root("ac7ed96f9c8ced36cb005661a0083cd2284e1487c578e4ffbfa4f2550fcfd947");


    auto overlay_db = dev::OverlayDB(std::move(db));
    dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(&overlay_db);

    state.setRoot(root);
    //state root needs initializing
    std::cout << "integratedTest3 state root after db open " << state.root() << std::endl;
    std::unordered_map<dev::Address, dev::eth::Account> m_cache;
    std::ostringstream os;
    state.debugStructure(os);
    std::cout << "trie op << " << os.str() << std::endl;

    std::cout << "state root after retrieving from db " << state.root() << std::endl;
    dev::Address tmpAddr("1234567890123456789012345678901234567890");
    string account_str = state.at(tmpAddr);
    dev::RLP rlp_state(account_str,0);
    auto const nonce = rlp_state[0].toInt<dev::u256>();
    std::cout << "extracted nonce " << nonce << std::endl;
    auto const balance = rlp_state[1].toInt<dev::u256>();
    std::cout << "extracted balance " << balance << std::endl;
    auto const storageRoot = rlp_state[2].toHash<dev::h256>();
    std::cout << "extracted storageRoot " << storageRoot << std::endl;
    auto const codeHash = rlp_state[3].toHash<dev::h256>();
    std::cout << "extracted codeHash " << codeHash << std::endl;

    dev::eth::Account retrievedAcc(nonce,balance,storageRoot,codeHash,0,dev::eth::Account::Unchanged);
//map<h256, pair<u256, u256>> State::storage(Address const& _id) const
    {
        map<dev::h256, pair<dev::u256, dev::u256>> ret;

        {
            // Pull out all values from trie storage.
            if (dev::h256 root = retrievedAcc.baseRoot())
            {
                dev::SecureTrieDB<dev::h256, dev::OverlayDB> memdb(const_cast<dev::OverlayDB*>(&overlay_db), root);       // promise we won't alter the overlay! :)

                for (auto it = memdb.hashedBegin(); it != memdb.hashedEnd(); ++it)
                {
                    dev::h256 const hashedKey((*it).first);
                    dev::u256 const key = dev::h256(it.key());
                    dev::u256 const value = dev::RLP((*it).second).toInt<dev::u256>();
                    ret[hashedKey] = make_pair(key, value);
                }
            }

            // Then merge cached storage over the top.
            for (auto const& i : retrievedAcc.storageOverlay())
            {
                dev::h256 const key = i.first;
                dev::h256 const hashedKey = sha3(key);
                if (i.second)
                    ret[hashedKey] = i;
                else
                    ret.erase(hashedKey);
            }
        }

        std::cout << "ret.size() " << ret.size() << std::endl;
        for(auto i : ret)
        {
            std::cout << "map key " << i.first << std::endl;
            pair<dev::u256,dev::u256> val = i.second;
            std::cout << "map val.first " << val.first << std::endl;
            std::cout << "map val.second " << val.second << std::endl;
        }
    }
    /*
        dev::h256 tmp;
        dev::SpecificTrieDB<dev::FatGenericTrieDB<dev::OverlayDB>, dev::FixedHash<20> >::iterator iter = state.begin();
        while(iter != state.end())
        {
          std::cout << "--- " << (*iter).first.hex() << std::endl;
          std::cout << "--- " << (*iter).second << std::endl;
          //dev::SecureTrieDB<dev::h256, dev::OverlayDB> const memdb(const_cast<dev::OverlayDB*>(&overlay_db), (*iter).second);
          ++iter;
        }
        std::cout << "state root after test insert " << state.root() << std::endl;
        */
    vector<Value> res;
    return res;
}


Value integratedTest4(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 0)
        throw runtime_error(
            "integratedTest4 no args"
            + HelpRequiringPassphrase());

    // Generate a new key that is added to wallet
    CPubKey newKey_contract_address;
    if(!pwalletMain->GetKeyFromPool(newKey_contract_address, false))
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    CKeyID keyID_contract_address = newKey_contract_address.GetID();
    dvmc::address create_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_contract_address.GetHex().c_str());

    CPubKey newKey_test_sender_address;
    if(!pwalletMain->GetKeyFromPool(newKey_test_sender_address, false))
        throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    CKeyID keyID_test_sender_address = newKey_test_sender_address.GetID();
    dvmc::address test_sender_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_test_sender_address.GetHex().c_str());

    dvmc::address test_recipient_address = test_sender_address;

    std::cout << "integratedTest4 : initialised create address " << dvmc::hex(create_address.bytes) << "\n";
    std::cout << "integratedTest4 : initialised from address " << dvmc::hex(test_sender_address.bytes) << "\n";
    std::cout << "integratedTest4 : initialised to address " << dvmc::hex(test_recipient_address.bytes) << "\n";
    const auto code = dvmc::from_hex(erc20_22112022_withdraw_with_parameter_subtract_15);

    dvmc::VertexNode host;
    dvmc::TransitionalNode created_account;
    dvmc::TransitionalNode sender_account;
    sender_account.set_balance(3141);
    dvmc_message msg{};
    msg.track = std::numeric_limits<int64_t>::max();

    dvmc::bytes_view exec_code = code;
    {
        dvmc_message create_msg{};
        create_msg.kind = DVMC_CREATE;
        create_msg.recipient = create_address;
        create_msg.track = std::numeric_limits<int64_t>::max();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());
        if (create_result.status_code != DVMC_SUCCESS)
        {
            std::cout << "integratedTest4 : Contract creation failed: " << create_result.status_code << "\n";
            return create_result.status_code;
        }

        auto& created_account = host.accounts[create_address];
        created_account.set_balance(100000000000000);
        created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);

        msg.recipient = create_address;
        exec_code = created_account.code;
    }
    //totalSupply
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x18160ddd"); //totalSupply
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest14: Result: totalSupply  " << result.status_code << "\nGas used: " << track_used << "\n";
    }
    //burn
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x42966c68000000000000000000000000000000000000000000000000000000000000029a"); //burn
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: Burn  " << result.status_code << "\nGas used: " << track_used << "\n";
    }
    //totalSupply
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x18160ddd"); //totalSupply
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: totalSupply  " << result.status_code << "\nGas used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest1 : Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //approve
    {
        std::string s = dvmc::hex(test_sender_address.bytes);
        const std::string code_str = "0x095ea7b3000000000000000000000000" + s +  "0000000000000000000000000000000000000000000000000000000000000050";
        std::cout << "approve op input code " << code_str << std::endl;
        const auto input = dvmc::from_hex(code_str); //approve spender,amount
        msg.sender = test_sender_address;
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest4 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";
    }
    //print allowance
    {
        std::cout << "\n";
        std::string s = dvmc::hex(test_sender_address.bytes);
        //auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"); //approve spender,amount
        std::string code_str = "0xdd62ed3e000000000000000000000000" + s + "000000000000000000000000" + s; //approve spender,amount
        //const std::string code_str = "0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"; //approve spender,amount
        std::cout << "print allowance op input code " << code_str << std::endl;
        const auto input = dvmc::from_hex(code_str); //approve spender,amount
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest4 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";
        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest4 : Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //transfer_from
    {
        std::cout << "\n";
        std::string s = dvmc::hex(test_sender_address.bytes);
        std::string code_str = "0x23b872dd000000000000000000000000" + s + "000000000000000000000000" + s + "0000000000000000000000000000000000000000000000000000000000000037"; //transfer_from
        std::cout << "transfer_from op input code " << code_str << std::endl;
        const auto input = dvmc::from_hex(code_str); //transfer_from

        msg.track = std::numeric_limits<int64_t>::max();
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: transfer_from  " << result.status_code << "\nGas used: " << track_used << "\n";
        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest4 : Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";

    }
    //print allowance
    {
        std::cout << "\n";
        std::string s = dvmc::hex(test_sender_address.bytes);
        std::string code_str = "0xdd62ed3e000000000000000000000000" + s + "000000000000000000000000" + s; //approve spender,amount
        std::cout << "print allowance op input code " << code_str << std::endl;
        const auto input = dvmc::from_hex(code_str); //approve spender,amount
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";
        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest1 : Output: allowance  " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }


    vector<Value> res;
    return res;
}

Value integratedTest5(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 0)
        throw runtime_error(
            "integratedTest5 no args"
            + HelpRequiringPassphrase());

    std::vector<CTransaction> testTxVector;

    {
        std::string contractCodeStr = erc20_22112022_withdraw_with_parameter_subtract_15;
        std::string ctrN    = "vertex_trigger";

        vector<PathIndex> vtxPos;
        PathIndex txPos2;
        txPos2.vValue = vchFromString(contractCodeStr);

        CPubKey vchPubKey;
        CReserveKey reservekey(pwalletMain);
        if(!reservekey.GetReservedKey(vchPubKey))
        {
            return false;
        }

        reservekey.KeepKey();

        cba keyAddress(vchPubKey.GetID());
        txPos2.vAddress = keyAddress.ToString();
        vtxPos.push_back(txPos2);
        LocatorNodeDB transSetup__("r+");
        transSetup__.lPut(vchFromString(ctrN), vtxPos);
    }
    {
        std::string refStr = ctrN + ":" + "0x18160ddd";
        std::string executableTxName = "executable_test_0x18160ddd";
        __wx__Tx vertex;
        vertex_serial_aux_cycle(executableTxName, refStr, vertex);

        testTxVector.push_back(vertex);
    }

	    sectionTest(testTxVector);

    vector<Value> res;
    return res;
}
