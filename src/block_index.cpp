#include "BlockIndex.h"


uint256 CBlockIndex::GetBlockTrust() const
{
  CBigNum bnTarget;
  bnTarget.SetCompact(nBits);

  if (bnTarget <= 0)
  {
    return 0;
  }

  return ((CBigNum(1)<<256) / (bnTarget+1)).getuint256();
}
bool CBlockIndex::IsSuperMajority(int minVersion, const CBlockIndex* pstart, unsigned int nRequired, unsigned int nToCheck)
{
  unsigned int nFound = 0;

  for (unsigned int i = 0; i < nToCheck && nFound < nRequired && pstart != NULL; i++)
  {
    if (pstart->nVersion >= minVersion)
    {
      ++nFound;
    }

    pstart = pstart->pprev;
  }

  return (nFound >= nRequired);
}
