#pragma once

#include "MiningFace.h"

class Mining : public MiningFace
{
public:
  Mining()
  {
  }
  virtual json_spirit::Value tmpTest(const json_spirit::Array&, bool) override;
};

