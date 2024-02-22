#pragma once

#include "validation/interface/Validator.h"
#include "ccoin/Transaction.h"

class SMCValidator : public Validator
{
  public:
    SMCValidator() { };
    ~SMCValidator() = default;

    void init(CTransaction& tx)
    {
	    std::cout << "XXXX VALIDATOR::init" << std::endl;
      this->tx_ = tx;
    }

    bool validate() override;

  private:
    CTransaction tx_;
};
