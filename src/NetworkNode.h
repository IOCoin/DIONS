#pragma once 

#include "TxDB.h"
#include "wallet/WalletDB.h"
#include "rpc/client.h"
#include "Net.h"
#include "core/Util.h"
#include "UIInterface.h"
#include "CheckPoints.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <openssl/crypto.h>
#include "libdevcore/DBFactory.h"
#include "libdevcore/OverlayDB.h"
#include "libdevcore/TrieDB.h"
#include "libdevcore/Address.h"
#include "ccoin/Process.h"
#include "DiskBlockLocator.h"
#ifndef WIN32
#include <signal.h>
#endif

#include "rpc/network_face.h"
#include "ccoin/Client.h"

class NetworkNode : public NetworkFace
{
  public:
    NetworkNode() 
    { 
	    pwalletMain_ = new __wx__("wallet.dat");
    }

    NetworkNode(std::map<string,string> argsMap) 
	    : client_(&uiFace_)
    { 
	    this->argsMap_ = argsMap;
	    pwalletMain_ = new __wx__("wallet.dat");
    }

    NetworkNode(boost::filesystem::path const& _dbPath, bool testNet = false);

    ~NetworkNode() = default;

    bool init();
    __wx__* wallet() { return this->pwalletMain_; }

    Client& client() { return this->client_; }

    virtual json_spirit::Value getconnectioncount(const json_spirit::Array&,bool) override;
    virtual json_spirit::Value getpeerinfo(const json_spirit::Array&,bool) override;
    virtual json_spirit::Value sendalert(const json_spirit::Array&,bool) override;

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
    std::unique_ptr<GenericServer<>> rpcServer_;
    CClientUIInterface uiFace_;
};
