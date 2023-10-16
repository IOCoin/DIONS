#include"ExtTor.h"
#include "Twister.h"

class Toroid : public ExtTor
{
public:
  Toroid() {};
  virtual void gr() {};
  virtual void genbase() {};
  virtual void field_embed() {};
  virtual void translate(int,int,int);
  virtual void shift(int) {};
  virtual void aror(int) {};
  virtual void arol(int) {};
  virtual void streamx(std::vector<unsigned char>&) {};
  virtual void merge(std::vector<unsigned char>&) {};
  virtual void prod(std::vector<unsigned char>&,std::vector<unsigned char>&) {};

  virtual fi2& base()
  {
    return this->base_;
  }
  virtual ~Toroid() { };
private:
  std::vector<unsigned char> torserial;
  fi2 base_;
  rot thetaTrans_;
  transelt translate_;
  spec ffspec;
};
