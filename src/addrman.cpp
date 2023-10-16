
#include "AddrMan.h"
using namespace std;
int CAddrInfo::GetTriedBucket(const std::vector<unsigned char> &nKey) const
{
  CDataStream ss1(SER_GETHASH, 0);
  std::vector<unsigned char> vchKey = GetKey();
  ss1 << nKey << vchKey;
  uint64_t hash1 = Hash(ss1.begin(), ss1.end()).Get64();
  CDataStream ss2(SER_GETHASH, 0);
  std::vector<unsigned char> vchGroupKey = GetGroup();
  ss2 << nKey << vchGroupKey << (hash1 % ADDRMAN_TRIED_BUCKETS_PER_GROUP);
  uint64_t hash2 = Hash(ss2.begin(), ss2.end()).Get64();
  return hash2 % ADDRMAN_TRIED_BUCKET_COUNT;
}
int CAddrInfo::GetNewBucket(const std::vector<unsigned char> &nKey, const CNetAddr& src) const
{
  CDataStream ss1(SER_GETHASH, 0);
  std::vector<unsigned char> vchGroupKey = GetGroup();
  std::vector<unsigned char> vchSourceGroupKey = src.GetGroup();
  ss1 << nKey << vchGroupKey << vchSourceGroupKey;
  uint64_t hash1 = Hash(ss1.begin(), ss1.end()).Get64();
  CDataStream ss2(SER_GETHASH, 0);
  ss2 << nKey << vchSourceGroupKey << (hash1 % ADDRMAN_NEW_BUCKETS_PER_SOURCE_GROUP);
  uint64_t hash2 = Hash(ss2.begin(), ss2.end()).Get64();
  return hash2 % ADDRMAN_NEW_BUCKET_COUNT;
}
bool CAddrInfo::IsTerrible(int64_t nNow) const
{
  if (nLastTry && nLastTry >= nNow-60)
  {
    return false;
  }

  if (nTime > nNow + 10*60)
  {
    return true;
  }

  if (nTime==0 || nNow-nTime > ADDRMAN_HORIZON_DAYS*86400)
  {
    return true;
  }

  if (nLastSuccess==0 && nAttempts>=ADDRMAN_RETRIES)
  {
    return true;
  }

  if (nNow-nLastSuccess > ADDRMAN_MIN_FAIL_DAYS*86400 && nAttempts>=ADDRMAN_MAX_FAILURES)
  {
    return true;
  }

  return false;
}
double CAddrInfo::GetChance(int64_t nNow) const
{
  double fChance = 1.0;
  int64_t nSinceLastSeen = nNow - nTime;
  int64_t nSinceLastTry = nNow - nLastTry;

  if (nSinceLastSeen < 0)
  {
    nSinceLastSeen = 0;
  }

  if (nSinceLastTry < 0)
  {
    nSinceLastTry = 0;
  }

  fChance *= 600.0 / (600.0 + nSinceLastSeen);

  if (nSinceLastTry < 60*10)
  {
    fChance *= 0.01;
  }

  for (int n=0; n<nAttempts; n++)
  {
    fChance /= 1.5;
  }

  return fChance;
}
CAddrInfo* CAddrMan::Find(const CNetAddr& addr, int *pnId)
{
  std::map<CNetAddr, int>::iterator it = mapAddr.find(addr);

  if (it == mapAddr.end())
  {
    return NULL;
  }

  if (pnId)
  {
    *pnId = (*it).second;
  }

  std::map<int, CAddrInfo>::iterator it2 = mapInfo.find((*it).second);

  if (it2 != mapInfo.end())
  {
    return &(*it2).second;
  }

  return NULL;
}
CAddrInfo* CAddrMan::Create(const CAddress &addr, const CNetAddr &addrSource, int *pnId)
{
  int nId = nIdCount++;
  mapInfo[nId] = CAddrInfo(addr, addrSource);
  mapAddr[addr] = nId;
  mapInfo[nId].nRandomPos = vRandom.size();
  vRandom.push_back(nId);

  if (pnId)
  {
    *pnId = nId;
  }

  return &mapInfo[nId];
}
void CAddrMan::SwapRandom(unsigned int nRndPos1, unsigned int nRndPos2)
{
  if (nRndPos1 == nRndPos2)
  {
    return;
  }

  assert(nRndPos1 < vRandom.size() && nRndPos2 < vRandom.size());
  int nId1 = vRandom[nRndPos1];
  int nId2 = vRandom[nRndPos2];
  assert(mapInfo.count(nId1) == 1);
  assert(mapInfo.count(nId2) == 1);
  mapInfo[nId1].nRandomPos = nRndPos2;
  mapInfo[nId2].nRandomPos = nRndPos1;
  vRandom[nRndPos1] = nId2;
  vRandom[nRndPos2] = nId1;
}
int CAddrMan::SelectTried(int nKBucket)
{
  std::vector<int> &vTried = vvTried[nKBucket];
  int64_t nOldest = -1;
  int nOldestPos = -1;

  for (unsigned int i = 0; i < ADDRMAN_TRIED_ENTRIES_INSPECT_ON_EVICT && i < vTried.size(); i++)
  {
    int nPos = GetRandInt(vTried.size() - i) + i;
    int nTemp = vTried[nPos];
    vTried[nPos] = vTried[i];
    vTried[i] = nTemp;
    assert(nOldest == -1 || mapInfo.count(nTemp) == 1);

    if (nOldest == -1 || mapInfo[nTemp].nLastSuccess < mapInfo[nOldest].nLastSuccess)
    {
      nOldest = nTemp;
      nOldestPos = nPos;
    }
  }

  return nOldestPos;
}
int CAddrMan::ShrinkNew(int nUBucket)
{
  assert(nUBucket >= 0 && (unsigned int)nUBucket < vvNew.size());
  std::set<int> &vNew = vvNew[nUBucket];

  for (std::set<int>::iterator it = vNew.begin(); it != vNew.end(); it++)
  {
    assert(mapInfo.count(*it));
    CAddrInfo &info = mapInfo[*it];

    if (info.IsTerrible())
    {
      if (--info.nRefCount == 0)
      {
        SwapRandom(info.nRandomPos, vRandom.size()-1);
        vRandom.pop_back();
        mapAddr.erase(info);
        mapInfo.erase(*it);
        nNew--;
      }

      vNew.erase(it);
      return 0;
    }
  }

  int n[4] = {GetRandInt(vNew.size()), GetRandInt(vNew.size()), GetRandInt(vNew.size()), GetRandInt(vNew.size())};
  int nI = 0;
  int nOldest = -1;

  for (std::set<int>::iterator it = vNew.begin(); it != vNew.end(); it++)
  {
    if (nI == n[0] || nI == n[1] || nI == n[2] || nI == n[3])
    {
      assert(nOldest == -1 || mapInfo.count(*it) == 1);

      if (nOldest == -1 || mapInfo[*it].nTime < mapInfo[nOldest].nTime)
      {
        nOldest = *it;
      }
    }

    nI++;
  }

  assert(mapInfo.count(nOldest) == 1);
  CAddrInfo &info = mapInfo[nOldest];

  if (--info.nRefCount == 0)
  {
    SwapRandom(info.nRandomPos, vRandom.size()-1);
    vRandom.pop_back();
    mapAddr.erase(info);
    mapInfo.erase(nOldest);
    nNew--;
  }

  vNew.erase(nOldest);
  return 1;
}
void CAddrMan::MakeTried(CAddrInfo& info, int nId, int nOrigin)
{
  assert(vvNew[nOrigin].count(nId) == 1);

  for (std::vector<std::set<int> >::iterator it = vvNew.begin(); it != vvNew.end(); it++)
  {
    if ((*it).erase(nId))
    {
      info.nRefCount--;
    }
  }

  nNew--;
  assert(info.nRefCount == 0);
  int nKBucket = info.GetTriedBucket(nKey);
  std::vector<int> &vTried = vvTried[nKBucket];

  if (vTried.size() < ADDRMAN_TRIED_BUCKET_SIZE)
  {
    vTried.push_back(nId);
    nTried++;
    info.fInTried = true;
    return;
  }

  int nPos = SelectTried(nKBucket);
  assert(mapInfo.count(vTried[nPos]) == 1);
  int nUBucket = mapInfo[vTried[nPos]].GetNewBucket(nKey);
  std::set<int> &vNew = vvNew[nUBucket];
  CAddrInfo& infoOld = mapInfo[vTried[nPos]];
  infoOld.fInTried = false;
  infoOld.nRefCount = 1;

  if (vNew.size() < ADDRMAN_NEW_BUCKET_SIZE)
  {
    vNew.insert(vTried[nPos]);
  }
  else
  {
    vvNew[nOrigin].insert(vTried[nPos]);
  }

  nNew++;
  vTried[nPos] = nId;
  info.fInTried = true;
  return;
}
void CAddrMan::Good_(const CService &addr, int64_t nTime)
{
  int nId;
  CAddrInfo *pinfo = Find(addr, &nId);

  if (!pinfo)
  {
    return;
  }

  CAddrInfo &info = *pinfo;

  if (info != addr)
  {
    return;
  }

  info.nLastSuccess = nTime;
  info.nLastTry = nTime;
  info.nTime = nTime;
  info.nAttempts = 0;

  if (info.fInTried)
  {
    return;
  }

  int nRnd = GetRandInt(vvNew.size());
  int nUBucket = -1;

  for (unsigned int n = 0; n < vvNew.size(); n++)
  {
    int nB = (n+nRnd) % vvNew.size();
    std::set<int> &vNew = vvNew[nB];

    if (vNew.count(nId))
    {
      nUBucket = nB;
      break;
    }
  }

  if (nUBucket == -1)
  {
    return;
  }

  printf("Moving %s to tried\n", addr.ToString().c_str());
  MakeTried(info, nId, nUBucket);
}
bool CAddrMan::Add_(const CAddress &addr, const CNetAddr& source, int64_t nTimePenalty)
{
  if (!addr.IsRoutable())
  {
    return false;
  }

  bool fNew = false;
  int nId;
  CAddrInfo *pinfo = Find(addr, &nId);

  if (pinfo)
  {
    bool fCurrentlyOnline = (GetAdjustedTime() - addr.nTime < 24 * 60 * 60);
    int64_t nUpdateInterval = (fCurrentlyOnline ? 60 * 60 : 24 * 60 * 60);

    if (addr.nTime && (!pinfo->nTime || pinfo->nTime < addr.nTime - nUpdateInterval - nTimePenalty))
    {
      pinfo->nTime = max((int64_t)0, addr.nTime - nTimePenalty);
    }

    pinfo->nServices |= addr.nServices;

    if (!addr.nTime || (pinfo->nTime && addr.nTime <= pinfo->nTime))
    {
      return false;
    }

    if (pinfo->fInTried)
    {
      return false;
    }

    if (pinfo->nRefCount == ADDRMAN_NEW_BUCKETS_PER_ADDRESS)
    {
      return false;
    }

    int nFactor = 1;

    for (int n=0; n<pinfo->nRefCount; n++)
    {
      nFactor *= 2;
    }

    if (nFactor > 1 && (GetRandInt(nFactor) != 0))
    {
      return false;
    }
  }
  else
  {
    pinfo = Create(addr, source, &nId);
    pinfo->nTime = max((int64_t)0, (int64_t)pinfo->nTime - nTimePenalty);
    nNew++;
    fNew = true;
  }

  int nUBucket = pinfo->GetNewBucket(nKey, source);
  std::set<int> &vNew = vvNew[nUBucket];

  if (!vNew.count(nId))
  {
    pinfo->nRefCount++;

    if (vNew.size() == ADDRMAN_NEW_BUCKET_SIZE)
    {
      ShrinkNew(nUBucket);
    }

    vvNew[nUBucket].insert(nId);
  }

  return fNew;
}
void CAddrMan::Attempt_(const CService &addr, int64_t nTime)
{
  CAddrInfo *pinfo = Find(addr);

  if (!pinfo)
  {
    return;
  }

  CAddrInfo &info = *pinfo;

  if (info != addr)
  {
    return;
  }

  info.nLastTry = nTime;
  info.nAttempts++;
}
CAddress CAddrMan::Select_(int nUnkBias)
{
  if (size() == 0)
  {
    return CAddress();
  }

  double nCorTried = sqrt((double)nTried) * (100.0 - nUnkBias);
  double nCorNew = sqrt((double)nNew) * nUnkBias;

  if ((nCorTried + nCorNew)*GetRandInt(1<<30)/(1<<30) < nCorTried)
  {
    double fChanceFactor = 1.0;

    while(1)
    {
      int nKBucket = GetRandInt(vvTried.size());
      std::vector<int> &vTried = vvTried[nKBucket];

      if (vTried.size() == 0)
      {
        continue;
      }

      int nPos = GetRandInt(vTried.size());
      assert(mapInfo.count(vTried[nPos]) == 1);
      CAddrInfo &info = mapInfo[vTried[nPos]];

      if (GetRandInt(1<<30) < fChanceFactor*info.GetChance()*(1<<30))
      {
        return info;
      }

      fChanceFactor *= 1.2;
    }
  }
  else
  {
    double fChanceFactor = 1.0;

    while(1)
    {
      int nUBucket = GetRandInt(vvNew.size());
      std::set<int> &vNew = vvNew[nUBucket];

      if (vNew.size() == 0)
      {
        continue;
      }

      int nPos = GetRandInt(vNew.size());
      std::set<int>::iterator it = vNew.begin();

      while (nPos--)
      {
        it++;
      }

      assert(mapInfo.count(*it) == 1);
      CAddrInfo &info = mapInfo[*it];

      if (GetRandInt(1<<30) < fChanceFactor*info.GetChance()*(1<<30))
      {
        return info;
      }

      fChanceFactor *= 1.2;
    }
  }
}
#ifdef DEBUG_ADDRMAN
int CAddrMan::Check_()
{
  std::set<int> setTried;
  std::map<int, int> mapNew;

  if (vRandom.size() != nTried + nNew)
  {
    return -7;
  }

  for (std::map<int, CAddrInfo>::iterator it = mapInfo.begin(); it != mapInfo.end(); it++)
  {
    int n = (*it).first;
    CAddrInfo &info = (*it).second;

    if (info.fInTried)
    {
      if (!info.nLastSuccess)
      {
        return -1;
      }

      if (info.nRefCount)
      {
        return -2;
      }

      setTried.insert(n);
    }
    else
    {
      if (info.nRefCount < 0 || info.nRefCount > ADDRMAN_NEW_BUCKETS_PER_ADDRESS)
      {
        return -3;
      }

      if (!info.nRefCount)
      {
        return -4;
      }

      mapNew[n] = info.nRefCount;
    }

    if (mapAddr[info] != n)
    {
      return -5;
    }

    if (info.nRandomPos<0 || info.nRandomPos>=vRandom.size() || vRandom[info.nRandomPos] != n)
    {
      return -14;
    }

    if (info.nLastTry < 0)
    {
      return -6;
    }

    if (info.nLastSuccess < 0)
    {
      return -8;
    }
  }

  if (setTried.size() != nTried)
  {
    return -9;
  }

  if (mapNew.size() != nNew)
  {
    return -10;
  }

  for (int n=0; n<vvTried.size(); n++)
  {
    std::vector<int> &vTried = vvTried[n];

    for (std::vector<int>::iterator it = vTried.begin(); it != vTried.end(); it++)
    {
      if (!setTried.count(*it))
      {
        return -11;
      }

      setTried.erase(*it);
    }
  }

  for (int n=0; n<vvNew.size(); n++)
  {
    std::set<int> &vNew = vvNew[n];

    for (std::set<int>::iterator it = vNew.begin(); it != vNew.end(); it++)
    {
      if (!mapNew.count(*it))
      {
        return -12;
      }

      if (--mapNew[*it] == 0)
      {
        mapNew.erase(*it);
      }
    }
  }

  if (setTried.size())
  {
    return -13;
  }

  if (mapNew.size())
  {
    return -15;
  }

  return 0;
}
#endif
void CAddrMan::GetAddr_(std::vector<CAddress> &vAddr)
{
  int nNodes = ADDRMAN_GETADDR_MAX_PCT*vRandom.size()/100;

  if (nNodes > ADDRMAN_GETADDR_MAX)
  {
    nNodes = ADDRMAN_GETADDR_MAX;
  }

  for (int n = 0; n<nNodes; n++)
  {
    int nRndPos = GetRandInt(vRandom.size() - n) + n;
    SwapRandom(n, nRndPos);
    assert(mapInfo.count(vRandom[n]) == 1);
    vAddr.push_back(mapInfo[vRandom[n]]);
  }
}
void CAddrMan::Connected_(const CService &addr, int64_t nTime)
{
  CAddrInfo *pinfo = Find(addr);

  if (!pinfo)
  {
    return;
  }

  CAddrInfo &info = *pinfo;

  if (info != addr)
  {
    return;
  }

  int64_t nUpdateInterval = 20 * 60;

  if (nTime - info.nTime > nUpdateInterval)
  {
    info.nTime = nTime;
  }
}
