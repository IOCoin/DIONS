#ifndef COORDINATE_H
#define COORDINATE_H

#include "interface_crypt.h"

#include "state.h"

class CoordinatePatch : InterfaceCrypt
{
  public:
    CoordinatePatch()
    {
    }

    CoordinatePatch(string& s) 
    {
      this->r_ = s;
    };

    virtual ~CoordinatePatch()
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

  private:
    string r_;
    string locator_;
    vector<unsigned int> connectionVector_;
};

#endif
