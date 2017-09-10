#ifndef COORDINATE_H
#define COORDINATE_H

#include "interface_crypt.h"

class CoordinateVector : InterfaceCrypt
{
  public:
    CoordinateVector()
    {
    }

    CoordinateVector(string& s) 
    {
      this->r_ = s;
    };

    virtual ~CoordinateVector()
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

    inline bool mapNode(string l, int& p) 
    {         
      vector<string>::iterator iter;
      iter=find(this->d1_.begin(),this->d1_.end(),l);
      if(iter != this->d1_.end())
      {
        p = iter - this->d1_.begin();
        return true;
      }

      return false;
    }

    inline bool scale_() { return this->d0_.size() == 0; }
    inline bool scale() { return this->d1_.size() == 0; }
    inline string&   domainImage() { return this->d0_.back(); }
    inline string& codomainImage() { return this->d1_.back(); }
    inline void   domain(string s) { this->d0_.push_back(s); }
    inline void codomain(string s) { this->d1_.push_back(s); }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->d0_);
        READWRITE(this->d1_);
    )

  private:
    string r_;
    string locator_;
    vector<string> d0_;
    vector<string> d1_;
};

#endif
