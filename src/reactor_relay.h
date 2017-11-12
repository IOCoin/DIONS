#ifndef REACTOR_RELAY
#define REACTOR_RELAY

#include "bbuffer.h"

#define s_put(q,x) ((q)[(x)/64] |= (((uint64_t) 1) << (uint64_t) ((x)%64)))
#define s_extract(q,x) (((q)[(x)/64] >> (uint64_t) ((x)%64)) & 1)
#define xb__(q,x) ((q)[(x)/64] &= ((~((uint64_t) 0)) - (((uint64_t) 1) << (uint64_t) ((x)%64))))

class Encode__N
{
  public:
  Encode__N() : BASE__(-1) { }
  int status__point(int n)
  {
    int r, i;
    if ((n % 2) == 0)
    return 0;

    r = (int) (sqrt(n));
    for (i = 3; i <= r; i += 2)
    if ((n % i) == 0)
    {
        return 0;
    }
    return 1;
  }

  int seek__node(int n)
  {
    do 
    {
      n++;
    } 
    while (!status__point(n));
    return n;
  }

  bool conformal(int n)
  {
    return this->BASE__ == n; 
  }

  private:
    long BASE__;
};

class RLWE__CTRL__ 
{
  template<int N>
  void operator()() 
  {
    return N;
  }
};

class Acceptor
{
  virtual void rwle__base() = 0;
  virtual void rwle__trans__() = 0;
  virtual void* operator()() = 0;
};

class ReactorRelay
{
  virtual void burstBufferCRC() = 0;
  virtual void acceptor() = 0;
};

#endif
