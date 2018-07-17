#ifndef RAY_SHADE_H
#define RAY_SHADE_H

#include "interface_crypt.h"


struct __pq__
{
  vector<unsigned char> __fq1;
  vector<unsigned char> __fq9;
  vector<unsigned char> __fq0;
  vector<unsigned char> __fq2;
  vector<unsigned char> __fq5;
};

struct __inv__
{
  vector<unsigned char> __inv7;
  vector<unsigned char> __inv1;
};

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
    inline virtual bool ctrlExternalDtx() 
    { 
      return this->l7_ == RAY_SET;
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
    inline virtual void streamID(vector<unsigned char> o) { this->stream_id = o; }
    inline virtual vector<unsigned char> streamID() { return this->stream_id; }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->tgt_);
        READWRITE(this->o_);
        READWRITE(this->l7_);
        READWRITE(this->vtx_);
        READWRITE(this->stream_id);
    )

  private:
    void init()
    {
      stream_id.resize(0x20);
    }

    int l7_;
    string tgt_;
    uint160 o_;
    string vtx_;
    vector<unsigned char> stream_id;
};

#endif
