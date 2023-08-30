#ifndef CLIENT_H
#define CLIENT_H

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
class Client
{
  public:
    Client() = default;
    Client(CClientUIInterface* uiFace) { uiFace_=uiFace; }

    ~Client() = default;

    dev::OverlayDB const& stateDB() const { return overlayDB_; }

  private:
    bool init(boost::filesystem::path const& _dbPath);

    CClientUIInterface* uiFace_;
    dev::OverlayDB overlayDB_;
};

#endif
