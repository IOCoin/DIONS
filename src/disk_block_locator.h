#ifndef DISK_BLOCK_LOCATOR_H
#define DISK_BLOCK_LOCATOR_H

#include "block_index.h"

class CBlockLocator
{
protected:
  std::vector<uint256> vHave;
public:

  CBlockLocator()
  {
  }

  explicit CBlockLocator(const CBlockIndex* pindex)
  {
    Set(pindex);
  }

  explicit CBlockLocator(uint256 hashBlock)
  {
    std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);

    if (mi != mapBlockIndex.end())
    {
      Set((*mi).second);
    }
  }

  CBlockLocator(const std::vector<uint256>& vHaveIn)
  {
    vHave = vHaveIn;
  }

  IMPLEMENT_SERIALIZE
  (

    if (!(nType & SER_GETHASH))
    READWRITE(nVersion);
    READWRITE(vHave);
  )

    void SetNull()
  {
    vHave.clear();
  }

  bool IsNull()
  {
    return vHave.empty();
  }

  void Set(const CBlockIndex* pindex)
  {
    vHave.clear();
    int nStep = 1;

    while (pindex)
    {
      vHave.push_back(pindex->GetBlockHash());

      for (int i = 0; pindex && i < nStep; i++)
      {
        pindex = pindex->pprev;
      }

      if (vHave.size() > 10)
      {
        nStep *= 2;
      }
    }

    vHave.push_back((!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
  }

  int GetDistanceBack()
  {
    int nDistance = 0;
    int nStep = 1;
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);

      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;

        if (pindex->IsInMainChain())
        {
          return nDistance;
        }
      }

      nDistance += nStep;

      if (nDistance > 10)
      {
        nStep *= 2;
      }
    }
    return nDistance;
  }

  CBlockIndex* GetBlockIndex()
  {
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);

      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;

        if (pindex->IsInMainChain())
        {
          return pindex;
        }
      }
    }
    return pindexGenesisBlock;
  }

  uint256 GetBlockHash()
  {
    BOOST_FOREACH(const uint256& hash, vHave)
    {
      std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);

      if (mi != mapBlockIndex.end())
      {
        CBlockIndex* pindex = (*mi).second;

        if (pindex->IsInMainChain())
        {
          return hash;
        }
      }
    }
    return (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet);
  }

  int GetHeight()
  {
    CBlockIndex* pindex = GetBlockIndex();

    if (!pindex)
    {
      return 0;
    }

    return pindex->nHeight;
  }
};

#endif
