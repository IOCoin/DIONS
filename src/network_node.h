#ifndef NETWORK_NODE_H
#define NETWORK_NODE_H

#include "txdb.h"
#include "walletdb.h"
#include "bitcoinrpc.h"
#include "net.h"
#include "init.h"
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
#include "main.h"
#include "disk_block_locator.h"
#ifndef WIN32
#include <signal.h>
#endif
#include "ptrie/OverlayDB.h"

#include "client.h"

class NetworkNode
{
  public:
    NetworkNode() = default;

    NetworkNode(boost::filesystem::path const& _dbPath, bool testNet = false);

    ~NetworkNode() = default;

    bool init();

    Client& client() { return this->client_; }

  private:
    __wx__ pwalletMain_;
    LocatorNodeDB* ln1Db_;
    VertexNodeDB* vertexDB_;
    Client client_;
    CClientUIInterface uiFace;
};

#endif
