#include "txmempool.h"
#include "txdb-leveldb.h"
#include "wallet.h"

extern void RemoveFromMemoryPoolPost(const CTransaction&);
extern inline bool V3(int nHeight);
extern bool IsStandardTx(const CTransaction& tx);
extern bool IsFinalTx(const CTransaction &tx, int nBlockHeight, int64_t nBlockTime);
extern bool AcceptToMemoryPoolPost(const CTransaction& tx);
extern CCriticalSection cs_main;
unsigned int CTxMemPool::nTransactionsUpdated = 0;
extern set<__wx__*> setpwalletRegistered;
bool static IsFromMe(CTransaction& tx)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)

  if (pwallet->IsFromMe(tx))
  {
    return true;
  }

  return false;
}
void static EraseFromWallets(uint256 hash)
{
  BOOST_FOREACH(__wx__* pwallet, setpwalletRegistered)
  pwallet->EraseFromWallet(hash);
}
bool CTxMemPool::addUnchecked(const uint256& hash, CTransaction &tx)
{
  {
    mapTx[hash] = tx;

    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
      mapNextTx[tx.vin[i].prevout] = CInPoint(&mapTx[hash], i);
    }

    nTransactionsUpdated++;
  }
  return true;
}
bool CTxMemPool::remove(const CTransaction &tx, bool fRecursive)
{
  RemoveFromMemoryPoolPost(tx);
  {
    LOCK(cs);
    uint256 hash = tx.GetHash();

    if (mapTx.count(hash))
    {
      if (fRecursive)
      {
        for (unsigned int i = 0; i < tx.vout.size(); i++)
        {
          std::map<COutPoint, CInPoint>::iterator it = mapNextTx.find(COutPoint(hash, i));

          if (it != mapNextTx.end())
          {
            remove(*it->second.ptx, true);
          }
        }
      }

      BOOST_FOREACH(const CTxIn& txin, tx.vin)
      mapNextTx.erase(txin.prevout);
      mapTx.erase(hash);
      nTransactionsUpdated++;
    }
  }
  return true;
}
bool CTxMemPool::removeConflicts(const CTransaction &tx)
{
  LOCK(cs);
  BOOST_FOREACH(const CTxIn &txin, tx.vin)
  {
    std::map<COutPoint, CInPoint>::iterator it = mapNextTx.find(txin.prevout);

    if (it != mapNextTx.end())
    {
      const CTransaction &txConflict = *it->second.ptx;

      if (txConflict != tx)
      {
        remove(txConflict, true);
      }
    }
  }
  return true;
}
void CTxMemPool::clear()
{
  LOCK(cs);
  mapTx.clear();
  mapNextTx.clear();
  ++nTransactionsUpdated;
}
void CTxMemPool::queryHashes(std::vector<uint256>& vtxid)
{
  vtxid.clear();
  LOCK(cs);
  vtxid.reserve(mapTx.size());

  for (map<uint256, CTransaction>::iterator mi = mapTx.begin(); mi != mapTx.end(); ++mi)
  {
    vtxid.push_back((*mi).first);
  }
}
bool CTxMemPool::AcceptToMemoryPool(CTxMemPool& pool, CTransaction &tx,
                        bool* pfMissingInputs)
{
  AssertLockHeld(cs_main);

  if (pfMissingInputs)
  {
    *pfMissingInputs = false;
  }

  if (!tx.CheckTransaction())
  {
    return error("AcceptToMemoryPool : CheckTransaction failed");
  }

  if (tx.IsCoinBase())
  {
    return tx.DoS(100, error("AcceptToMemoryPool : coinbase as individual tx"));
  }

  if (tx.IsCoinStake())
  {
    return tx.DoS(100, error("AcceptToMemoryPool : coinstake as individual tx"));
  }

  if (!fTestNet && tx.nVersion == CTransaction::DION_TX_VERSION && !V3(nBestHeight))
  {
    return error("AcceptToMemoryPool : type");
  }

  if (!IsStandardTx(tx) && tx.nVersion != CTransaction::DION_TX_VERSION)
  {
    return error("AcceptToMemoryPool : nonstandard transaction type");
  }

  uint256 hash = tx.GetHash();

  if (pool.exists(hash))
  {
    return false;
  }

  CTransaction* ptxOld = NULL;
  {
    LOCK(pool.cs);

    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
      COutPoint outpoint = tx.vin[i].prevout;

      if (pool.mapNextTx.count(outpoint))
      {
        return false;

        if (i != 0)
        {
          return false;
        }

        ptxOld = pool.mapNextTx[outpoint].ptx;

        if (IsFinalTx(*ptxOld))
        {
          return false;
        }

        if (!tx.IsNewerThan(*ptxOld))
        {
          return false;
        }

        for (unsigned int i = 0; i < tx.vin.size(); i++)
        {
          COutPoint outpoint = tx.vin[i].prevout;

          if (!pool.mapNextTx.count(outpoint) || pool.mapNextTx[outpoint].ptx != ptxOld)
          {
            return false;
          }
        }

        break;
      }
    }
  }
  {
    CTxDB txdb("r");

    if (txdb.ContainsTx(hash))
    {
      return false;
    }

    MapPrevTx mapInputs;
    map<uint256, CTxIndex> mapUnused;
    bool fInvalid = false;

    if (!tx.FetchInputs(txdb, mapUnused, false, false, mapInputs, fInvalid))
    {
      if (fInvalid)
      {
        return error("AcceptToMemoryPool : FetchInputs found invalid tx %s", hash.ToString().substr(0,10).c_str());
      }

      if (pfMissingInputs)
      {
        *pfMissingInputs = true;
      }

      return false;
    }

    if (!tx.AreInputsStandard(mapInputs))
    {
      return error("AcceptToMemoryPool : nonstandard transaction input");
    }

    int64_t nFees = tx.GetValueIn(mapInputs)-tx.GetValueOut();
    unsigned int nSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
    int64_t txMinFee = tx.GetMinFee(1000, GMF_RELAY, nSize);

    if (nFees < txMinFee)
      return error("AcceptToMemoryPool : not enough fees %s, %" PRId64 " < %" PRId64,
                   hash.ToString().c_str(),
                   nFees, txMinFee);

    if(nFees < CTransaction::MIN_RELAY_TX_FEE)
    {
      static CCriticalSection cs;
      static double dFreeCount;
      static int64_t nLastTime;
      int64_t nNow = GetTime();
      {
        LOCK(pool.cs);
        dFreeCount *= pow(1.0 - 1.0/600.0, (double)(nNow - nLastTime));
        nLastTime = nNow;

        if (dFreeCount > GetArg("-limitfreerelay", 15)*10*1000 && !IsFromMe(tx))
        {
          return error("AcceptToMemoryPool : free transaction rejected by rate limiter");
        }

        if (fDebug)
        {
          printf("Rate limit dFreeCount: %g => %g\n", dFreeCount, dFreeCount+nSize);
        }

        dFreeCount += nSize;
      }
    }

    CDiskTxPos cDiskTxPos = CDiskTxPos(1,1,1);

    if(!tx.ConnectInputs(txdb, mapInputs, mapUnused, cDiskTxPos, pindexBest, false, false, STANDARD_SCRIPT_VERIFY_FLAGS))
    {
      return error("AcceptToMemoryPool : ConnectInputs failed %s", hash.ToString().substr(0,10).c_str());
    }
  }
  AcceptToMemoryPoolPost(tx);
  {
    LOCK(pool.cs);

    if (ptxOld)
    {
      printf("AcceptToMemoryPool : replacing tx %s with new version\n", ptxOld->GetHash().ToString().c_str());
      pool.remove(*ptxOld);
    }

    pool.addUnchecked(hash, tx);
  }

  if (ptxOld)
  {
    EraseFromWallets(ptxOld->GetHash());
  }

  printf("AcceptToMemoryPool : accepted %s (poolsz %" PRIszu ")\n",
         hash.ToString().substr(0,10).c_str(),
         pool.mapTx.size());
  return true;
}
