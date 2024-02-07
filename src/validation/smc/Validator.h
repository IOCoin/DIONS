#pragma once

class Validator
{
  public:
    Validator() = default;
    ~Validator() = default;

    virtual bool validate() = 0;

  private:
};
