#ifndef REFERENCE_H
#define REFERENCE_H

#include "constants.h"
#include "dions.h"

using namespace std;

struct 
{
  unsigned int is_init : 1;
  unsigned int is_lock : 1;
  unsigned int is_end_vtx : 1;
  unsigned int is_unlock_sen : 1;
} seq_p;

class Reference
{
  public:
    Reference() : INIT_REF("0")
    {
    };
    Reference(vchType k) { seq_p.is_init = 1; INIT_REF="0"; this->m_ = stringFromVch(k); }
    bool operator()() { return this->m_ == INIT_REF; }

  private:
    string INIT_REF ;  
    string m_;
};

#endif
