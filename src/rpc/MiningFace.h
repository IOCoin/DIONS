#pragma once

#include "GenericServer.h"

class MiningFace : public ServerInterface<MiningFace>
{
public:
  MiningFace()
  {
    this->bindMethod("getstakinginfo", RPCMethod<MiningFace> {&MiningFace::getstakinginfo, "true","false"});
    this->bindMethod("getworkex", RPCMethod<MiningFace> {&MiningFace::getworkex, "true","false"});
    this->bindMethod("getwork", RPCMethod<MiningFace> {&MiningFace::getwork, "true","false"});
    this->bindMethod("getblocktemplate", RPCMethod<MiningFace> {&MiningFace::getblocktemplate, "true","false"});
    this->bindMethod("tmpTest", RPCMethod<MiningFace> {&MiningFace::tmpTest, "true","false"});
    this->bindMethod("submitblock", RPCMethod<MiningFace> {&MiningFace::submitblock, "true","false"});
    this->bindMethod("getnetworkmhashps", RPCMethod<MiningFace> {&MiningFace::getnetworkmhashps, "true","false"});
    this->bindMethod("getsubsidy", RPCMethod<MiningFace> {&MiningFace::getsubsidy, "true","false"});
    this->bindMethod("getmininginfo", RPCMethod<MiningFace> {&MiningFace::getmininginfo, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"MINING", "dvm.1.0"}};
  }

  inline virtual void getstakinginfo(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getstakinginfo(req,false);
  }
  inline virtual void getwork(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getwork(req,false);
  }
  inline virtual void getworkex(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getworkex(req,false);
  }
  inline virtual void getblocktemplate(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getblocktemplate(req,false);
  }
  inline virtual void tmpTest(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->tmpTest(req,false);
  }
  inline virtual void submitblock(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->submitblock(req,false);
  }
  inline virtual void getnetworkmhashps(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getnetworkmhashps(req,false);
  }
  inline virtual void getsubsidy(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getsubsidy(req,false);
  }
  inline virtual void getmininginfo(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getmininginfo(req,false);
  }

  ///////////////////////////////////////////////////////////////////////////////////

  virtual json_spirit::Value getstakinginfo(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value getworkex(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value getwork(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value getblocktemplate(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value submitblock(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value tmpTest(const json_spirit::Array&, bool) = 0;
  virtual json_spirit::Value getnetworkmhashps(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value getsubsidy(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value getmininginfo(const json_spirit::Array& params, bool fHelp) = 0;
};
