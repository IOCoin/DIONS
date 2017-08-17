#ifndef INTERFACE_CRYPTH
#define INTERFACE_CRYPTH

#include "bbuffer.hpp"

class InterfaceCrypt
{
  public:
    virtual int    sig() = 0;
    virtual bool   burstRelay(BurstBuffer& data) = 0;
    virtual void   burstTx(BurstBuffer& data) = 0;
    virtual void   open() = 0;
    virtual void   close() = 0;
    virtual string alias() = 0;
    virtual string ctrl() = 0;
};

#endif
