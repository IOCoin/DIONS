#ifndef REACTOR_RELAY
#define REACTOR_RELAY

#include "bbuffer.h"

class ReactorRelay
{
  virtual void burstBufferCRC() = 0;
  virtual void acceptor() = 0;
};

#endif
