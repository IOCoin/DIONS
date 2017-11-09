#ifndef REFERENCE_H
#define REFERENCE_H

#include "constants.h"
#include "dions.h"

using namespace std;

class Reference
{
  public:
    Reference() : INIT_REF("0")
    {
    };
    Reference(vchType k) { INIT_REF="0"; this->m_ = stringFromVch(k); }
    bool operator()() { return this->m_ == INIT_REF; }

  private:
    string INIT_REF ;  
    string m_;
};

#endif
