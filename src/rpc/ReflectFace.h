#pragma once

#include "GenericServer.h"

class ReflectFace : public ServerInterface<ReflectFace>
{
public:
  ReflectFace()
  {
    this->bindMethod("integratedTest1", RPCMethod<ReflectFace> {&ReflectFace::integratedTest1, "true","false"});
    this->bindMethod("integratedTest2", RPCMethod<ReflectFace> {&ReflectFace::integratedTest2, "true","false"});
    this->bindMethod("integratedTest3", RPCMethod<ReflectFace> {&ReflectFace::integratedTest3, "true","false"});
    this->bindMethod("integratedTest4", RPCMethod<ReflectFace> {&ReflectFace::integratedTest4, "true","false"});
    this->bindMethod("integratedTest5", RPCMethod<ReflectFace> {&ReflectFace::integratedTest5, "true","false"});
    this->bindMethod("integratedTest6", RPCMethod<ReflectFace> {&ReflectFace::integratedTest6, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"REFLECT", "dvm.1.0"}};
  }
  inline virtual void integratedTest1(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->integratedTest1(req,false);
  }
  inline virtual void integratedTest2(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->integratedTest2(req,false);
  }
  inline virtual void integratedTest3(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->integratedTest3(req,false);
  }
  inline virtual void integratedTest4(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->integratedTest4(req,false);
  }
  inline virtual void integratedTest5(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->integratedTest5(req,false);
  }

  inline virtual void integratedTest6(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->integratedTest6(req,false);
  }

  ///////////////////////////////////////////////////////////////////////////////////

  virtual json_spirit::Value integratedTest1(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value integratedTest2(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value integratedTest3(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value integratedTest4(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value integratedTest5(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value integratedTest6(const json_spirit::Array&,bool) = 0;
};
