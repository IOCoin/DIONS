#include "wallet/DB.h"
#include "TxDBLevelDB.h"
#include "KeyStore.h"
#include "wallet/Wallet.h"
#include "Base.h"
#include "core/uint256.h"
#include "rpc/Client.h"
#include "ccoin/Process.h"
#include "ptrie/TrieDB.h"
#include "ptrie/StateCacheDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "ptrie/Account.h"
#include "ptrie/DBFactory.h"
#include "State.h"
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
extern int scaleMonitor();
extern string txRelay(const CScript& scriptPubKey, int64_t nValue, const __wx__Tx& wtxIn, __wx__Tx& wtxNew, bool fAskFee);
extern bool aliasAddress(const CTransaction& tx, std::string& strAddress);
extern Object JSONRPCError(int code, const string& message);
extern Value xtu_url__(const string& url);
extern __wx__* pwalletMainId;
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
typedef struct vx_bytes32
{
  uint8_t bytes[32];
} vx_bytes32;
typedef struct vx_bytes32 evmc_uint256be;
extern vchType vchFromValue(const Value& value);
extern vchType vchFromString(const std::string& str);
extern string stringFromVch(const vector<unsigned char> &vch);
bool searchPathEncrypted2(string l, uint256& wtxInHash);
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
  CReserveKey reservekey(pwalletMainId);

  if(!reservekey.GetReservedKey(vchPubKey))
  {
    return false;
  }

  reservekey.KeepKey();
  cba keyAddress(vchPubKey.GetID());
  CKeyID keyID;
  keyAddress.GetKeyID(keyID);
  pwalletMainId->SetAddressBookName(keyID, "");
  __wx__DB walletdb(pwalletMainId->strWalletFile, "r+");
  CKey key;

  if(!pwalletMainId->GetKey(keyID, key))
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");
  }

  CScript scriptPubKeyOrig;
  scriptPubKeyOrig.SetBitcoinAddress(vchPubKey.Raw());
  CScript scriptPubKey;
  vchType vchPath = vchFromString(locatorStr);
  vchType vchValue = vchFromString(indexStr);
  scriptPubKey += scriptPubKeyOrig;
  ENTER_CRITICAL_SECTION(cs_main)
  {
    EnsureWalletIsUnlocked();
    string strError = pwalletMainId->SendMoney__(scriptPubKey, CTRL__, serial_n, false);

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
  CReserveKey reservekey(pwalletMainId);

  if(!reservekey.GetReservedKey(vchPubKey))
  {
    return false;
  }

  reservekey.KeepKey();
  cba keyAddress(vchPubKey.GetID());
  CKeyID keyID;
  keyAddress.GetKeyID(keyID);
  pwalletMainId->SetAddressBookName(keyID, "");
  __wx__DB walletdb(pwalletMainId->strWalletFile, "r+");
  CKey key;

  if(!pwalletMainId->GetKey(keyID, key))
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");
  }

  CScript scriptPubKeyOrig;
  scriptPubKeyOrig.SetBitcoinAddress(vchPubKey.Raw());
  CScript scriptPubKey;
  vchType vchPath = vchFromString(locatorStr);
  vchType vchValue = vchFromString(indexStr);
  scriptPubKey += scriptPubKeyOrig;
  ENTER_CRITICAL_SECTION(cs_main)
  {
    EnsureWalletIsUnlocked();
    {
      LEAVE_CRITICAL_SECTION(cs_main)
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
  CReserveKey reservekey(pwalletMainId);

  if(!reservekey.GetReservedKey(vchPubKey))
  {
    return false;
  }

  reservekey.KeepKey();
  cba keyAddress(vchPubKey.GetID());
  CKeyID keyID;
  keyAddress.GetKeyID(keyID);
  pwalletMainId->SetAddressBookName(keyID, "");
  __wx__DB walletdb(pwalletMainId->strWalletFile, "r+");
  CKey key;

  if(!pwalletMainId->GetKey(keyID, key))
  {
    throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");
  }

  serial_n.nVersion = CTransaction::DION_TX_VERSION;
  CScript scriptPubKeyOrig;
  scriptPubKeyOrig.SetBitcoinAddress(vchPubKey.Raw());
  CScript scriptPubKey;
  vchType vchPath = vchFromString(locatorStr);
  vchType vchValue = vchFromString(indexStr);
  scriptPubKey += scriptPubKeyOrig;
  vector< pair<CScript, int64_t> > vecSend;
  vecSend.push_back(make_pair(scriptPubKey, CTRL__));
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
          vector<AliasIndex> vtxPos;
          CDataStream ssValue((char*)data.get_data(), (char*)data.get_data() + data.get_size(), SER_DISK, CLIENT_VERSION);
          ssValue >> vtxPos;
          AliasIndex i = vtxPos.back();
          string i_address = i.vAddress;
          code = stringFromVch(i.vValue);
          found=true;
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
  {
    cursorp->close();
  }

  return found;
}
void sectionTest(std::vector<CTransaction>& testTxVector, dev::SecureTrieDB<dev::Address, dev::OverlayDB>& state)
{
  std::cout << "XXXX Entered sectionTest" << std::endl;

  for(unsigned int index=0; index<testTxVector.size(); index++)
  {
    cba cAddr_;
    CTransaction& tx = testTxVector[index];
    {
      string origin;

      for(unsigned int i = 0; i < tx.vout.size(); i++)
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
        vchType vchPath = vchFromString(target_contract);

        if (ln1Db->lKey (vchPath))
        {
          printf("sectionTest : name found in db\n");
          {
            std::cout << "error read : name db" << std::endl;
          }
          {
            std::cout << "error vec : empty" << std::endl;
          }
        }
        else
        {
          throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid I/OCoin address or unknown alias");
        }
      }
      string contract_input = contract_execution_data[1];
      string contractCode;
      bool f = sectionVertex(target_contract,contractCode);

      if(f != true)
      {
        continue;
      }

      std::cout << "sectionTest : found target contract : name " << target_contract << std::endl;
      CKeyID keyID;
      cAddr_.GetKeyID(keyID);
      dev::Address target_addr(keyID.GetHex().c_str());
      std::cout << "sectionTest : target contract address in 20 byte form " << keyID.GetHex() << std::endl;
      std::cout << "sectionTest : retrieve storage root for contract 1" << std::endl;
      std::cout << "sectionTest : retrieve storage root for contract 2" << std::endl;
      std::cout << "sectionTest : retrieve storage root for contract 3" << std::endl;
      std::cout << "sectuonTest : retrieve addr : " << target_addr << std::endl;
      string account_str = state.at(target_addr);
      dvmc::TransitionalNode account_recon;
      dev::eth::Account retrievedAcc(0,0);

      if(account_str.size() != 0)
      {
        std::cout << "RECOVERED EXISTING STORAGE FOR ACCOUNT" << std::endl;
        std::cout << "sectionTest : retrieve storage root for contract 4 : account_str.size() " << account_str.size() << std::endl;
        dev::RLP rlp_state(account_str,0);
        std::cout << "sectionTest : retrieve storage root for contract 5" << std::endl;
        auto const targetAccStorageRoot = rlp_state[2].toHash<dev::h256>();
        std::cout << "sectionTest : retrieve storage root for contract 6" << std::endl;
        auto const nonce = rlp_state[0].toInt<dev::u256>();
        std::cout << "sectionTest : retrieve storage root for contract 7" << std::endl;
        auto const balance = rlp_state[1].toInt<dev::u256>();
        std::cout << "sectionTest : retrieve storage root for contract 8" << std::endl;
        auto const codeHash = rlp_state[3].toHash<dev::h256>();
        std::cout << "sectionTest : retrieve storage root for contract 9" << std::endl;
        retrievedAcc.setNonce(nonce);
        retrievedAcc.setStorageRoot(targetAccStorageRoot);
        std::cout << "sectionTest : retrieve storage root for contract 10" << std::endl;
        string code_hex_str = contractCode;
        uint256 r;
        string val;
        {
          map<dev::h256, pair<dev::bytes, dev::bytes>> ret_;
          ret_.clear();
          {
            {
              std::cout << "sectionTest : memdb" << std::endl;
            }
          }
          std::unordered_map<dvmc::bytes32, dvmc::storage_value>& storage_recon = account_recon.storage;

          for(auto i_ : ret_)
          {
            dvmc::bytes32 reconKey;
            dev::bytes key = i_.second.first;

            for(int idx=0; idx<32; idx++)
            {
              reconKey.bytes[idx] = key[idx];
            }

            dvmc::storage_value sv;

            for(int idx=0; idx<32; idx++)
            {
              sv.value.bytes[idx] = i_.second.second[idx];
            }

            storage_recon[reconKey] = sv;
          }
        }
      }

      {
        dvmc::VertexNode vTrans;
        dvmc_address create_address;
        uint160 h160;
        AddressToHash160(cAddr_.ToString(),h160);
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

        if (result.status_code == DVMC_SUCCESS)
        {
          std::cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
        }

        dvmc::TransitionalNode updatedAcc = vTrans.accounts[create_address];
        std::cout << "sectionTest : node processed - storing updated state ..." << std::endl;
        dev::eth::Account tmpAcc(0,0);
        {
          dev::u256 nonce = 12345678;
          dev::u256 balance = 1010101010101;
          dev::RLPStream s(4);
          s << nonce << balance;
          {
            std::cout << "sectionTest STORE 1 : baseRoot " << retrievedAcc.baseRoot() << std::endl;
            std::cout << "sectionTest STORE 2 : state root " << state.root() << std::endl;
            dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state.db(), tmpAcc.baseRoot());

            for(auto pair : updatedAcc.storage)
            {
              auto storage_key = pair.first.bytes;
              dev::bytes key;

              for(int i=0; i<32; i++)
              {
                key.push_back(storage_key[i]);
              }

              dev::h256 key256(key);
              auto storage_bytes = pair.second.value;
              dev::bytes val;

              for(int i=0; i<32; i++)
              {
                val.push_back(storage_bytes.bytes[i]);
              }

              storageDB.insert(key256, dev::rlp(val));
            }

            s << storageDB.root();
          }
          s << retrievedAcc.codeHash();
          state.insert(target_addr, &s.out());
          std::cout << "sectionTest new state root " << state.root() << std::endl;
        }
      }
    }
  }
}
std::string code_hex_str = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200152838038062001528833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6111ef80620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff161415610ece57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610f1a57600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610fa757600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401146111b457fe5b5050505056fea265627a7a723158205d49fa73a530923e8947e52c7716fccdab177cd126ce5642208726148b661e2064736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a0000000000000000000000000000000000000000000000000000000000000000d6368616d2d32393130323032320000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000126368616d656c656f6e2d32393130323032320000000000000000000000000000";
string contract_transfer_no_check_no__transfer = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200149438038062001494833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b61115b80620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561087b565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b810190808035906020019092919050505061088e565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610990565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506109a8565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610bbe565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610c5c565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610c73565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610ddb565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b600081600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156108dc57600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156109f657600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610a7f57600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610c545780601f10610c2957610100808354040283529160200191610c54565b820191906000526020600020905b815481529060010190602001808311610c3757829003601f168201915b505050505081565b6000610c69338484610e00565b6001905092915050565b600080849050610c8385856106ec565b15610dd2578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610d61578082015181840152602081019050610d46565b50505050905090810190601f168015610d8e5780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610db057600080fd5b505af1158015610dc4573d6000803e3d6000fd5b505050506001915050610dd4565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff161415610e3a57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610e8657600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f1357600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011461112057fe5b5050505056fea265627a7a723158205cd0c528800733362a414f53d0de9cc3a44cd4d401d8d1a7fe25b00da51626bb64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000004746573740000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000047465737400000000000000000000000000000000000000000000000000000000";
string code_add_require_no_trans = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200151d3803806200151d833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6111e480620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b50610275610904565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610917565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a19565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a31565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c47565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610ce5565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610cfc565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e64565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561096557600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a7f57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b0857600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610cdd5780601f10610cb257610100808354040283529160200191610cdd565b820191906000526020600020905b815481529060010190602001808311610cc057829003601f168201915b505050505081565b6000610cf2338484610e89565b6001905092915050565b600080849050610d0c85856106ec565b15610e5b578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610dea578082015181840152602081019050610dcf565b50505050905090810190601f168015610e175780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e3957600080fd5b505af1158015610e4d573d6000803e3d6000fd5b505050506001915050610e5d565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff161415610ec357600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610f0f57600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f9c57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401146111a957fe5b5050505056fea265627a7a723158200e39f4a625dc933c525c3da67530525b17b1c85d58f8fa3e0958a4051e1b964164736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000077265717569726500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000b6e6f5f7472616e73666572000000000000000000000000000000000000000000";
string erc20__transfer_req1_commented_out = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b50604051620014ee380380620014ee833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6111b580620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610ee057600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f6d57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011461117a57fe5b5050505056fea265627a7a72315820853cd382da1ecfde57c0977c73926f83905fbb09d285ddabf78e0a04fcd713db64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000004726571310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000047265713100000000000000000000000000000000000000000000000000000000";
string erc20__transfer_req12_commented_out = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b50604051620014a2380380620014a2833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b61116980620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011015610f2157600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054011461112e57fe5b5050505056fea265627a7a723158202e8725742cad56203573046c92fc7f0cf76e425bf0d49af4aebb089db8bad81c64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000005726571313200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000057265713132000000000000000000000000000000000000000000000000000000";
string erc20__transfer_req123_commented_out = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b506040516200141538038062001415833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b6110dc80620003396000396000f3fe6080604052600436106100a75760003560e01c806370a082311161006457806370a08231146102e457806379cc67901461034957806395d89b41146103bc578063a9059cbb1461044c578063cae9ca51146104bf578063dd62ed3e146105c9576100a7565b806306fdde03146100ac578063095ea7b31461013c57806318160ddd146101af57806323b872dd146101da578063313ce5671461026057806342966c6814610291575b600080fd5b3480156100b857600080fd5b506100c161064e565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156101015780820151818401526020810190506100e6565b50505050905090810190601f16801561012e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561014857600080fd5b506101956004803603604081101561015f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506106ec565b604051808215151515815260200191505060405180910390f35b3480156101bb57600080fd5b506101c46107de565b6040518082815260200191505060405180910390f35b610246600480360360608110156101f057600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107e4565b604051808215151515815260200191505060405180910390f35b34801561026c57600080fd5b5061027561090f565b604051808260ff1660ff16815260200191505060405180910390f35b34801561029d57600080fd5b506102ca600480360360208110156102b457600080fd5b8101908080359060200190929190505050610922565b604051808215151515815260200191505060405180910390f35b3480156102f057600080fd5b506103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610a24565b6040518082815260200191505060405180910390f35b34801561035557600080fd5b506103a26004803603604081101561036c57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a3c565b604051808215151515815260200191505060405180910390f35b3480156103c857600080fd5b506103d1610c52565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156104115780820151818401526020810190506103f6565b50505050905090810190601f16801561043e5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b34801561045857600080fd5b506104a56004803603604081101561046f57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610cf0565b604051808215151515815260200191505060405180910390f35b3480156104cb57600080fd5b506105af600480360360608110156104e257600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561052957600080fd5b82018360208201111561053b57600080fd5b8035906020019184600183028401116401000000008311171561055d57600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050610d07565b604051808215151515815260200191505060405180910390f35b3480156105d557600080fd5b50610638600480360360408110156105ec57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610e6f565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156106e45780601f106106b9576101008083540402835291602001916106e4565b820191906000526020600020905b8154815290600101906020018083116106c757829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561086f57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610904848484610e94565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561097057600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b60046020528060005260406000206000915090505481565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610a8a57600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610b1357600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f016020809104026020016040519081016040528092919081815260200182805460018160011615610100020316600290048015610ce85780601f10610cbd57610100808354040283529160200191610ce8565b820191906000526020600020905b815481529060010190602001808311610ccb57829003601f168201915b505050505081565b6000610cfd338484610e94565b6001905092915050565b600080849050610d1785856106ec565b15610e66578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015610df5578082015181840152602081019050610dda565b50505050905090810190601f168015610e225780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b158015610e4457600080fd5b505af1158015610e58573d6000803e3d6000fd5b505050506001915050610e68565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401146110a157fe5b5050505056fea265627a7a72315820418ee216505cc5effbca7a293373664a352de7826158b3a4e59ccc6554d1a9db64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000006726571313233000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000067265713132330000000000000000000000000000000000000000000000000000";
string code_hex_08112022_trace = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b5060405162001e7538038062001e75833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b611b3c80620003396000396000f3fe608060405234801561001057600080fd5b50600436106100f55760003560e01c80636eef7a051161009757806395d89b411161006657806395d89b41146105fa578063a9059cbb1461067d578063cae9ca51146106e3578063dd62ed3e146107e0576100f5565b80636eef7a05146103ae57806370a082311461045557806374546363146104ad57806379cc679014610594576100f5565b806323b872dd116100d357806323b872dd14610201578063313ce5671461028757806342966c68146102ab5780635e57966d146102f1576100f5565b806306fdde03146100fa578063095ea7b31461017d57806318160ddd146101e3575b600080fd5b610102610858565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610142578082015181840152602081019050610127565b50505050905090810190601f16801561016f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6101c96004803603604081101561019357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506108f6565b604051808215151515815260200191505060405180910390f35b6101eb6109e8565b6040518082815260200191505060405180910390f35b61026d6004803603606081101561021757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506109ee565b604051808215151515815260200191505060405180910390f35b61028f610b19565b604051808260ff1660ff16815260200191505060405180910390f35b6102d7600480360360208110156102c157600080fd5b8101908080359060200190929190505050610b2c565b604051808215151515815260200191505060405180910390f35b6103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610c2e565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610373578082015181840152602081019050610358565b50505050905090810190601f1680156103a05780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6103da600480360360208110156103c457600080fd5b8101908080359060200190929190505050610eaf565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561041a5780820151818401526020810190506103ff565b50505050905090810190601f1680156104475780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6104976004803603602081101561046b57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611110565b6040518082815260200191505060405180910390f35b610519600480360360608110156104c357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611128565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561055957808201518184015260208101905061053e565b50505050905090810190601f1680156105865780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6105e0600480360360408110156105aa57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611389565b604051808215151515815260200191505060405180910390f35b61060261159f565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610642578082015181840152602081019050610627565b50505050905090810190601f16801561066f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6106c96004803603604081101561069357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff1690602001909291908035906020019092919050505061163d565b604051808215151515815260200191505060405180910390f35b6107c6600480360360608110156106f957600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561074057600080fd5b82018360208201111561075257600080fd5b8035906020019184600183028401116401000000008311171561077457600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f820116905080830192505050505050509192919290505050611654565b604051808215151515815260200191505060405180910390f35b610842600480360360408110156107f657600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff1690602001909291905050506117bc565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156108ee5780601f106108c3576101008083540402835291602001916108ee565b820191906000526020600020905b8154815290600101906020018083116108d157829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610a7957600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610b0e8484846117e1565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610b7a57600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060606040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610cbe5781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610cef57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d4c57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b6014811015610ea35782600485600c840160208110610d9c57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610dd457fe5b602001015160f81c60f81b826002830260020181518110610df157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c840160208110610e3557fe5b1a60f81b1660f81c60ff1681518110610e4a57fe5b602001015160f81c60f81b826002830260030181518110610e6757fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610d81565b50809350505050919050565b6060806040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610f205781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610f5157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610fae57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b60148110156111055782600486600c840160208110610ffe57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff168151811061103657fe5b602001015160f81c60f81b82600283026002018151811061105357fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84016020811061109757fe5b1a60f81b1660f81c60ff16815181106110ac57fe5b602001015160f81c60f81b8260028302600301815181106110c957fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610fe3565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b905061113b85610c2e565b61114485610c2e565b61114d83610eaf565b6040516020018082805190602001908083835b602083106111835780518252602082019150602081019050602083039250611160565b6001836020036101000a0380198251168184511680821785525050505050509050019150506040516020818303038152906040526040516020018083805190602001908083835b602083106111ed57805182526020820191506020810190506020830392506111ca565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020182805190602001908083835b602083106112665780518252602082019150602081019050602083039250611243565b6001836020036101000a038019825116818451168082178552505050505050905001925050506040516020818303038152906040526040516020018083805190602001908083835b602083106112d157805182526020820191506020810190506020830392506112ae565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020182805190602001908083835b6020831061134a5780518252602082019150602081019050602083039250611327565b6001836020036101000a038019825116818451168082178552505050505050905001925050506040516020818303038152906040529150509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156113d757600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561146057600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156116355780601f1061160a57610100808354040283529160200191611635565b820191906000526020600020905b81548152906001019060200180831161161857829003601f168201915b505050505081565b600061164a3384846117e1565b6001905092915050565b60008084905061166485856108f6565b156117b3578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b83811015611742578082015181840152602081019050611727565b50505050905090810190601f16801561176f5780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b15801561179157600080fd5b505af11580156117a5573d6000803e3d6000fd5b5050505060019150506117b5565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff16141561181b57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561186757600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540110156118f457600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540114611b0157fe5b5050505056fea265627a7a7231582090a370df1504357cad55837085e9ed659bf0984b9ee6449a3f98e528bd51085664736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";
string code_hex_09112022_trace = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b5060405162001efe38038062001efe833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b611bc580620003396000396000f3fe608060405234801561001057600080fd5b50600436106100f55760003560e01c80636eef7a051161009757806395d89b411161006657806395d89b41146105fa578063a9059cbb1461067d578063cae9ca51146106e3578063dd62ed3e146107e0576100f5565b80636eef7a05146103ae57806370a082311461045557806374546363146104ad57806379cc679014610594576100f5565b806323b872dd116100d357806323b872dd14610201578063313ce5671461028757806342966c68146102ab5780635e57966d146102f1576100f5565b806306fdde03146100fa578063095ea7b31461017d57806318160ddd146101e3575b600080fd5b610102610858565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610142578082015181840152602081019050610127565b50505050905090810190601f16801561016f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6101c96004803603604081101561019357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506108f6565b604051808215151515815260200191505060405180910390f35b6101eb6109e8565b6040518082815260200191505060405180910390f35b61026d6004803603606081101561021757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506109ee565b604051808215151515815260200191505060405180910390f35b61028f610b19565b604051808260ff1660ff16815260200191505060405180910390f35b6102d7600480360360208110156102c157600080fd5b8101908080359060200190929190505050610b2c565b604051808215151515815260200191505060405180910390f35b6103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610c2e565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610373578082015181840152602081019050610358565b50505050905090810190601f1680156103a05780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6103da600480360360208110156103c457600080fd5b8101908080359060200190929190505050610eaf565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561041a5780820151818401526020810190506103ff565b50505050905090810190601f1680156104475780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6104976004803603602081101561046b57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611110565b6040518082815260200191505060405180910390f35b610519600480360360608110156104c357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611128565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561055957808201518184015260208101905061053e565b50505050905090810190601f1680156105865780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6105e0600480360360408110156105aa57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611412565b604051808215151515815260200191505060405180910390f35b610602611628565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610642578082015181840152602081019050610627565b50505050905090810190601f16801561066f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6106c96004803603604081101561069357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506116c6565b604051808215151515815260200191505060405180910390f35b6107c6600480360360608110156106f957600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561074057600080fd5b82018360208201111561075257600080fd5b8035906020019184600183028401116401000000008311171561077457600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f8201169050808301925050505050505091929192905050506116dd565b604051808215151515815260200191505060405180910390f35b610842600480360360408110156107f657600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611845565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156108ee5780601f106108c3576101008083540402835291602001916108ee565b820191906000526020600020905b8154815290600101906020018083116108d157829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610a7957600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610b0e84848461186a565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610b7a57600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060606040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610cbe5781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610cef57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d4c57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b6014811015610ea35782600485600c840160208110610d9c57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610dd457fe5b602001015160f81c60f81b826002830260020181518110610df157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c840160208110610e3557fe5b1a60f81b1660f81c60ff1681518110610e4a57fe5b602001015160f81c60f81b826002830260030181518110610e6757fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610d81565b50809350505050919050565b6060806040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610f205781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610f5157fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610fae57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b60148110156111055782600486600c840160208110610ffe57fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff168151811061103657fe5b602001015160f81c60f81b82600283026002018151811061105357fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84016020811061109757fe5b1a60f81b1660f81c60ff16815181106110ac57fe5b602001015160f81c60f81b8260028302600301815181106110c957fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610fe3565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b9050606061113d33610c2e565b905061114886610c2e565b61115186610c2e565b61115a84610eaf565b6040516020018082805190602001908083835b60208310611190578051825260208201915060208101905060208303925061116d565b6001836020036101000a038019825116818451168082178552505050505050905001915050604051602081830303815290604052836040516020018084805190602001908083835b602083106111fb57805182526020820191506020810190506020830392506111d8565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020183805190602001908083835b602083106112745780518252602082019150602081019050602083039250611251565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c206d73672e73656e6465722000000000000000000000000000000000000000815250600d0182805190602001908083835b602083106112ed57805182526020820191506020810190506020830392506112ca565b6001836020036101000a03801982511681845116808217855250505050505090500193505050506040516020818303038152906040526040516020018083805190602001908083835b602083106113595780518252602082019150602081019050602083039250611336565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020182805190602001908083835b602083106113d257805182526020820191506020810190506020830392506113af565b6001836020036101000a03801982511681845116808217855250505050505090500192505050604051602081830303815290604052925050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561146057600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548211156114e957600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156116be5780601f10611693576101008083540402835291602001916116be565b820191906000526020600020905b8154815290600101906020018083116116a157829003601f168201915b505050505081565b60006116d333848461186a565b6001905092915050565b6000808490506116ed85856108f6565b1561183c578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b838110156117cb5780820151818401526020810190506117b0565b50505050905090810190601f1680156117f85780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b15801561181a57600080fd5b505af115801561182e573d6000803e3d6000fd5b50505050600191505061183e565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff1614156118a457600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156118f057600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401101561197d57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540114611b8a57fe5b5050505056fea265627a7a72315820af7e8bbbb6d6a9741aba102f7f3596e217365cf91a782527fe8726f22d31901964736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";
string code_with_balance_adjust_in_approve_str = "0x60806040526012600260006101000a81548160ff021916908360ff1602179055503480156200002d57600080fd5b5060405162001f3f38038062001f3f833981810160405260608110156200005357600080fd5b8101908080519060200190929190805160405193929190846401000000008211156200007e57600080fd5b838201915060208201858111156200009557600080fd5b8251866001820283011164010000000082111715620000b357600080fd5b8083526020830192505050908051906020019080838360005b83811015620000e9578082015181840152602081019050620000cc565b50505050905090810190601f168015620001175780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200013b57600080fd5b838201915060208201858111156200015257600080fd5b82518660018202830111640100000000821117156200017057600080fd5b8083526020830192505050908051906020019080838360005b83811015620001a657808201518184015260208101905062000189565b50505050905090810190601f168015620001d45780820380516001836020036101000a031916815260200191505b50604052505050600260009054906101000a900460ff1660ff16600a0a8302600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009080519060200190620002579291906200027a565b508060019080519060200190620002709291906200027a565b5050505062000329565b828054600181600116156101000203166002900490600052602060002090601f016020900481019282601f10620002bd57805160ff1916838001178555620002ee565b82800160010185558215620002ee579182015b82811115620002ed578251825591602001919060010190620002d0565b5b509050620002fd919062000301565b5090565b6200032691905b808211156200032257600081600090555060010162000308565b5090565b90565b611c0680620003396000396000f3fe608060405234801561001057600080fd5b50600436106100f55760003560e01c80636eef7a051161009757806395d89b411161006657806395d89b41146105fa578063a9059cbb1461067d578063cae9ca51146106e3578063dd62ed3e146107e0576100f5565b80636eef7a05146103ae57806370a082311461045557806374546363146104ad57806379cc679014610594576100f5565b806323b872dd116100d357806323b872dd14610201578063313ce5671461028757806342966c68146102ab5780635e57966d146102f1576100f5565b806306fdde03146100fa578063095ea7b31461017d57806318160ddd146101e3575b600080fd5b610102610858565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610142578082015181840152602081019050610127565b50505050905090810190601f16801561016f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6101c96004803603604081101561019357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506108f6565b604051808215151515815260200191505060405180910390f35b6101eb610a2c565b6040518082815260200191505060405180910390f35b61026d6004803603606081101561021757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610a32565b604051808215151515815260200191505060405180910390f35b61028f610b5d565b604051808260ff1660ff16815260200191505060405180910390f35b6102d7600480360360208110156102c157600080fd5b8101908080359060200190929190505050610b70565b604051808215151515815260200191505060405180910390f35b6103336004803603602081101561030757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050610c72565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610373578082015181840152602081019050610358565b50505050905090810190601f1680156103a05780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6103da600480360360208110156103c457600080fd5b8101908080359060200190929190505050610ef3565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561041a5780820151818401526020810190506103ff565b50505050905090810190601f1680156104475780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6104976004803603602081101561046b57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611154565b6040518082815260200191505060405180910390f35b610519600480360360608110156104c357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff1690602001909291908035906020019092919050505061116c565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561055957808201518184015260208101905061053e565b50505050905090810190601f1680156105865780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6105e0600480360360408110156105aa57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611453565b604051808215151515815260200191505060405180910390f35b610602611669565b6040518080602001828103825283818151815260200191508051906020019080838360005b83811015610642578082015181840152602081019050610627565b50505050905090810190601f16801561066f5780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6106c96004803603604081101561069357600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050611707565b604051808215151515815260200191505060405180910390f35b6107c6600480360360608110156106f957600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291908035906020019064010000000081111561074057600080fd5b82018360208201111561075257600080fd5b8035906020019184600183028401116401000000008311171561077457600080fd5b91908080601f016020809104026020016040519081016040528093929190818152602001838380828437600081840152601f19601f82011690508083019250505050505050919291929050505061171e565b604051808215151515815260200191505060405180910390f35b610842600480360360408110156107f657600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff169060200190929190505050611886565b6040518082815260200191505060405180910390f35b60008054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156108ee5780601f106108c3576101008083540402835291602001916108ee565b820191906000526020600020905b8154815290600101906020018083116108d157829003601f168201915b505050505081565b600081600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60035481565b6000600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054821115610abd57600080fd5b81600560008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550610b528484846118ab565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020541015610bbe57600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a260019050919050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060606040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610d025781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610d3357fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d9057fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b6014811015610ee75782600485600c840160208110610de057fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610e1857fe5b602001015160f81c60f81b826002830260020181518110610e3557fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c840160208110610e7957fe5b1a60f81b1660f81c60ff1681518110610e8e57fe5b602001015160f81c60f81b826002830260030181518110610eab57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050610dc5565b50809350505050919050565b6060806040518060400160405280601081526020017f30313233343536373839616263646566000000000000000000000000000000008152509050606060336040519080825280601f01601f191660200182016040528015610f645781602001600182028038833980820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610f9557fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610ff257fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060008090505b60148110156111495782600486600c84016020811061104257fe5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff168151811061107a57fe5b602001015160f81c60f81b82600283026002018151811061109757fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c8401602081106110db57fe5b1a60f81b1660f81c60ff16815181106110f057fe5b602001015160f81c60f81b82600283026003018151811061110d57fe5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080600101915050611027565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b90506000600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205460001b905060606111c882610ef3565b905060606111d533610c72565b90506111e088610c72565b6111e988610c72565b6111f286610ef3565b83856040516020018086805190602001908083835b6020831061122a5780518252602082019150602081019050602083039250611207565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020185805190602001908083835b602083106112a35780518252602082019150602081019050602083039250611280565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2000000000000000000000000000000000000000000000000000000000000081525060020184805190602001908083835b6020831061131c57805182526020820191506020810190506020830392506112f9565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c206d73672e73656e6465722000000000000000000000000000000000000000815250600d0183805190602001908083835b602083106113955780518252602082019150602081019050602083039250611372565b6001836020036101000a038019825116818451168082178552505050505050905001807f2c2062616c616e63654f665b5f66726f6d5d200000000000000000000000000081525060130182805190602001908083835b6020831061140e57805182526020820191506020810190506020830392506113eb565b6001836020036101000a038019825116818451168082178552505050505050905001955050505050506040516020818303038152906040529450505050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156114a157600080fd5b600560008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561152a57600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600560008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550816003600082825403925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040518082815260200191505060405180910390a26001905092915050565b60018054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156116ff5780601f106116d4576101008083540402835291602001916116ff565b820191906000526020600020905b8154815290600101906020018083116116e257829003601f168201915b505050505081565b60006117143384846118ab565b6001905092915050565b60008084905061172e85856108f6565b1561187d578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401808573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020018481526020018373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200180602001828103825283818151815260200191508051906020019080838360005b8381101561180c5780820151818401526020810190506117f1565b50505050905090810190601f1680156118395780820380516001836020036101000a031916815260200191505b5095505050505050600060405180830381600087803b15801561185b57600080fd5b505af115801561186f573d6000803e3d6000fd5b50505050600191505061187f565b505b9392505050565b6005602052816000526040600020602052806000526040600020600091509150505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff1614156118e557600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561193157600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540110156119be57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205401905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020540114611bcb57fe5b5050505056fea265627a7a72315820f2148c884af1db1c042aaef791c843bf8cec31e876a8874f9ccaa2aab01c941c64736f6c6343000511003200000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";
string erc20_15112022_endowed_with_5_eth = "0x60806040526012600260006101000a81548160ff021916908360ff160217905550604051620029cd380380620029cd8339818101604052810190620000459190620002ba565b600260009054906101000a900460ff1660ff16600a620000669190620004d7565b8362000073919062000528565b600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009081620000d09190620007b4565b508060019081620000e29190620007b4565b505050506200089b565b6000604051905090565b600080fd5b600080fd5b6000819050919050565b620001158162000100565b81146200012157600080fd5b50565b60008151905062000135816200010a565b92915050565b600080fd5b600080fd5b6000601f19601f8301169050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b620001908262000145565b810181811067ffffffffffffffff82111715620001b257620001b162000156565b5b80604052505050565b6000620001c7620000ec565b9050620001d5828262000185565b919050565b600067ffffffffffffffff821115620001f857620001f762000156565b5b620002038262000145565b9050602081019050919050565b60005b838110156200023057808201518184015260208101905062000213565b60008484015250505050565b6000620002536200024d84620001da565b620001bb565b90508281526020810184848401111562000272576200027162000140565b5b6200027f84828562000210565b509392505050565b600082601f8301126200029f576200029e6200013b565b5b8151620002b18482602086016200023c565b91505092915050565b600080600060608486031215620002d657620002d5620000f6565b5b6000620002e68682870162000124565b935050602084015167ffffffffffffffff8111156200030a5762000309620000fb565b5b620003188682870162000287565b925050604084015167ffffffffffffffff8111156200033c576200033b620000fb565b5b6200034a8682870162000287565b9150509250925092565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b60008160011c9050919050565b6000808291508390505b6001851115620003e257808604811115620003ba57620003b962000354565b5b6001851615620003ca5780820291505b8081029050620003da8562000383565b94506200039a565b94509492505050565b600082620003fd5760019050620004d0565b816200040d5760009050620004d0565b8160018114620004265760028114620004315762000467565b6001915050620004d0565b60ff84111562000446576200044562000354565b5b8360020a91508482111562000460576200045f62000354565b5b50620004d0565b5060208310610133831016604e8410600b8410161715620004a15782820a9050838111156200049b576200049a62000354565b5b620004d0565b620004b0848484600162000390565b92509050818404811115620004ca57620004c962000354565b5b81810290505b9392505050565b6000620004e48262000100565b9150620004f18362000100565b9250620005207fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8484620003eb565b905092915050565b6000620005358262000100565b9150620005428362000100565b9250828202620005528162000100565b915082820484148315176200056c576200056b62000354565b5b5092915050565b600081519050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680620005c657607f821691505b602082108103620005dc57620005db6200057e565b5b50919050565b60008190508160005260206000209050919050565b60006020601f8301049050919050565b600082821b905092915050565b600060088302620006467fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8262000607565b62000652868362000607565b95508019841693508086168417925050509392505050565b6000819050919050565b6000620006956200068f620006898462000100565b6200066a565b62000100565b9050919050565b6000819050919050565b620006b18362000674565b620006c9620006c0826200069c565b84845462000614565b825550505050565b600090565b620006e0620006d1565b620006ed818484620006a6565b505050565b5b81811015620007155762000709600082620006d6565b600181019050620006f3565b5050565b601f82111562000764576200072e81620005e2565b6200073984620005f7565b8101602085101562000749578190505b620007616200075885620005f7565b830182620006f2565b50505b505050565b600082821c905092915050565b6000620007896000198460080262000769565b1980831691505092915050565b6000620007a4838362000776565b9150826002028217905092915050565b620007bf8262000573565b67ffffffffffffffff811115620007db57620007da62000156565b5b620007e78254620005ad565b620007f482828562000719565b600060209050601f8311600181146200082c576000841562000817578287015190505b62000823858262000796565b86555062000893565b601f1984166200083c86620005e2565b60005b8281101562000866578489015182556001820191506020850194506020810190506200083f565b8683101562000886578489015162000882601f89168262000776565b8355505b6001600288020188555050505b505050505050565b61212280620008ab6000396000f3fe6080604052600436106101095760003560e01c806370a0823111610095578063a9059cbb11610064578063a9059cbb146103cb578063cae9ca5114610408578063d0e30db014610445578063dd62ed3e1461044f578063f665b8d81461048c57610109565b806370a08231146102e9578063745463631461032657806379cc67901461036357806395d89b41146103a057610109565b8063313ce567116100dc578063313ce567146101de57806342966c6814610209578063486556ce146102465780635e57966d1461026f5780636eef7a05146102ac57610109565b806306fdde031461010e578063095ea7b31461013957806318160ddd1461017657806323b872dd146101a1575b600080fd5b34801561011a57600080fd5b506101236104c9565b6040516101309190611831565b60405180910390f35b34801561014557600080fd5b50610160600480360381019061015b91906118fb565b610557565b60405161016d9190611956565b60405180910390f35b34801561018257600080fd5b5061018b61068d565b6040516101989190611980565b60405180910390f35b3480156101ad57600080fd5b506101c860048036038101906101c3919061199b565b610693565b6040516101d59190611956565b60405180910390f35b3480156101ea57600080fd5b506101f36107c7565b6040516102009190611a0a565b60405180910390f35b34801561021557600080fd5b50610230600480360381019061022b9190611a25565b6107da565b60405161023d9190611956565b60405180910390f35b34801561025257600080fd5b5061026d60048036038101906102689190611a25565b6108ee565b005b34801561027b57600080fd5b5061029660048036038101906102919190611a52565b61097d565b6040516102a39190611831565b60405180910390f35b3480156102b857600080fd5b506102d360048036038101906102ce9190611ab5565b610c88565b6040516102e09190611831565b60405180910390f35b3480156102f557600080fd5b50610310600480360381019061030b9190611a52565b610f74565b60405161031d9190611980565b60405180910390f35b34801561033257600080fd5b5061034d6004803603810190610348919061199b565b610f8c565b60405161035a9190611831565b60405180910390f35b34801561036f57600080fd5b5061038a600480360381019061038591906118fb565b611044565b6040516103979190611956565b60405180910390f35b3480156103ac57600080fd5b506103b5611275565b6040516103c29190611831565b60405180910390f35b3480156103d757600080fd5b506103f260048036038101906103ed91906118fb565b611303565b6040516103ff9190611956565b60405180910390f35b34801561041457600080fd5b5061042f600480360381019061042a9190611c17565b61131a565b60405161043c9190611956565b60405180910390f35b61044d6113b3565b005b34801561045b57600080fd5b5061047660048036038101906104719190611c86565b61140b565b6040516104839190611980565b60405180910390f35b34801561049857600080fd5b506104b360048036038101906104ae9190611a52565b611430565b6040516104c09190611980565b60405180910390f35b600080546104d690611cf5565b80601f016020809104026020016040519081016040528092919081815260200182805461050290611cf5565b801561054f5780601f106105245761010080835404028352916020019161054f565b820191906000526020600020905b81548152906001019060200180831161053257829003601f168201915b505050505081565b600081600660003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161067b9190611980565b60405180910390a36001905092915050565b60035481565b6000600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561071e57600080fd5b81600660008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546107aa9190611d55565b925050819055506107bc848484611448565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561082857600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546108779190611d55565b9250508190555081600360008282546108909190611d55565b925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040516108dd9190611980565b60405180910390a260019050919050565b6000600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055503373ffffffffffffffffffffffffffffffffffffffff166108fc829081150290604051600060405180830381858888f19350505050158015610979573d6000803e3d6000fd5b5050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff8111156109f4576109f3611aec565b5b6040519080825280601f01601f191660200182016040528015610a265781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610a5e57610a5d611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610ac257610ac1611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610c7c5782600485600c84610b0e9190611db8565b60208110610b1f57610b1e611d89565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610b5e57610b5d611d89565b5b602001015160f81c60f81b82600283610b779190611dec565b6002610b839190611db8565b81518110610b9457610b93611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c84610bd79190611db8565b60208110610be857610be7611d89565b5b1a60f81b1660f81c60ff1681518110610c0457610c03611d89565b5b602001015160f81c60f81b82600283610c1d9190611dec565b6003610c299190611db8565b81518110610c3a57610c39611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610c7490611e2e565b915050610af4565b50809350505050919050565b606060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff811115610ce157610ce0611aec565b5b6040519080825280601f01601f191660200182016040528015610d135781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610d4b57610d4a611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610daf57610dae611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610f695782600486600c84610dfb9190611db8565b60208110610e0c57610e0b611d89565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610e4b57610e4a611d89565b5b602001015160f81c60f81b82600283610e649190611dec565b6002610e709190611db8565b81518110610e8157610e80611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84610ec49190611db8565b60208110610ed557610ed4611d89565b5b1a60f81b1660f81c60ff1681518110610ef157610ef0611d89565b5b602001015160f81c60f81b82600283610f0a9190611dec565b6003610f169190611db8565b81518110610f2757610f26611d89565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610f6190611e2e565b915050610de1565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b90506000600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205460001b90506000610fe882610c88565b90506000610ff53361097d565b90506110008861097d565b6110098861097d565b61101286610c88565b8385604051602001611028959493929190611f96565b6040516020818303038152906040529450505050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561109257600080fd5b600660008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561111b57600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825461116a9190611d55565b9250508190555081600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111fd9190611d55565b9250508190555081600360008282546112169190611d55565b925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040516112639190611980565b60405180910390a26001905092915050565b6001805461128290611cf5565b80601f01602080910402602001604051908101604052809291908181526020018280546112ae90611cf5565b80156112fb5780601f106112d0576101008083540402835291602001916112fb565b820191906000526020600020905b8154815290600101906020018083116112de57829003601f168201915b505050505081565b6000611310338484611448565b6001905092915050565b60008084905061132a8585610557565b156113aa578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b815260040161136e9493929190612071565b600060405180830381600087803b15801561138857600080fd5b505af115801561139c573d6000803e3d6000fd5b5050505060019150506113ac565b505b9392505050565b34600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546114029190611db8565b92505081905550565b6006602052816000526040600020602052806000526040600020600091509150505481565b60056020528060005260406000206000915090505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff160361148157600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156114cd57600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115589190611db8565b101561156357600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115ef9190611db8565b905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546116409190611d55565b9250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546116969190611db8565b925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040516116fa9190611980565b60405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205461178d9190611db8565b1461179b5761179a6120bd565b5b50505050565b600081519050919050565b600082825260208201905092915050565b60005b838110156117db5780820151818401526020810190506117c0565b60008484015250505050565b6000601f19601f8301169050919050565b6000611803826117a1565b61180d81856117ac565b935061181d8185602086016117bd565b611826816117e7565b840191505092915050565b6000602082019050818103600083015261184b81846117f8565b905092915050565b6000604051905090565b600080fd5b600080fd5b600073ffffffffffffffffffffffffffffffffffffffff82169050919050565b600061189282611867565b9050919050565b6118a281611887565b81146118ad57600080fd5b50565b6000813590506118bf81611899565b92915050565b6000819050919050565b6118d8816118c5565b81146118e357600080fd5b50565b6000813590506118f5816118cf565b92915050565b600080604083850312156119125761191161185d565b5b6000611920858286016118b0565b9250506020611931858286016118e6565b9150509250929050565b60008115159050919050565b6119508161193b565b82525050565b600060208201905061196b6000830184611947565b92915050565b61197a816118c5565b82525050565b60006020820190506119956000830184611971565b92915050565b6000806000606084860312156119b4576119b361185d565b5b60006119c2868287016118b0565b93505060206119d3868287016118b0565b92505060406119e4868287016118e6565b9150509250925092565b600060ff82169050919050565b611a04816119ee565b82525050565b6000602082019050611a1f60008301846119fb565b92915050565b600060208284031215611a3b57611a3a61185d565b5b6000611a49848285016118e6565b91505092915050565b600060208284031215611a6857611a6761185d565b5b6000611a76848285016118b0565b91505092915050565b6000819050919050565b611a9281611a7f565b8114611a9d57600080fd5b50565b600081359050611aaf81611a89565b92915050565b600060208284031215611acb57611aca61185d565b5b6000611ad984828501611aa0565b91505092915050565b600080fd5b600080fd5b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b611b24826117e7565b810181811067ffffffffffffffff82111715611b4357611b42611aec565b5b80604052505050565b6000611b56611853565b9050611b628282611b1b565b919050565b600067ffffffffffffffff821115611b8257611b81611aec565b5b611b8b826117e7565b9050602081019050919050565b82818337600083830152505050565b6000611bba611bb584611b67565b611b4c565b905082815260208101848484011115611bd657611bd5611ae7565b5b611be1848285611b98565b509392505050565b600082601f830112611bfe57611bfd611ae2565b5b8135611c0e848260208601611ba7565b91505092915050565b600080600060608486031215611c3057611c2f61185d565b5b6000611c3e868287016118b0565b9350506020611c4f868287016118e6565b925050604084013567ffffffffffffffff811115611c7057611c6f611862565b5b611c7c86828701611be9565b9150509250925092565b60008060408385031215611c9d57611c9c61185d565b5b6000611cab858286016118b0565b9250506020611cbc858286016118b0565b9150509250929050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680611d0d57607f821691505b602082108103611d2057611d1f611cc6565b5b50919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b6000611d60826118c5565b9150611d6b836118c5565b9250828203905081811115611d8357611d82611d26565b5b92915050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052603260045260246000fd5b6000611dc3826118c5565b9150611dce836118c5565b9250828201905080821115611de657611de5611d26565b5b92915050565b6000611df7826118c5565b9150611e02836118c5565b9250828202611e10816118c5565b91508282048414831517611e2757611e26611d26565b5b5092915050565b6000611e39826118c5565b91507fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8203611e6b57611e6a611d26565b5b600182019050919050565b600081905092915050565b6000611e8c826117a1565b611e968185611e76565b9350611ea68185602086016117bd565b80840191505092915050565b7f2c20000000000000000000000000000000000000000000000000000000000000600082015250565b6000611ee8600283611e76565b9150611ef382611eb2565b600282019050919050565b7f2c206d73672e73656e6465722000000000000000000000000000000000000000600082015250565b6000611f34600d83611e76565b9150611f3f82611efe565b600d82019050919050565b7f2c2062616c616e63654f665b5f66726f6d5d2000000000000000000000000000600082015250565b6000611f80601383611e76565b9150611f8b82611f4a565b601382019050919050565b6000611fa28288611e81565b9150611fad82611edb565b9150611fb98287611e81565b9150611fc482611edb565b9150611fd08286611e81565b9150611fdb82611f27565b9150611fe78285611e81565b9150611ff282611f73565b9150611ffe8284611e81565b91508190509695505050505050565b61201681611887565b82525050565b600081519050919050565b600082825260208201905092915050565b60006120438261201c565b61204d8185612027565b935061205d8185602086016117bd565b612066816117e7565b840191505092915050565b6000608082019050612086600083018761200d565b6120936020830186611971565b6120a0604083018561200d565b81810360608301526120b28184612038565b905095945050505050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052600160045260246000fdfea26469706673582212200137661fe66354246db4b7e9780409355797a4bafa06718384bcfff85e8b3c1464736f6c6343000811003300000000000000000000000000000000000000000000000000000000000f4240000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";
string erc20_17112022_send_fixed_amount_1 = "0x60806040526012600260006101000a81548160ff021916908360ff160217905550604051620029ce380380620029ce8339818101604052810190620000459190620002ba565b600260009054906101000a900460ff1660ff16600a620000669190620004d7565b8362000073919062000528565b600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009081620000d09190620007b4565b508060019081620000e29190620007b4565b505050506200089b565b6000604051905090565b600080fd5b600080fd5b6000819050919050565b620001158162000100565b81146200012157600080fd5b50565b60008151905062000135816200010a565b92915050565b600080fd5b600080fd5b6000601f19601f8301169050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b620001908262000145565b810181811067ffffffffffffffff82111715620001b257620001b162000156565b5b80604052505050565b6000620001c7620000ec565b9050620001d5828262000185565b919050565b600067ffffffffffffffff821115620001f857620001f762000156565b5b620002038262000145565b9050602081019050919050565b60005b838110156200023057808201518184015260208101905062000213565b60008484015250505050565b6000620002536200024d84620001da565b620001bb565b90508281526020810184848401111562000272576200027162000140565b5b6200027f84828562000210565b509392505050565b600082601f8301126200029f576200029e6200013b565b5b8151620002b18482602086016200023c565b91505092915050565b600080600060608486031215620002d657620002d5620000f6565b5b6000620002e68682870162000124565b935050602084015167ffffffffffffffff8111156200030a5762000309620000fb565b5b620003188682870162000287565b925050604084015167ffffffffffffffff8111156200033c576200033b620000fb565b5b6200034a8682870162000287565b9150509250925092565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b60008160011c9050919050565b6000808291508390505b6001851115620003e257808604811115620003ba57620003b962000354565b5b6001851615620003ca5780820291505b8081029050620003da8562000383565b94506200039a565b94509492505050565b600082620003fd5760019050620004d0565b816200040d5760009050620004d0565b8160018114620004265760028114620004315762000467565b6001915050620004d0565b60ff84111562000446576200044562000354565b5b8360020a91508482111562000460576200045f62000354565b5b50620004d0565b5060208310610133831016604e8410600b8410161715620004a15782820a9050838111156200049b576200049a62000354565b5b620004d0565b620004b0848484600162000390565b92509050818404811115620004ca57620004c962000354565b5b81810290505b9392505050565b6000620004e48262000100565b9150620004f18362000100565b9250620005207fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8484620003eb565b905092915050565b6000620005358262000100565b9150620005428362000100565b9250828202620005528162000100565b915082820484148315176200056c576200056b62000354565b5b5092915050565b600081519050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680620005c657607f821691505b602082108103620005dc57620005db6200057e565b5b50919050565b60008190508160005260206000209050919050565b60006020601f8301049050919050565b600082821b905092915050565b600060088302620006467fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8262000607565b62000652868362000607565b95508019841693508086168417925050509392505050565b6000819050919050565b6000620006956200068f620006898462000100565b6200066a565b62000100565b9050919050565b6000819050919050565b620006b18362000674565b620006c9620006c0826200069c565b84845462000614565b825550505050565b600090565b620006e0620006d1565b620006ed818484620006a6565b505050565b5b81811015620007155762000709600082620006d6565b600181019050620006f3565b5050565b601f82111562000764576200072e81620005e2565b6200073984620005f7565b8101602085101562000749578190505b620007616200075885620005f7565b830182620006f2565b50505b505050565b600082821c905092915050565b6000620007896000198460080262000769565b1980831691505092915050565b6000620007a4838362000776565b9150826002028217905092915050565b620007bf8262000573565b67ffffffffffffffff811115620007db57620007da62000156565b5b620007e78254620005ad565b620007f482828562000719565b600060209050601f8311600181146200082c576000841562000817578287015190505b62000823858262000796565b86555062000893565b601f1984166200083c86620005e2565b60005b8281101562000866578489015182556001820191506020850194506020810190506200083f565b8683101562000886578489015162000882601f89168262000776565b8355505b6001600288020188555050505b505050505050565b61212380620008ab6000396000f3fe6080604052600436106101095760003560e01c806370a0823111610095578063a9059cbb11610064578063a9059cbb146103cb578063cae9ca5114610408578063d0e30db014610445578063dd62ed3e1461044f578063f665b8d81461048c57610109565b806370a08231146102e9578063745463631461032657806379cc67901461036357806395d89b41146103a057610109565b8063313ce567116100dc578063313ce567146101de57806342966c6814610209578063486556ce146102465780635e57966d1461026f5780636eef7a05146102ac57610109565b806306fdde031461010e578063095ea7b31461013957806318160ddd1461017657806323b872dd146101a1575b600080fd5b34801561011a57600080fd5b506101236104c9565b6040516101309190611832565b60405180910390f35b34801561014557600080fd5b50610160600480360381019061015b91906118fc565b610557565b60405161016d9190611957565b60405180910390f35b34801561018257600080fd5b5061018b61068d565b6040516101989190611981565b60405180910390f35b3480156101ad57600080fd5b506101c860048036038101906101c3919061199c565b610693565b6040516101d59190611957565b60405180910390f35b3480156101ea57600080fd5b506101f36107c7565b6040516102009190611a0b565b60405180910390f35b34801561021557600080fd5b50610230600480360381019061022b9190611a26565b6107da565b60405161023d9190611957565b60405180910390f35b34801561025257600080fd5b5061026d60048036038101906102689190611a26565b6108ee565b005b34801561027b57600080fd5b5061029660048036038101906102919190611a53565b61097e565b6040516102a39190611832565b60405180910390f35b3480156102b857600080fd5b506102d360048036038101906102ce9190611ab6565b610c89565b6040516102e09190611832565b60405180910390f35b3480156102f557600080fd5b50610310600480360381019061030b9190611a53565b610f75565b60405161031d9190611981565b60405180910390f35b34801561033257600080fd5b5061034d6004803603810190610348919061199c565b610f8d565b60405161035a9190611832565b60405180910390f35b34801561036f57600080fd5b5061038a600480360381019061038591906118fc565b611045565b6040516103979190611957565b60405180910390f35b3480156103ac57600080fd5b506103b5611276565b6040516103c29190611832565b60405180910390f35b3480156103d757600080fd5b506103f260048036038101906103ed91906118fc565b611304565b6040516103ff9190611957565b60405180910390f35b34801561041457600080fd5b5061042f600480360381019061042a9190611c18565b61131b565b60405161043c9190611957565b60405180910390f35b61044d6113b4565b005b34801561045b57600080fd5b5061047660048036038101906104719190611c87565b61140c565b6040516104839190611981565b60405180910390f35b34801561049857600080fd5b506104b360048036038101906104ae9190611a53565b611431565b6040516104c09190611981565b60405180910390f35b600080546104d690611cf6565b80601f016020809104026020016040519081016040528092919081815260200182805461050290611cf6565b801561054f5780601f106105245761010080835404028352916020019161054f565b820191906000526020600020905b81548152906001019060200180831161053257829003601f168201915b505050505081565b600081600660003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161067b9190611981565b60405180910390a36001905092915050565b60035481565b6000600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561071e57600080fd5b81600660008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546107aa9190611d56565b925050819055506107bc848484611449565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561082857600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546108779190611d56565b9250508190555081600360008282546108909190611d56565b925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040516108dd9190611981565b60405180910390a260019050919050565b6000600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055503373ffffffffffffffffffffffffffffffffffffffff166108fc60019081150290604051600060405180830381858888f1935050505015801561097a573d6000803e3d6000fd5b5050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff8111156109f5576109f4611aed565b5b6040519080825280601f01601f191660200182016040528015610a275781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610a5f57610a5e611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610ac357610ac2611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610c7d5782600485600c84610b0f9190611db9565b60208110610b2057610b1f611d8a565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610b5f57610b5e611d8a565b5b602001015160f81c60f81b82600283610b789190611ded565b6002610b849190611db9565b81518110610b9557610b94611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c84610bd89190611db9565b60208110610be957610be8611d8a565b5b1a60f81b1660f81c60ff1681518110610c0557610c04611d8a565b5b602001015160f81c60f81b82600283610c1e9190611ded565b6003610c2a9190611db9565b81518110610c3b57610c3a611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610c7590611e2f565b915050610af5565b50809350505050919050565b606060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff811115610ce257610ce1611aed565b5b6040519080825280601f01601f191660200182016040528015610d145781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610d4c57610d4b611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610db057610daf611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610f6a5782600486600c84610dfc9190611db9565b60208110610e0d57610e0c611d8a565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610e4c57610e4b611d8a565b5b602001015160f81c60f81b82600283610e659190611ded565b6002610e719190611db9565b81518110610e8257610e81611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84610ec59190611db9565b60208110610ed657610ed5611d8a565b5b1a60f81b1660f81c60ff1681518110610ef257610ef1611d8a565b5b602001015160f81c60f81b82600283610f0b9190611ded565b6003610f179190611db9565b81518110610f2857610f27611d8a565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610f6290611e2f565b915050610de2565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b90506000600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205460001b90506000610fe982610c89565b90506000610ff63361097e565b90506110018861097e565b61100a8861097e565b61101386610c89565b8385604051602001611029959493929190611f97565b6040516020818303038152906040529450505050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561109357600080fd5b600660008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561111c57600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825461116b9190611d56565b9250508190555081600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111fe9190611d56565b9250508190555081600360008282546112179190611d56565b925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040516112649190611981565b60405180910390a26001905092915050565b6001805461128390611cf6565b80601f01602080910402602001604051908101604052809291908181526020018280546112af90611cf6565b80156112fc5780601f106112d1576101008083540402835291602001916112fc565b820191906000526020600020905b8154815290600101906020018083116112df57829003601f168201915b505050505081565b6000611311338484611449565b6001905092915050565b60008084905061132b8585610557565b156113ab578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b815260040161136f9493929190612072565b600060405180830381600087803b15801561138957600080fd5b505af115801561139d573d6000803e3d6000fd5b5050505060019150506113ad565b505b9392505050565b34600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546114039190611db9565b92505081905550565b6006602052816000526040600020602052806000526040600020600091509150505481565b60056020528060005260406000206000915090505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff160361148257600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205410156114ce57600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115599190611db9565b101561156457600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115f09190611db9565b905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546116419190611d56565b9250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546116979190611db9565b925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040516116fb9190611981565b60405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205461178e9190611db9565b1461179c5761179b6120be565b5b50505050565b600081519050919050565b600082825260208201905092915050565b60005b838110156117dc5780820151818401526020810190506117c1565b60008484015250505050565b6000601f19601f8301169050919050565b6000611804826117a2565b61180e81856117ad565b935061181e8185602086016117be565b611827816117e8565b840191505092915050565b6000602082019050818103600083015261184c81846117f9565b905092915050565b6000604051905090565b600080fd5b600080fd5b600073ffffffffffffffffffffffffffffffffffffffff82169050919050565b600061189382611868565b9050919050565b6118a381611888565b81146118ae57600080fd5b50565b6000813590506118c08161189a565b92915050565b6000819050919050565b6118d9816118c6565b81146118e457600080fd5b50565b6000813590506118f6816118d0565b92915050565b600080604083850312156119135761191261185e565b5b6000611921858286016118b1565b9250506020611932858286016118e7565b9150509250929050565b60008115159050919050565b6119518161193c565b82525050565b600060208201905061196c6000830184611948565b92915050565b61197b816118c6565b82525050565b60006020820190506119966000830184611972565b92915050565b6000806000606084860312156119b5576119b461185e565b5b60006119c3868287016118b1565b93505060206119d4868287016118b1565b92505060406119e5868287016118e7565b9150509250925092565b600060ff82169050919050565b611a05816119ef565b82525050565b6000602082019050611a2060008301846119fc565b92915050565b600060208284031215611a3c57611a3b61185e565b5b6000611a4a848285016118e7565b91505092915050565b600060208284031215611a6957611a6861185e565b5b6000611a77848285016118b1565b91505092915050565b6000819050919050565b611a9381611a80565b8114611a9e57600080fd5b50565b600081359050611ab081611a8a565b92915050565b600060208284031215611acc57611acb61185e565b5b6000611ada84828501611aa1565b91505092915050565b600080fd5b600080fd5b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b611b25826117e8565b810181811067ffffffffffffffff82111715611b4457611b43611aed565b5b80604052505050565b6000611b57611854565b9050611b638282611b1c565b919050565b600067ffffffffffffffff821115611b8357611b82611aed565b5b611b8c826117e8565b9050602081019050919050565b82818337600083830152505050565b6000611bbb611bb684611b68565b611b4d565b905082815260208101848484011115611bd757611bd6611ae8565b5b611be2848285611b99565b509392505050565b600082601f830112611bff57611bfe611ae3565b5b8135611c0f848260208601611ba8565b91505092915050565b600080600060608486031215611c3157611c3061185e565b5b6000611c3f868287016118b1565b9350506020611c50868287016118e7565b925050604084013567ffffffffffffffff811115611c7157611c70611863565b5b611c7d86828701611bea565b9150509250925092565b60008060408385031215611c9e57611c9d61185e565b5b6000611cac858286016118b1565b9250506020611cbd858286016118b1565b9150509250929050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680611d0e57607f821691505b602082108103611d2157611d20611cc7565b5b50919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b6000611d61826118c6565b9150611d6c836118c6565b9250828203905081811115611d8457611d83611d27565b5b92915050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052603260045260246000fd5b6000611dc4826118c6565b9150611dcf836118c6565b9250828201905080821115611de757611de6611d27565b5b92915050565b6000611df8826118c6565b9150611e03836118c6565b9250828202611e11816118c6565b91508282048414831517611e2857611e27611d27565b5b5092915050565b6000611e3a826118c6565b91507fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8203611e6c57611e6b611d27565b5b600182019050919050565b600081905092915050565b6000611e8d826117a2565b611e978185611e77565b9350611ea78185602086016117be565b80840191505092915050565b7f2c20000000000000000000000000000000000000000000000000000000000000600082015250565b6000611ee9600283611e77565b9150611ef482611eb3565b600282019050919050565b7f2c206d73672e73656e6465722000000000000000000000000000000000000000600082015250565b6000611f35600d83611e77565b9150611f4082611eff565b600d82019050919050565b7f2c2062616c616e63654f665b5f66726f6d5d2000000000000000000000000000600082015250565b6000611f81601383611e77565b9150611f8c82611f4b565b601382019050919050565b6000611fa38288611e82565b9150611fae82611edc565b9150611fba8287611e82565b9150611fc582611edc565b9150611fd18286611e82565b9150611fdc82611f28565b9150611fe88285611e82565b9150611ff382611f74565b9150611fff8284611e82565b91508190509695505050505050565b61201781611888565b82525050565b600081519050919050565b600082825260208201905092915050565b60006120448261201d565b61204e8185612028565b935061205e8185602086016117be565b612067816117e8565b840191505092915050565b6000608082019050612087600083018761200e565b6120946020830186611972565b6120a1604083018561200e565b81810360608301526120b38184612039565b905095945050505050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052600160045260246000fdfea2646970667358221220633950a7fce02636af738f400c3d392baf178d3cc6cff8b88b2198937ed3969e64736f6c6343000811003300000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";
string erc20_19112022_withdraw_with_parameter = "0x60806040526012600260006101000a81548160ff021916908360ff16021790555060405162002988380380620029888339818101604052810190620000459190620002ba565b600260009054906101000a900460ff1660ff16600a620000669190620004d7565b8362000073919062000528565b600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009081620000d09190620007b4565b508060019081620000e29190620007b4565b505050506200089b565b6000604051905090565b600080fd5b600080fd5b6000819050919050565b620001158162000100565b81146200012157600080fd5b50565b60008151905062000135816200010a565b92915050565b600080fd5b600080fd5b6000601f19601f8301169050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b620001908262000145565b810181811067ffffffffffffffff82111715620001b257620001b162000156565b5b80604052505050565b6000620001c7620000ec565b9050620001d5828262000185565b919050565b600067ffffffffffffffff821115620001f857620001f762000156565b5b620002038262000145565b9050602081019050919050565b60005b838110156200023057808201518184015260208101905062000213565b60008484015250505050565b6000620002536200024d84620001da565b620001bb565b90508281526020810184848401111562000272576200027162000140565b5b6200027f84828562000210565b509392505050565b600082601f8301126200029f576200029e6200013b565b5b8151620002b18482602086016200023c565b91505092915050565b600080600060608486031215620002d657620002d5620000f6565b5b6000620002e68682870162000124565b935050602084015167ffffffffffffffff8111156200030a5762000309620000fb565b5b620003188682870162000287565b925050604084015167ffffffffffffffff8111156200033c576200033b620000fb565b5b6200034a8682870162000287565b9150509250925092565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b60008160011c9050919050565b6000808291508390505b6001851115620003e257808604811115620003ba57620003b962000354565b5b6001851615620003ca5780820291505b8081029050620003da8562000383565b94506200039a565b94509492505050565b600082620003fd5760019050620004d0565b816200040d5760009050620004d0565b8160018114620004265760028114620004315762000467565b6001915050620004d0565b60ff84111562000446576200044562000354565b5b8360020a91508482111562000460576200045f62000354565b5b50620004d0565b5060208310610133831016604e8410600b8410161715620004a15782820a9050838111156200049b576200049a62000354565b5b620004d0565b620004b0848484600162000390565b92509050818404811115620004ca57620004c962000354565b5b81810290505b9392505050565b6000620004e48262000100565b9150620004f18362000100565b9250620005207fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8484620003eb565b905092915050565b6000620005358262000100565b9150620005428362000100565b9250828202620005528162000100565b915082820484148315176200056c576200056b62000354565b5b5092915050565b600081519050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680620005c657607f821691505b602082108103620005dc57620005db6200057e565b5b50919050565b60008190508160005260206000209050919050565b60006020601f8301049050919050565b600082821b905092915050565b600060088302620006467fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8262000607565b62000652868362000607565b95508019841693508086168417925050509392505050565b6000819050919050565b6000620006956200068f620006898462000100565b6200066a565b62000100565b9050919050565b6000819050919050565b620006b18362000674565b620006c9620006c0826200069c565b84845462000614565b825550505050565b600090565b620006e0620006d1565b620006ed818484620006a6565b505050565b5b81811015620007155762000709600082620006d6565b600181019050620006f3565b5050565b601f82111562000764576200072e81620005e2565b6200073984620005f7565b8101602085101562000749578190505b620007616200075885620005f7565b830182620006f2565b50505b505050565b600082821c905092915050565b6000620007896000198460080262000769565b1980831691505092915050565b6000620007a4838362000776565b9150826002028217905092915050565b620007bf8262000573565b67ffffffffffffffff811115620007db57620007da62000156565b5b620007e78254620005ad565b620007f482828562000719565b600060209050601f8311600181146200082c576000841562000817578287015190505b62000823858262000796565b86555062000893565b601f1984166200083c86620005e2565b60005b8281101562000866578489015182556001820191506020850194506020810190506200083f565b8683101562000886578489015162000882601f89168262000776565b8355505b6001600288020188555050505b505050505050565b6120dd80620008ab6000396000f3fe6080604052600436106101095760003560e01c806370a0823111610095578063a9059cbb11610064578063a9059cbb146103cb578063cae9ca5114610408578063d0e30db014610445578063dd62ed3e1461044f578063f665b8d81461048c57610109565b806370a08231146102e9578063745463631461032657806379cc67901461036357806395d89b41146103a057610109565b8063313ce567116100dc578063313ce567146101de57806342966c6814610209578063486556ce146102465780635e57966d1461026f5780636eef7a05146102ac57610109565b806306fdde031461010e578063095ea7b31461013957806318160ddd1461017657806323b872dd146101a1575b600080fd5b34801561011a57600080fd5b506101236104c9565b60405161013091906117ec565b60405180910390f35b34801561014557600080fd5b50610160600480360381019061015b91906118b6565b610557565b60405161016d9190611911565b60405180910390f35b34801561018257600080fd5b5061018b61068d565b604051610198919061193b565b60405180910390f35b3480156101ad57600080fd5b506101c860048036038101906101c39190611956565b610693565b6040516101d59190611911565b60405180910390f35b3480156101ea57600080fd5b506101f36107c7565b60405161020091906119c5565b60405180910390f35b34801561021557600080fd5b50610230600480360381019061022b91906119e0565b6107da565b60405161023d9190611911565b60405180910390f35b34801561025257600080fd5b5061026d600480360381019061026891906119e0565b6108ee565b005b34801561027b57600080fd5b5061029660048036038101906102919190611a0d565b610938565b6040516102a391906117ec565b60405180910390f35b3480156102b857600080fd5b506102d360048036038101906102ce9190611a70565b610c43565b6040516102e091906117ec565b60405180910390f35b3480156102f557600080fd5b50610310600480360381019061030b9190611a0d565b610f2f565b60405161031d919061193b565b60405180910390f35b34801561033257600080fd5b5061034d60048036038101906103489190611956565b610f47565b60405161035a91906117ec565b60405180910390f35b34801561036f57600080fd5b5061038a600480360381019061038591906118b6565b610fff565b6040516103979190611911565b60405180910390f35b3480156103ac57600080fd5b506103b5611230565b6040516103c291906117ec565b60405180910390f35b3480156103d757600080fd5b506103f260048036038101906103ed91906118b6565b6112be565b6040516103ff9190611911565b60405180910390f35b34801561041457600080fd5b5061042f600480360381019061042a9190611bd2565b6112d5565b60405161043c9190611911565b60405180910390f35b61044d61136e565b005b34801561045b57600080fd5b5061047660048036038101906104719190611c41565b6113c6565b604051610483919061193b565b60405180910390f35b34801561049857600080fd5b506104b360048036038101906104ae9190611a0d565b6113eb565b6040516104c0919061193b565b60405180910390f35b600080546104d690611cb0565b80601f016020809104026020016040519081016040528092919081815260200182805461050290611cb0565b801561054f5780601f106105245761010080835404028352916020019161054f565b820191906000526020600020905b81548152906001019060200180831161053257829003601f168201915b505050505081565b600081600660003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161067b919061193b565b60405180910390a36001905092915050565b60035481565b6000600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561071e57600080fd5b81600660008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546107aa9190611d10565b925050819055506107bc848484611403565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561082857600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546108779190611d10565b9250508190555081600360008282546108909190611d10565b925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040516108dd919061193b565b60405180910390a260019050919050565b3373ffffffffffffffffffffffffffffffffffffffff166108fc829081150290604051600060405180830381858888f19350505050158015610934573d6000803e3d6000fd5b5050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff8111156109af576109ae611aa7565b5b6040519080825280601f01601f1916602001820160405280156109e15781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610a1957610a18611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610a7d57610a7c611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610c375782600485600c84610ac99190611d73565b60208110610ada57610ad9611d44565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610b1957610b18611d44565b5b602001015160f81c60f81b82600283610b329190611da7565b6002610b3e9190611d73565b81518110610b4f57610b4e611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c84610b929190611d73565b60208110610ba357610ba2611d44565b5b1a60f81b1660f81c60ff1681518110610bbf57610bbe611d44565b5b602001015160f81c60f81b82600283610bd89190611da7565b6003610be49190611d73565b81518110610bf557610bf4611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610c2f90611de9565b915050610aaf565b50809350505050919050565b606060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff811115610c9c57610c9b611aa7565b5b6040519080825280601f01601f191660200182016040528015610cce5781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610d0657610d05611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d6a57610d69611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610f245782600486600c84610db69190611d73565b60208110610dc757610dc6611d44565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610e0657610e05611d44565b5b602001015160f81c60f81b82600283610e1f9190611da7565b6002610e2b9190611d73565b81518110610e3c57610e3b611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84610e7f9190611d73565b60208110610e9057610e8f611d44565b5b1a60f81b1660f81c60ff1681518110610eac57610eab611d44565b5b602001015160f81c60f81b82600283610ec59190611da7565b6003610ed19190611d73565b81518110610ee257610ee1611d44565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610f1c90611de9565b915050610d9c565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b90506000600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205460001b90506000610fa382610c43565b90506000610fb033610938565b9050610fbb88610938565b610fc488610938565b610fcd86610c43565b8385604051602001610fe3959493929190611f51565b6040516020818303038152906040529450505050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561104d57600080fd5b600660008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548211156110d657600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111259190611d10565b9250508190555081600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111b89190611d10565b9250508190555081600360008282546111d19190611d10565b925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca58360405161121e919061193b565b60405180910390a26001905092915050565b6001805461123d90611cb0565b80601f016020809104026020016040519081016040528092919081815260200182805461126990611cb0565b80156112b65780601f1061128b576101008083540402835291602001916112b6565b820191906000526020600020905b81548152906001019060200180831161129957829003601f168201915b505050505081565b60006112cb338484611403565b6001905092915050565b6000808490506112e58585610557565b15611365578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401611329949392919061202c565b600060405180830381600087803b15801561134357600080fd5b505af1158015611357573d6000803e3d6000fd5b505050506001915050611367565b505b9392505050565b34600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546113bd9190611d73565b92505081905550565b6006602052816000526040600020602052806000526040600020600091509150505481565b60056020528060005260406000206000915090505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff160361143c57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561148857600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115139190611d73565b101561151e57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115aa9190611d73565b905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546115fb9190611d10565b9250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546116519190611d73565b925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040516116b5919061193b565b60405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546117489190611d73565b1461175657611755612078565b5b50505050565b600081519050919050565b600082825260208201905092915050565b60005b8381101561179657808201518184015260208101905061177b565b60008484015250505050565b6000601f19601f8301169050919050565b60006117be8261175c565b6117c88185611767565b93506117d8818560208601611778565b6117e1816117a2565b840191505092915050565b6000602082019050818103600083015261180681846117b3565b905092915050565b6000604051905090565b600080fd5b600080fd5b600073ffffffffffffffffffffffffffffffffffffffff82169050919050565b600061184d82611822565b9050919050565b61185d81611842565b811461186857600080fd5b50565b60008135905061187a81611854565b92915050565b6000819050919050565b61189381611880565b811461189e57600080fd5b50565b6000813590506118b08161188a565b92915050565b600080604083850312156118cd576118cc611818565b5b60006118db8582860161186b565b92505060206118ec858286016118a1565b9150509250929050565b60008115159050919050565b61190b816118f6565b82525050565b60006020820190506119266000830184611902565b92915050565b61193581611880565b82525050565b6000602082019050611950600083018461192c565b92915050565b60008060006060848603121561196f5761196e611818565b5b600061197d8682870161186b565b935050602061198e8682870161186b565b925050604061199f868287016118a1565b9150509250925092565b600060ff82169050919050565b6119bf816119a9565b82525050565b60006020820190506119da60008301846119b6565b92915050565b6000602082840312156119f6576119f5611818565b5b6000611a04848285016118a1565b91505092915050565b600060208284031215611a2357611a22611818565b5b6000611a318482850161186b565b91505092915050565b6000819050919050565b611a4d81611a3a565b8114611a5857600080fd5b50565b600081359050611a6a81611a44565b92915050565b600060208284031215611a8657611a85611818565b5b6000611a9484828501611a5b565b91505092915050565b600080fd5b600080fd5b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b611adf826117a2565b810181811067ffffffffffffffff82111715611afe57611afd611aa7565b5b80604052505050565b6000611b1161180e565b9050611b1d8282611ad6565b919050565b600067ffffffffffffffff821115611b3d57611b3c611aa7565b5b611b46826117a2565b9050602081019050919050565b82818337600083830152505050565b6000611b75611b7084611b22565b611b07565b905082815260208101848484011115611b9157611b90611aa2565b5b611b9c848285611b53565b509392505050565b600082601f830112611bb957611bb8611a9d565b5b8135611bc9848260208601611b62565b91505092915050565b600080600060608486031215611beb57611bea611818565b5b6000611bf98682870161186b565b9350506020611c0a868287016118a1565b925050604084013567ffffffffffffffff811115611c2b57611c2a61181d565b5b611c3786828701611ba4565b9150509250925092565b60008060408385031215611c5857611c57611818565b5b6000611c668582860161186b565b9250506020611c778582860161186b565b9150509250929050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680611cc857607f821691505b602082108103611cdb57611cda611c81565b5b50919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b6000611d1b82611880565b9150611d2683611880565b9250828203905081811115611d3e57611d3d611ce1565b5b92915050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052603260045260246000fd5b6000611d7e82611880565b9150611d8983611880565b9250828201905080821115611da157611da0611ce1565b5b92915050565b6000611db282611880565b9150611dbd83611880565b9250828202611dcb81611880565b91508282048414831517611de257611de1611ce1565b5b5092915050565b6000611df482611880565b91507fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8203611e2657611e25611ce1565b5b600182019050919050565b600081905092915050565b6000611e478261175c565b611e518185611e31565b9350611e61818560208601611778565b80840191505092915050565b7f2c20000000000000000000000000000000000000000000000000000000000000600082015250565b6000611ea3600283611e31565b9150611eae82611e6d565b600282019050919050565b7f2c206d73672e73656e6465722000000000000000000000000000000000000000600082015250565b6000611eef600d83611e31565b9150611efa82611eb9565b600d82019050919050565b7f2c2062616c616e63654f665b5f66726f6d5d2000000000000000000000000000600082015250565b6000611f3b601383611e31565b9150611f4682611f05565b601382019050919050565b6000611f5d8288611e3c565b9150611f6882611e96565b9150611f748287611e3c565b9150611f7f82611e96565b9150611f8b8286611e3c565b9150611f9682611ee2565b9150611fa28285611e3c565b9150611fad82611f2e565b9150611fb98284611e3c565b91508190509695505050505050565b611fd181611842565b82525050565b600081519050919050565b600082825260208201905092915050565b6000611ffe82611fd7565b6120088185611fe2565b9350612018818560208601611778565b612021816117a2565b840191505092915050565b60006080820190506120416000830187611fc8565b61204e602083018661192c565b61205b6040830185611fc8565b818103606083015261206d8184611ff3565b905095945050505050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052600160045260246000fdfea26469706673582212206c3ad942588438fb94141288b441ad0e00dcf2d1e6d7110d108efbb416a1856964736f6c6343000811003300000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";
string erc20_22112022_withdraw_with_parameter_subtract_15 = "0x60806040526012600260006101000a81548160ff021916908360ff16021790555060405162002997380380620029978339818101604052810190620000459190620002ba565b600260009054906101000a900460ff1660ff16600a620000669190620004d7565b8362000073919062000528565b600381905550600354600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508160009081620000d09190620007b4565b508060019081620000e29190620007b4565b505050506200089b565b6000604051905090565b600080fd5b600080fd5b6000819050919050565b620001158162000100565b81146200012157600080fd5b50565b60008151905062000135816200010a565b92915050565b600080fd5b600080fd5b6000601f19601f8301169050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b620001908262000145565b810181811067ffffffffffffffff82111715620001b257620001b162000156565b5b80604052505050565b6000620001c7620000ec565b9050620001d5828262000185565b919050565b600067ffffffffffffffff821115620001f857620001f762000156565b5b620002038262000145565b9050602081019050919050565b60005b838110156200023057808201518184015260208101905062000213565b60008484015250505050565b6000620002536200024d84620001da565b620001bb565b90508281526020810184848401111562000272576200027162000140565b5b6200027f84828562000210565b509392505050565b600082601f8301126200029f576200029e6200013b565b5b8151620002b18482602086016200023c565b91505092915050565b600080600060608486031215620002d657620002d5620000f6565b5b6000620002e68682870162000124565b935050602084015167ffffffffffffffff8111156200030a5762000309620000fb565b5b620003188682870162000287565b925050604084015167ffffffffffffffff8111156200033c576200033b620000fb565b5b6200034a8682870162000287565b9150509250925092565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b60008160011c9050919050565b6000808291508390505b6001851115620003e257808604811115620003ba57620003b962000354565b5b6001851615620003ca5780820291505b8081029050620003da8562000383565b94506200039a565b94509492505050565b600082620003fd5760019050620004d0565b816200040d5760009050620004d0565b8160018114620004265760028114620004315762000467565b6001915050620004d0565b60ff84111562000446576200044562000354565b5b8360020a91508482111562000460576200045f62000354565b5b50620004d0565b5060208310610133831016604e8410600b8410161715620004a15782820a9050838111156200049b576200049a62000354565b5b620004d0565b620004b0848484600162000390565b92509050818404811115620004ca57620004c962000354565b5b81810290505b9392505050565b6000620004e48262000100565b9150620004f18362000100565b9250620005207fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8484620003eb565b905092915050565b6000620005358262000100565b9150620005428362000100565b9250828202620005528162000100565b915082820484148315176200056c576200056b62000354565b5b5092915050565b600081519050919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680620005c657607f821691505b602082108103620005dc57620005db6200057e565b5b50919050565b60008190508160005260206000209050919050565b60006020601f8301049050919050565b600082821b905092915050565b600060088302620006467fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8262000607565b62000652868362000607565b95508019841693508086168417925050509392505050565b6000819050919050565b6000620006956200068f620006898462000100565b6200066a565b62000100565b9050919050565b6000819050919050565b620006b18362000674565b620006c9620006c0826200069c565b84845462000614565b825550505050565b600090565b620006e0620006d1565b620006ed818484620006a6565b505050565b5b81811015620007155762000709600082620006d6565b600181019050620006f3565b5050565b601f82111562000764576200072e81620005e2565b6200073984620005f7565b8101602085101562000749578190505b620007616200075885620005f7565b830182620006f2565b50505b505050565b600082821c905092915050565b6000620007896000198460080262000769565b1980831691505092915050565b6000620007a4838362000776565b9150826002028217905092915050565b620007bf8262000573565b67ffffffffffffffff811115620007db57620007da62000156565b5b620007e78254620005ad565b620007f482828562000719565b600060209050601f8311600181146200082c576000841562000817578287015190505b62000823858262000796565b86555062000893565b601f1984166200083c86620005e2565b60005b8281101562000866578489015182556001820191506020850194506020810190506200083f565b8683101562000886578489015162000882601f89168262000776565b8355505b6001600288020188555050505b505050505050565b6120ec80620008ab6000396000f3fe6080604052600436106101095760003560e01c806370a0823111610095578063a9059cbb11610064578063a9059cbb146103cb578063cae9ca5114610408578063d0e30db014610445578063dd62ed3e1461044f578063f665b8d81461048c57610109565b806370a08231146102e9578063745463631461032657806379cc67901461036357806395d89b41146103a057610109565b8063313ce567116100dc578063313ce567146101de57806342966c6814610209578063486556ce146102465780635e57966d1461026f5780636eef7a05146102ac57610109565b806306fdde031461010e578063095ea7b31461013957806318160ddd1461017657806323b872dd146101a1575b600080fd5b34801561011a57600080fd5b506101236104c9565b60405161013091906117fb565b60405180910390f35b34801561014557600080fd5b50610160600480360381019061015b91906118c5565b610557565b60405161016d9190611920565b60405180910390f35b34801561018257600080fd5b5061018b61068d565b604051610198919061194a565b60405180910390f35b3480156101ad57600080fd5b506101c860048036038101906101c39190611965565b610693565b6040516101d59190611920565b60405180910390f35b3480156101ea57600080fd5b506101f36107c7565b60405161020091906119d4565b60405180910390f35b34801561021557600080fd5b50610230600480360381019061022b91906119ef565b6107da565b60405161023d9190611920565b60405180910390f35b34801561025257600080fd5b5061026d600480360381019061026891906119ef565b6108ee565b005b34801561027b57600080fd5b5061029660048036038101906102919190611a1c565b610947565b6040516102a391906117fb565b60405180910390f35b3480156102b857600080fd5b506102d360048036038101906102ce9190611a7f565b610c52565b6040516102e091906117fb565b60405180910390f35b3480156102f557600080fd5b50610310600480360381019061030b9190611a1c565b610f3e565b60405161031d919061194a565b60405180910390f35b34801561033257600080fd5b5061034d60048036038101906103489190611965565b610f56565b60405161035a91906117fb565b60405180910390f35b34801561036f57600080fd5b5061038a600480360381019061038591906118c5565b61100e565b6040516103979190611920565b60405180910390f35b3480156103ac57600080fd5b506103b561123f565b6040516103c291906117fb565b60405180910390f35b3480156103d757600080fd5b506103f260048036038101906103ed91906118c5565b6112cd565b6040516103ff9190611920565b60405180910390f35b34801561041457600080fd5b5061042f600480360381019061042a9190611be1565b6112e4565b60405161043c9190611920565b60405180910390f35b61044d61137d565b005b34801561045b57600080fd5b5061047660048036038101906104719190611c50565b6113d5565b604051610483919061194a565b60405180910390f35b34801561049857600080fd5b506104b360048036038101906104ae9190611a1c565b6113fa565b6040516104c0919061194a565b60405180910390f35b600080546104d690611cbf565b80601f016020809104026020016040519081016040528092919081815260200182805461050290611cbf565b801561054f5780601f106105245761010080835404028352916020019161054f565b820191906000526020600020905b81548152906001019060200180831161053257829003601f168201915b505050505081565b600081600660003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000208190555081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161067b919061194a565b60405180910390a36001905092915050565b60035481565b6000600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205482111561071e57600080fd5b81600660008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546107aa9190611d1f565b925050819055506107bc848484611412565b600190509392505050565b600260009054906101000a900460ff1681565b600081600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561082857600080fd5b81600460003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546108779190611d1f565b9250508190555081600360008282546108909190611d1f565b925050819055503373ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca5836040516108dd919061194a565b60405180910390a260019050919050565b600f816108fb9190611d1f565b90503373ffffffffffffffffffffffffffffffffffffffff166108fc829081150290604051600060405180830381858888f19350505050158015610943573d6000803e3d6000fd5b5050565b606060008273ffffffffffffffffffffffffffffffffffffffff1660001b905060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff8111156109be576109bd611ab6565b5b6040519080825280601f01601f1916602001820160405280156109f05781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610a2857610a27611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610a8c57610a8b611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610c465782600485600c84610ad89190611d82565b60208110610ae957610ae8611d53565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610b2857610b27611d53565b5b602001015160f81c60f81b82600283610b419190611db6565b6002610b4d9190611d82565b81518110610b5e57610b5d611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b85600c84610ba19190611d82565b60208110610bb257610bb1611d53565b5b1a60f81b1660f81c60ff1681518110610bce57610bcd611d53565b5b602001015160f81c60f81b82600283610be79190611db6565b6003610bf39190611d82565b81518110610c0457610c03611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610c3e90611df8565b915050610abe565b50809350505050919050565b606060006040518060400160405280601081526020017f303132333435363738396162636465660000000000000000000000000000000081525090506000603367ffffffffffffffff811115610cab57610caa611ab6565b5b6040519080825280601f01601f191660200182016040528015610cdd5781602001600182028036833780820191505090505b5090507f300000000000000000000000000000000000000000000000000000000000000081600081518110610d1557610d14611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053507f780000000000000000000000000000000000000000000000000000000000000081600181518110610d7957610d78611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535060005b6014811015610f335782600486600c84610dc59190611d82565b60208110610dd657610dd5611d53565b5b1a60f81b7effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916901c60f81c60ff1681518110610e1557610e14611d53565b5b602001015160f81c60f81b82600283610e2e9190611db6565b6002610e3a9190611d82565b81518110610e4b57610e4a611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a90535082600f60f81b86600c84610e8e9190611d82565b60208110610e9f57610e9e611d53565b5b1a60f81b1660f81c60ff1681518110610ebb57610eba611d53565b5b602001015160f81c60f81b82600283610ed49190611db6565b6003610ee09190611d82565b81518110610ef157610ef0611d53565b5b60200101907effffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1916908160001a9053508080610f2b90611df8565b915050610dab565b508092505050919050565b60046020528060005260406000206000915090505481565b606060008260001b90506000600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205460001b90506000610fb282610c52565b90506000610fbf33610947565b9050610fca88610947565b610fd388610947565b610fdc86610c52565b8385604051602001610ff2959493929190611f60565b6040516020818303038152906040529450505050509392505050565b600081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561105c57600080fd5b600660008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020548211156110e557600080fd5b81600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111349190611d1f565b9250508190555081600660008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546111c79190611d1f565b9250508190555081600360008282546111e09190611d1f565b925050819055508273ffffffffffffffffffffffffffffffffffffffff167fcc16f5dbb4873280815c1ee09dbd06736cffcc184412cf7a71a0fdb75d397ca58360405161122d919061194a565b60405180910390a26001905092915050565b6001805461124c90611cbf565b80601f016020809104026020016040519081016040528092919081815260200182805461127890611cbf565b80156112c55780601f1061129a576101008083540402835291602001916112c5565b820191906000526020600020905b8154815290600101906020018083116112a857829003601f168201915b505050505081565b60006112da338484611412565b6001905092915050565b6000808490506112f48585610557565b15611374578073ffffffffffffffffffffffffffffffffffffffff16638f4ffcb1338630876040518563ffffffff1660e01b8152600401611338949392919061203b565b600060405180830381600087803b15801561135257600080fd5b505af1158015611366573d6000803e3d6000fd5b505050506001915050611376565b505b9392505050565b34600560003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546113cc9190611d82565b92505081905550565b6006602052816000526040600020602052806000526040600020600091509150505481565b60056020528060005260406000206000915090505481565b600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff160361144b57600080fd5b80600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054101561149757600080fd5b600460008373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000205481600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115229190611d82565b101561152d57600080fd5b6000600460008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546115b99190611d82565b905081600460008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825461160a9190611d1f565b9250508190555081600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282546116609190611d82565b925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040516116c4919061194a565b60405180910390a380600460008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002054600460008773ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020546117579190611d82565b1461176557611764612087565b5b50505050565b600081519050919050565b600082825260208201905092915050565b60005b838110156117a557808201518184015260208101905061178a565b60008484015250505050565b6000601f19601f8301169050919050565b60006117cd8261176b565b6117d78185611776565b93506117e7818560208601611787565b6117f0816117b1565b840191505092915050565b6000602082019050818103600083015261181581846117c2565b905092915050565b6000604051905090565b600080fd5b600080fd5b600073ffffffffffffffffffffffffffffffffffffffff82169050919050565b600061185c82611831565b9050919050565b61186c81611851565b811461187757600080fd5b50565b60008135905061188981611863565b92915050565b6000819050919050565b6118a28161188f565b81146118ad57600080fd5b50565b6000813590506118bf81611899565b92915050565b600080604083850312156118dc576118db611827565b5b60006118ea8582860161187a565b92505060206118fb858286016118b0565b9150509250929050565b60008115159050919050565b61191a81611905565b82525050565b60006020820190506119356000830184611911565b92915050565b6119448161188f565b82525050565b600060208201905061195f600083018461193b565b92915050565b60008060006060848603121561197e5761197d611827565b5b600061198c8682870161187a565b935050602061199d8682870161187a565b92505060406119ae868287016118b0565b9150509250925092565b600060ff82169050919050565b6119ce816119b8565b82525050565b60006020820190506119e960008301846119c5565b92915050565b600060208284031215611a0557611a04611827565b5b6000611a13848285016118b0565b91505092915050565b600060208284031215611a3257611a31611827565b5b6000611a408482850161187a565b91505092915050565b6000819050919050565b611a5c81611a49565b8114611a6757600080fd5b50565b600081359050611a7981611a53565b92915050565b600060208284031215611a9557611a94611827565b5b6000611aa384828501611a6a565b91505092915050565b600080fd5b600080fd5b7f4e487b7100000000000000000000000000000000000000000000000000000000600052604160045260246000fd5b611aee826117b1565b810181811067ffffffffffffffff82111715611b0d57611b0c611ab6565b5b80604052505050565b6000611b2061181d565b9050611b2c8282611ae5565b919050565b600067ffffffffffffffff821115611b4c57611b4b611ab6565b5b611b55826117b1565b9050602081019050919050565b82818337600083830152505050565b6000611b84611b7f84611b31565b611b16565b905082815260208101848484011115611ba057611b9f611ab1565b5b611bab848285611b62565b509392505050565b600082601f830112611bc857611bc7611aac565b5b8135611bd8848260208601611b71565b91505092915050565b600080600060608486031215611bfa57611bf9611827565b5b6000611c088682870161187a565b9350506020611c19868287016118b0565b925050604084013567ffffffffffffffff811115611c3a57611c3961182c565b5b611c4686828701611bb3565b9150509250925092565b60008060408385031215611c6757611c66611827565b5b6000611c758582860161187a565b9250506020611c868582860161187a565b9150509250929050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052602260045260246000fd5b60006002820490506001821680611cd757607f821691505b602082108103611cea57611ce9611c90565b5b50919050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052601160045260246000fd5b6000611d2a8261188f565b9150611d358361188f565b9250828203905081811115611d4d57611d4c611cf0565b5b92915050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052603260045260246000fd5b6000611d8d8261188f565b9150611d988361188f565b9250828201905080821115611db057611daf611cf0565b5b92915050565b6000611dc18261188f565b9150611dcc8361188f565b9250828202611dda8161188f565b91508282048414831517611df157611df0611cf0565b5b5092915050565b6000611e038261188f565b91507fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8203611e3557611e34611cf0565b5b600182019050919050565b600081905092915050565b6000611e568261176b565b611e608185611e40565b9350611e70818560208601611787565b80840191505092915050565b7f2c20000000000000000000000000000000000000000000000000000000000000600082015250565b6000611eb2600283611e40565b9150611ebd82611e7c565b600282019050919050565b7f2c206d73672e73656e6465722000000000000000000000000000000000000000600082015250565b6000611efe600d83611e40565b9150611f0982611ec8565b600d82019050919050565b7f2c2062616c616e63654f665b5f66726f6d5d2000000000000000000000000000600082015250565b6000611f4a601383611e40565b9150611f5582611f14565b601382019050919050565b6000611f6c8288611e4b565b9150611f7782611ea5565b9150611f838287611e4b565b9150611f8e82611ea5565b9150611f9a8286611e4b565b9150611fa582611ef1565b9150611fb18285611e4b565b9150611fbc82611f3d565b9150611fc88284611e4b565b91508190509695505050505050565b611fe081611851565b82525050565b600081519050919050565b600082825260208201905092915050565b600061200d82611fe6565b6120178185611ff1565b9350612027818560208601611787565b612030816117b1565b840191505092915050565b60006080820190506120506000830187611fd7565b61205d602083018661193b565b61206a6040830185611fd7565b818103606083015261207c8184612002565b905095945050505050565b7f4e487b7100000000000000000000000000000000000000000000000000000000600052600160045260246000fdfea26469706673582212209df52aa99bc435db58f014a9bc16704c150672cdc4c01c997e89f93a9e9eb6fa64736f6c6343000811003300000000000000000000000000000000000000000000000000000000000003e8000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001610000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000016200000000000000000000000000000000000000000000000000000000000000";
Value Reflect::integratedTest1(const Array& params, bool fHelp)
{
  if(fHelp || params.size() > 0)
    throw runtime_error(
      "integratedTest1 no args"
      + HelpRequiringPassphrase());

  vector<Value> res;
  constexpr auto create_address = 0xc9ea7ed000000000000000000000000000000001_address;
  constexpr auto test_sender_address = 0x5b38da6a701c568545dcfcb03fcb875f56beddc4_address;
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
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x18160ddd");
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest1 : Result: totalSupply  " << result.status_code << "\nGas used: " << track_used << "\n";

    if (result.status_code == DVMC_SUCCESS)
      std::cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
  }
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x42966c68000000000000000000000000000000000000000000000000000000000000029a");
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
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x18160ddd");
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
  {
    const auto input = dvmc::from_hex("0x095ea7b30000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000050");
    msg.sender = test_sender_address;
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";
  }
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4");
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";
  }
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x23b872dd0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000000000000000000000000000000000000000000037");
    msg.track = std::numeric_limits<int64_t>::max();
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest1 : Result: transfer_from  " << result.status_code << "\nGas used: " << track_used << "\n";
  }
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0xdd62ed3e0000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc40000000000000000000000005b38da6a701c568545dcfcb03fcb875f56beddc4");
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
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x486556ce0000000000000000000000000000000000000000000000000000000000031415");
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
    unsigned char test_bytes[] =
    {
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
      unsigned int b1 = test_bytes[i*4];
      b1<<=24;
      unsigned int b2 = test_bytes[i*4+1];
      b2<<=16;
      unsigned int b3 = test_bytes[i*4+2];
      b3<<=8;
      unsigned int b4 = test_bytes[i*4+3];
      b4<<=0;
      unsigned int p_i = b1 + b2 + b3 + b4;
    }

    msg_test_bytes +=1;
    const auto track_used = msg.track - result.track_left;
  }
  dev::OverlayDB* overlayDB__ = new dev::OverlayDB();
  dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(overlayDB__);
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
        {
          key.push_back(storage_key[i]);
        }

        dev::h256 key256(key);
        auto storage_bytes = pair.second.value;
        dev::bytes val;
        std::cout << "preparing insert dev::bytes val in trie, val.size " << val.size() << std::endl;

        for(int i=0; i<32; i++)
        {
          val.push_back(storage_bytes.bytes[i]);
        }

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
    overlayDB__->commit();
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
  std::cout << "========================================================================" << std::endl;
  {
    string account_str = state.at(tmpAddr);
    std::cout << "integratedTest1 : RETRIEVE : account_str.size " << account_str.size() << std::endl;
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
      map<dev::h256, pair<dev::bytes, dev::bytes>> ret_;
      ret_.clear();
      std::cout << "created ret_.size " << ret_.size() << std::endl;

      for(auto p : ret_)
      {
        std::cout << "after creation ret_ entry" << std::endl;
      }

      {
        {
		dev::OverlayDB* overlayDB__ = new dev::OverlayDB();
          dev::SecureTrieDB<dev::h256, dev::OverlayDB> memdb(const_cast<dev::OverlayDB*>(overlayDB__), tmpAccstorageRoot);

          for (auto it = memdb.hashedBegin(); it != memdb.hashedEnd(); ++it)
          {
            dev::h256 const hashedKey((*it).first);
            auto const key = it.key();
            dev::bytes const value = dev::RLP((*it).second).toBytes();
            std::cout << "integratedTest 1 set ret_ hash map key value" << std::endl;
            ret_[hashedKey] = make_pair(key, value);
          }
        }
        std::cout << "integratedTest 1 map ret_.size() " << ret_.size() << std::endl;
        std::cout << "after first setting entries ret_.size " << ret_.size() << std::endl;

        for(auto p : ret_)
        {
          std::cout << "after setting ret_ entry" << std::endl;
        }

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
            {
              std::cout << "found matching key bytes" << std::endl;
            }
          }
        }

        std::cout << "integratedTest 1 map ret_.size() " << ret_.size() << std::endl;
        std::cout << "prior to val scan entries ret_.size " << ret_.size() << std::endl;

        for(auto p : ret_)
        {
          std::cout << "prior to val scan ret_ entry" << std::endl;
        }

        std::cout << "================== scan values ================== " << std::endl;
        std::cout << "ret_.size " << ret_.size() << std::endl;
        std::cout << "prior to val scan created_account__.storage.size " << created_account__.storage.size() << std::endl;

        for(auto pair : created_account__.storage)
        {
          auto storage_value = pair.second.value;

          for(auto i_ret_ : ret_)
          {
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
            {
              std::cout << "found matching value bytes" << std::endl;
            }
          }
        }

        std::cout << "================== scan values end ================== " << std::endl;
      }

      dvmc::TransitionalNode account_recon;
      std::unordered_map<dvmc::bytes32, dvmc::storage_value>& storage_recon = account_recon.storage;

      for(auto i_ : ret_)
      {
        dvmc::bytes32 reconKey;
        dev::bytes key = i_.second.first;

        for(int idx=0; idx<32; idx++)
        {
          reconKey.bytes[idx] = key[idx];
        }

        dvmc::storage_value sv;

        for(int idx=0; idx<32; idx++)
        {
          sv.value.bytes[idx] = i_.second.second[idx];
        }

        storage_recon[reconKey] = sv;
      }

      auto& created_account__ = host.accounts[create_address];
      {
        for(auto pair : created_account__.storage)
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

            if(match)
            {
              std::cout << "key match" << std::endl;
            }
          }
        }
      }
      {
        for(auto pair : created_account__.storage)
        {
          dvmc::storage_value sv = account_recon.storage[pair.first];

          if(sv.value == pair.second.value)
          {
            std::cout << "payload match" << std::endl;
          }
          else
          {
            std::cout << "payload does not match" << std::endl;
          }
        }
      }
      account_recon.nonce = created_account__.nonce;
      account_recon.balance = created_account__.balance;
      account_recon.code = created_account__.code;
      created_account__ = account_recon;
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
Value Reflect::integratedTest2(const Array& params, bool fHelp)
{
  if(fHelp || params.size() > 0)
    throw runtime_error(
      "integratedTest2"
      + HelpRequiringPassphrase());
  std::cout << "Entered integratedTest2" << std::endl;
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
  const char* args[] =
  {
    "program",
    "--key1=123",
    "--val1=456"
  };
  po::parsed_options parsed = po::parse_command_line(arg_count, args, desc);
  po::store(parsed, m);
  po::notify(m);

  if(m.count("key1"))
  {
    std::cout << "key1" << std::endl;
  }

  if(m.count("key2"))
  {
    std::cout << "key2" << std::endl;
  }

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
    std::cout << "XXXX constructed rlp" << std::endl;
    auto const nonce_ = rlp_state_[0].toInt<dev::u256>();
    std::cout << "XXXX extracted nonce " << nonce_ << std::endl;
    auto const balance_ = rlp_state_[1].toInt<dev::u256>();
    std::cout << "XXXX extracted balance " << balance << std::endl;
    auto const storageRoot_ = rlp_state_[2].toHash<dev::h256>();
    std::cout << "XXXX extracted storageRoot " << storageRoot_ << std::endl;
    auto const codeHash_ = rlp_state_[3].toHash<dev::h256>();
    std::cout << "XXXX extracted codeHash " << codeHash_ << std::endl;
  }
  std::cout << "commit 0" << std::endl;
  overlay_db.commit();
  std::cout << "commit 1" << std::endl;
  std::cout << "state root after test insert " << state.root() << std::endl;
  std::cout << "tmpAcc inserted - storageRoot " << tmpAcc.baseRoot() << std::endl;
  string account_str = state.at(tmpAddr);

  if(account_str.empty())
  {
    std::cout << "account str empty" << std::endl;
  }
  else
  {
    std::cout << "account str not empty" << std::endl;
  }

  dev::h256 storageRoot__("650276fe18bc32afd3f79fc876c678269635c037e52f178402cfaf1a0c6ea911");
  dev::Address tmpAddr2("9299567890123456789012345678901234567890");
  string account_str2 = state.at(tmpAddr2);

  if(account_str2.empty())
  {
    std::cout << "account str2 empty" << std::endl;
  }
  else
  {
    std::cout << "account str2 not empty" << std::endl;
  }

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
  vector<Value> res;
  return res;
}
Value Reflect::integratedTest3(const Array& params, bool fHelp)
{
  if(fHelp || params.size() > 0)
    throw runtime_error(
      "integratedTest3 no args"
      + HelpRequiringPassphrase());

  std::cout << "Entered integratedTest3" << std::endl;
  std::unique_ptr<dev::db::DatabaseFace> db = dev::db::DBFactory::create("/home/argon/data1/testnet/state");
  dev::h256 root("ac7ed96f9c8ced36cb005661a0083cd2284e1487c578e4ffbfa4f2550fcfd947");
  auto overlay_db = dev::OverlayDB(std::move(db));
  dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(&overlay_db);
  state.setRoot(root);
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
          dev::u256 const value = dev::RLP((*it).second).toInt<dev::u256>();
          ret[hashedKey] = make_pair(key, value);
        }
      }

      for (auto const& i : retrievedAcc.storageOverlay())
      {
        dev::h256 const key = i.first;
        dev::h256 const hashedKey = sha3(key);

        if (i.second)
        {
          ret[hashedKey] = i;
        }
        else
        {
          ret.erase(hashedKey);
        }
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
  vector<Value> res;
  return res;
}
Value Reflect::integratedTest4(const Array& params, bool fHelp)
{
  if(fHelp || params.size() > 0)
    throw runtime_error(
      "integratedTest4 no args"
      + HelpRequiringPassphrase());

  std::cout << "Entered integratedTest4" << std::endl;
  CPubKey newKey_contract_address;

  if(!pwalletMainId->GetKeyFromPool(newKey_contract_address, false))
  {
    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
  }

  CKeyID keyID_contract_address = newKey_contract_address.GetID();
  dvmc::address create_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_contract_address.GetHex().c_str());
  CPubKey newKey_test_sender_address;

  if(!pwalletMainId->GetKeyFromPool(newKey_test_sender_address, false))
  {
    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
  }

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
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x18160ddd");
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest14: Result: totalSupply  " << result.status_code << "\nGas used: " << track_used << "\n";
  }
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x42966c68000000000000000000000000000000000000000000000000000000000000029a");
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest1 : Result: Burn  " << result.status_code << "\nGas used: " << track_used << "\n";
  }
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0x18160ddd");
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
  {
    std::string s = dvmc::hex(test_sender_address.bytes);
    const std::string code_str = "0x095ea7b3000000000000000000000000" + s + "0000000000000000000000000000000000000000000000000000000000000050";
    std::cout << "approve op input code " << code_str << std::endl;
    const auto input = dvmc::from_hex(code_str);
    msg.sender = test_sender_address;
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest4 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";
  }
  {
    std::cout << "\n";
    std::string s = dvmc::hex(test_sender_address.bytes);
    std::string code_str = "0xdd62ed3e000000000000000000000000" + s + "000000000000000000000000" + s;
    std::cout << "print allowance op input code " << code_str << std::endl;
    const auto input = dvmc::from_hex(code_str);
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
  {
    std::cout << "\n";
    std::string s = dvmc::hex(test_sender_address.bytes);
    std::string code_str = "0x23b872dd000000000000000000000000" + s + "000000000000000000000000" + s + "0000000000000000000000000000000000000000000000000000000000000037";
    std::cout << "transfer_from op input code " << code_str << std::endl;
    const auto input = dvmc::from_hex(code_str);
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
  {
    std::cout << "\n";
    std::string s = dvmc::hex(test_sender_address.bytes);
    std::string code_str = "0xdd62ed3e000000000000000000000000" + s + "000000000000000000000000" + s;
    std::cout << "print allowance op input code " << code_str << std::endl;
    const auto input = dvmc::from_hex(code_str);
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest1 : Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

    if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
      std::cout << "integratedTest4 : Output: allowance  " << dvmc::hex({result.output_data, result.output_size}) << "\n";
  }
  vector<Value> res;
  return res;
}
Value Reflect::integratedTest5(const Array& params, bool fHelp)
{
  if(fHelp || params.size() > 0)
    throw runtime_error(
      "integratedTest5 no args"
      + HelpRequiringPassphrase());

  std::cout << "Entered integratedTest5" << std::endl;
  std::vector<CTransaction> testTxVector;
  dev::h256 ROOT("7a8a519e3b0f84ef31cc28b83f4a27e2e376dba75789cecf5252811f2682974f");
  dev::OverlayDB* overlayDB__ = new dev::OverlayDB();
  dev::SecureTrieDB<dev::Address, dev::OverlayDB> state(overlayDB__,ROOT);

  if(state.isNull())
  {
    std::cout << "state db isNull calling init" << std::endl;
    state.init();
  }

  CPubKey newKey_contract_address;
  {
    if(!pwalletMainId->GetKeyFromPool(newKey_contract_address, false))
    {
      throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    }

    CKeyID keyID_contract_address = newKey_contract_address.GetID();
    dvmc::address create_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_contract_address.GetHex().c_str());
    dev::Address target_addr(keyID_contract_address.GetHex().c_str());
    CPubKey newKey_test_sender_address;

    if(!pwalletMainId->GetKeyFromPool(newKey_test_sender_address, false))
    {
      throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    }

    CKeyID keyID_test_sender_address = newKey_test_sender_address.GetID();
    dvmc::address test_sender_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_test_sender_address.GetHex().c_str());
    dvmc::address test_recipient_address = test_sender_address;
    std::cout << "integratedTest5 : initialised create address " << dvmc::hex(create_address.bytes) << "\n";
    std::cout << "integratedTest5 : initialised from address " << dvmc::hex(test_sender_address.bytes) << "\n";
    std::cout << "integratedTest5 : initialised to address " << dvmc::hex(test_recipient_address.bytes) << "\n";
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
        std::cout << "integratedTest5 : Contract creation failed: " << create_result.status_code << "\n";
        return create_result.status_code;
      }

      auto& created_account = host.accounts[create_address];
      created_account.set_balance(100000000000000);
      created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);
      msg.recipient = create_address;
      exec_code = created_account.code;
      std::cout << "CREATE SUCCESS - STORE STATE IN PTRIE ..." << std::endl;
      dev::eth::Account tmpAcc(0,0);
      {
        dev::u256 nonce = 12345678;
        dev::u256 balance = 1010101010101;
        dev::RLPStream s(4);
        s << nonce << balance;
        {
          std::cout << "STORE STATE OF CREATED CONTRACT... 1" << std::endl;
          dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state.db(), tmpAcc.baseRoot());
          std::cout << "STORE STATE OF CREATED CONTRACT... 2" << std::endl;

          for(auto pair : created_account.storage)
          {
            std::cout << "integratedTest5 : store key value" << std::endl;
            auto storage_key = pair.first.bytes;
            dev::bytes key;

            for(int i=0; i<32; i++)
            {
              key.push_back(storage_key[i]);
            }

            dev::h256 key256(key);
            auto storage_bytes = pair.second.value;
            dev::bytes val;

            for(int i=0; i<32; i++)
            {
              val.push_back(storage_bytes.bytes[i]);
            }

            std::cout << "STORE STATE OF CREATED CONTRACT... 3" << std::endl;
            storageDB.insert(key256, dev::rlp(val));
            std::cout << "STORE STATE OF CREATED CONTRACT... 4" << std::endl;
          }

          s << storageDB.root();
        }
        s << tmpAcc.codeHash();
        std::cout << "integratedTest5 BEFORE contract state insert : state root " << state.root() << std::endl;
        std::cout << "STORE STATE OF CREATED CONTRACT... 5" << std::endl;
        state.insert(target_addr, &s.out());
        std::cout << "STORE STATE OF CREATED CONTRACT... 6" << std::endl;
        overlayDB__->commit();
        std::ostringstream os;
        state.debugStructure(os);
        std::cout << "global state traversal " << os.str() << std::endl;
        std::cout << "integratedTest5 AFTER contract state insert : state root " << state.root() << std::endl;
      }
    }
  }
  vector<Value> res;
  return res;
}
Value Reflect::integratedTest6(const Array& params, bool fHelp)
{
  if(fHelp || params.size() > 0)
    throw runtime_error(
      "integratedTest6"
      + HelpRequiringPassphrase());

  vector<Value> res;
  constexpr auto create_address = 0xc9ea7ed000000000000000000000000000000001_address;
  constexpr auto test_sender_address = 0x5b38da6a701c568545dcfcb03fcb875f56beddc4_address;
  constexpr auto test_recipient_address = 0x5b38da6a701c568545dcfcb03fcb875f56beddc4_address;
  std::cout << "integratedTest6 : initialised create address " << dvmc::hex(create_address.bytes) << "\n";
  std::cout << "integratedTest6 : initialised from address " << dvmc::hex(test_sender_address.bytes) << "\n";
  std::cout << "integratedTest6 : initialised to address " << dvmc::hex(test_recipient_address.bytes) << "\n";
  const auto code = dvmc::from_hex("0x608060405234801561001057600080fd5b5061013f806100206000396000f300608060405260043610610041576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff168063f15fad2314610046575b600080fd5b34801561005257600080fd5b5061005b6100d6565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561009b578082015181840152602081019050610080565b50505050905090810190601f1680156100c85780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b60606040805190810160405280600c81526020017f74657374206d65737361676500000000000000000000000000000000000000008152509050905600a165627a7a72305820372dc2a85eb89dcac537c6716f78472845e4b9454fe9177f9c4f324b8ca8e37a0029");
  //0x608060405234801561001057600080fd5b5061013f806100206000396000f300608060405260043610610041576000357c0100000000000000000000000000000000000000000000000000000000900463ffffffff168063f15fad2314610046575b600080fd5b34801561005257600080fd5b5061005b6100d6565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561009b578082015181840152602081019050610080565b50505050905090810190601f1680156100c85780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b60606040805190810160405280600c81526020017f74657374206d65737361676500000000000000000000000000000000000000008152509050905600a165627a7a72305820372dc2a85eb89dcac537c6716f78472845e4b9454fe9177f9c4f324b8ca8e37a0029
  //const auto code = dvmc::from_hex(erc20_22112022_withdraw_with_parameter_subtract_15);
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
    std::cout << "code.size " << code.size() << std::endl;
    const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());

    if (create_result.status_code != DVMC_SUCCESS)
    {
      std::cout << "integratedTest6 : Contract creation failed: " << create_result.status_code << "\n";
      return create_result.status_code;
    }

      std::cout << "integratedTest6 : Contract creation success" << "\n";
    auto& created_account = host.accounts[create_address];
    created_account.set_balance(100000000000000);
    created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);
    msg.recipient = create_address;
    exec_code = created_account.code;
  }
  
  {
    std::cout << "\n";
    const auto input = dvmc::from_hex("0xf15fad23");
    msg.input_data = input.data();
    msg.input_size = input.size();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    std::cout << "execute - test -> code : verify " << dvmc::hex(host.accounts[create_address].code) << std::endl;
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest6 : Result: " << result.status_code << "\nGas used - track : " << track_used << "\n";

    if (result.status_code == DVMC_SUCCESS)
      std::cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
  } 
  return res;
}
