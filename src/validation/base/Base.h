#pragma once
#include "validation/interface/Validator.h"

class Base : publish Validator
{
  public:
    Base() = default;
    ~Base() = default;

    virtual bool validate() override;

  private:
};
