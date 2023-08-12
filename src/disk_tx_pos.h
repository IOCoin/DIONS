#ifndef DISK_TX_POS_H
#define DISK_TX_POS_H

enum GetMinFee_mode
{
  GMF_BLOCK,
  GMF_RELAY,
  GMF_SEND,
};
class CDiskTxPos
{
public:
  unsigned int nFile;
  unsigned int nBlockPos;
  unsigned int nTxPos;

  CDiskTxPos()
  {
    SetNull();
  }

  CDiskTxPos(unsigned int nFileIn, unsigned int nBlockPosIn, unsigned int nTxPosIn)
  {
    nFile = nFileIn;
    nBlockPos = nBlockPosIn;
    nTxPos = nTxPosIn;
  }

  IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
  void SetNull()
  {
    nFile = (unsigned int) -1;
    nBlockPos = 0;
    nTxPos = 0;
  }
  bool IsNull() const
  {
    return (nFile == (unsigned int) -1);
  }

  friend bool operator==(const CDiskTxPos& a, const CDiskTxPos& b)
  {
    return (a.nFile == b.nFile &&
            a.nBlockPos == b.nBlockPos &&
            a.nTxPos == b.nTxPos);
  }

  friend bool operator!=(const CDiskTxPos& a, const CDiskTxPos& b)
  {
    return !(a == b);
  }


  std::string ToString() const
  {
    if (IsNull())
    {
      return "null";
    }
    else
    {
      return strprintf("(nFile=%u, nBlockPos=%u, nTxPos=%u)", nFile, nBlockPos, nTxPos);
    }
  }

  void print() const
  {
    printf("%s", ToString().c_str());
  }
};

class CTxIndex
{
public:
  CDiskTxPos pos;
  std::vector<CDiskTxPos> vSpent;

  CTxIndex()
  {
    SetNull();
  }

  CTxIndex(const CDiskTxPos& posIn, unsigned int nOutputs)
  {
    pos = posIn;
    vSpent.resize(nOutputs);
  }

  IMPLEMENT_SERIALIZE
  (

    if (!(nType & SER_GETHASH))
    READWRITE(nVersion);
    READWRITE(pos);
    READWRITE(vSpent);
  )

  void SetNull()
  {
    pos.SetNull();
    vSpent.clear();
  }

  bool IsNull()
  {
    return pos.IsNull();
  }

  friend bool operator==(const CTxIndex& a, const CTxIndex& b)
  {
    return (a.pos == b.pos &&
            a.vSpent == b.vSpent);
  }

  friend bool operator!=(const CTxIndex& a, const CTxIndex& b)
  {
    return !(a == b);
  }
  int GetDepthInMainChain() const;

};
typedef std::map<uint256, std::pair<CTxIndex, CTransaction> > MapPrevTx;

class CTxIn
{
public:
  COutPoint prevout;
  CScript scriptSig;
  unsigned int nSequence;

  CTxIn()
  {
    nSequence = std::numeric_limits<unsigned int>::max();
  }

  explicit CTxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  CTxIn(uint256 hashPrevTx, unsigned int nOut, CScript scriptSigIn=CScript(), unsigned int nSequenceIn=std::numeric_limits<unsigned int>::max())
  {
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(prevout);
    READWRITE(scriptSig);
    READWRITE(nSequence);
  )

  bool IsFinal() const
  {
    return (nSequence == std::numeric_limits<unsigned int>::max());
  }

  CTxIn& operator=(const CTxIn& o)
  {
    this->prevout = o.prevout;
    this->scriptSig = o.scriptSig;
    this->nSequence = o.nSequence;
    return *this;
  }

  friend bool operator==(const CTxIn& a, const CTxIn& b)
  {
    return (a.prevout == b.prevout &&
            a.scriptSig == b.scriptSig &&
            a.nSequence == b.nSequence);
  }

  friend bool operator!=(const CTxIn& a, const CTxIn& b)
  {
    return !(a == b);
  }

  std::string ToStringShort() const
  {
    return strprintf(" %s %d", prevout.hash.ToString().c_str(), prevout.n);
  }

  std::string ToString() const
  {
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();

    if (prevout.IsNull())
    {
      str += strprintf(", coinbase %s", HexStr(scriptSig).c_str());
    }
    else
    {
      str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24).c_str());
    }

    if (nSequence != std::numeric_limits<unsigned int>::max())
    {
      str += strprintf(", nSequence=%u", nSequence);
    }

    str += ")";
    return str;
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};
class CTxOut
{
public:
  int64_t nValue;
  CScript scriptPubKey;

  CTxOut()
  {
    SetNull();
  }

  CTxOut(int64_t nValueIn, CScript scriptPubKeyIn)
  {
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(nValue);
    READWRITE(scriptPubKey);
  )

  void SetNull()
  {
    nValue = -1;
    scriptPubKey.clear();
  }

  bool IsNull() const
  {
    return (nValue == -1);
  }

  void SetEmpty()
  {
    nValue = 0;
    scriptPubKey.clear();
  }

  bool IsEmpty() const
  {
    return (nValue == 0 && scriptPubKey.empty());
  }

  uint256 GetHash() const
  {
    return SerializeHash(*this);
  }

  bool IsUnspendable() const
  {
    return IsEmpty() ||
           (scriptPubKey.size() > 0 && *scriptPubKey.begin() == OP_RETURN);
  }

  CTxOut& operator=(const CTxOut& o)
  {
    this->nValue = o.nValue;
    this->scriptPubKey = o.scriptPubKey;
    return *this;
  }

  friend bool operator==(const CTxOut& a, const CTxOut& b)
  {
    return (a.nValue == b.nValue &&
            a.scriptPubKey == b.scriptPubKey);
  }

  friend bool operator!=(const CTxOut& a, const CTxOut& b)
  {
    return !(a == b);
  }

  std::string ToStringShort() const
  {
    return strprintf(" out %s %s", FormatMoney(nValue).c_str(), scriptPubKey.ToString(true).c_str());
  }

  std::string ToString() const
  {
    if (IsEmpty())
    {
      return "CTxOut(empty)";
    }

    return strprintf("CTxOut(nValue=%s, scriptPubKey=%s)", FormatMoney(nValue).c_str(), scriptPubKey.ToString().c_str());
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};

#endif
