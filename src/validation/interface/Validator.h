#pragma once

class Validator
{
  public:
    virtual bool validate() = 0;
    virtual bool trap() = 0;
};
