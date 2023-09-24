#pragma once

#include "generic_server.h"

class WalletFace : public ServerInterface<WalletFace>
{
public:
  WalletFace()
  {
    this->bindMethod("getbalance", RPCMethod<WalletFace> {&WalletFace::getbalance, "true","false"});
    this->bindMethod("getinfo", RPCMethod<WalletFace> {&WalletFace::getinfo, "true","false"});
    this->bindMethod("getnewpubkey", RPCMethod<WalletFace> {&WalletFace::getnewpubkey, "true","false"});
    this->bindMethod("getnewaddress", RPCMethod<WalletFace> {&WalletFace::getnewaddress, "true","false"});
    this->bindMethod("sectionlog", RPCMethod<WalletFace> {&WalletFace::sectionlog, "true","false"});
    this->bindMethod("shade", RPCMethod<WalletFace> {&WalletFace::shade, "true","false"});
    this->bindMethod("sr71", RPCMethod<WalletFace> {&WalletFace::sr71, "true","false"});
    this->bindMethod("center__base__0", RPCMethod<WalletFace> {&WalletFace::center__base__0, "true","false"});
    this->bindMethod("getaccountaddres", RPCMethod<WalletFace> {&WalletFace::getaccountaddress, "true","false"});
    this->bindMethod("setaccount", RPCMethod<WalletFace> {&WalletFace::setaccount, "true","false"});
    this->bindMethod("sa", RPCMethod<WalletFace> {&WalletFace::sa, "true","false"});
    this->bindMethod("getaccount", RPCMethod<WalletFace> {&WalletFace::getaccount, "true","false"});
    this->bindMethod("getaddressesbyaccount", RPCMethod<WalletFace> {&WalletFace::getaddressesbyaccount, "true","false"});
    this->bindMethod("addresstodion", RPCMethod<WalletFace> {&WalletFace::addresstodion, "true","false"});
    this->bindMethod("sendtodion", RPCMethod<WalletFace> {&WalletFace::sendtodion, "true","false"});
    this->bindMethod("sendtoaddress", RPCMethod<WalletFace> {&WalletFace::sendtoaddress, "true","false"});
    this->bindMethod("listaddressgroupings", RPCMethod<WalletFace> {&WalletFace::listaddressgroupings, "true","false"});
    this->bindMethod("signmessage", RPCMethod<WalletFace> {&WalletFace::signmessage, "true","false"});
    this->bindMethod("verifymessage", RPCMethod<WalletFace> {&WalletFace::verifymessage, "true","false"});
    this->bindMethod("xtu_url", RPCMethod<WalletFace> {&WalletFace::xtu_url, "true","false"});
    this->bindMethod("getreceivedbyaddress", RPCMethod<WalletFace> {&WalletFace::getreceivedbyaddress, "true","false"});
    this->bindMethod("gra", RPCMethod<WalletFace> {&WalletFace::gra, "true","false"});
    this->bindMethod("getreceivedbyaccount", RPCMethod<WalletFace> {&WalletFace::getreceivedbyaccount, "true","false"});
    this->bindMethod("pending", RPCMethod<WalletFace> {&WalletFace::pending, "true","false"});
    this->bindMethod("movecmd", RPCMethod<WalletFace> {&WalletFace::movecmd, "true","false"});
    this->bindMethod("sendfrom", RPCMethod<WalletFace> {&WalletFace::sendfrom, "true","false"});
    this->bindMethod("sendmany", RPCMethod<WalletFace> {&WalletFace::sendmany, "true","false"});
    this->bindMethod("addmultisigaddress", RPCMethod<WalletFace> {&WalletFace::addmultisigaddress, "true","false"});
    this->bindMethod("addredeemscript", RPCMethod<WalletFace> {&WalletFace::addredeemscript, "true","false"});
    this->bindMethod("listreceivedbyaddress", RPCMethod<WalletFace> {&WalletFace::listreceivedbyaddress, "true","false"});
    this->bindMethod("listreceivedbyaccount", RPCMethod<WalletFace> {&WalletFace::listreceivedbyaccount, "true","false"});
    this->bindMethod("listtransactions__", RPCMethod<WalletFace> {&WalletFace::listtransactions__, "true","false"});
    this->bindMethod("listtransactions", RPCMethod<WalletFace> {&WalletFace::listtransactions, "true","false"});
    this->bindMethod("listaccounts", RPCMethod<WalletFace> {&WalletFace::listaccounts, "true","false"});
    this->bindMethod("listsinceblock", RPCMethod<WalletFace> {&WalletFace::listsinceblock, "true","false"});
    this->bindMethod("gettransaction", RPCMethod<WalletFace> {&WalletFace::gettransaction, "true","false"});
    this->bindMethod("backupwallet", RPCMethod<WalletFace> {&WalletFace::backupwallet, "true","false"});
    this->bindMethod("keypoolrefill", RPCMethod<WalletFace> {&WalletFace::keypoolrefill, "true","false"});
    this->bindMethod("walletpassphrase", RPCMethod<WalletFace> {&WalletFace::walletpassphrase, "true","false"});
    this->bindMethod("walletpassphrasechange", RPCMethod<WalletFace> {&WalletFace::walletpassphrasechange, "true","false"});
    this->bindMethod("walletlockstatus", RPCMethod<WalletFace> {&WalletFace::walletlockstatus, "true","false"});
    this->bindMethod("walletlock", RPCMethod<WalletFace> {&WalletFace::walletlock, "true","false"});
    this->bindMethod("getencryptionstatus", RPCMethod<WalletFace> {&WalletFace::getencryptionstatus, "true","false"});
    this->bindMethod("encryptwallet", RPCMethod<WalletFace> {&WalletFace::encryptwallet, "true","false"});
    this->bindMethod("validateaddress", RPCMethod<WalletFace> {&WalletFace::validateaddress, "true","false"});
    this->bindMethod("validatepubkey", RPCMethod<WalletFace> {&WalletFace::validatepubkey, "true","false"});
    this->bindMethod("reservebalance", RPCMethod<WalletFace> {&WalletFace::reservebalance, "true","false"});
    this->bindMethod("checkwallet", RPCMethod<WalletFace> {&WalletFace::checkwallet, "true","false"});
    this->bindMethod("repairwallet", RPCMethod<WalletFace> {&WalletFace::repairwallet, "true","false"});
    this->bindMethod("resendtx", RPCMethod<WalletFace> {&WalletFace::resendtx, "true","false"});
    this->bindMethod("makekeypair", RPCMethod<WalletFace> {&WalletFace::makekeypair, "true","false"});
    this->bindMethod("rmtx", RPCMethod<WalletFace> {&WalletFace::rmtx, "true","false"});
    this->bindMethod("shadesend", RPCMethod<WalletFace> {&WalletFace::shadesend, "true","false"});
    this->bindMethod("__vtx_s", RPCMethod<WalletFace> {&WalletFace::__vtx_s, "true","false"});
    this->bindMethod("sublimateYdwi", RPCMethod<WalletFace> {&WalletFace::sublimateYdwi, "true","false"});
    this->bindMethod("shadeK", RPCMethod<WalletFace> {&WalletFace::shadeK, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"WALLET", "dvm.1.0"}};
  }

  inline virtual void getbalance(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getbalance(req,false);
  }
  inline virtual void getinfo(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getinfo(req,false);
  }
  inline virtual void getnewpubkey(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getnewpubkey(req,false);
  }
  inline virtual void getnewaddress(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getnewaddress(req,false);
  }
  inline virtual void sectionlog(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sectionlog(req,false);
  }
  inline virtual void shade(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->shade(req,false);
  }
  inline virtual void sr71(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sr71(req,false);
  }
  inline virtual void center__base__0(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->center__base__0(req,false);
  }
  inline virtual void getaccountaddress(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getaccountaddress(req,false);
  }
  inline virtual void setaccount(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->setaccount(req,false);
  }
  inline virtual void sa(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sa(req,false);
  }
  inline virtual void getaccount(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getaccount(req,false);
  }
  inline virtual void getaddressesbyaccount(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getaddressesbyaccount(req,false);
  }
  inline virtual void addresstodion(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->addresstodion(req,false);
  }
  inline virtual void sendtodion(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sendtodion(req,false);
  }
  inline virtual void sendtoaddress(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sendtoaddress(req,false);
  }
  inline virtual void listaddressgroupings(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->listaddressgroupings(req,false);
  }
  inline virtual void signmessage(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->signmessage(req,false);
  }
  inline virtual void verifymessage(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->verifymessage(req,false);
  }
  inline virtual void xtu_url(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->xtu_url(req,false);
  }
  inline virtual void getreceivedbyaddress(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getreceivedbyaddress(req,false);
  }
  inline virtual void gra(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->gra(req,false);
  }
  inline virtual void getreceivedbyaccount(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getreceivedbyaccount(req,false);
  }
  inline virtual void pending(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->pending(req,false);
  }
  inline virtual void movecmd(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->movecmd(req,false);
  }
  inline virtual void sendfrom(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sendfrom(req,false);
  }
  inline virtual void sendmany(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sendmany(req,false);
  }
  inline virtual void addmultisigaddress(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->addmultisigaddress(req,false);
  }
  inline virtual void addredeemscript(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->addredeemscript(req,false);
  }
  inline virtual void listreceivedbyaddress(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->listreceivedbyaddress(req,false);
  }
  inline virtual void listreceivedbyaccount(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->listreceivedbyaccount(req,false);
  }
  inline virtual void listtransactions__(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->listtransactions__(req,false);
  }
  inline virtual void listtransactions(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->listtransactions(req,false);
  }
  inline virtual void listaccounts(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->listaccounts(req,false);
  }
  inline virtual void listsinceblock(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->listsinceblock(req,false);
  }
  inline virtual void gettransaction(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->gettransaction(req,false);
  }
  inline virtual void backupwallet(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->backupwallet(req,false);
  }
  inline virtual void keypoolrefill(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->keypoolrefill(req,false);
  }
  inline virtual void walletpassphrase(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->walletpassphrase(req,false);
  }
  inline virtual void walletpassphrasechange(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->walletpassphrasechange(req,false);
  }
  inline virtual void walletlockstatus(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->walletlockstatus(req,false);
  }
  inline virtual void walletlock(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->walletlock(req,false);
  }
  inline virtual void getencryptionstatus(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->getencryptionstatus(req,false);
  }
  inline virtual void encryptwallet(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->encryptwallet(req,false);
  }
  inline virtual void validateaddress(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->validateaddress(req,false);
  }
  inline virtual void validatepubkey(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->validatepubkey(req,false);
  }
  inline virtual void reservebalance(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->reservebalance(req,false);
  }
  inline virtual void checkwallet(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->checkwallet(req,false);
  }
  inline virtual void repairwallet(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->repairwallet(req,false);
  }
  inline virtual void resendtx(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->resendtx(req,false);
  }
  inline virtual void makekeypair(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->makekeypair(req,false);
  }
  inline virtual void rmtx(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->rmtx(req,false);
  }
  inline virtual void shadesend(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->shadesend(req,false);
  }
  inline virtual void __vtx_s(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->__vtx_s(req,false);
  }
  inline virtual void sublimateYdwi(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->sublimateYdwi(req,false);
  }
  inline virtual void shadeK(const json_spirit::Array& req,json_spirit::Value& res)
  {
    (void)req;
    res = this->shadeK(req,false);
  }

  virtual json_spirit::Value getbalance(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getinfo(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getnewpubkey(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getnewaddress(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sectionlog(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value shade(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sr71(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value center__base__0(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getaccountaddress(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value setaccount(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sa(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getaccount(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getaddressesbyaccount(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value addresstodion(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendtodion(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendtoaddress(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value listaddressgroupings(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value signmessage(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value verifymessage(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value xtu_url(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getreceivedbyaddress(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value gra(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getreceivedbyaccount(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value pending(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value movecmd(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendfrom(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value sendmany(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value addmultisigaddress(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value addredeemscript(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value listreceivedbyaddress(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value listreceivedbyaccount(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value listtransactions__(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value listtransactions(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value listaccounts(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value listsinceblock(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value gettransaction(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value backupwallet(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value keypoolrefill(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value walletpassphrase(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value walletpassphrasechange(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value walletlockstatus(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value walletlock(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value getencryptionstatus(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value encryptwallet(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value validateaddress(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value validatepubkey(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value reservebalance(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value checkwallet(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value repairwallet(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value resendtx(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value makekeypair(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value rmtx(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value shadesend(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value __vtx_s(const json_spirit::Array&,bool ) = 0;
  virtual json_spirit::Value sublimateYdwi(const json_spirit::Array&,bool) = 0;
  virtual json_spirit::Value shadeK(const json_spirit::Array&,bool) = 0;
};
