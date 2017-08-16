#ifndef REACTOR_RELAY
#define REACTOR_RELAY

#include "bbuffer.h"

class RLWE__CTRL__ 
{
  template<int N>
  void operator()() 
  {
    return N;
  }
};

class ReactorRelay
{
  virtual void burstBufferCRC() = 0;
  virtual void acceptor() = 0;
};

#endif
