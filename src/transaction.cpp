#include "transaction.h"
#include "block.h"
#include "dions.h"
#include "checkpoints.h"
extern int nCoinbaseMaturity;
bool CTransaction::ConnectInputs(CTxDB& txdb, MapPrevTx inputs, map<uint256, CTxIndex>& mapTestPool, CDiskTxPos& posThisTx,
                                 CBlockIndex* pindexBlock, bool fBlock, bool fMiner, int flags)
{
  if (!IsCoinBase())
  {
    vector<CTransaction> vTxPrev;
    vector<CTxIndex> vTxindex;
    int64_t nValueIn = 0;
    int64_t nFees = 0;

    for (unsigned int i = 0; i < vin.size(); i++)
    {
      COutPoint prevout = vin[i].prevout;
      assert(inputs.count(prevout.hash) > 0);
      CTxIndex& txindex = inputs[prevout.hash].first;
      CTransaction& txPrev = inputs[prevout.hash].second;

      if (prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
      {
        return DoS(100, error("ConnectInputs() : %s prevout.n out of range %d %" PRIszu " %" PRIszu " prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str()));
      }

      if (txPrev.IsCoinBase() || txPrev.IsCoinStake())
        for (const CBlockIndex* pindex = pindexBlock; pindex && pindexBlock->nHeight - pindex->nHeight < nCoinbaseMaturity; pindex = pindex->pprev)
          if (pindex->nBlockPos == txindex.pos.nBlockPos && pindex->nFile == txindex.pos.nFile)
          {
            return error("ConnectInputs() : tried to spend %s at depth %d", txPrev.IsCoinBase() ? "coinbase" : "coinstake", pindexBlock->nHeight - pindex->nHeight);
          }

      if (txPrev.nTime > nTime)
      {
        return DoS(100, error("ConnectInputs() : transaction timestamp earlier than input transaction"));
      }

      nValueIn += txPrev.vout[prevout.n].nValue;

      if (!MoneyRange(txPrev.vout[prevout.n].nValue) || !MoneyRange(nValueIn))
      {
        return DoS(100, error("ConnectInputs() : txin values out of range"));
      }

      vTxPrev.push_back(txPrev);
      vTxindex.push_back(txindex);
    }

    for (unsigned int i = 0; i < vin.size(); i++)
    {
      COutPoint prevout = vin[i].prevout;
      assert(inputs.count(prevout.hash) > 0);
      CTxIndex& txindex = inputs[prevout.hash].first;
      CTransaction& txPrev = inputs[prevout.hash].second;

      if (!txindex.vSpent[prevout.n].IsNull() && txPrev.vout[prevout.n].nValue > 0)
      {
        return fMiner ? false : error("ConnectInputs() : %s prev tx already used at %s", GetHash().ToString().substr(0,10).c_str(), txindex.vSpent[prevout.n].ToString().c_str());
      }

      if (!(fBlock && (nBestHeight < Checkpoints::GetTotalBlocksEstimate())))
      {
        if (!VerifySignature(txPrev, *this, i, flags, 0))
        {
          if (flags & STANDARD_NOT_MANDATORY_VERIFY_FLAGS)
          {
            if (VerifySignature(txPrev, *this, i, flags & ~STANDARD_NOT_MANDATORY_VERIFY_FLAGS, 0))
            {
              return error("ConnectInputs() : %s non-mandatory VerifySignature failed", GetHash().ToString().c_str());
            }
          }

          return DoS(100,error("ConnectInputs() : %s VerifySignature failed", GetHash().ToString().substr(0,10).c_str()));
        }
      }

      txindex.vSpent[prevout.n] = posThisTx;

      if (fBlock || fMiner)
      {
        mapTestPool[prevout.hash] = txindex;
      }
    }

    if (!IsCoinStake())
    {
      if (nValueIn < GetValueOut())
      {
        return DoS(100, error("ConnectInputs() : %s value in < value out", GetHash().ToString().substr(0,10).c_str()));
      }

      if (!ConnectInputsPost (mapTestPool, *this, vTxPrev, vTxindex,
                              pindexBlock, posThisTx, fBlock, fMiner))
      {
        return DoS(100, error("pre forward %s\n", GetHash().ToString().substr(0,10).c_str()));
      }

      int64_t nTxFee = nValueIn - GetValueOut();

      if (nTxFee < 0)
      {
        return DoS(100, error("ConnectInputs() : %s nTxFee < 0", GetHash().ToString().substr(0,10).c_str()));
      }

      if (nTxFee < GetMinFee())
      {
        return fBlock? DoS(100, error("ConnectInputs() : %s not paying required fee=%s, paid=%s", GetHash().ToString().substr(0,10).c_str(), FormatMoney(GetMinFee()).c_str(), FormatMoney(nTxFee).c_str())) : false;
      }

      nFees += nTxFee;

      if (!MoneyRange(nFees))
      {
        return DoS(100, error("ConnectInputs() : nFees out of range"));
      }
    }
  }

  return true;
}
