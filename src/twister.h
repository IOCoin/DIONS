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
                               0xae, 0xfc, 0xdc, 0xea, 0xbd };
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
};

struct TransitionElement
{
  FI2__ key_center_;
  vector<R1_mtx_rotate> morph_l_;
};

#endif
