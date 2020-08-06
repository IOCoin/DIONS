#ifndef TWISTER
#define TWISTER

#include "boost/random/mersenne_twister.hpp"
#include "boost/math/constants/constants.hpp"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/multiprecision/cpp_dec_float.hpp"
#include "boost/random/independent_bits.hpp"

#include <vector>
#include <map>
#include<tuple>
//#include "state.h"

using namespace::std;

using namespace boost::multiprecision;
using namespace boost::random;

//Suggest abstraction of bits engine which can be given concrete
//instantiation for each generator base
typedef independent_bits_engine<mt19937, 256, cpp_int> GEN__;
typedef struct displ
{
  int sect_;   
  GEN__ strm_;
  cpp_int offset_;
  cpp_dec_float_50 scale_;
  cpp_dec_float_50 range_;
  vector<int> coord_;
} view;

// Suggestion, a second generator class would be cleaner
// decoupling the index basis from the mix util structs. Blastdoor7
typedef struct ex_mix
{
  int pos_;
  GEN__ g_;  
  vector<displ> descTable_;
} mix;

// codomain added here
typedef struct mtx
{
  ex_mix disc_;
  ex_mix codom_;
} mtx_co;

typedef struct FI1__
{
  ex_mix r;
  ex_mix th;
  ex_mix ph;
} fi1;


//Decriptor basis same as mix len optimal ?
typedef struct FI2__
{
  ex_mix desc_;
  ex_mix path_;
  ex_mix desc1_;
  map<FI1__, displ> transMap_;
  map<FI1__, displ> internMap_;     
  map<FI2__, displ> extTransMap_;
  map<FI2__, displ> torTransMap_; //Explicit base for now .
} fi2;

typedef struct R1_mtx_rotate
{
  FI2__ alpha_;
  FI2__ beta_;
  FI2__ gamma_;

  FI1__ res_;
  FI2__ reference_;
  FI2__ basis_;
} rot;


typedef struct TransitionElement
{
  vector<double> ent_indicator_;
  FI2__ key_center_;
  vector<R1_mtx_rotate> morph_l_;
  vector<FI1__> reference_;
  vector<FI2__> cbase_;
  vector<FI2__> key_l_;
} transelt;

typedef struct Spectra
{
  FI1__ basis_;
  FI2__ mix_;
  FI2__ dim_;
} spec;

vector<double> f_dist(vector<unsigned char>& in);
double s_entropy(vector<double> v);

void trans(vector<unsigned char>, unsigned char (*f)(unsigned char));
void transHom(vector<unsigned char>, unsigned char (*f)(unsigned char), unsigned char (*g)(unsigned char));
void transHomExt(vector<unsigned char>, unsigned char (*f)(unsigned char), unsigned char (*g)(unsigned char));
class SpecExec
{
  public:	
  SpecExec() {}
  ~SpecExec() {}

  virtual double entropy(vector<double> v);
  virtual double sect(vector<double> v);
  virtual double sect_outer(vector<double> v);
  virtual double trans_ext(vector<double> v);

};

class TLV
{
  public:  
    TLV() {};
    ~TLV() {};
    TransitionElement trans;
    TransitionElement co_dom_;
    TransitionElement trans_gnd_;
    TransitionElement trans_atom_;    
    TransitionElement trans_ion_;    
    TransitionElement trans_ex_;    
    SpecExec se;
    SpecExec list;
    SpecExec trans_base;
    SpecExec trans_gcd_ext;
  private:
    FI1__ mix;
};

double ic(const string& );

void trans(vector<unsigned char>& data, unsigned char (*f)(unsigned char));
double s_entropy(vector<double>&) ;
void switchIO(unsigned char (*p)(unsigned char, unsigned char), unsigned char);
void transHom(vector<unsigned char>& , unsigned char (*f)(unsigned char), unsigned char (*g)(unsigned char));
void multiChan(unsigned char* (*p)(unsigned char, unsigned char), unsigned char);
void hPerm(int s, int n, void (*p)(int), void (*inv)(int, int), void (*center)(int));
double sw(double weight, int i, int j, int (*inv)(int, int));
void rms(const string& , string& );
vector<double> f_dist(vector<unsigned char>&);
void transHomExt(vector<unsigned char>& , unsigned char (*f)(unsigned char), unsigned char (*g)(unsigned char));
int outer_sect(int (*s)(int), int (*t)(int), int , int);
std::tuple<int, int, int> extended_gcd(int , int , int (*p)(int,int));
std::tuple<int, int, int> extended_gcd(int , int );

#endif
