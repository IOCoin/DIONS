#pragma once

#include "NodeEx.h"
#include "InterfaceCrypt.h"
#include "Coordinate.h"
#include "CoordinatePatch.h"
#include "RayShade.h"

class Relay : InterfaceCrypt
{
public:
  Relay()
  {
  }

  Relay(string& s)
  {
    this->r_ = s;
  };

  virtual ~Relay()
  {
  }

  inline virtual int sig()
  {
    return 0;
  }
  inline virtual void open() {}
  inline virtual void close() {}
  inline virtual string alias()
  {
    return "";
  }
  inline virtual string ctrl_()
  {
    return this->r_;
  }
  inline virtual void ctrl(string& c)
  {
    this->r_ = c;
  }

  IMPLEMENT_SERIALIZE
  (
    READWRITE(this->r_);
    READWRITE(this->locator_);
  )

private:
  string r_;
  string locator_;
};

