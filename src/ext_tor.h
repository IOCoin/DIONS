#ifndef EXT_TOR
#define EXT_TOR 

#include<vector>

class ExtTor
{
public:
  virtual void gr() = 0;
  virtual void genbase() = 0;
  virtual void field_embed() = 0;
  virtual void translate(int,int,int) = 0;
  virtual void shift(int) = 0;
  virtual void aror(int) = 0;
  virtual void arol(int) = 0;
  virtual void streamx(std::vector<unsigned char>&) = 0;
  virtual void merge(std::vector<unsigned char>&) = 0;
  virtual void prod(std::vector<unsigned char>&,std::vector<unsigned char>&) = 0;
private:
};

#endif
