#ifndef NETWORK_NODE_H
#define NETWORK_NODE_H

#include "txdb.h"
#include "wallet/walletdb.h"
#include "rpc/bitcoin.h"
#include "net.h"
#include "util.h"
#include "ui_interface.h"
#include "checkpoints.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <openssl/crypto.h>
#include "ptrie/DBFactory.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/TrieDB.h"
#include "ptrie/Address.h"
#include "process.h"
#include "disk_block_locator.h"
#ifndef WIN32
#include <signal.h>
#endif
#include "ptrie/OverlayDB.h"

#include "rpc/server.h"
#include "client.h"

class NetworkNode
{
  public:
    NetworkNode() 
    { 
	    pwalletMain_ = new __wx__("wallet.dat");
    }

    NetworkNode(std::map<string,string> argsMap) 
	    : client_(&uiFace),
	      rpcServer_(&uiFace)
    { 
	    this->argsMap_ = argsMap;
	    pwalletMain_ = new __wx__("wallet.dat");
	    //this->rpcServer_.start(this->argsMap_);

    }

    NetworkNode(boost::filesystem::path const& _dbPath, bool testNet = false);

    ~NetworkNode() = default;

    bool init();
    __wx__* wallet() { return this->pwalletMain_; }

    Client& client() { return this->client_; }

  private:
    bool InitError_(const std::string &str);
    bool InitWarning_(const std::string &str);
    bool Bind_(const CService &addr, bool fError = true);
    bool InitSanityCheck_();

    std::map<string,string> argsMap_;
    __wx__* pwalletMain_;
    LocatorNodeDB* ln1Db_;
    VertexNodeDB* vertexDB_;
    Client client_;
    RPCServer rpcServer_;
    CClientUIInterface uiFace;
};

#endif
