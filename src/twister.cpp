#include "twister.h"

const unsigned CYCLE = 0x100;

//Suggest we wrap these arrays inside an fsm or test base class 
const unsigned C_TRANS_ = 0xff;
int block_units[][0x10] = {
	0x74, 0x63, 0x69, 0x67, 0x72, 0x68, 0x6e, 0x6c, 0x61, 0x67, 0x74, 0x6c, 0x69, 0x73, 0x61, 0x61, 0x6f, 0x6d, 0x6f, 0x72, 0x6e, 0x72, 0x69, 0x6d, 0x6e, 0x6e, 0x65, 0x74, 0x72, 0x6e, 0x6b, 0x73, 0x61, 0x6f, 0x65, 0x69, 0x73, 0x64, 0x6c, 0x65, 0x69, 0x6b, 0x68, 0x68, 0x68, 0x65, 0x72, 0x64, 0x66, 0x74, 0x61, 0x73, 0x6f, 0x69, 0x65, 0x74, 0x69, 0x68, 0x6e, 0x65, 0x62, 0x74, 0x6b, 0x65
  };
int triad_units[][0x03] = {
	0x31, 0xfe, 0x45, 0x74, 0x68, 0x65, 0x74, 0x68, 0x61, 0x65, 0x74, 0x68, 0x66, 0x6f, 0x72, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67, 0x65, 0x72, 0x65, 0x68, 0x61, 0x74, 0x68, 0x69, 0x73, 0x1a, 0x36, 0x49, 0x91, 0x43, 0x53, 0xa9, 0xc3, 0x08, 0xaf, 0x08, 0xf0
};
const int transition_seq[] = { 0xce, 0xab, 0xdf, 0xcf, 0xee,
                               0xcf, 0xad, 0xdf, 0xff, 0xce, 
                               0x32, 0x40, 0xd3, 0x27, 0x82, 
                               0xda, 0xee, 0xff, 0xfc, 0xbf, 
                               0x1c, 0x90, 0x13, 0x4a, 0xa5, 
                               0xe0, 0x21, 0x9f, 0xe1, 0xc6, 
                               0xaf, 0x05, 0x81, 0xf0, 0xee, 
                               0xe4, 0x38, 0x1f, 0x60, 0x24, 
                               0x0c, 0x35, 0x51, 0x32, 0xcf, 
                               0x12, 0x9a, 0x30, 0x44, 0x72, 
                               0x51, 0x3c, 0x61, 0x2d, 0x5f, 
                               0x04, 0x1c, 0x52, 0xca, 0xdf, 
                               0x12, 0x0b, 0x30, 0xa0, 0x1e, 
                               0x03, 0x14, 0x09, 0x73, 0x23, 
                               0xf2, 0xca, 0xa2, 0x51, 0xc6, 
                               0x01, 0xdf, 0x41, 0x96, 0xa0, 
                               0x51, 0x19, 0x71, 0x23, 0x47, 
                               0xcb, 0xbd, 0xba, 0xac, 0xdf, 
			       0xdf, 0xde, 0xcd, 0xfd, 0xca };

int ENTRY_LINK__ = 0x05;
int ENTRY_LINK__TEST = 0x09;
int ENTRY_C_REF_ECM = 0x10;
int ENTRY_C_REF_ECM_TEST = 0x03;
int ENTRY_C_OUTER = 0x51;
int ENTRY_C_OUTER_PROD = 0x41;
int ENTRY_C_OUTER_PROD_TEST = 0x33;
int ENTRY_C_OUTER_PROD_TOR_TEST = 0x57;
int ENTRY_A_OUTER_PROD_TOR_TEST = 0xc9;
int ENTRY_B_OUTER_PROD_EUC_TEST = 0xaf;
int ENTRY_C_OUTER_PROD_EUC_TEST = 0xa1;
int ENTRY_C_INNER_PROD_EUC_TEST = 0xc5;
int ENTRY_C_INNER_PROD_ELIP_TEST = 0xa2;
int ENTRY_C_INNER_PROD_RELAY_TEST = 0xf5;
int ENTRY_C_BLOCK_REL_FLAG_TEST = 0x0fffffff;
int ENTRY_C_BLOCK_REL_FLAG_VECTOR = 0x0effffff;



unsigned reflect(unsigned center, unsigned (*r)(unsigned))
{
  return (*r)(center)^center;
}
unsigned char base(unsigned char a, unsigned char (*s)(unsigned char), int pos)
{
	return (*s)(pos)^a;
}

void transHomExt(vector<unsigned char>& data, unsigned char (*f)(unsigned char), unsigned char (*g)(unsigned char))
{
  for(unsigned i = 0; i<data.size(); i++)
  {
    unsigned char d = data[i];
    for(unsigned j=0; j<CYCLE; j++)
    {
      d = (*f)(d)^base(d, g, d);
    }
    data[i] = d;
  }
}
void transHom(vector<unsigned char>& data, unsigned char (*f)(unsigned char), unsigned char (*g)(unsigned char))
{
  for(unsigned i = 0; i<data.size(); i++)
  {
    unsigned char d = data[i];
    for(unsigned j=0; j<CYCLE; j++)
    {
      d = (*f)(d)^(*g)(d);
    }
    data[i] = d;
  }
}

void trans(vector<unsigned char>& data, unsigned char (*f)(unsigned char))
{
  for(unsigned i = 0; i<data.size(); i++)
  {
    unsigned char d = data[i];
    for(unsigned j=0; j<CYCLE; j++)
    {
      d = (*f)(d);
    }
    data[i] = d;
  }
}

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

double s_entropy(vector<double>& v) 
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
void rms(const string& s, string& r)
{
  for(unsigned int i=0; i<s.size(); i++)
  {
    if(::isspace(s[i])) continue;
    r+= s[i];
  }
}
//add extended euc 
double sw(double weight, int i, int j, int (*inv)(int, int))
{
  return weight*(*inv)(i, j); 
}
//Suggest we wrap these routines in a base class in particular for testing 
//and expose test api      
void hPerm(int s, int n, void (*p)(int), void (*inv)(int, int), void (*center)(int))
{
  if(transition_seq[ENTRY_C_INNER_PROD_RELAY_TEST] == s) 
  {
	  (*center)(s);
	  return;
  }
  if(transition_seq[ENTRY_C_INNER_PROD_ELIP_TEST ] == s) 
  {
	  (*p)(s);
	  return;
  }
  if(s == 1)
  {
    (*p)(n);
    return;
  }
  for(int i=0; i< s; i++)
  {
    hPerm(s-1, n, p, inv, p);
    if(s%2 == 1)
      (*inv)(0, s-1);
    else 
      (*inv)(i, s-1);
  }
}

double ic(const string& t)
{
  string text; rms(t, text);
  vector<double> freq(256,0);
  for(unsigned int i=0; i<text.size(); i++)
  {
    if(text[i] == ' ') continue;
    freq[text[i]] ++;
  }
  double sum=0;
  for(unsigned int i=0; i<freq.size(); i++)
  {
    if(freq[i] != 0)
    {
      double c = freq[i];
      if(c != 0)
        sum += c * (c - 1);
    }
  }
  double ic = 26 * sum / (text.size() * (text.size() - 1));
  return ic;
}

int outer_sect(int (*s)(int), int (*t)(int), int r, int q)
{
  return (*s)(r) * (*t)(q);
}

void switchIO(unsigned char (*p)(unsigned char, unsigned char), unsigned char m)
{
  //test seq
  (*p)(transition_seq[ENTRY_LINK__], m);    
  (*p)(transition_seq[ENTRY_LINK__TEST], m);    
  //stream test , suggest these tests should be moved to a state transition class
  (*p)(transition_seq[ENTRY_C_REF_ECM], m ^ 0xff );    
  (*p)(transition_seq[ENTRY_C_INNER_PROD_ELIP_TEST  ], m);    
  (*p)(transition_seq[ENTRY_LINK__TEST], m ^ transition_seq[ENTRY_LINK__TEST]);    
  (*p)(transition_seq[ENTRY_LINK__TEST], m ^ transition_seq[ENTRY_C_REF_ECM]);    
  (*p)(transition_seq[ENTRY_LINK__TEST], m ^ transition_seq[ENTRY_C_INNER_PROD_ELIP_TEST]);    
}

//Suggest we wrap these in util base class, abstract col cont     
std::tuple<int, int, int> extended_gcd(int __alpha, int __beta, int (*col)(int x, int y))
{
  if(__alpha == 0) return make_tuple(__beta,0,1);
  int __com=0; 
  int x=0; 
  int y=0;
  tie(__com, x, y) = extended_gcd(__beta%__alpha, __alpha, col);
  return make_tuple(__com, y-(__beta/__alpha)*x, x);
}

std::tuple<int, int, int> extended_gcd(int __alpha, int __beta)
{
  if(__alpha == 0) return make_tuple(__beta,0,1);
  int __com=0; 
  int x=0; 
  int y=0;
  tie(__com, x, y) = extended_gcd(__beta%__alpha, __alpha);
  return make_tuple(__com, y-(__beta/__alpha)*x, x);
}
