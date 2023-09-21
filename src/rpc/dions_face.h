#pragma once

#include "generic_server.h"

class DionsFace : public ServerInterface<DionsFace>
{
public:
  DionsFace()
  {
    this->bindMethod("aliasList", RPCMethod<DionsFace> {&DionsFace::aliasList, "true","false"});
    this->bindMethod("vertexScan", RPCMethod<DionsFace> {&DionsFace::vertexScan, "true","false"});
    this->bindMethod("gw1", RPCMethod<DionsFace> {&DionsFace::gw1, "true","false"});
    this->bindMethod("myRSAKeys", RPCMethod<DionsFace> {&DionsFace::myRSAKeys, "true","false"});
    this->bindMethod("myRSAKeys__", RPCMethod<DionsFace> {&DionsFace::myRSAKeys__, "true","false"});
    this->bindMethod("publicKeyExports", RPCMethod<DionsFace> {&DionsFace::publicKeyExports, "true","false"});
    this->bindMethod("publicKeys", RPCMethod<DionsFace> {&DionsFace::publicKeys, "true","false"});
    this->bindMethod("decryptedMessageList", RPCMethod<DionsFace> {&DionsFace::decryptedMessageList, "true","false"});
    this->bindMethod("plainTextMessageList", RPCMethod<DionsFace> {&DionsFace::plainTextMessageList, "true","false"});
    this->bindMethod("externFrame__", RPCMethod<DionsFace> {&DionsFace::externFrame__, "true","false"});
    this->bindMethod("internFrame__", RPCMethod<DionsFace> {&DionsFace::internFrame__, "true","false"});
    this->bindMethod("aliasOut", RPCMethod<DionsFace> {&DionsFace::aliasOut, "true","false"});
    this->bindMethod("nodeValidate", RPCMethod<DionsFace> {&DionsFace::nodeValidate, "true","false"});
    this->bindMethod("validateLocator", RPCMethod<DionsFace> {&DionsFace::validateLocator, "true","false"});
    this->bindMethod("nodeRetrieve", RPCMethod<DionsFace> {&DionsFace::nodeRetrieve, "true","false"});
    this->bindMethod("getNodeRecord", RPCMethod<DionsFace> {&DionsFace::getNodeRecord, "true","false"});
    this->bindMethod("aliasList__", RPCMethod<DionsFace> {&DionsFace::aliasList__, "true","false"});
    this->bindMethod("nodeDebug", RPCMethod<DionsFace> {&DionsFace::nodeDebug, "true","false"});
    this->bindMethod("nodeDebug1", RPCMethod<DionsFace> {&DionsFace::nodeDebug1, "true","false"});
    this->bindMethod("transform", RPCMethod<DionsFace> {&DionsFace::transform, "true","false"});
    this->bindMethod("primaryCXValidate", RPCMethod<DionsFace> {&DionsFace::primaryCXValidate, "true","false"});
    this->bindMethod("vextract", RPCMethod<DionsFace> {&DionsFace::vextract, "true","false"});
    this->bindMethod("vEPID", RPCMethod<DionsFace> {&DionsFace::vEPID, "true","false"});
    this->bindMethod("validate", RPCMethod<DionsFace> {&DionsFace::validate, "true","false"});
    this->bindMethod("transientStatus__C", RPCMethod<DionsFace> {&DionsFace::transientStatus__C, "true","false"});
    this->bindMethod("updateEncryptedAliasFile", RPCMethod<DionsFace> {&DionsFace::updateEncryptedAliasFile, "true","false"});
    this->bindMethod("updateEncryptedAlias", RPCMethod<DionsFace> {&DionsFace::updateEncryptedAlias, "true","false"});
    this->bindMethod("decryptAlias", RPCMethod<DionsFace> {&DionsFace::decryptAlias, "true","false"});
    this->bindMethod("transferEncryptedExtPredicate", RPCMethod<DionsFace> {&DionsFace::transferEncryptedExtPredicate, "true","false"});
    this->bindMethod("transferEncryptedAlias", RPCMethod<DionsFace> {&DionsFace::transferEncryptedAlias, "true","false"});
    this->bindMethod("transferAlias", RPCMethod<DionsFace> {&DionsFace::transferAlias, "true","false"});
    this->bindMethod("uC", RPCMethod<DionsFace> {&DionsFace::uC, "true","false"});
    this->bindMethod("transientStatus__", RPCMethod<DionsFace> {&DionsFace::transientStatus__, "true","false"});
    this->bindMethod("updateAliasFile", RPCMethod<DionsFace> {&DionsFace::updateAliasFile, "true","false"});
    this->bindMethod("updateAlias_executeContractPayload", RPCMethod<DionsFace> {&DionsFace::updateAlias_executeContractPayload, "true","false"});
    this->bindMethod("updateAlias", RPCMethod<DionsFace> {&DionsFace::updateAlias, "true","false"});
    this->bindMethod("publicKey", RPCMethod<DionsFace> {&DionsFace::publicKey, "true","false"});
    this->bindMethod("sendSymmetric", RPCMethod<DionsFace> {&DionsFace::sendSymmetric, "true","false"});
    this->bindMethod("sendPublicKey", RPCMethod<DionsFace> {&DionsFace::sendPublicKey, "true","false"});
    this->bindMethod("sendPlainMessage", RPCMethod<DionsFace> {&DionsFace::sendPlainMessage, "true","false"});
    this->bindMethod("sendMessage", RPCMethod<DionsFace> {&DionsFace::sendMessage, "true","false"});
    this->bindMethod("registerAliasGenerate", RPCMethod<DionsFace> {&DionsFace::registerAliasGenerate, "true","false"});
    this->bindMethod("registerAlias", RPCMethod<DionsFace> {&DionsFace::registerAlias, "true","false"});
    this->bindMethod("alias", RPCMethod<DionsFace> {&DionsFace::alias, "true","false"});
    this->bindMethod("statusList", RPCMethod<DionsFace> {&DionsFace::statusList, "true","false"});
    this->bindMethod("updateEncrypt", RPCMethod<DionsFace> {&DionsFace::updateEncrypt, "true","false"});
    this->bindMethod("downloadDecrypt", RPCMethod<DionsFace> {&DionsFace::downloadDecrypt, "true","false"});
    this->bindMethod("downloadDecryptEPID", RPCMethod<DionsFace> {&DionsFace::downloadDecryptEPID, "true","false"});
    this->bindMethod("ioget", RPCMethod<DionsFace> {&DionsFace::ioget, "true","false"});
    this->bindMethod("extract", RPCMethod<DionsFace> {&DionsFace::extract, "true","false"});
    this->bindMethod("vtx", RPCMethod<DionsFace> {&DionsFace::vtx, "true","false"});
    this->bindMethod("mapVertex", RPCMethod<DionsFace> {&DionsFace::mapVertex, "true","false"});
    this->bindMethod("vtxtrace", RPCMethod<DionsFace> {&DionsFace::vtxtrace, "true","false"});
    this->bindMethod("mapProject", RPCMethod<DionsFace> {&DionsFace::mapProject, "true","false"});
    this->bindMethod("projection", RPCMethod<DionsFace> {&DionsFace::projection, "true","false"});
    this->bindMethod("xstat", RPCMethod<DionsFace> {&DionsFace::xstat, "true","false"});
    this->bindMethod("svtx", RPCMethod<DionsFace> {&DionsFace::svtx, "true","false"});
    this->bindMethod("simplexU", RPCMethod<DionsFace> {&DionsFace::simplexU, "true","false"});
    this->bindMethod("psimplex", RPCMethod<DionsFace> {&DionsFace::psimplex, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"DIONS", "dvm.1.0"}};
  }

  inline virtual void aliasList(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->aliasList(req,false);
  }

  inline virtual void vertexScan(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->vertexScan(req,false);
  }
  inline virtual void gw1(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->gw1(req,false);
  }
  inline virtual void myRSAKeys(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->myRSAKeys(req,false);
  }
  inline virtual void myRSAKeys__(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->myRSAKeys__(req,false);
  }
  inline virtual void publicKeyExports(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->publicKeyExports(req,false);
  }
  inline virtual void publicKeys(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->publicKeys(req,false);
  }
  inline virtual void decryptedMessageList(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->decryptedMessageList(req,false);
  }
  inline virtual void plainTextMessageList(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->plainTextMessageList(req,false);
  }
  inline virtual void externFrame__(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->externFrame__(req,false);
  }
  inline virtual void internFrame__(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->internFrame__(req,false);
  }
  inline virtual void aliasOut(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->aliasOut(req,false);
  }
  inline virtual void nodeValidate(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->nodeValidate(req,false);
  }
  inline virtual void validateLocator(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->validateLocator(req,false);
  }
  inline virtual void nodeRetrieve(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->nodeRetrieve(req,false);
  }
  inline virtual void getNodeRecord(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->getNodeRecord(req,false);
  }
  inline virtual void aliasList__(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->aliasList__(req,false);
  }
  inline virtual void nodeDebug(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->nodeDebug(req,false);
  }
  inline virtual void nodeDebug1(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->nodeDebug1(req,false);
  }
  inline virtual void transform(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->transform(req,false);
  }
  inline virtual void primaryCXValidate(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->primaryCXValidate(req,false);
  }
  inline virtual void vextract(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->vextract(req,false);
  }
  inline virtual void vEPID(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->vEPID(req,false);
  }
  inline virtual void validate(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->validate(req,false);
  }
  inline virtual void transientStatus__C(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->transientStatus__C(req,false);
  }
  inline virtual void updateEncryptedAliasFile(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->updateEncryptedAliasFile(req,false);
  }
  inline virtual void updateEncryptedAlias(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->updateEncryptedAlias(req,false);
  }
  inline virtual void decryptAlias(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->decryptAlias(req,false);
  }
  inline virtual void transferEncryptedExtPredicate(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->transferEncryptedExtPredicate(req,false);
  }
  inline virtual void transferEncryptedAlias(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->transferEncryptedAlias(req,false);
  }
  inline virtual void transferAlias(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->transferAlias(req,false);
  }
  inline virtual void uC(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->uC(req,false);
  }
  inline virtual void transientStatus__(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->transientStatus__(req,false);
  }
  inline virtual void updateAliasFile(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->updateAliasFile(req,false);
  }
  inline virtual void updateAlias_executeContractPayload(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->updateAlias_executeContractPayload(req,false);
  }
  inline virtual void updateAlias(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->updateAlias(req,false);
  }
  inline virtual void publicKey(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->publicKey(req,false);
  }
  inline virtual void sendSymmetric(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->sendSymmetric(req,false);
  }
  inline virtual void sendPublicKey(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->sendPublicKey(req,false);
  }
  inline virtual void sendPlainMessage(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->sendPlainMessage(req,false);
  }
  inline virtual void sendMessage(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->sendMessage(req,false);
  }
  inline virtual void registerAliasGenerate(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->registerAliasGenerate(req,false);
  }
  inline virtual void registerAlias(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->registerAlias(req,false);
  }
  inline virtual void alias(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->alias(req,false);
  }
  inline virtual void statusList(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->statusList(req,false);
  }
  inline virtual void updateEncrypt(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->updateEncrypt(req,false);
  }
  inline virtual void downloadDecrypt(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->downloadDecrypt(req,false);
  }
  inline virtual void downloadDecryptEPID(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->downloadDecryptEPID(req,false);
  }
  inline virtual void ioget(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->ioget(req,false);
  }
  inline virtual void extract(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->extract(req,false);
  }
  inline virtual void vtx(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->vtx(req,false);
  }
  inline virtual void mapVertex(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->mapVertex(req,false);
  }
  inline virtual void vtxtrace(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->vtxtrace(req,false);
  }
  inline virtual void mapProject(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->mapProject(req,false);
  }
  inline virtual void projection(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->projection(req,false);
  }
  inline virtual void xstat(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->xstat(req,false);
  }
  inline virtual void svtx(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->svtx(req,false);
  }
  inline virtual void simplexU(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->simplexU(req,false);
  }
  inline virtual void psimplex(const json_spirit::Array& req,json_spirit::Value& res) 
  {
    (void)req;
    res = this->psimplex(req,false);
  }
  ///////////////////////////////////////////////////////////////////////////////////

  virtual json_spirit::Value aliasList(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value vertexScan(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value gw1(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value myRSAKeys(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value myRSAKeys__(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value publicKeyExports(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value publicKeys(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value decryptedMessageList(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value plainTextMessageList(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value externFrame__(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value internFrame__(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value aliasOut(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value nodeValidate(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value validateLocator(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value nodeRetrieve(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getNodeRecord(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value aliasList__(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value nodeDebug(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value nodeDebug1(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value transform(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value primaryCXValidate(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value vextract(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value vEPID(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value validate(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value transientStatus__C(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value updateEncryptedAliasFile(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value updateEncryptedAlias(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value decryptAlias(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value transferEncryptedExtPredicate(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value transferEncryptedAlias(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value transferAlias(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value uC(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value transientStatus__(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value updateAliasFile(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value updateAlias_executeContractPayload(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value updateAlias(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value publicKey(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendSymmetric(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendPublicKey(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendPlainMessage(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendMessage(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value registerAliasGenerate(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value registerAlias(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value alias(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value statusList(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value updateEncrypt(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value downloadDecrypt(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value downloadDecryptEPID(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value ioget(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value extract(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value vtx(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value mapVertex(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value vtxtrace(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value mapProject(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value projection(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value xstat(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value svtx(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value simplexU(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value psimplex(const json_spirit::Array&,bool) = 0;
};
