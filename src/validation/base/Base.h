#pragma once
#include "validator/interface/Validator.h"

class Base
{
  public:
    Base() = default;
    ~Base() = default;

    virtual bool validate() { }

  private:
};
