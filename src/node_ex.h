#ifndef NODE_EX
#define NODE_EX


using namespace::std;

class NodeEx
{
  public:
    virtual void relay(unsigned char*) = 0;
    virtual int  open() = 0;
    virtual int  close() = 0;
};

#endif
