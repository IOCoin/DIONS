#pragma once

#include<vector>

#include "validation/base/Validator.h"

class ValidationEngine 
{
  public:
    ValidationEngine() { }

  private:
    std::vector<Validator> validiators;
};