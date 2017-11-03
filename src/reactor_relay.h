#ifndef REACTOR_RELAY
#define REACTOR_RELAY

#include "bbuffer.h"

#define s_put(q,x) ((q)[(x)/64] |= (((uint64_t) 1) << (uint64_t) ((x)%64)))
#define s_extract(q,x) (((q)[(x)/64] >> (uint64_t) ((x)%64)) & 1)
#define xb__(q,x) ((q)[(x)/64] &= ((~((uint64_t) 0)) - (((uint64_t) 1) << (uint64_t) ((x)%64))))

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
