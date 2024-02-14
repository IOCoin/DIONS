#pragma once

#include<vector>
#include<memory>

#include "validation/base/Validator.h"

template <typename T>
class ValidationEngine 
{
  public:
    ValidationEngine() { }

   void addPlugin(std::unique_ptr<T>& v);

  private:
    std::vector<std::unique_ptr<T>> validators;
};
