#include "SMCValidator.h"

#include "ccoin/Process.h"
#include "ccoin/Client.h"
#include "wallet/Wallet.h"
#include "TxDB.h"
#include "Miner.h"
#include "Kernel.h"
#include "ptrie/FixedHash.h"
#include "ptrie/TrieDB.h"
#include "ptrie/StateCacheDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "ptrie/Account.h"
#include "ptrie/DBFactory.h"
#include "State.h"
#include <dvmone/dvmone.h>
#include <dvmc/transitional_node.hpp>
#include <dvmc/dvmc.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/loader.h>
#include <dvmc/tooling.hpp>
#include "rpc/Client.h"

#include "Miner.h"

using dvmc::operator""_address;
extern bool sectionVertex(const string&, string&);

bool SMCValidator::validate()
{
  int64_t nMinTxFee = CTransaction::MIN_TX_FEE;

  {
    {
      std::vector<vchType> vvchArgs;

      for(unsigned int i=0; i < this->tx_.vout.size(); i++)
      {
        int op;

        if(aliasScript(this->tx_.vout[i].scriptPubKey, op, vvchArgs))
        {
          if(op == OP_ALIAS_RELAY && vvchArgs[1].size() > 32)
          {
            vector<unsigned char> hash_bytes(vvchArgs[1].begin(),vvchArgs[1].begin()+32);
            uint256 hash256_from_payload_bytes(hash_bytes);
            vector<unsigned char> data_bytes(vvchArgs[1].begin()+32,vvchArgs[1].end());
            vector<unsigned char> data_relay_bytes(data_bytes.begin()+20,data_bytes.end());
            std::cout << "Extracted payload relay data " << stringFromVch(data_relay_bytes) << std::endl;
            uint256 recon_hash256 = Hash(data_bytes.begin(), data_bytes.end());

            if(hash256_from_payload_bytes == recon_hash256)
            {
              vector<unsigned char> str_bytes(vvchArgs[1].begin()+32,vvchArgs[1].begin()+52);
              const string target20ByteAliasStr = stringFromVch(str_bytes);
              string contractCode;

              if(sectionVertex(target20ByteAliasStr, contractCode))
              {
                //std::cout << ">>>> retrieved contract code " << contractCode << std::endl;
              }

              dev::SecureTrieDB<dev::Address, dev::OverlayDB>* state;

              {
                {
                  vector<json_spirit::Value> res;
                  constexpr auto create_address__ = 0xc9ea7ed000000000000000000000000000000001_address;
                  dvmc::VertexNode host;
                  dvmc::TransitionalNode created_account;
                  dvmc::TransitionalNode sender_account;
                  dvmc_message msg{};
                  msg.track = std::numeric_limits<int64_t>::max();
                  dvmc::bytes code__;
                  dvmc::bytes_view exec_code = code__;
                  {
                    dvmc_message create_msg{};
                    create_msg.kind = DVMC_CREATE;
                    create_msg.recipient = create_address__;
                    create_msg.track = std::numeric_limits<int64_t>::max();
                    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
                    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
                    const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code__.data(), code__.size());

                    if (create_result.status_code != DVMC_SUCCESS)
                    {
                      std::cout << "integratedTest6 : Contract creation failed: " << create_result.status_code << "\n";
                    }

                    std::cout << "integratedTest6 : Contract creation success" << "\n";
                    auto& created_account = host.accounts[create_address__];
                    created_account.set_balance(100000000000000);
                    created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);
                    msg.recipient = create_address__;
                    exec_code = created_account.code;
                  }

                  {
                    std::cout << "\n";
                    const auto input = dvmc::from_hex(stringFromVch(data_relay_bytes));
                    msg.input_data = input.data();
                    msg.input_size = input.size();
                    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
                    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
                    std::cout << "execute - test -> code : verify " << dvmc::hex(host.accounts[create_address__].code) << std::endl;
                    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
                    const auto track_used = msg.track - result.track_left;
                    std::cout << "integratedTest6 : Result: " << result.status_code << "\nGas used - track : " << track_used << "\n";

                    if (result.status_code == DVMC_SUCCESS)
                      std::cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
                  }
                  dev::eth::Account tmpAcc(0,0);
                  {
                    dev::u256 nonce = 12345678;
                    dev::u256 balance = 1010101010101;
                    dev::RLPStream s(4);
                    s << nonce << balance;
                    {
                      dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state->db(), tmpAcc.baseRoot());

                      for(auto pair : created_account.storage)
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
                    s << tmpAcc.codeHash();
		    state->insert(target_addr,&s.out());
                    std::ostringstream os;
                    state->debugStructure(os);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return true;
}
