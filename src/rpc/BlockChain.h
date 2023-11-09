#pragma once

#include "BlockChainFace.h"

class BlockChain : public BlockChainFace
{
public:
  BlockChain()
  {
  }
  virtual json_spirit::Value getbestblockhash(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getblockcount(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getpowblocks(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getpowblocksleft(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getpowtimeleft(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getdifficulty(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value settxfee(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getrawmempool(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getblockhash(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getblock(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getblockbynumber(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value getcheckpoint(const json_spirit::Array& params, bool fHelp) override;
  virtual json_spirit::Value gettxout(const json_spirit::Array& params, bool fHelp) override;
};

