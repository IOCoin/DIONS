#pragma once

#include "ccoin/Process.h"
#include "ccoin/Client.h"
#include "wallet/Wallet.h"


CBlock* CreateNewBlockBASE(__wx__* pwallet, bool fProofOfStake=false, int64_t* pFees = 0);


void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);


void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1);


bool CheckWork(CBlock* pblock, __wx__& wallet, CReserveKey& reservekey);


bool CheckStake(CBlock* pblock, __wx__& wallet);


void SHA256Transform(void* pstate, void* pinput, const void* pinit);

class Worker
{
  public:
    void static process(Worker* w)
    {
      w->operate();
    }

    virtual void operate() {};

    bool start()
    {
      try
      {
        boost::thread(Worker::process,this);
      }
      catch(boost::thread_resource_error &e)
      {
        printf("Error creating thread: %s\n", e.what());
        return false;
      }

      return true;
    }
};

class Miner : public Worker
{
  public:
    Miner() = default;
    Miner(Client* c, __wx__* wallet,unsigned int s)
    {
      this->c_ = c;
      this->wallet_ = wallet;
      this->sleep_ = s;
    }

    virtual void operate() override
    {
      std::cout << "Miner::operate" << std::endl;
      ThreadStakeMiner();
    }

    void wallet(__wx__* w)
    {
      this->wallet_ = w;
    }
    void client(Client* c)
    {
      this->c_ = c;
    }
    void sleep(unsigned int s)
    {
      this->sleep_ = s;
    }

    void ThreadStakeMiner();
    void StakeMiner();
    CBlock* CreateNewBlock(__wx__* pwallet, bool fProofOfStake, int64_t* pFees);

  private:
    unsigned int sleep_;
    __wx__* wallet_;
    Client* c_;
};
