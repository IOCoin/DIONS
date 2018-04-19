#ifndef BBUFFER_H
#define BBUFFER_H

#include<stdlib.h>

typedef char Burst;

class BurstBuffer
{
  public:
    BurstBuffer();
    BurstBuffer(const BurstBuffer& other);
    virtual ~BurstBuffer();

    int put(Burst* data, int offset, int length);

    int get(Burst* data, int offset, int length);

    virtual Burst* __(int offset );

    Burst* array()
    {
      return this->b_;
    }

    int size() const
    {
      return this->limit_;
    }

    int resize(int limit);

    int position() const
    {
      return this->delta_;
    }

    int rewind();

    BurstBuffer& clear()
    {
      if(this->b_ != NULL)
        delete[] this->b_;

      b_=NULL;
      delta_=0;
      limit_=0;

      return *this;
    }

    int prepend(const BurstBuffer& other);

    BurstBuffer& operator=(const BurstBuffer& other);

    void dump();

  private:
    Burst* b_;
    int   limit_;
    int   delta_;
};

#endif
