#pragma once

#include "json/json_spirit.h"

#include "ReactorRelay.h"
#include "rpc/ReflectFace.h"

static const int64_t CTRL___cycle = 0;

class cba;

class Reflect : public ReflectFace
{
public:
  Reflect()
  {
  }
  virtual json_spirit::Value integratedTest1(const json_spirit::Array&, bool) override;
  virtual json_spirit::Value integratedTest2(const json_spirit::Array&, bool) override;
  virtual json_spirit::Value integratedTest3(const json_spirit::Array&, bool) override;
  virtual json_spirit::Value integratedTest4(const json_spirit::Array&, bool) override;
  virtual json_spirit::Value integratedTest5(const json_spirit::Array&, bool) override;
  virtual json_spirit::Value integratedTest6(const json_spirit::Array&, bool) override;
};
