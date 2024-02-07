#pragma once

class Trap
{
  public:
    Trap() = default;
    ~Trap() = default;

    virtual bool trap() = 0;

  private:
};
