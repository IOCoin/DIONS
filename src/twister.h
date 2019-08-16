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
  vector<double> ent_indicator_;
  FI2__ key_center_;
  vector<R1_mtx_rotate> morph_l_;
  vector<FI1__> reference_;
};

vector<double> f_dist(vector<unsigned char>& in)
{
  vector<double> fdist;
  fdist.resize(256);
  for(unsigned i = 0; i<in.size(); i++)
  {
    fdist[in[i]]++;
  }

  for(unsigned i=0; i<fdist.size(); i++)
  {
    fdist[i] = (fdist[i] / in.size()) * 100;
  }

  return fdist;
}

double s_entropy(vector<double> v) 
{
  double entropy = 0;
  double p;

  for(unsigned i = 0; i < v.size(); i++) 
  {
    p = v[i] / 100;
    if (p == 0) continue;
    entropy += p * std::log(p) / std::log(2);
  }
  return -entropy;
}

#endif
