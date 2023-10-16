#ifndef _BITCOIN_ADDRMAN
#define _BITCOIN_ADDRMAN 1

#include "NetBase.h"
#include "Protocol.h"
#include "core/util.h"
#include "Sync.h"


#include <map>
#include <vector>

#include <openssl/rand.h>



class CAddrInfo : public CAddress
{
private:

  CNetAddr source;


  int64_t nLastSuccess;





  int nAttempts;


  int nRefCount;


  bool fInTried;


  int nRandomPos;

  friend class CAddrMan;

public:

  IMPLEMENT_SERIALIZE(
    CAddress* pthis = (CAddress*)(this);
    READWRITE(*pthis);
    READWRITE(source);
    READWRITE(nLastSuccess);
    READWRITE(nAttempts);
  )

  void Init()
  {
    nLastSuccess = 0;
    nLastTry = 0;
    nAttempts = 0;
    nRefCount = 0;
    fInTried = false;
    nRandomPos = -1;
  }

  CAddrInfo(const CAddress &addrIn, const CNetAddr &addrSource) : CAddress(addrIn), source(addrSource)
  {
    Init();
  }

  CAddrInfo() : CAddress(), source()
  {
    Init();
  }


  int GetTriedBucket(const std::vector<unsigned char> &nKey) const;


  int GetNewBucket(const std::vector<unsigned char> &nKey, const CNetAddr& src) const;


  int GetNewBucket(const std::vector<unsigned char> &nKey) const
  {
    return GetNewBucket(nKey, source);
  }


  bool IsTerrible(int64_t nNow = GetAdjustedTime()) const;


  double GetChance(int64_t nNow = GetAdjustedTime()) const;

};
# 123 "addrman.h"
#define ADDRMAN_TRIED_BUCKET_COUNT 64


#define ADDRMAN_TRIED_BUCKET_SIZE 64


#define ADDRMAN_NEW_BUCKET_COUNT 256


#define ADDRMAN_NEW_BUCKET_SIZE 64


#define ADDRMAN_TRIED_BUCKETS_PER_GROUP 4


#define ADDRMAN_NEW_BUCKETS_PER_SOURCE_GROUP 32


#define ADDRMAN_NEW_BUCKETS_PER_ADDRESS 4


#define ADDRMAN_TRIED_ENTRIES_INSPECT_ON_EVICT 4


#define ADDRMAN_HORIZON_DAYS 30


#define ADDRMAN_RETRIES 3


#define ADDRMAN_MAX_FAILURES 10


#define ADDRMAN_MIN_FAIL_DAYS 7


#define ADDRMAN_GETADDR_MAX_PCT 23


#define ADDRMAN_GETADDR_MAX 2500


class CAddrMan
{
private:

  mutable CCriticalSection cs;


  std::vector<unsigned char> nKey;


  int nIdCount;


  std::map<int, CAddrInfo> mapInfo;


  std::map<CNetAddr, int> mapAddr;


  std::vector<int> vRandom;


  int nTried;


  std::vector<std::vector<int> > vvTried;


  int nNew;


  std::vector<std::set<int> > vvNew;

protected:


  CAddrInfo* Find(const CNetAddr& addr, int *pnId = NULL);



  CAddrInfo* Create(const CAddress &addr, const CNetAddr &addrSource, int *pnId = NULL);


  void SwapRandom(unsigned int nRandomPos1, unsigned int nRandomPos2);


  int SelectTried(int nKBucket);




  int ShrinkNew(int nUBucket);



  void MakeTried(CAddrInfo& info, int nId, int nOrigin);


  void Good_(const CService &addr, int64_t nTime);


  bool Add_(const CAddress &addr, const CNetAddr& source, int64_t nTimePenalty);


  void Attempt_(const CService &addr, int64_t nTime);



  CAddress Select_(int nUnkBias);

#ifdef DEBUG_ADDRMAN

  int Check_();
#endif


  void GetAddr_(std::vector<CAddress> &vAddr);


  void Connected_(const CService &addr, int64_t nTime);

public:

  IMPLEMENT_SERIALIZE
  ((
  {
# 271 "addrman.h"
    {
      LOCK(cs);
      unsigned char nVersion = 0;
      READWRITE(nVersion);
      READWRITE(nKey);
      READWRITE(nNew);
      READWRITE(nTried);

      CAddrMan *am = const_cast<CAddrMan*>(this);
      if (fWrite)
      {
        int nUBuckets = ADDRMAN_NEW_BUCKET_COUNT;
        READWRITE(nUBuckets);
        std::map<int, int> mapUnkIds;
        int nIds = 0;
        for (std::map<int, CAddrInfo>::iterator it = am->mapInfo.begin(); it != am->mapInfo.end(); it++)
        {
          if (nIds == nNew)
          {
            break;
          }
          mapUnkIds[(*it).first] = nIds;
          CAddrInfo &info = (*it).second;
          if (info.nRefCount)
          {
            READWRITE(info);
            nIds++;
          }
        }
        nIds = 0;
        for (std::map<int, CAddrInfo>::iterator it = am->mapInfo.begin(); it != am->mapInfo.end(); it++)
        {
          if (nIds == nTried)
          {
            break;
          }
          CAddrInfo &info = (*it).second;
          if (info.fInTried)
          {
            READWRITE(info);
            nIds++;
          }
        }
        for (std::vector<std::set<int> >::iterator it = am->vvNew.begin(); it != am->vvNew.end(); it++)
        {
          const std::set<int> &vNew = (*it);
          int nSize = vNew.size();
          READWRITE(nSize);
          for (std::set<int>::iterator it2 = vNew.begin(); it2 != vNew.end(); it2++)
          {
            int nIndex = mapUnkIds[*it2];
            READWRITE(nIndex);
          }
        }
      }
      else
      {
        int nUBuckets = 0;
        READWRITE(nUBuckets);
        am->nIdCount = 0;
        am->mapInfo.clear();
        am->mapAddr.clear();
        am->vRandom.clear();
        am->vvTried = std::vector<std::vector<int> >(ADDRMAN_TRIED_BUCKET_COUNT, std::vector<int>(0));
        am->vvNew = std::vector<std::set<int> >(ADDRMAN_NEW_BUCKET_COUNT, std::set<int>());
        for (int n = 0; n < am->nNew; n++)
        {
          CAddrInfo &info = am->mapInfo[n];
          READWRITE(info);
          am->mapAddr[info] = n;
          info.nRandomPos = vRandom.size();
          am->vRandom.push_back(n);
          if (nUBuckets != ADDRMAN_NEW_BUCKET_COUNT)
          {
            am->vvNew[info.GetNewBucket(am->nKey)].insert(n);
            info.nRefCount++;
          }
        }
        am->nIdCount = am->nNew;
        int nLost = 0;
        for (int n = 0; n < am->nTried; n++)
        {
          CAddrInfo info;
          READWRITE(info);
          std::vector<int> &vTried = am->vvTried[info.GetTriedBucket(am->nKey)];
          if (vTried.size() < ADDRMAN_TRIED_BUCKET_SIZE)
          {
            info.nRandomPos = vRandom.size();
            info.fInTried = true;
            am->vRandom.push_back(am->nIdCount);
            am->mapInfo[am->nIdCount] = info;
            am->mapAddr[info] = am->nIdCount;
            vTried.push_back(am->nIdCount);
            am->nIdCount++;
          }
          else
          {
            nLost++;
          }
        }
        am->nTried -= nLost;
        for (int b = 0; b < nUBuckets; b++)
        {
          std::set<int> &vNew = am->vvNew[b];
          int nSize = 0;
          READWRITE(nSize);
          for (int n = 0; n < nSize; n++)
          {
            int nIndex = 0;
            READWRITE(nIndex);
            CAddrInfo &info = am->mapInfo[nIndex];
            if (nUBuckets == ADDRMAN_NEW_BUCKET_COUNT && info.nRefCount < ADDRMAN_NEW_BUCKETS_PER_ADDRESS)
            {
              info.nRefCount++;
              vNew.insert(nIndex);
            }
          }
        }
      }
    }
  });)

  CAddrMan() : vRandom(0), vvTried(ADDRMAN_TRIED_BUCKET_COUNT, std::vector<int>(0)), vvNew(ADDRMAN_NEW_BUCKET_COUNT, std::set<int>())
  {
    nKey.resize(32);
    RAND_bytes(&nKey[0], 32);

    nIdCount = 0;
    nTried = 0;
    nNew = 0;
  }


  int size()
  {
    return vRandom.size();
  }


  void Check()
  {
#ifdef DEBUG_ADDRMAN
    {
      LOCK(cs);
      int err;
      if ((err=Check_()))
      {
        printf("ADDRMAN CONSISTENCY CHECK FAILED!!! err=%i\n", err);
      }
    }
#endif
  }


  bool Add(const CAddress &addr, const CNetAddr& source, int64_t nTimePenalty = 0)
  {
    bool fRet = false;
    {
      LOCK(cs);
      Check();
      fRet |= Add_(addr, source, nTimePenalty);
      Check();
    }
    if (fRet)
    {
      printf("Added %s from %s: %i tried, %i new\n", addr.ToStringIPPort().c_str(), source.ToString().c_str(), nTried, nNew);
    }
    return fRet;
  }


  bool Add(const std::vector<CAddress> &vAddr, const CNetAddr& source, int64_t nTimePenalty = 0)
  {
    int nAdd = 0;
    {
      LOCK(cs);
      Check();
      for (std::vector<CAddress>::const_iterator it = vAddr.begin(); it != vAddr.end(); it++)
      {
        nAdd += Add_(*it, source, nTimePenalty) ? 1 : 0;
      }
      Check();
    }
    if (nAdd)
    {
      printf("Added %i addresses from %s: %i tried, %i new\n", nAdd, source.ToString().c_str(), nTried, nNew);
    }
    return nAdd > 0;
  }


  void Good(const CService &addr, int64_t nTime = GetAdjustedTime())
  {
    {
      LOCK(cs);
      Check();
      Good_(addr, nTime);
      Check();
    }
  }


  void Attempt(const CService &addr, int64_t nTime = GetAdjustedTime())
  {
    {
      LOCK(cs);
      Check();
      Attempt_(addr, nTime);
      Check();
    }
  }



  CAddress Select(int nUnkBias = 50)
  {
    CAddress addrRet;
    {
      LOCK(cs);
      Check();
      addrRet = Select_(nUnkBias);
      Check();
    }
    return addrRet;
  }


  std::vector<CAddress> GetAddr()
  {
    Check();
    std::vector<CAddress> vAddr;
    {
      LOCK(cs);
      GetAddr_(vAddr);
    }
    Check();
    return vAddr;
  }


  void Connected(const CService &addr, int64_t nTime = GetAdjustedTime())
  {
    {
      LOCK(cs);
      Check();
      Connected_(addr, nTime);
      Check();
    }
  }
};

#endif
