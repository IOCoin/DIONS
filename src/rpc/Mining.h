#pragma once

#include "MiningFace.h"

class Mining : public MiningFace
{
public:
  Mining()
  {
  }
  virtual json_spirit::Value getstakinginfo(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getworkex(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getwork(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getblocktemplate(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value submitblock(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value tmpTest(const json_spirit::Array&, bool) override;
  virtual json_spirit::Value getnetworkmhashps(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getsubsidy(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getmininginfo(const json_spirit::Array& params, bool fHelp) override;
};

