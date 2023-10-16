#ifndef TX_MEMPOOL_H
#define TX_MEMPOOL_H
#include<cstdint>
#include "Serialize.h"
#include "BigNum.h"
#include "Sync.h"
#include "Net.h"
#include "Script.h"
#include "InputPoint.h"
#include "OutputPoint.h"
#include "ccoin/transaction.h"

class CTxMemPool
{
public:
  static unsigned int nTransactionsUpdated;
  mutable CCriticalSection cs;
  std::map<uint256, CTransaction> mapTx;
  std::map<COutPoint, CInPoint> mapNextTx;

  bool addUnchecked(const uint256& hash, CTransaction &tx);
  bool remove(const CTransaction &tx, bool fRecursive = false);
  bool removeConflicts(const CTransaction &tx);
  void clear();
  void queryHashes(std::vector<uint256>& vtxid);
  bool accept(CTransaction &tx,
                        bool* pfMissingInputs);

  unsigned long size() const
  {
    LOCK(cs);
    return mapTx.size();
  }

  bool exists(uint256 hash) const
  {
    LOCK(cs);
    return (mapTx.count(hash) != 0);
  }

  bool lookup(uint256 hash, CTransaction& result) const
  {
    LOCK(cs);
    std::map<uint256, CTransaction>::const_iterator i = mapTx.find(hash);

    if (i == mapTx.end())
    {
      return false;
    }

    result = i->second;
    return true;
  }
};


#endif
