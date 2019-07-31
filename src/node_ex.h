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
    virtual int  accept(const NRelay&) = 0;
    virtual int  react(const NRelay&) = 0;
    virtual void chan(__BASE__&) = 0;
    virtual void channel_relay(__BASE__&, NRelay&) = 0;
    virtual void record(__BASE__&, const NRelay&) = 0;
    virtual void relayAtomic(const NRelay&) const = 0;
    virtual void relayIon(const NRelay&) const = 0;
    virtual void relayGround(const NRelay&) const = 0;
};


class ChiS__
{
  public:
    ChiS__() = default;
    ~ChiS__() = default;

    int base() { return this->s_base_; }
    void base(int b) { this->s_base_ = b; }
    
    int correlate() { return this->corr_; }
    void correlate(int c) { this->corr_ = c; }

    int g_sample(NodeEx& ex) { return ex.react(this->rel_); }

    int g_resolve(NodeEx& ex) { return ex.reg(this->rel_); }
  private: 
    
    int s_base_;
    int corr_;
    vector<unsigned char> nList;
    NRelay rel_;
};

class Outer 
{
  public:
    Outer() = default;
    ~Outer() = default;
    virtual int bufRangeIndex(unsigned char* p) 
    {
      for(unsigned int i=0; i<buffer_.size(); i++)
      {
        if(buffer_[i] == *p)
	  return i;	
      }     
    }

    virtual void locator(int index) 
    {
      this->locatorRelay_ = index;
    }

    virtual int locator()
    {
      return this->locatorRelay_;
    }

    virtual const NRelay& relSample() const
    {
      return this->relay_;
    }

    virtual void accept(NRelay& rel) { this->relay_ = rel; }
    virtual int lmap() { return this->lMap_; }
    virtual void lmap(int lm) { this->lMap_ = lm; } 
  private:
    int lMap_;
    int locatorRelay_; 
    NRelay relay_;
    vector<unsigned char> buffer_;
};

#endif
