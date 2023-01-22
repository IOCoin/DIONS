




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

Value integratedTest__c(const Array& params, bool fHelp)
{
    if(fHelp || params.size() > 0)
        throw runtime_error(
            "integratedTest"
            + HelpRequiringPassphrase());

    vector<Value> res;
    constexpr auto empty_address         = 0x1234567000000000000000000000000000000001_address;
    constexpr auto create_address         = 0xc9ea7ed000000000000000000000000000000001_address;
    constexpr auto test_sender_address    = 0x5b38da6a701c568545dcfcb03fcb875f56beddc4_address;
    constexpr auto test_recipient_address = 0x5b38da6a701c568545dcfcb03fcb875f56beddc4_address;
    //code READ STR <PATH_TO_FILE>
    //const auto code = dvmc::from_hex(contract_transfer_no_check_no__transfer);
    //const auto code = dvmc::from_hex(code_add_require_no_trans);
    //req12 success
    //const auto code = dvmc::from_hex(erc20__transfer_req12_commented_out);
    //const auto code = dvmc::from_hex(code_hex_str);
    //const auto code = dvmc::from_hex(code_hex_08112022_trace);
    //const auto code = dvmc::from_hex(code_hex_09112022_trace);
    //const auto code = dvmc::from_hex(code_with_balance_adjust_in_approve_str);
    //const auto code = dvmc::from_hex(erc20_15112022_endowed_with_5_eth);
    //const auto code = dvmc::from_hex(erc20_17112022_send_fixed_amount_1);
    //const auto code = dvmc::from_hex(erc20_19112022_withdraw_with_parameter);
    const auto code = dvmc::from_hex(erc20_22012023_withdraw_with_parameter_subtract_15);

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
            std::cout << "integratedTest : Contract creation failed: " << create_result.status_code << "\n";
            return create_result.status_code;
        }

        auto& created_account = host.accounts[create_address];
        created_account.set_balance(100000000000000);
        //created_account.set(10);
        //created_account = host.acc[create_address];
        //uint64_t b = std::uint64_t(1) << 35;
        //created_acc.set(b);
        //const auto sender= host.acc[test_sender_address];

        created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);

        msg.recipient = create_address;
        exec_code = created_account.code;
    }
    //circulate total
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x18160ddd"); //totalSupply
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest : Result: circulate total  " << result.status_code << "\ntrack used: " << track_used << "\n";
    }
    //deficit
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x42966c68000000000000000000000000000000000000000000000000000000000000029a"); //burn
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest : Result: deficit  " << result.status_code << "\ntrack used: " << track_used << "\n";

    }
    //channel buffer
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x18160ddd"); //totalSupply
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest : Result: channel  " << result.status_code << "\ntrack used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest : Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //approve
    {
        //std::cout << "\n";
        const auto input = dvmc::from_hex("0x095ea7b30000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000050"); //approve spender,amount
        //msg.recipient = test_recipient_address;
        msg.sender = test_sender_address;
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest : Result:   " << result.status_code << "\ntrack used: " << track_used << "\n";

    }
    //echo allowance
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"); //approve spender,amount
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";
    }
    //erc20 transfer_from
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0x23b872dd0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000037"); //transfer_from


        //trace version
        //const auto input = dvmc::from_hex("0x745463630000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000100"); //transfer_from
        //msg.sender = test_recipient_address;
        //msg.value.bytes[0] = 0x32;
        //msg.recipient = test_recipient_address;


        //msg.sender = test_sender_address;
        //msg.kind = DVMC_CALL;
        //msg.recipient = test_recipient_address;
        //msg.input_data = input.data();
        //msg.input_size = input.size();
        //dvmc_message msg{};
        //msg.kind = DVMC_CALL;
        msg.track = std::numeric_limits<int64_t>::max();
        //msg.sender = test_sender_address;
        //msg.recipient = test_recipient_address;
        //host.acc[msg.sender].set_balance(10000000);
        //host.acc[test_recipient_address].set_balance(10000000);
        //host.acc[test_sender_address].set_balance(10000000);
        msg.input_data = input.data();
        msg.input_size = input.size();
        //msg.sender = test_sender_address;
        //msg.recipient = test_recipient_address;

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest1 : Result: transfer_from  " << result.status_code << "\ntrack used: " << track_used << "\n";
    }
    //update allowance
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"); //approve spender,amount
        //msg.recipient = test_recipient_address;
        //msg.sender = test_recipient_address;
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest : Result:   " << result.status_code << "\ntrack used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest1 : Output: updated allowance  " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    //withdraw 4 ioc <-> (eth)
    {
        std::cout << "\n";
        //const auto input = dvmc::from_hex("0x486556ce0000000000000000000000000000000000000000000000003782dace9d900000"); //approve spender,amount
        //const auto input =   dvmc::from_hex("0x486556ce0000000000000000000000000000000000000000000000001000000000000000"); //approve spender,amount
        const auto input =   dvmc::from_hex("0x486556ce0000000000000000000000000000000000000000000000000000000000031415"); //approve spender,amount
        //msg.recipient = test_recipient_address;
        msg.sender = test_sender_address;
        msg.recipient = create_address;
        msg.input_data = input.data();
        msg.input_size = input.size();
        /*
        dvmc::bytes32 bytes_ = host.get(create_address);
               bytes_.bytes[0] = 0xa0;
               bytes_.bytes[1] = 0x1c;
               bytes_.bytes[2] = 0x0d;
               bytes_.bytes[3] = 0x55;
               bytes_.bytes[4] = 0xde;
               bytes_.bytes[5] = 0x4c;
               bytes_.bytes[6] = 0x2b;
               bytes_.bytes[7] = 0x8a;
               bytes_.bytes[8] = 0xfb; */
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
        //msg_test_bytes.pn[0] = 0x031406;
        for(int i=0; i<8; i++)
        {
            //28     24    20   16   12    8     4     0
            //-28   -20   -12   -4   +4   +12   +20  +28

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

        //BENCH MARK INCLUDE <RUN> BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "Result:   " << result.status_code << "\ntrack used: " << track_used << "\n";
    }

    std::unique_ptr<dev::db::DatabaseFace> db = dev::db::DBFactory::create("/home/argon/data1/testnet/state");
    auto overlay_db = dev::OverlayDB(std::move(db));
    dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(&overlay_db);
    ///state.init();
    //dev::h256 root("ac7ed96f9c8ced36cb005661a0083cd2284e1487c578e4ffbfa4f2550fcfd947");
    //state.setRoot(root);
    dev::Address tmpAddr("9999999999999996789012345678901234567890");
    dev::eth::Account tmpAcc(0,0);

    auto& created_account__ = host.accounts[create_address];
    auto account_storage = created_account__.storage;
    std::cout << "account storage size " <<  account_storage.size() << std::endl;

    {
        dev::u256 nonce = 12345678;
        dev::u256 balance = 1010101010101;
        dev::RLPStream s(4);
        s << nonce << balance;
        {
            dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state.db(), tmpAcc.baseRoot());
            for(auto i : account_storage)
            {
                auto storage_key = i.first.bytes;
                dev::bytes key;
                for(int i=0; i<32; i++)
                    key.push_back(storage_key[i]);

                dev::h256 key256(key);

                auto storage_bytes = i.second.value;
                dev::bytes val;
                for(int i=0; i<32; i++)
                    val.push_back(storage_bytes.bytes[i]);
                std::cout << "insert " << storageDB.root() << std::endl;
                storageDB.insert(key256, dev::rlp(val));
            }
            s << storageDB.root();
        }

        s << tmpAcc.codeHash();
        state.insert(tmpAddr, &s.out());
        overlay_db.commit();

        string account_str = state.at(tmpAddr);
        dev::RLP rlp_state(account_str,0);
        auto const nonce_ = rlp_state[0].toInt<dev::u256>();
        auto const balance_ = rlp_state[1].toInt<dev::u256>();
        auto const storageRoot = rlp_state[2].toHash<dev::h256>();
        std::ostringstream os;
        state.debugStructure(os);
    }
    auto created_acc = host.accounts[create_address];
    dvmc::TransitionalNode account_recon;
    auto& node_map = host.accounts;
    account_recon = created_acc;
    node_map[create_address] = account_recon;
    //RECON -> CLI QUERY BATCH
    {
        std::cout << "\n";
        const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4"); //approve spender,amount
        //msg.recipient = test_recipient_address;
        //msg.sender = test_recipient_address;
        msg.input_data = input.data();
        msg.input_size = input.size();

        dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
        dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
        const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());


        //RECON -> CLI QUERY BATCH
        //if (bench)
        //    tooling::bench(host, vm, rev, msg, exec_code, result, out);

        const auto track_used = msg.track - result.track_left;
        std::cout << "integratedTest : after recon Result:   " << result.status_code << "\ntrack used: " << track_used << "\n";

        if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
            std::cout << "integratedTest : Output: after recon allowance  " << dvmc::hex({result.output_data, result.output_size}) << "\n";
    }
    {
        string account_str = state.at(tmpAddr);
        dev::RLP rlp_state(account_str,0);
        auto const nonce = rlp_state[0].toInt<dev::u256>();
        //std::cout << "extracted nonce " << nonce << std::endl;
        auto const balance = rlp_state[1].toInt<dev::u256>();
        //std::cout << "extracted balance " << balance << std::endl;
        auto const storageRoot = rlp_state[2].toHash<dev::h256>();
        //std::cout << "extracted storageRoot " << storageRoot << std::endl;
        auto const codeHash = rlp_state[3].toHash<dev::h256>();
        //std::cout << "integratedTest 1 extracted codeHash " << codeHash << std::endl;
        dvmc::TransitionalNode account_recon;
        std::unordered_map<dvmc::bytes32, dvmc::storage_value>& storage_recon = account_recon.storage;

        dev::eth::Account retrievedAcc(nonce,balance,storageRoot,codeHash,0,dev::eth::Account::Unchanged);
//map<h256, pair<u256, u256>> State::storage(Address const& _id) const
        std::cout << "integratedTest 1 pull from trie storage 1" << std::endl;
        {
            map<dev::h256, pair<dev::u256, dev::u256>> ret;

            {
                if (dev::h256 root = retrievedAcc.baseRoot())
                {
                    dev::SecureTrieDB<dev::h256, dev::OverlayDB> memdb(const_cast<dev::OverlayDB*>(&overlay_db), root);
                    for (auto it = memdb.hashedBegin(); it != memdb.hashedEnd(); ++it)
                    {
                        dev::h256 const hashedKey((*it).first);
                        dev::u256 const key = dev::h256(it.key());
                        std::cout << "integratedTest 1 set value u256 0" << std::endl;
                        dev::u256 const value = dev::RLP((*it).second).toInt<dev::u256>();
                        std::cout << "integratedTest 1 set value u256 1" << std::endl;
                        ret[hashedKey] = make_pair(key, value);
                    }
                }
            }
            for(auto i_ : ret)
            {
                dvmc::bytes32 reconKey;
                dev::u256 key = i_.second.first;
                evmc_uint256be key_bytes_ = reinterpret_cast<evmc_uint256be const&>(key);
                for(int i=0; i<32; i++)
                {
                    reconKey.bytes[i] = key_bytes_.bytes[i];
                }
                dvmc::bytes32 reconVal;
                dev::u256 val = i_.second.second;
                evmc_uint256be val_bytes_ = reinterpret_cast<evmc_uint256be const&>(val);
                for(int i=0; i<32; i++)
                {
                    reconVal.bytes[i] = val_bytes_.bytes[i];
                }

                dvmc::storage_value sv;
                sv.value = reconVal;
                storage_recon[reconKey] = sv;
            }
        }
    }


    return res;
}
//# burn 666 0x42966c68000000000000000000000000000000000000000000000000000000000000029a
//# totalSupply 0x18160ddd
//# approve (sender,spender,allowance)
//# hex address bytes and following transfer from (sender,spender,amount)
//# allowance (sender,spender) 0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4
//# name 0x06fdde03
//# get_name 0x3a525c29
//# msgtest 0xf15fad23
// # transfer_from from,to,amount 0x23b872dd0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000025
