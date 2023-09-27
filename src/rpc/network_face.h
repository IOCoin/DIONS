#pragma once

#include "generic_server.h"

class NetworkFace : public ServerInterface<NetworkFace>
{
public:
  NetworkFace()
  {
    this->bindMethod("getconnectioncount", RPCMethod<NetworkFace> {&NetworkFace::getconnectioncount, "true","false"});
    this->bindMethod("getpeerinfo", RPCMethod<NetworkFace> {&NetworkFace::getpeerinfo, "true","false"});
    this->bindMethod("sendalert", RPCMethod<NetworkFace> {&NetworkFace::sendalert, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"NETWORK", "dvm.1.0"}};
  }

  inline virtual void getconnectioncount(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getconnectioncount(req,false);
  }
  inline virtual void getpeerinfo(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getpeerinfo(req,false);
  }
  inline virtual void sendalert(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->sendalert(req,false);
  }

  virtual json_spirit::Value getconnectioncount(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getpeerinfo(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendalert(const json_spirit::Array&,bool) = 0;
};
