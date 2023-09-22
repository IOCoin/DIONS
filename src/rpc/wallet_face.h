#pragma once

#include "generic_server.h"

class WalletFace : public ServerInterface<WalletFace>
{
public:
  WalletFace()
  {
    this->bindMethod("getbalance", RPCMethod<WalletFace> {&WalletFace::getBalance, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"WALLET", "dvm.1.0"}};
  }

  inline virtual void getBalance(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getBalance(req,false);
  }

  virtual json_spirit::Value getBalance(const json_spirit::Array&,bool) = 0;
};
