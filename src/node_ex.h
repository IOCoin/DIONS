#ifndef NODE_EX
#define NODE_EX

#include "twister.h"
#include <vector>

using namespace::std;

struct __BASE__
{
  int st_;
  int sen_;
};

class NRelay
{
  NRelay() = default; 
  virtual ~NRelay() = default;
  virtual void* exch(unsigned char*) { return 0; }

  virtual void* trans(unsigned char*, int) { return 0; }
  virtual void* sen() { return 0; }
};

class NodeEx
{
  public:
    virtual void relay(unsigned char*) = 0;
    virtual int  open() = 0;
    virtual int  close() = 0;
    virtual int  reg(const NRelay&) = 0;
};

class Outer 
{
  public:
    Outer() = default;
    ~Outer() = default;
    virtual int bufRangeIndex(unsigned char* p) 
    {
      for(unsigned int i=0; i<buffer.size(); i++)
      {
        if(buffer[i] == *p)
	  return i;	
      }     
    }

    virtual void locator(int index) 
    {
      this->locatorRelay_ = index;
    }
  private:
    int locatorRelay_; 
    vector<unsigned char> buffer;
};

#endif
