#pragma once

#include "TxDB.h"
#include "wallet/WalletDB.h"
#include "rpc/Client.h"
#include "Net.h"
#include "wallet/Wallet.h"
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
#include "Process.h"
#include "DiskBlockLocator.h"
#ifndef WIN32
#include <signal.h>
#endif
#include "libdevcore/OverlayDB.h"
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

