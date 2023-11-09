#pragma once

#include "GenericServer.h"

class BlockChainFace : public ServerInterface<BlockChainFace>
{
public:
  BlockChainFace()
  {
    this->bindMethod("getblockhash", RPCMethod<BlockChainFace> {&BlockChainFace::getblockhash, "true","false"});
    this->bindMethod("getblockcount", RPCMethod<BlockChainFace> {&BlockChainFace::getblockcount, "true","false"});
    this->bindMethod("getpowblocks", RPCMethod<BlockChainFace> {&BlockChainFace::getpowblocks, "true","false"});
    this->bindMethod("getpowblocksleft", RPCMethod<BlockChainFace> {&BlockChainFace::getpowblocksleft, "true","false"});
    this->bindMethod("getpowtimeleft", RPCMethod<BlockChainFace> {&BlockChainFace::getpowtimeleft, "true","false"});
    this->bindMethod("getdifficulty", RPCMethod<BlockChainFace> {&BlockChainFace::getdifficulty, "true","false"});
    this->bindMethod("settxfee", RPCMethod<BlockChainFace> {&BlockChainFace::settxfee, "true","false"});
    this->bindMethod("getrawmempool", RPCMethod<BlockChainFace> {&BlockChainFace::getrawmempool, "true","false"});
    this->bindMethod("getblock", RPCMethod<BlockChainFace> {&BlockChainFace::getblock, "true","false"});
    this->bindMethod("getblockbynumber", RPCMethod<BlockChainFace> {&BlockChainFace::getblockbynumber, "true","false"});
    this->bindMethod("getcheckpoint", RPCMethod<BlockChainFace> {&BlockChainFace::getcheckpoint, "true","false"});
    this->bindMethod("gettxout", RPCMethod<BlockChainFace> {&BlockChainFace::gettxout, "true","false"});
    this->bindMethod("getbestblockhash", RPCMethod<BlockChainFace> {&BlockChainFace::getbestblockhash, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"BLOCKCHAIN", "dvm.1.0"}};
  }

  inline virtual void getblockhash(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getblockhash(req,false);
  }
  inline virtual void getblockcount(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getblockcount(req,false);
  }
  inline virtual void getpowblocks(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getpowblocks(req,false);
  }
  inline virtual void getpowblocksleft(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getpowblocksleft(req,false);
  }
  inline virtual void getpowtimeleft(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getpowtimeleft(req,false);
  }
  inline virtual void getdifficulty(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getdifficulty(req,false);
  }
  inline virtual void settxfee(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->settxfee(req,false);
  }
  inline virtual void getrawmempool(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getrawmempool(req,false);
  }
  inline virtual void getblock(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getblock(req,false);
  }
  inline virtual void getblockbynumber(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getblockbynumber(req,false);
  }
  inline virtual void getcheckpoint(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getcheckpoint(req,false);
  }
  inline virtual void gettxout(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->gettxout(req,false);
  }
  inline virtual void getbestblockhash(const json_spirit::Array& req, json_spirit::Value& res)
  {
    (void)req;
    res = this->getbestblockhash(req,false);
  }
  ///////////////////////////////////////////////////////////////////////////////////

  virtual json_spirit::Value getblockcount(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getpowblocks(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getpowblocksleft(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getpowtimeleft(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getdifficulty(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value settxfee(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getrawmempool(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getblockhash(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getblock(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getblockbynumber(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getcheckpoint(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value gettxout(const json_spirit::Array& req, bool fHelp) = 0;
  virtual json_spirit::Value getbestblockhash(const json_spirit::Array& req, bool fHelp) = 0;
};
