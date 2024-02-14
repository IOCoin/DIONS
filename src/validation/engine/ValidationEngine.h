#pragma once

#include<vector>
#include<memory>

#include "validation/base/Validator.h"

template <typename Validator>
class ValidationEngine 
{
  public:
    ValidationEngine() { }

    void addPlugin(Validator&);

  private:
    std::vector<std::unique_ptr<Validator>> validators;
};
