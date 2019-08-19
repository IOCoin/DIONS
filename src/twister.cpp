#include "twister.h"

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
