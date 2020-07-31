#include"ext_tor.h"


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
    virtual ~Toroid() { };
  private:
};
