#ifndef RELAY_H
#define RELAY_H

#include "node_ex.h"

#include "interface_crypt.h"
#include "coordinate.h"
#include "coordinate_patch.h"
#include "ray_shade.h"

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
    inline virtual string ctrl_() { return this->r_; }
    inline virtual void ctrl(string& c) { this->r_ = c; }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->r_);
        READWRITE(this->locator_);
    )

  private:
    string r_;
    string locator_;
};

#endif
