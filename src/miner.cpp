#include "txdb.h"
#include "miner.h"
#include "kernel.h"
#include "ptrie/FixedHash.h"
#include "ptrie/TrieDB.h"
#include "ptrie/StateCacheDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "ptrie/Account.h"
#include "ptrie/DBFactory.h"
#include "state.h"

#include <dvmone/dvmone.h>
#include <dvmc/transitional_node.hpp>
#include <dvmc/dvmc.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/loader.h>
#include <dvmc/tooling.hpp>
#include "bitcoinrpc.h"
using namespace std;






extern __wx__* pwalletMain;
extern unsigned int nMinerSleep;
extern dev::OverlayDB* overlayDB__;
extern bool sectionVertex(const string&, string&);

int static FormatHashBlocks(void* pbuffer, unsigned int len)
{
  unsigned char* pdata = (unsigned char*)pbuffer;
  unsigned int blocks = 1 + ((len + 8) / 64);
  unsigned char* pend = pdata + 64 * blocks;
  memset(pdata + len, 0, 64 * blocks - len);
  pdata[len] = 0x80;
  unsigned int bits = len * 8;
  pend[-1] = (bits >> 0) & 0xff;
  pend[-2] = (bits >> 8) & 0xff;
  pend[-3] = (bits >> 16) & 0xff;
  pend[-4] = (bits >> 24) & 0xff;
  return blocks;
}

static const unsigned int pSHA256InitState[8] =
{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

void SHA256Transform(void* pstate, void* pinput, const void* pinit)
{
  SHA256_CTX ctx;
  unsigned char data[64];

  SHA256_Init(&ctx);

  for (int i = 0; i < 16; i++)
  {
    ((uint32_t*)data)[i] = ByteReverse(((uint32_t*)pinput)[i]);
  }

  for (int i = 0; i < 8; i++)
  {
    ctx.h[i] = ((uint32_t*)pinit)[i];
  }

  SHA256_Update(&ctx, data, sizeof(data));
  for (int i = 0; i < 8; i++)
  {
    ((uint32_t*)pstate)[i] = ctx.h[i];
  }
}


class COrphan
{
public:
  CTransaction* ptx;
  set<uint256> setDependsOn;
  double dPriority;
  double dFeePerKb;

  COrphan(CTransaction* ptxIn)
  {
    ptx = ptxIn;
    dPriority = dFeePerKb = 0;
  }

  void print() const
  {
    printf("COrphan(hash=%s, dPriority=%.1f, dFeePerKb=%.1f)\n",
           ptx->GetHash().ToString().substr(0,10).c_str(), dPriority, dFeePerKb);
    BOOST_FOREACH(uint256 hash, setDependsOn)
    printf("   setDependsOn %s\n", hash.ToString().substr(0,10).c_str());
  }
};


uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;
int64_t nLastCoinStakeSearchInterval = 0;


typedef boost::tuple<double, double, CTransaction*> TxPriority;
class TxPriorityCompare
{
  bool byFee;
public:
  TxPriorityCompare(bool _byFee) : byFee(_byFee) { }
  bool operator()(const TxPriority& a, const TxPriority& b)
  {
    if (byFee)
    {
      if (a.get<1>() == b.get<1>())
      {
        return a.get<0>() < b.get<0>();
      }
      return a.get<1>() < b.get<1>();
    }
    else
    {
      if (a.get<0>() == b.get<0>())
      {
        return a.get<1>() < b.get<1>();
      }
      return a.get<0>() < b.get<0>();
    }
  }
};


CBlock* CreateNewBlock(__wx__* pwallet, bool fProofOfStake, int64_t* pFees)
{

  unique_ptr<CBlock> pblock(new CBlock());
  if (!pblock.get())
  {
    return NULL;
  }

  CBlockIndex* pindexPrev = pindexBest;
  int nHeight = pindexPrev->nHeight + 1;

  if(!fTestNet && !IsProtocolV2(nHeight))
  {
    pblock->nVersion = 6;
  }


  CTransaction txNew;
  txNew.vin.resize(1);
  txNew.vin[0].prevout.SetNull();
  txNew.vout.resize(1);

  if (!fProofOfStake)
  {
    CReserveKey reservekey(pwallet);
    CPubKey pubkey;
    if (!reservekey.GetReservedKey(pubkey))
    {
      return NULL;
    }
    txNew.vout[0].scriptPubKey.SetDestination(pubkey.GetID());
  }
  else
  {

    txNew.vin[0].scriptSig = (CScript() << nHeight) + COINBASE_FLAGS;
    assert(txNew.vin[0].scriptSig.size() <= 100);

    txNew.vout[0].SetEmpty();
  }


  pblock->vtx.push_back(txNew);


  unsigned int nBlockMaxSize = GetArg("-blockmaxsize", MAX_BLOCK_SIZE_GEN/2);

  nBlockMaxSize = std::max((unsigned int)1000, std::min((unsigned int)(MAX_BLOCK_SIZE-1000), nBlockMaxSize));



  unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", 27000);
  nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);



  unsigned int nBlockMinSize = GetArg("-blockminsize", 0);
  nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);






  int64_t nMinTxFee = MIN_TX_FEE;
  if (mapArgs.count("-mintxfee"))
  {
    ParseMoney(mapArgs["-mintxfee"], nMinTxFee);
  }


  int64_t nFees = 0;
  {
    LOCK2(cs_main, mempool.cs);
    CTxDB txdb("r");


    list<COrphan> vOrphan;
    map<uint256, vector<COrphan*> > mapDependers;


    vector<TxPriority> vecPriority;
    vecPriority.reserve(mempool.mapTx.size());

    for (map<uint256, CTransaction>::iterator mi = mempool.mapTx.begin(); mi != mempool.mapTx.end(); ++mi)
    {
      CTransaction& tx = (*mi).second;
# 250 "miner.cpp"
      std::vector<vchType> vvchArgs;
      for(unsigned int i=0; i < tx.vout.size(); i++)
      {
        int op;
        if(aliasScript(tx.vout[i].scriptPubKey, op, vvchArgs))
        {
          if(op == OP_ALIAS_RELAY)
          {

            vector<unsigned char> hash_bytes(vvchArgs[1].begin(),vvchArgs[1].begin()+32);
            uint256 hash256_from_payload_bytes(hash_bytes);
            vector<unsigned char> data_bytes(vvchArgs[1].begin()+32,vvchArgs[1].end());
            uint256 recon_hash256 = Hash(data_bytes.begin(), data_bytes.end());

            if(hash256_from_payload_bytes == recon_hash256)
            {
              vector<unsigned char> str_bytes(vvchArgs[1].begin()+32,vvchArgs[1].begin()+52);
              const string target20ByteAliasStr = stringFromVch(str_bytes);


              pblock->stateRoot = pindexPrev->stateRootIndex;
              std::cout << "execution request - previous state root " << pblock->stateRoot << std::endl;

              dev::SecureTrieDB<dev::Address, dev::OverlayDB>* state;
              dev::h256 Empty;
              if(pblock->stateRoot == Empty)
              {


                state = new dev::SecureTrieDB<dev::Address, dev::OverlayDB>(overlayDB__);
                if(state->isNull())
                {

                  state->init();

                  overlayDB__->commit();
                }

              }
              else
              {
                std::cout << "state root is non zero creating securetriedb instance from existing ROOT" << std::endl;
                std::cout << "state root is " << pblock->stateRoot << std::endl;
                state = new dev::SecureTrieDB<dev::Address, dev::OverlayDB>(overlayDB__,pblock->stateRoot);
              }

              {
                std::vector<CTransaction> testTxVector;
                CPubKey newKey_contract_address;
                {
                  if(!pwalletMain->GetKeyFromPool(newKey_contract_address, false))
                  {
                    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
                  }
                  CKeyID keyID_contract_address = newKey_contract_address.GetID();
                  dvmc::address create_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_contract_address.GetHex().c_str());
                  dev::Address target_addr(keyID_contract_address.GetHex().c_str());

                  CPubKey newKey_test_sender_address;
                  if(!pwalletMain->GetKeyFromPool(newKey_test_sender_address, false))
                  {
                    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
                  }
                  CKeyID keyID_test_sender_address = newKey_test_sender_address.GetID();
                  dvmc::address test_sender_address = dvmc::literals::internal::from_hex<dvmc::address>(keyID_test_sender_address.GetHex().c_str());

                  dvmc::address test_recipient_address = test_sender_address;

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

                    }

                    auto& created_account = host.accounts[create_address];
                    created_account.set_balance(100000000000000);
                    created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);

                    msg.recipient = create_address;
                    exec_code = created_account.code;



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


                      state->insert(target_addr, &s.out());

                      overlayDB__->commit();
                      std::ostringstream os;
                      state->debugStructure(os);


                      std::cout << "integratedTest5 AFTER contract state insert : state root " << state->root() << std::endl;
                      pblock->stateRoot=state->root();
                    }
                  }
                }

                {
                }

                {
                }

                {
                }
              }
# 445 "miner.cpp"
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
      if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
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
      if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
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


void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{

  static uint256 hashPrevBlock;
  if (hashPrevBlock != pblock->hashPrevBlock)
  {
    nExtraNonce = 0;
    hashPrevBlock = pblock->hashPrevBlock;
  }
  ++nExtraNonce;

  unsigned int nHeight = pindexPrev->nHeight+1;
  pblock->vtx[0].vin[0].scriptSig = (CScript() << nHeight << CBigNum(nExtraNonce)) + COINBASE_FLAGS;
  assert(pblock->vtx[0].vin[0].scriptSig.size() <= 100);

  pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}


void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1)
{



  struct
  {
    struct unnamed2
    {
      int nVersion;
      uint256 hashPrevBlock;
      uint256 hashMerkleRoot;
      unsigned int nTime;
      unsigned int nBits;
      unsigned int nNonce;
    }
    block;
    unsigned char pchPadding0[64];
    uint256 hash1;
    unsigned char pchPadding1[64];
  }
  tmp;
  memset(&tmp, 0, sizeof(tmp));

  tmp.block.nVersion = pblock->nVersion;
  tmp.block.hashPrevBlock = pblock->hashPrevBlock;
  tmp.block.hashMerkleRoot = pblock->hashMerkleRoot;
  tmp.block.nTime = pblock->nTime;
  tmp.block.nBits = pblock->nBits;
  tmp.block.nNonce = pblock->nNonce;

  FormatHashBlocks(&tmp.block, sizeof(tmp.block));
  FormatHashBlocks(&tmp.hash1, sizeof(tmp.hash1));


  for (unsigned int i = 0; i < sizeof(tmp)/4; i++)
  {
    ((unsigned int*)&tmp)[i] = ByteReverse(((unsigned int*)&tmp)[i]);
  }


  SHA256Transform(pmidstate, &tmp.block, pSHA256InitState);

  memcpy(pdata, &tmp.block, 128);
  memcpy(phash1, &tmp.hash1, 64);
}


bool CheckWork(CBlock* pblock, __wx__& wallet, CReserveKey& reservekey)
{
  uint256 hashBlock = pblock->GetHash();
  uint256 hashProof = pblock->GetPoWHash();
  uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

  if(!pblock->IsProofOfWork())
  {
    return error("CheckWork() : %s is not a proof-of-work block", hashBlock.GetHex().c_str());
  }

  if (hashProof > hashTarget)
  {
    return false;
  }


  printf("CheckWork() : new proof-of-work block found  \n  proof hash: %s  \ntarget: %s\n", hashProof.GetHex().c_str(), hashTarget.GetHex().c_str());
  pblock->print();
  printf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue).c_str());


  {
    LOCK(cs_main);
    if (pblock->hashPrevBlock != hashBestChain)
    {
      return error("CheckWork() : generated block is stale");
    }


    reservekey.KeepKey();


    {
      LOCK(wallet.cs_wallet);
      wallet.mapRequestCount[hashBlock] = 0;
    }


    if (!ProcessBlock(NULL, pblock))
    {
      return error("CheckWork() : ProcessBlock, block not accepted");
    }
  }

  return true;
}

bool CheckStake(CBlock* pblock, __wx__& wallet)
{
  uint256 proofHash = 0, hashTarget = 0;
  uint256 hashBlock = pblock->GetHash();

  if(!pblock->IsProofOfStake())
  {
    return error("CheckStake() : %s is not a proof-of-stake block", hashBlock.GetHex().c_str());
  }


  if (!CheckProofOfStake(mapBlockIndex[pblock->hashPrevBlock], pblock->vtx[1], pblock->nBits, proofHash, hashTarget))
  {
    return error("CheckStake() : proof-of-stake checking failed");
  }







  {
    LOCK(cs_main);
    if (pblock->hashPrevBlock != hashBestChain)
    {
      return error("CheckStake() : generated block is stale");
    }


    {
      LOCK(wallet.cs_wallet);
      wallet.mapRequestCount[hashBlock] = 0;
    }


    if (!ProcessBlock(NULL, pblock))
    {
      return error("CheckStake() : ProcessBlock, block not accepted");
    }
  }

  return true;
}

void StakeMiner(__wx__ *pwallet)
{
  SetThreadPriority(THREAD_PRIORITY_LOWEST);


  RenameThread("iocoin-miner");

  bool fTryToSync = true;

  while (true)
  {
    if (fShutdown)
    {
      return;
    }

    while (pwallet->as())
    {
      nLastCoinStakeSearchInterval = 0;
      MilliSleep(1000);
      if (fShutdown)
      {
        return;
      }
    }

    while (vNodes.empty() || IsInitialBlockDownload())
    {
      nLastCoinStakeSearchInterval = 0;
      fTryToSync = true;
      MilliSleep(1000);
      if (fShutdown)
      {
        return;
      }
    }

    if (fTryToSync)
    {
      fTryToSync = false;
      if (vNodes.size() < 3 || nBestHeight < GetNumBlocksOfPeers())
      {
        MilliSleep(60000);
        continue;
      }
    }




    int64_t nFees;
    unique_ptr<CBlock> pblock(CreateNewBlock(pwallet, true, &nFees));
    if (!pblock.get())
    {
      return;
    }


    if (pblock->SignBlock(*pwallet, nFees))
    {
      SetThreadPriority(THREAD_PRIORITY_NORMAL);
      CheckStake(pblock.get(), *pwallet);
      SetThreadPriority(THREAD_PRIORITY_LOWEST);
      MilliSleep(500);
    }
    else
    {
      MilliSleep(nMinerSleep);
    }
  }
}
