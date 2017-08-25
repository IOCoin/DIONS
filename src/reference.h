#ifndef REFERENCE_H
#define REFERENCE_H

#include "constants.h"
#include "dions.h"

using namespace std;

class Reference
{
  public:
    Reference()
    {
    };
    Reference(vchType k) { this->m_ = stringFromVch(k); }
    bool operator()() { return this->m_ == INIT_REF; }

  private:
    string INIT_REF = "0";  
    string m_;
};

#endif
