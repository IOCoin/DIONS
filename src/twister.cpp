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
void rms(const string& s, string& r)
{
  for(unsigned int i=0; i<s.size(); i++)
  {
    if(::isspace(s[i])) continue;
    r+= s[i];
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
