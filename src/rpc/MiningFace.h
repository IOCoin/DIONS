#pragma once

#include "GenericServer.h"

class MiningFace : public ServerInterface<MiningFace>
{
public:
  MiningFace()
  {
    this->bindMethod("tmpTest", RPCMethod<MiningFace> {&MiningFace::tmpTest, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"MINER", "dvm.1.0"}};
  }

  inline virtual void tmpTest(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->tmpTest(req,false);
  }

  ///////////////////////////////////////////////////////////////////////////////////

  virtual json_spirit::Value tmpTest(const json_spirit::Array&,bool) = 0;
};
