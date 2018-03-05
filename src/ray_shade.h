#ifndef RAY_SHADE_H
#define RAY_SHADE_H

#include "interface_crypt.h"


class RayShade : InterfaceCrypt
{
  public:
    RayShade()
    {
    }

    RayShade(string& s) 
    {
      this->tgt_ = s;
    };

    virtual ~RayShade()
    {
    }

    inline virtual int sig() { return 0; }
    inline virtual bool burstRelay(BurstBuffer& d) { return true; }
    inline virtual void burstTx(BurstBuffer& d) { }
    inline virtual void open() {}
    inline virtual void close() {}
    inline virtual string alias() { return ""; }
    inline virtual string ctrl_() { return this->tgt_; }
    inline virtual void ctrl(string& c) { this->tgt_ = c; }
    inline virtual void ctrlExternalDtx(string& o) { this->o_ = o; }
    inline virtual void ctrlDtx(string& o) { this->vtx_ = o; }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->tgt_);
        READWRITE(this->o_);
        READWRITE(this->vtx_);
    )

  private:
    string tgt_;
    string o_;
    string vtx_;
};

#endif
