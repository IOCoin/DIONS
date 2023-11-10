#pragma once

#include "GenericServer.h"

class RawTransactionFace : public ServerInterface<RawTransactionFace>
{
public:
  RawTransactionFace()
  {
    this->bindMethod("getrawtransaction", RPCMethod<RawTransactionFace> {&RawTransactionFace::getrawtransaction, "true","false"});
    this->bindMethod("listunspent", RPCMethod<RawTransactionFace> {&RawTransactionFace::listunspent, "true","false"});
    this->bindMethod("crawgen", RPCMethod<RawTransactionFace> {&RawTransactionFace::crawgen, "true","false"});
    this->bindMethod("createrawtransaction", RPCMethod<RawTransactionFace> {&RawTransactionFace::createrawtransaction, "true","false"});
    this->bindMethod("decoderawtransaction", RPCMethod<RawTransactionFace> {&RawTransactionFace::decoderawtransaction, "true","false"});
    this->bindMethod("decodescript", RPCMethod<RawTransactionFace> {&RawTransactionFace::decodescript, "true","false"});
    this->bindMethod("signrawtransaction", RPCMethod<RawTransactionFace> {&RawTransactionFace::signrawtransaction, "true","false"});
    this->bindMethod("sendrawtransaction", RPCMethod<RawTransactionFace> {&RawTransactionFace::sendrawtransaction, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"RAWTRANSACTION", "dvm.1.0"}};
  }

  inline virtual void getrawtransaction(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getrawtransaction(req,false);
  }
  inline virtual void listunspent(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->listunspent(req,false);
  }
  inline virtual void crawgen(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->crawgen(req,false);
  }
  inline virtual void createrawtransaction(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->createrawtransaction(req,false);
  }
  inline virtual void decoderawtransaction(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->decoderawtransaction(req,false);
  }
  inline virtual void decodescript(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->decodescript(req,false);
  }
  inline virtual void signrawtransaction(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->signrawtransaction(req,false);
  }
  inline virtual void sendrawtransaction(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->sendrawtransaction(req,false);
  }
  ///////////////////////////////////////////////////////////////////////////////////

  virtual json_spirit::Value getrawtransaction(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value listunspent(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value crawgen(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value createrawtransaction(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value decoderawtransaction(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value decodescript(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value signrawtransaction(const json_spirit::Array& params, bool fHelp) = 0;
  virtual json_spirit::Value sendrawtransaction(const json_spirit::Array& params, bool fHelp) = 0;
};
