#ifndef INTERFACE_CRYPTH
#define INTERFACE_CRYPTH 

using namespace::std;

class InterfaceCrypt
{
public:
  virtual int sig() = 0;
  virtual void open() = 0;
  virtual void close() = 0;
  virtual string alias() = 0;
  virtual string ctrl_() = 0;
  virtual void ctrl(string& c) = 0;
};

#endif
