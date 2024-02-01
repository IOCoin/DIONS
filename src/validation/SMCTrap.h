#pragma once

#include "Trap.h"
#include "ccoin/Transaction.h"

class SMCTrap : public Trap
{
  public:
    SMCTrap() { };
    ~SMCTrap() = default;

    void init(CTransaction& tx)
    {
	    std::cout << "XXXX VALIDATOR::init" << std::endl;
      this->tx_ = tx;
    }

    bool trap() override;

  private:
    CTransaction tx_;
};
