
#include "transaction.h"
#include "txdb-leveldb.h"
#include "configuration_state.h"
#include "block.h"
#include "dions.h"
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
bool CTransaction::DisconnectInputs(CTxDB& txdb)
{
  if (!IsCoinBase())
  {
    BOOST_FOREACH(const CTxIn& txin, vin)
    {
      COutPoint prevout = txin.prevout;
      CTxIndex txindex;

      if (!txdb.ReadTxIndex(prevout.hash, txindex))
      {
        return error("DisconnectInputs() : ReadTxIndex failed");
      }

      if (prevout.n >= txindex.vSpent.size())
      {
        return error("DisconnectInputs() : prevout.n out of range");
      }

      txindex.vSpent[prevout.n].SetNull();

      if (!txdb.UpdateTxIndex(prevout.hash, txindex))
      {
        return error("DisconnectInputs() : UpdateTxIndex failed");
      }
    }
  }

  txdb.EraseTxIndex(*this);
  return true;
}
bool CTransaction::FetchInputs(CTxDB& txdb, const map<uint256, CTxIndex>& mapTestPool,
                               bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid)
{
  fInvalid = false;

  if (IsCoinBase())
  {
    return true;
  }

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    COutPoint prevout = vin[i].prevout;

    if (inputsRet.count(prevout.hash))
    {
      continue;
    }

    CTxIndex& txindex = inputsRet[prevout.hash].first;
    bool fFound = true;

    if ((fBlock || fMiner) && mapTestPool.count(prevout.hash))
    {
      txindex = mapTestPool.find(prevout.hash)->second;
    }
    else
    {
      fFound = txdb.ReadTxIndex(prevout.hash, txindex);
    }

    if (!fFound && (fBlock || fMiner))
    {
      return fMiner ? false : error("FetchInputs() : %s prev tx %s index entry not found", GetHash().ToString().substr(0,10).c_str(), prevout.hash.ToString().substr(0,10).c_str());
    }

    CTransaction& txPrev = inputsRet[prevout.hash].second;

    if (!fFound || txindex.pos == CDiskTxPos(1,1,1))
    {
      if (!mempool.lookup(prevout.hash, txPrev))
      {
        return error("FetchInputs() : %s mempool Tx prev not found %s", GetHash().ToString().substr(0,10).c_str(), prevout.hash.ToString().substr(0,10).c_str());
      }

      if (!fFound)
      {
        txindex.vSpent.resize(txPrev.vout.size());
      }
    }
    else
    {
      if (!txPrev.ReadFromDisk(txindex.pos))
      {
        return error("FetchInputs() : %s ReadFromDisk prev tx %s failed", GetHash().ToString().substr(0,10).c_str(), prevout.hash.ToString().substr(0,10).c_str());
      }
    }
  }

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    const COutPoint prevout = vin[i].prevout;
    assert(inputsRet.count(prevout.hash) != 0);
    const CTxIndex& txindex = inputsRet[prevout.hash].first;
    const CTransaction& txPrev = inputsRet[prevout.hash].second;

    if (prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
    {
      fInvalid = true;
      return DoS(100, error("FetchInputs() : %s prevout.n out of range %d %" PRIszu " %" PRIszu " prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str()));
    }
  }

  return true;
}
const CTxOut& CTransaction::GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const
{
  MapPrevTx::const_iterator mi = inputs.find(input.prevout.hash);

  if (mi == inputs.end())
  {
    throw std::runtime_error("CTransaction::GetOutputFor() : prevout.hash not found");
  }

  const CTransaction& txPrev = (mi->second).second;

  if (input.prevout.n >= txPrev.vout.size())
  {
    throw std::runtime_error("CTransaction::GetOutputFor() : prevout.n out of range");
  }

  return txPrev.vout[input.prevout.n];
}
int64_t CTransaction::GetValueIn(const MapPrevTx& inputs) const
{
  if (IsCoinBase())
  {
    return 0;
  }

  int64_t nResult = 0;

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    nResult += GetOutputFor(vin[i], inputs).nValue;
  }

  return nResult;
}
unsigned int CTransaction::GetP2SHSigOpCount(const MapPrevTx& inputs) const
{
  if (IsCoinBase())
  {
    return 0;
  }

  unsigned int nSigOps = 0;

  for (unsigned int i = 0; i < vin.size(); i++)
  {
    const CTxOut& prevout = GetOutputFor(vin[i], inputs);

    if (prevout.scriptPubKey.IsPayToScriptHash())
    {
      nSigOps += prevout.scriptPubKey.GetSigOpCount(vin[i].scriptSig);
    }
  }

  return nSigOps;
}
bool CTransaction::GetCoinAge(CTxDB& txdb, uint64_t& nCoinAge) const
{
  CBigNum bnCentSecond = 0;
  nCoinAge = 0;

  if (IsCoinBase())
  {
    return true;
  }

  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    CTransaction txPrev;
    CTxIndex txindex;

    if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
    {
      continue;
    }

    if (nTime < txPrev.nTime)
    {
      return false;
    }

    CBlock block;

    if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
    {
      return false;
    }

    if (block.GetBlockTime() + nStakeMinAge > nTime)
    {
      continue;
    }

    int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
    bnCentSecond += CBigNum(nValueIn) * (nTime-txPrev.nTime) / CENT;

    if (fDebug && GetBoolArg("-printcoinage"))
    {
      printf("coin age nValueIn=%" PRId64 " nTimeDiff=%d bnCentSecond=%s\n", nValueIn, nTime - txPrev.nTime, bnCentSecond.ToString().c_str());
    }
  }
  CBigNum bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);

  if (fDebug && GetBoolArg("-printcoinage"))
  {
    printf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());
  }

  nCoinAge = bnCoinDay.getuint64();
  return true;
}
bool CTransaction::GetCoinAge(CTxDB& txdb, const CBlockIndex* pindexPrev, uint64_t& nCoinAge) const
{
  CBigNum bnCentSecond = 0;
  nCoinAge = 0;

  if (IsCoinBase())
  {
    return true;
  }

  BOOST_FOREACH(const CTxIn& txin, vin)
  {
    CTransaction txPrev;
    CTxIndex txindex;

    if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
    {
      continue;
    }

    if (nTime < txPrev.nTime)
    {
      return false;
    }

    if(V3(pindexPrev->nHeight))
    {
      int nSpendDepth;

      if(!minBase(txindex, pindexPrev, ConfigurationState::nStakeMinConfirmations - 1, nSpendDepth))
      {
        continue;
      }
    }
    else
    {
      CBlock block;

      if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
      {
        return false;
      }

      if (block.GetBlockTime() + nStakeMinAge > nTime)
      {
        continue;
      }

      int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
      bnCentSecond += CBigNum(nValueIn) * (nTime-txPrev.nTime) / CENT;

      if (fDebug && GetBoolArg("-printcoinage"))
      {
        printf("coin age nValueIn=%" PRId64 " nTimeDiff=%d bnCentSecond=%s\n", nValueIn, nTime - txPrev.nTime, bnCentSecond.ToString().c_str());
      }
    }
  }
  CBigNum bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);

  if (fDebug && GetBoolArg("-printcoinage"))
  {
    printf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());
  }

  nCoinAge = bnCoinDay.getuint64();
  return true;
}
