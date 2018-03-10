#ifndef RAY_SHADE_H
#define RAY_SHADE_H

#include "interface_crypt.h"


class RayShade : InterfaceCrypt
{
  public:
    static const int RAY_VTX = 0xa1;
    static const int RAY_SET = 0xc2; 

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
    inline virtual void ctrlExternalDtx(int i, uint160 o) 
    { 
      this->l7_ = i; this->o_ = o; 
    }
    inline virtual bool ctrlExternalAngle() 
    { 
      return this->l7_ == RAY_VTX;
    }
    inline virtual int ctrlIndex() 
    { 
      return this->l7_ ;
    }
    inline virtual uint160 ctrlPath() 
    { 
      return this->o_ ;
    }

    inline virtual void ctrlDtx(string& o) { this->vtx_ = o; }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->tgt_);
        READWRITE(this->o_);
        READWRITE(this->l7_);
        READWRITE(this->vtx_);
    )

  private:
    int l7_;
    string tgt_;
    uint160 o_;
    string vtx_;
};

#endif
