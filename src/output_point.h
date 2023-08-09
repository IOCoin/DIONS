#ifndef OUTPUT_POINT_H
#define OUTPUT_POINT_H

class COutPoint
{
public:
  uint256 hash;
  unsigned int n;

  COutPoint()
  {
    SetNull();
  }
  COutPoint(uint256 hashIn, unsigned int nIn)
  {
    hash = hashIn;
    n = nIn;
  }
  IMPLEMENT_SERIALIZE( READWRITE(FLATDATA(*this)); )
  void SetNull()
  {
    hash = 0;
    n = (unsigned int) -1;
  }
  bool IsNull() const
  {
    return (hash == 0 && n == (unsigned int) -1);
  }

  unsigned int GetIndex() const
  {
    return n;
  }
  const char* GetHash() const
  {
    return hash.ToString().c_str();
  };

  friend bool operator<(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
  }

  friend bool operator==(const COutPoint& a, const COutPoint& b)
  {
    return (a.hash == b.hash && a.n == b.n);
  }

  friend bool operator!=(const COutPoint& a, const COutPoint& b)
  {
    return !(a == b);
  }

  std::string ToString() const
  {
    return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10).c_str(), n);
  }

  void print() const
  {
    printf("%s\n", ToString().c_str());
  }
};

#endif
