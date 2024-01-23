#pragma once

#include "Validator.h"
#include "ccoin/Transaction.h"

class SMCValidator : public Validator
{
  public:
    SMCValidator() = default;
    ~SMCValidator() = default;

    void init(CTransaction& tx)
    {
      this->tx_ = tx;
    }

    bool validate() override;

  private:
    CTransaction& tx_;
};
