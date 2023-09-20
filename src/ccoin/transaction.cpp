#include "transaction.h"
#include "txdb-leveldb.h"
#include "configuration_state.h"
#include "block.h"
#include "dions/dions.h"
#include "checkpoints.h"
extern int nCoinbaseMaturity;

extern inline bool MoneyRange(int64_t nValue);
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
bool CTransaction::CheckTransaction() const
{
  if (vin.empty())
  {
    return DoS(10, error("CTransaction::CheckTransaction() : vin empty"));
  }

  if (vout.empty())
  {
    return DoS(10, error("CTransaction::CheckTransaction() : vout empty"));
  }

  if (::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > CBlock::MAX_BLOCK_SIZE)
  {
    return DoS(100, error("CTransaction::CheckTransaction() : size limits failed"));
  }

  int64_t nValueOut = 0;

  for (unsigned int i = 0; i < vout.size(); i++)
  {
    const CTxOut& txout = vout[i];

    if (txout.IsEmpty() && !IsCoinBase() && !IsCoinStake())
    {
      return DoS(100, error("CTransaction::CheckTransaction() : txout empty for user transaction"));
    }

    if (txout.nValue < 0)
    {
      return DoS(100, error("CTransaction::CheckTransaction() : txout.nValue negative"));
    }

    if (txout.nValue > ConfigurationState::MAX_MONEY)
    {
      return DoS(100, error("CTransaction::CheckTransaction() : txout.nValue too high"));
    }

    nValueOut += txout.nValue;

    if (!MoneyRange(nValueOut))
    {
      return DoS(100, error("CTransaction::CheckTransaction() : txout total out of range"));
    }
  }

  set<COutPoint> vInOutPoints;
  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    if (vInOutPoints.count(txin.prevout))
    {
      return false;
    }

    vInOutPoints.insert(txin.prevout);
  }

  if (IsCoinBase())
  {
    if (vin[0].scriptSig.size() < 2 || vin[0].scriptSig.size() > 100)
    {
      return DoS(100, error("CTransaction::CheckTransaction() : coinbase script size is invalid"));
    }
  }
  else
  {
    BOOST_FOREACH(const CTxIn& txin, vin)

    if (txin.prevout.IsNull())
    {
      return DoS(10, error("CTransaction::CheckTransaction() : prevout is null"));
    }
  }

  return true;
}
int64_t CTransaction::GetMinFee(unsigned int nBlockSize, enum GetMinFee_mode mode, unsigned int nBytes) const
{
  int64_t nBaseFee = (mode == GMF_RELAY) ? MIN_RELAY_TX_FEE : MIN_TX_FEE;
  unsigned int nNewBlockSize = nBlockSize + nBytes;
  int64_t nMinFee = (1 + (int64_t)nBytes / 1000) * nBaseFee;

  if (nMinFee < nBaseFee)
  {
    BOOST_FOREACH(const CTxOut& txout, vout)

    if (txout.nValue < CENT)
    {
      nMinFee = nBaseFee;
    }
  }

  if (nBlockSize != 1 && nNewBlockSize >= CBlock::MAX_BLOCK_SIZE_GEN/2)
  {
    if (nNewBlockSize >= CBlock::MAX_BLOCK_SIZE_GEN)
    {
      return ConfigurationState::MAX_MONEY;
    }

    nMinFee *= CBlock::MAX_BLOCK_SIZE_GEN / (CBlock::MAX_BLOCK_SIZE_GEN - nNewBlockSize);
  }

  if (!MoneyRange(nMinFee))
  {
    nMinFee = ConfigurationState::MAX_MONEY;
  }

  return nMinFee;
}
bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet)
{
  SetNull();

  if (!txdb.ReadTxIndex(prevout.hash, txindexRet))
  {
    return false;
  }

  if (!ReadFromDisk(txindexRet.pos))
  {
    return false;
  }

  if (prevout.n >= vout.size())
  {
    SetNull();
    return false;
  }

  return true;
}
bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout)
{
  CTxIndex txindex;
  return ReadFromDisk(txdb, prevout, txindex);
}
bool CTransaction::ReadFromDisk(COutPoint prevout)
{
  CTxDB txdb("r");
  CTxIndex txindex;
  return ReadFromDisk(txdb, prevout, txindex);
}
bool CTransaction::AreInputsStandard(const MapPrevTx& mapInputs) const
{
  if (IsCoinBase())
  {
    return true;
  }

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    const CTxOut& prev = GetOutputFor(vin[i], mapInputs);
    vector<vector<unsigned char> > vSolutions;
    txnouttype whichType;
    const CScript& prevScript = prev.scriptPubKey;
    int op;
    std::vector<vchType> vvch;
    CScript::const_iterator pc = prevScript.begin ();
    CScript rawScript;

    if (aliasScript(prevScript, op, vvch, pc))
    {
      rawScript = CScript(pc, prevScript.end ());
    }
    else
    {
      rawScript = prevScript;
    }

    if (!Solver(rawScript, whichType, vSolutions))
    {
      return false;
    }

    int nArgsExpected = ScriptSigArgsExpected(whichType, vSolutions);

    if (nArgsExpected < 0)
    {
      return false;
    }

    vector<vector<unsigned char> > stack;

    if(!EvalScript(stack, vin[i].scriptSig, *this, i, SCRIPT_VERIFY_NONE, 0))
    {
      return false;
    }

    if (whichType == TX_SCRIPTHASH)
    {
      if (stack.empty())
      {
        return false;
      }

      CScript subscript(stack.back().begin(), stack.back().end());
      vector<vector<unsigned char> > vSolutions2;
      txnouttype whichType2;

      if (!Solver(subscript, whichType2, vSolutions2))
      {
        return false;
      }

      if (whichType2 == TX_SCRIPTHASH)
      {
        return false;
      }

      int tmpExpected;
      tmpExpected = ScriptSigArgsExpected(whichType2, vSolutions2);

      if (tmpExpected < 0)
      {
        return false;
      }

      nArgsExpected += tmpExpected;
    }

    if (stack.size() != (unsigned int)nArgsExpected)
    {
      return false;
    }
  }

  return true;
}
unsigned int
CTransaction::GetLegacySigOpCount() const
{
  unsigned int nSigOps = 0;
  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    nSigOps += txin.scriptSig.GetSigOpCount(false);
  }
  BOOST_FOREACH(const CTxOut& txout, vout)
  {
    nSigOps += txout.scriptPubKey.GetSigOpCount(false);
  }
  return nSigOps;
}
