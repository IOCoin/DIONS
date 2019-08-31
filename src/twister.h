#ifndef TWISTER
#define TWISTER

#include "boost/random/mersenne_twister.hpp"
#include "boost/math/constants/constants.hpp"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/multiprecision/cpp_dec_float.hpp"
#include "boost/random/independent_bits.hpp"

#include <vector>
#include <map>

//#include "state.h"

using namespace::std;

using namespace boost::multiprecision;
using namespace boost::random;

typedef independent_bits_engine<mt19937, 256, cpp_int> GEN__;

const int transition_seq[] = { 0xce, 0xab, 0xdf, 0xcf, 0xee,
                               0xcf, 0xad, 0xdf, 0xff, 0xce, 
                               0xda, 0xee, 0xff, 0xfc, 0xbf, 
                               0xe0, 0x21, 0x9f, 0xe1, 0xc6, 
                               0xe4, 0x38, 0x1f, 0x60, 0x24, 
                               0x12, 0x9a, 0x30, 0x44, 0x72, 
                               0xf2, 0xca, 0xa2, 0x51, 0xc6, 
                               0xcb, 0xbd, 0xba, 0xac, 0xdf, 
                               0xdf, 0xde, 0xcd, 0xfd, 0xca };
struct displ
{
  int sect_;
  cpp_int offset_;
  cpp_dec_float_50 scale_;
  vector<int> coord_;
};

struct ex_mix
{
  int pos_;
  GEN__ g_;  
  vector<displ> descTable_;
};

struct mtx
{
  ex_mix disc_;
};

struct FI1__
{
  ex_mix r;
  ex_mix th;
  ex_mix ph;
};

struct FI2__
{
  ex_mix desc_;
  map<FI1__, displ> transMap_;
};

struct R1_mtx_rotate
{
  FI2__ alpha_;
  FI2__ beta_;

  FI1__ reference_;
  FI1__ basis_;
};


struct TransitionElement
{
  vector<double> ent_indicator_;
  FI2__ key_center_;
  vector<R1_mtx_rotate> morph_l_;
  vector<FI1__> reference_;
};


vector<double> f_dist(vector<unsigned char>& in);
double s_entropy(vector<double> v);

void trans(vector<unsigned char>, unsigned char (*f)(unsigned char));
class SpecExec
{
  public:	
  SpecExec() {}
  ~SpecExec() {}

  virtual double entropy(vector<double> v);

};

class TLV
{
  public:  
    TransitionElement trans;
    SpecExec se;
};

#endif
