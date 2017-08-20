#ifndef RELAY_H
#define RELAY_H

#include "interface_crypt.h"

class Relay : InterfaceCrypt
{
  public:
    Relay()
    {
    }

    Relay(string& s) 
    {
      this->r_ = s;
    };

    virtual ~Relay()
    {
    }

    inline virtual int sig() { return 0; }
    inline virtual bool burstRelay(BurstBuffer& d) { return true; }
    inline virtual void burstTx(BurstBuffer& d) { }
    inline virtual void open() {}
    inline virtual void close() {}
    inline virtual string alias() { return ""; }
    inline virtual string ctrl_() { return this->locator_; }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->r_);
    )

  private:
    string r_;
    string locator_;
};

#endif
