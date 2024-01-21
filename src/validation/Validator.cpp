#include "Validator.h"

bool Validator::validate(CTransaction &tx)
{
  {
    LOCK2(cs_main, mempool.cs);
    CTxDB txdb("r");
    list<COrphan> vOrphan;
    map<uint256, vector<COrphan*> > mapDependers;
    vector<TxPriority> vecPriority;
    vecPriority.reserve(mempool.mapTx.size());

    {
      std::vector<vchType> vvchArgs;

      for(unsigned int i=0; i < tx.vout.size(); i++)
      {
        int op;

        if(aliasScript(tx.vout[i].scriptPubKey, op, vvchArgs))
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

              pblock->stateRoot = pindexPrev->stateRootIndex;
              dev::SecureTrieDB<dev::Address, dev::OverlayDB>* state;
              dev::h256 Empty;

              if(pblock->stateRoot == Empty)
              {
		      std::cout << ">>>>>>>>>>>>>>>>>>>>>>>> STORAGE EMPTY  " << GetDataDir() << std::endl;
                state = new dev::SecureTrieDB<dev::Address, dev::OverlayDB>(c_->stateDB());

                if(state->isNull())
                {
                  state->init();
                  c_->stateDB()->commit();
                }
              }
              else
              {
		      std::cout << GetDataDir() << " >>>>>>>>>>>>>>>>>>>>>>>> RETRIEVE STORAGE " << pblock->stateRoot << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
                state = new dev::SecureTrieDB<dev::Address, dev::OverlayDB>(c_->stateDB(),pblock->stateRoot);
              }

              {
                std::vector<CTransaction> testTxVector;
                CPubKey newKey_contract_address;
                {
                  if(!this->wallet_->GetKeyFromPool(newKey_contract_address, false))
                  {
                    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
                  }

                  CKeyID keyID_contract_address = newKey_contract_address.GetID();
                  dvmc::address create_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_contract_address.GetHex().c_str());
                  dev::Address target_addr(keyID_contract_address.GetHex().c_str());
                  CPubKey newKey_test_sender_address;

                  if(!this->wallet_->GetKeyFromPool(newKey_test_sender_address, false))
                  {
                    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
                  }

                  //std::cout << "Retrieved contract code " << contractCode << std::endl;
                  const auto code = dvmc::from_hex(contractCode);
  vector<json_spirit::Value> res;
  constexpr auto create_address__ = 0xc9ea7ed000000000000000000000000000000001_address;
  std::cout << "integratedTest6 : initialised create address " << dvmc::hex(create_address.bytes) << "\n";
  const auto code__ = code;
  dvmc::VertexNode host;
  dvmc::TransitionalNode created_account;
  dvmc::TransitionalNode sender_account;
  sender_account.set_balance(3141);
  dvmc_message msg{};
  msg.track = std::numeric_limits<int64_t>::max();
  dvmc::bytes_view exec_code = code__;
  {
    dvmc_message create_msg{};
    create_msg.kind = DVMC_CREATE;
    create_msg.recipient = create_address__;
    create_msg.track = std::numeric_limits<int64_t>::max();
    dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
    dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
    std::cout << "code.size " << code.size() << std::endl;
    const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());

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
    std::cout << "execute - test -> code : verify " << dvmc::hex(host.accounts[create_address].code) << std::endl;
    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
    const auto track_used = msg.track - result.track_left;
    std::cout << "integratedTest6 : Result: " << result.status_code << "\nGas used - track : " << track_used << "\n";

    if (result.status_code == DVMC_SUCCESS)
      std::cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << "\n";
  } 
  //UPDATE STORAGE AFTER CALL
                    dev::eth::Account tmpAcc(0,0);
                    {
                      dev::u256 nonce = 12345678;
                      dev::u256 balance = 1010101010101;
                      dev::RLPStream s(4);
                      s << nonce << balance;
                      {
                        dev::SecureTrieDB<dev::h256, dev::StateCacheDB> storageDB(state->db(), tmpAcc.baseRoot());

			std::cout << "storage size " << created_account.storage.size() << std::endl;
                        for(auto pair : created_account.storage)
                        {
			std::cout << "iterate over storage..." << std::endl;
                          auto storage_key = pair.first.bytes;
                          dev::bytes key;

                          for(int i=0; i<32; i++)
                          {
                            key.push_back(storage_key[i]);
                          }

                          dev::h256 key256(key);
                          auto storage_bytes = pair.second.value;
			std::cout << "DIRTY ? " << pair.second.dirty << std::endl;

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
		      std::cout << "tmpAcc.codeHash " << tmpAcc.codeHash() << std::endl;
                      state->insert(target_addr, &s.out());
                      c_->stateDB()->commit();
                      std::ostringstream os;
                      state->debugStructure(os);
                      pblock->stateRoot=state->root();
		      std::cout << "UPDATED STATE ROOT " << pblock->stateRoot << std::endl;
                    }
                }
              }
            }
          }
        }
      }

      if (tx.IsCoinBase() || tx.IsCoinStake() || !IsFinalTx(tx, nHeight))
      {
        continue;
      }

      COrphan* porphan = NULL;
      double dPriority = 0;
      int64_t nTotalIn = 0;
      bool fMissingInputs = false;
      BOOST_FOREACH(const CTxIn& txin, tx.vin)
      {
        CTransaction txPrev;
        CTxIndex txindex;

        if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
        {
          if (!mempool.mapTx.count(txin.prevout.hash))
          {
            printf("ERROR: mempool transaction missing input\n");

            if (fDebug)
            {
              assert("mempool transaction missing input" == 0);
            }

            fMissingInputs = true;

            if (porphan)
            {
              vOrphan.pop_back();
            }

            break;
          }

          if (!porphan)
          {
            vOrphan.push_back(COrphan(&tx));
            porphan = &vOrphan.back();
          }

          mapDependers[txin.prevout.hash].push_back(porphan);
          porphan->setDependsOn.insert(txin.prevout.hash);
          nTotalIn += mempool.mapTx[txin.prevout.hash].vout[txin.prevout.n].nValue;
          continue;
        }

        int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
        nTotalIn += nValueIn;
        int nConf = txindex.GetDepthInMainChain();
        dPriority += (double)nValueIn * nConf;
      }

      if (fMissingInputs)
      {
        continue;
      }

      unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
      dPriority /= nTxSize;
      double dFeePerKb = double(nTotalIn-tx.GetValueOut()) / (double(nTxSize)/1000.0);

      if (porphan)
      {
        porphan->dPriority = dPriority;
        porphan->dFeePerKb = dFeePerKb;
      }
      else
      {
        vecPriority.push_back(TxPriority(dPriority, dFeePerKb, &(*mi).second));
      }
    }

    map<uint256, CTxIndex> mapTestPool;
    uint64_t nBlockSize = 1000;
    uint64_t nBlockTx = 0;
    int nBlockSigOps = 100;
    bool fSortedByFee = (nBlockPrioritySize <= 0);
    TxPriorityCompare comparer(fSortedByFee);
    std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);

    while (!vecPriority.empty())
    {
      double dPriority = vecPriority.front().get<0>();
      double dFeePerKb = vecPriority.front().get<1>();
      CTransaction& tx = *(vecPriority.front().get<2>());
      std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
      vecPriority.pop_back();
      unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

      if (nBlockSize + nTxSize >= nBlockMaxSize)
      {
        continue;
      }

      unsigned int nTxSigOps = tx.GetLegacySigOpCount();

      if (nBlockSigOps + nTxSigOps >= ConfigurationState::MAX_BLOCK_SIGOPS)
      {
        continue;
      }

      if (tx.nTime > GetAdjustedTime() || (fProofOfStake && tx.nTime > pblock->vtx[0].nTime))
      {
        continue;
      }

      int64_t nMinFee = tx.GetMinFee(nBlockSize, GMF_BLOCK);

      if (fSortedByFee && (dFeePerKb < nMinTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
      {
        continue;
      }

      if (!fSortedByFee &&
          ((nBlockSize + nTxSize >= nBlockPrioritySize) || (dPriority < COIN * 144 / 250)))
      {
        fSortedByFee = true;
        comparer = TxPriorityCompare(fSortedByFee);
        std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);
      }

      map<uint256, CTxIndex> mapTestPoolTmp(mapTestPool);
      MapPrevTx mapInputs;
      bool fInvalid;

      if (!tx.FetchInputs(txdb, mapTestPoolTmp, false, true, mapInputs, fInvalid))
      {
        continue;
      }

      int64_t nTxFees = tx.GetValueIn(mapInputs)-tx.GetValueOut();

      if (nTxFees < nMinFee)
      {
        continue;
      }

      nTxSigOps += tx.GetP2SHSigOpCount(mapInputs);

      if (nBlockSigOps + nTxSigOps >= ConfigurationState::MAX_BLOCK_SIGOPS)
      {
        continue;
      }

      CDiskTxPos cDiskTxPos = CDiskTxPos(1,1,1);

      if (!tx.ConnectInputs(txdb, mapInputs, mapTestPoolTmp, cDiskTxPos, pindexPrev, false, true, MANDATORY_SCRIPT_VERIFY_FLAGS))
      {
        continue;
      }

      mapTestPoolTmp[tx.GetHash()] = CTxIndex(CDiskTxPos(1,1,1), tx.vout.size());
      swap(mapTestPool, mapTestPoolTmp);
      pblock->vtx.push_back(tx);
      nBlockSize += nTxSize;
      ++nBlockTx;
      nBlockSigOps += nTxSigOps;
      nFees += nTxFees;

      if (fDebug && GetBoolArg("-printpriority"))
      {
        printf("priority %.1f feeperkb %.1f txid %s\n",
               dPriority, dFeePerKb, tx.GetHash().ToString().c_str());
      }

      uint256 hash = tx.GetHash();

      if (mapDependers.count(hash))
      {
        BOOST_FOREACH(COrphan* porphan, mapDependers[hash])
        {
          if (!porphan->setDependsOn.empty())
          {
            porphan->setDependsOn.erase(hash);

            if (porphan->setDependsOn.empty())
            {
              vecPriority.push_back(TxPriority(porphan->dPriority, porphan->dFeePerKb, porphan->ptx));
              std::push_heap(vecPriority.begin(), vecPriority.end(), comparer);
            }
          }
        }
      }
    }

    nLastBlockTx = nBlockTx;
    nLastBlockSize = nBlockSize;

    if (fDebug && GetBoolArg("-printpriority"))
    {
      printf("CreateNewBlock(): total size %" PRIu64 "\n", nBlockSize);
    }

    if (!fProofOfStake)
    {
      pblock->vtx[0].vout[0].nValue = GetProofOfWorkReward(nHeight, nFees);
    }

    if (pFees)
    {
      *pFees = nFees;
    }

    pblock->hashPrevBlock = pindexPrev->GetBlockHash();
    pblock->nTime = max(pindexPrev->GetPastTimeLimit()+1, pblock->GetMaxTransactionTime());
    pblock->nTime = max(pblock->GetBlockTime(), PastDrift(pindexPrev->GetBlockTime(), nHeight));

    if (!fProofOfStake)
    {
      pblock->UpdateTime(pindexPrev);
    }

    pblock->nNonce = 0;
  }
  pblock->nBits = GetNextTargetRequired(pindexPrev, fProofOfStake, nFees);
  return pblock.release();
}
