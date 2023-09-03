#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include "bitcoinrpc.h"
#include "ui_interface.h"

class RPCServer
{
public:
  RPCServer() = default;
  RPCServer(CClientUIInterface* uiFace) { uiFace_=uiFace; }
  ~RPCServer() = default;

  bool start(std::map<string,string>);

private:
  std::string trans(const char* psz);
  std::string strRPCUserColonPass;
  void ThreadRPCServer();
  void ThreadRPCServer2();
  void ThreadRPCServer3(void* parg);

  std::map<string,string> argsMap_;
  CClientUIInterface* uiFace_;

};

#endif
