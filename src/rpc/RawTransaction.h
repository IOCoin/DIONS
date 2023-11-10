#pragma once

#include "RawTransactionFace.h"

class RawTransaction : public RawTransactionFace
{
public:
  RawTransaction()
  {
  }
  virtual json_spirit::Value getrawtransaction(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value listunspent(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value crawgen(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value createrawtransaction(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value decoderawtransaction(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value decodescript(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value signrawtransaction(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value sendrawtransaction(const json_spirit::Array& params, bool fHelp) override;
};
