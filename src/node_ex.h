#ifndef NODE_EX
#define NODE_EX


using namespace::std;

class NRelay
{
  NRelay() = default; 
  virtual ~NRelay() = default;
  virtual void* exch(unsigned char*) { return 0; }
};

class NodeEx
{
  public:
    virtual void relay(unsigned char*) = 0;
    virtual int  open() = 0;
    virtual int  close() = 0;
};

#endif
