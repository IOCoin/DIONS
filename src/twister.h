#ifndef TWISTER
#define TWISTER

#include "boost/random/mersenne_twister.hpp"
#include "boost/math/constants/constants.hpp"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/multiprecision/cpp_dec_float.hpp"
#include "boost/random/independent_bits.hpp"

#include <vector>

using namespace::std;

using namespace boost::multiprecision;
using namespace boost::random;

typedef independent_bits_engine<mt19937, 256, cpp_int> GEN__;

struct displ
{
  int sect_;
  cpp_int offset_;
  cpp_dec_float_50 scale_;
  vector<int> coord_;
};

struct ex_mix
{
  GEN__ g_;  
  vector<displ> descTable_;
};


#endif
