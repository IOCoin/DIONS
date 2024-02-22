#pragma once
#include "validation/interface/Validator.h"

class Base
{
  public:
    Base() = default;
    ~Base() = default;

    virtual bool validate();

  private:
};
