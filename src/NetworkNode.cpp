#include "NetworkNode.h"
#include "Net.h"
#include "Alert.h"
#include "rpc/Client.h"
#include "rpc/Mining.h"
#include "rpc/BlockChain.h"
#include "rpc/RawTransaction.h"
#include "wallet/Wallet.h"
#include "wallet/DB.h"
#include "wallet/WalletDB.h"
#include "reflect/Base.h"
using namespace std;
using namespace boost;
using namespace json_spirit;

enum Checkpoints::CPMode CheckpointsMode_;
string strDNSSeedNode_;
extern ConfigurationState globalState;
extern CClientUIInterface uiInterface;
extern void xsc(CBlockIndex*);
static void HandleSIGTERM(int)
{
  fRequestShutdown = true;
}
static void HandleSIGHUP(int)
{
  fReopenDebugLog = true;
}

bool NetworkNode::InitError_(const std::string &str)
{
  uiInterface.ThreadSafeMessageBox(str, _("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::MODAL);
  return false;
}
bool NetworkNode::InitWarning_(const std::string &str)
{
  uiInterface.ThreadSafeMessageBox(str, _("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
  return true;
}

bool NetworkNode::InitSanityCheck_()
{
  if(!ECC_InitSanityCheck())
  {
    this->InitError_("OpenSSL appears to lack support for elliptic curve cryptography. For more "
              "information, visit https://en.bitcoin.it/wiki/OpenSSL_and_EC_Libraries");
    return false;
  }

  return true;
}

bool NetworkNode::Bind_(const CService &addr, bool fError )
{
  if (IsLimited(addr))
  {
    return false;
  }

  std::string strError;

  if (!BindListenPort(addr, strError))
  {
    if (fError)
    {
      return this->InitError_(strError);
    }

    return false;
  }

  return true;
}

NetworkNode::NetworkNode(boost::filesystem::path const& _dbPath, bool testNet)
{
}

bool NetworkNode::init()
{
#ifdef _MSC_VER
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifdef WIN32
#ifndef PROCESS_DEP_ENABLE
#define PROCESS_DEP_ENABLE 0x00000001
#endif
  typedef BOOL (WINAPI *PSETPROCDEPPOL)(DWORD);
  PSETPROCDEPPOL setProcDEPPol = (PSETPROCDEPPOL)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SetProcessDEPPolicy");

  if (setProcDEPPol != NULL)
  {
    setProcDEPPol(PROCESS_DEP_ENABLE);
  }

#endif
#ifndef WIN32
  umask(077);
  struct sigaction sa;
  sa.sa_handler = HandleSIGTERM;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  struct sigaction sa_hup;
  sa_hup.sa_handler = HandleSIGHUP;
  sigemptyset(&sa_hup.sa_mask);
  sa_hup.sa_flags = 0;
  sigaction(SIGHUP, &sa_hup, NULL);
#endif
  nNodeLifespan = GetArg("-addrlifespan", 7);
  fUseFastIndex = GetBoolArg("-fastindex", true);
  CheckpointsMode_ = Checkpoints::STRICT;
  std::string strCpMode = GetArg("-cppolicy", "strict");
  strDNSSeedNode_ = GetArg("-dnsseednode", "default");

  if(strCpMode == "strict")
  {
    CheckpointsMode_ = Checkpoints::STRICT;
  }

  if(strCpMode == "advisory")
  {
    CheckpointsMode_ = Checkpoints::ADVISORY;
  }

  if(strCpMode == "permissive")
  {
    CheckpointsMode_ = Checkpoints::PERMISSIVE;
  }

  nDerivationMethodIndex = 0;
  fTestNet = GetBoolArg("-testnet");

  if (fTestNet)
  {
    SoftSetBoolArg("-irc", true);
  }

  fViewWallet = GetBoolArg("-viewwallet");

  if(fViewWallet)
  {
    if(boost::filesystem::exists(GetDataDir() / "wallet.dat"))
    {
      return InitError_(_("Initialization error. Configured as view wallet but wallet.dat file exists in configuration directory. I/O Coin shutting down."));
    }
  }
  else
  {
    if(boost::filesystem::exists(GetDataDir() / "view.dat"))
    {
      return InitError_(_("Initialization error. Configured as default wallet but view.dat file exists in configuration directory. I/O Coin shutting down."));
    }
  }

  if (mapArgs.count("-bind"))
  {
    SoftSetBoolArg("-listen", true);
  }
  if (mapArgs.count("-connect") && mapMultiArgs["-connect"].size() > 0)
  {
    SoftSetBoolArg("-dnsseed", false);
    SoftSetBoolArg("-listen", false);
  }

  if (mapArgs.count("-proxy"))
  {
    SoftSetBoolArg("-listen", false);
  }

  if (!GetBoolArg("-listen", true))
  {
    SoftSetBoolArg("-discover", false);
  }

  if (mapArgs.count("-externalip"))
  {
    SoftSetBoolArg("-discover", false);
  }

  if (GetBoolArg("-salvagewallet"))
  {
    SoftSetBoolArg("-rescan", true);
  }

  if (GetBoolArg("-zapwallettxes", false))
  {
    if (SoftSetBoolArg("-rescan", true))
    {
      printf("AppInit2 : parameter interaction: -zapwallettxes=1 -> setting -rescan=1\n");
    }
  }

  fDebug = GetBoolArg("-debug");

  if (fDebug)
  {
    fDebugNet = true;
  }
  else
  {
    fDebugNet = GetBoolArg("-debugnet");
  }

#if !defined(WIN32) && !defined(QT_GUI)
  fDaemon = GetBoolArg("-daemon");
#else
  fDaemon = false;
#endif

  if (fDaemon)
  {
    fServer = true;
  }
  else
  {
    fServer = GetBoolArg("-server");
  }



#if !defined(QT_GUI)
  fServer = true;
#endif
  fPrintToConsole = GetBoolArg("-printtoconsole");
  fPrintToDebugger = GetBoolArg("-printtodebugger");
  fLogTimestamps = GetBoolArg("-logtimestamps");

  if (mapArgs.count("-timeout"))
  {
    int nNewTimeout = GetArg("-timeout", 5000);

    if (nNewTimeout > 0 && nNewTimeout < 600000)
    {
      nConnectTimeout = nNewTimeout;
    }
  }

  if (mapArgs.count("-paytxfee"))
  {
    if (!ParseMoney(mapArgs["-paytxfee"], globalState.nTransactionFee))
    {
      return InitError_(strprintf(_("Invalid amount for -paytxfee=<amount>: '%s'"), mapArgs["-paytxfee"].c_str()));
    }

    if (globalState.nTransactionFee > 0.25 * COIN)
    {
      InitWarning_(_("Warning: -paytxfee is set very high! This is the transaction fee you will pay if you send a transaction."));
    }
  }

  fConfChange = GetBoolArg("-confchange", false);
  fEnforceCanonical = GetBoolArg("-enforcecanonical", true);

  if (mapArgs.count("-mininput"))
  {
    if (!ParseMoney(mapArgs["-mininput"], nMinimumInputValue))
    {
      return InitError_(strprintf(_("Invalid amount for -mininput=<amount>: '%s'"), mapArgs["-mininput"].c_str()));
    }
  }

  if (!InitSanityCheck_())
  {
    return InitError_(_("Initialization sanity check failed. I/OCoin is shutting down."));
  }

  std::string strDataDir = GetDataDir().string();
  std::string strWalletFileName = GetArg("-wallet", "wallet.dat");

  if (strWalletFileName != boost::filesystem::basename(strWalletFileName) + boost::filesystem::extension(strWalletFileName))
  {
    return InitError_(strprintf(_("Wallet %s resides outside data directory %s."), strWalletFileName.c_str(), strDataDir.c_str()));
  }

  boost::filesystem::path pathLockFile = GetDataDir() / ".lock";
  FILE* file = fopen(pathLockFile.string().c_str(), "a");

  if (file)
  {
    fclose(file);
  }

  static boost::interprocess::file_lock lock(pathLockFile.string().c_str());

  if (!lock.try_lock())
  {
    return InitError_(strprintf(_("Cannot obtain a lock on data directory %s.  I/OCoin is probably already running."), strDataDir.c_str()));
  }

#if !defined(WIN32) && !defined(QT_GUI)

  if (fDaemon)
  {
    pid_t pid = fork();

    if (pid < 0)
    {
      fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
      return false;
    }

    if (pid > 0)
    {
      CreatePidFile(GetPidFile(), pid);
      return true;
    }

    pid_t sid = setsid();

    if (sid < 0)
    {
      fprintf(stderr, "Error: setsid() returned %d errno %d\n", sid, errno);
    }
  }

#endif

  if (GetBoolArg("-shrinkdebugfile", !fDebug))
  {
    ShrinkDebugFile();
  }

  printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
  printf("I/OCoin version %s (%s)\n", FormatFullVersion().c_str(), CLIENT_DATE.c_str());
  printf("Using OpenSSL version %s\n", SSLeay_version(SSLEAY_VERSION));

  if (!fLogTimestamps)
  {
    printf("Startup time: %s\n", DateTimeStrFormat("%x %H:%M:%S", GetTime()).c_str());
  }

  printf("Default data directory %s\n", GetDefaultDataDir().string().c_str());
  printf("Used data directory %s\n", strDataDir.c_str());
  std::ostringstream strErrors;

  if (fDaemon)
  {
    fprintf(stdout, "I/OCoin server starting\n");
  }

  int64_t nStart;
  uiInterface.InitMessage(_("Verifying database integrity..."));

  if (!bitdb.Open(GetDataDir()))
  {
    string msg = strprintf(_("Error initializing database environment %s!"
                             " To recover, BACKUP THAT DIRECTORY, then remove"
                             " everything from it except for wallet.dat."), strDataDir.c_str());
    return InitError_(msg);
  }

  if (GetBoolArg("-salvagewallet"))
  {
    if (!__wx__DB::Recover(bitdb, strWalletFileName, true))
    {
      return false;
    }
  }

  if (boost::filesystem::exists(GetDataDir() / strWalletFileName))
  {
    CDBEnv::VerifyResult r = bitdb.Verify(strWalletFileName, __wx__DB::Recover);

    if (r == CDBEnv::RECOVER_OK)
    {
      string msg = strprintf(_("Warning: wallet.dat corrupt, data salvaged!"
                               " Original wallet.dat saved as wallet.{timestamp}.bak in %s; if"
                               " your balance or transactions are incorrect you should"
                               " restore from a backup."), strDataDir.c_str());
      uiInterface.ThreadSafeMessageBox(msg, _("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
    }

    if (r == CDBEnv::RECOVER_FAIL)
    {
      return InitError_(_("wallet.dat corrupt, salvage failed"));
    }
  }

  int nSocksVersion = GetArg("-socks", 5);

  if (nSocksVersion != 4 && nSocksVersion != 5)
  {
    return InitError_(strprintf(_("Unknown -socks proxy version requested: %i"), nSocksVersion));
  }

  if (mapArgs.count("-onlynet"))
  {
    std::set<enum Network> nets;
    BOOST_FOREACH(std::string snet, mapMultiArgs["-onlynet"])
    {
      enum Network net = ParseNetwork(snet);

      if (net == NET_UNROUTABLE)
      {
        return InitError_(strprintf(_("Unknown network specified in -onlynet: '%s'"), snet.c_str()));
      }

      nets.insert(net);
    }

    for (int n = 0; n < NET_MAX; n++)
    {
      enum Network net = (enum Network)n;

      if (!nets.count(net))
      {
        SetLimited(net);
      }
    }
  }

  CService addrProxy;
  bool fProxy = false;

  if (mapArgs.count("-proxy"))
  {
    addrProxy = CService(mapArgs["-proxy"], 9050);

    if (!addrProxy.IsValid())
    {
      return InitError_(strprintf(_("Invalid -proxy address: '%s'"), mapArgs["-proxy"].c_str()));
    }

    if (!IsLimited(NET_IPV4))
    {
      SetProxy(NET_IPV4, addrProxy, nSocksVersion);
    }

    if (nSocksVersion > 4)
    {
      if (!IsLimited(NET_IPV6))
      {
        SetProxy(NET_IPV6, addrProxy, nSocksVersion);
      }

      SetNameProxy(addrProxy, nSocksVersion);
    }

    fProxy = true;
  }

  if (!(mapArgs.count("-tor") && mapArgs["-tor"] == "0") && (fProxy || mapArgs.count("-tor")))
  {
    CService addrOnion;

    if (!mapArgs.count("-tor"))
    {
      addrOnion = addrProxy;
    }
    else
    {
      addrOnion = CService(mapArgs["-tor"], 9050);
    }

    if (!addrOnion.IsValid())
    {
      return InitError_(strprintf(_("Invalid -tor address: '%s'"), mapArgs["-tor"].c_str()));
    }

    SetProxy(NET_TOR, addrOnion, 5);
    SetReachable(NET_TOR);
  }

  fNoListen = !GetBoolArg("-listen", true);
  fDiscover = GetBoolArg("-discover", true);
  fNameLookup = GetBoolArg("-dns", true);
  bool fBound = false;

  if (!fNoListen)
  {
    std::string strError;

    if (mapArgs.count("-bind"))
    {
      BOOST_FOREACH(std::string strBind_, mapMultiArgs["-bind"])
      {
        CService addrBind_;

        if (!Lookup(strBind_.c_str(), addrBind_, GetListenPort(), false))
        {
          return InitError_(strprintf(_("Cannot resolve -bind address: '%s'"), strBind_.c_str()));
        }

        fBound |= Bind_(addrBind_);
      }
    }
    else
    {
      struct in_addr inaddr_any;
      inaddr_any.s_addr = INADDR_ANY;

      if (!IsLimited(NET_IPV6))
      {
        fBound |= Bind_(CService(in6addr_any, GetListenPort()), false);
      }

      if (!IsLimited(NET_IPV4))
      {
        fBound |= Bind_(CService(inaddr_any, GetListenPort()), !fBound);
      }
    }

    if (!fBound)
    {
      return InitError_(_("Failed to listen on any port. Use -listen=0 if you want this."));
    }
  }

  if (mapArgs.count("-externalip"))
  {
    BOOST_FOREACH(string strAddr, mapMultiArgs["-externalip"])
    {
      CService addrLocal(strAddr, GetListenPort(), fNameLookup);

      if (!addrLocal.IsValid())
      {
        return InitError_(strprintf(_("Cannot resolve -externalip address: '%s'"), strAddr.c_str()));
      }

      AddLocal(CService(strAddr, GetListenPort(), fNameLookup), LOCAL_MANUAL);
    }
  }

  if (mapArgs.count("-reservebalance"))
  {
    if (!ParseMoney(mapArgs["-reservebalance"], nReserveBalance))
    {
      InitError_(_("Invalid amount for -reservebalance=<amount>"));
      return false;
    }
  }

  if (mapArgs.count("-checkpointkey"))
  {
    if (!Checkpoints::SetCheckpointPrivKey(GetArg("-checkpointkey", "")))
    {
      InitError_(_("Unable to sign checkpoint, wrong checkpointkey?\n"));
    }
  }

  BOOST_FOREACH(string strDest, mapMultiArgs["-seednode"])
  AddOneShot(strDest);

  this->client_.init(GetDataDir());

  if (!bitdb.Open(GetDataDir()))
  {
    string msg = strprintf(_("Error initializing database environment %s!"
                             " To recover, BACKUP THAT DIRECTORY, then remove"
                             " everything from it except for wallet.dat."), strDataDir.c_str());
    return InitError_(msg);
  }

  if (GetBoolArg("-loadblockindextest"))
  {
    CTxDB txdb("r");
    txdb.LoadBlockIndex();
    PrintBlockTree();
    return false;
  }

  uiInterface.InitMessage(_("Loading block index..."));
  printf("Loading block index...\n");
  nStart = GetTimeMillis();

  if (!LoadBlockIndex())
  {
    return InitError_(_("Error loading blkindex.dat"));
  }

  if (fRequestShutdown)
  {
    printf("Shutdown requested. Exiting.\n");
    return false;
  }

  printf(" block index %15" PRId64 "ms\n", GetTimeMillis() - nStart);

  if (GetBoolArg("-printblockindex") || GetBoolArg("-printblocktree"))
  {
    PrintBlockTree();
    return false;
  }

  if (mapArgs.count("-printblock"))
  {
    string strMatch = mapArgs["-printblock"];
    int nFound = 0;

    for (map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
    {
      uint256 hash = (*mi).first;

      if (strncmp(hash.ToString().c_str(), strMatch.c_str(), strMatch.size()) == 0)
      {
        CBlockIndex* pindex = (*mi).second;
        CBlock block;
        block.ReadFromDisk(pindex);
        block.BuildMerkleTree();
        block.print();
        printf("\n");
        nFound++;
      }
    }

    if (nFound == 0)
    {
      printf("No blocks matching %s were found\n", strMatch.c_str());
    }

    return false;
  }

  if (GetBoolArg("-zapwallettxes", false))
  {
    printf("Zapping all transactions from wallet...");
    __wx__* pwalletMain = new __wx__("wallet.dat");
    DBErrors nZapWalletRet = pwalletMain->ZapWalletTx();

    if (nZapWalletRet != DB_LOAD_OK)
    {
      printf("Error loading wallet.dat: Wallet corrupted");
      return false;
    }

    delete pwalletMain;
    pwalletMain = NULL;
  }

  uiInterface.InitMessage(_("Loading wallet..."));
  printf("Loading wallet...\n");
  nStart = GetTimeMillis();
  bool fFirstRun = true;
  DBErrors nLoadWalletRet = this->pwalletMain_->LoadWallet(fFirstRun);

  if (nLoadWalletRet != DB_LOAD_OK)
  {
    if (nLoadWalletRet == DB_CORRUPT)
    {
      strErrors << _("Error loading wallet.dat: Wallet corrupted") << "\n";
    }
    else if (nLoadWalletRet == DB_NONCRITICAL_ERROR)
    {
      string msg(_("Warning: error reading wallet.dat! All keys read correctly, but transaction data"
                   " or address book entries might be missing or incorrect."));
      uiInterface.ThreadSafeMessageBox(msg, _("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
    }
    else if (nLoadWalletRet == DB_TOO_NEW)
    {
      strErrors << _("Error loading wallet.dat: Wallet requires newer version of I/OCoin") << "\n";
    }
    else if (nLoadWalletRet == DB_NEED_REWRITE)
    {
      strErrors << _("Wallet needed to be rewritten: restart I/OCoin to complete") << "\n";
      printf("%s", strErrors.str().c_str());
      return InitError_(strErrors.str());
    }
    else
    {
      strErrors << _("Error loading wallet.dat") << "\n";
    }
  }

  if (GetBoolArg("-upgradewallet", fFirstRun))
  {
    int nMaxVersion = GetArg("-upgradewallet", 0);

    if (nMaxVersion == 0)
    {
      printf("Performing wallet upgrade to %i\n", FEATURE_LATEST);
      nMaxVersion = CLIENT_VERSION;
      this->pwalletMain_->SetMinVersion(FEATURE_LATEST);
      std::map<CKeyID, int64_t> mk;
      this->pwalletMain_->kt(mk);

      for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
      {
        CKeyID ck = it->first;
        this->pwalletMain_->kd[ck].nVersion = CKeyMetadata::CURRENT_VERSION;
      }
    }
    else
    {
      printf("Allowing wallet upgrade up to %i\n", nMaxVersion);
    }

    if (nMaxVersion < this->pwalletMain_->GetVersion())
    {
      strErrors << _("Cannot downgrade wallet") << "\n";
    }

    this->pwalletMain_->SetMaxVersion(nMaxVersion);
  }

  if (fFirstRun && !fViewWallet)
  {
    RandAddSeedPerfmon();
    CPubKey newDefaultKey;

    if (this->pwalletMain_->GetKeyFromPool(newDefaultKey, false))
    {
      this->pwalletMain_->SetDefaultKey(newDefaultKey);

      if (!this->pwalletMain_->SetAddressBookName(this->pwalletMain_->vchDefaultKey.GetID(), ""))
      {
        strErrors << _("Cannot write default address") << "\n";
      }
    }
  }

  printf("%s", strErrors.str().c_str());
  printf(" wallet      %15" PRId64 "ms\n", GetTimeMillis() - nStart);
  RegisterWallet(this->pwalletMain_);

  if(GetBoolArg("-xscan"))
  {
    boost::filesystem::path dc = GetDataDir() / "aliascache.dat";
    FILE *file = fopen(dc.string().c_str(), "rb");

    if (file)
    {
      boost::filesystem::path dc__ = GetDataDir() / "aliascache.dat.old";
      RenameOver(dc, dc__);
    }
  }

  this->ln1Db_ = new LocatorNodeDB("cr+");
  this->vertexDB_ = new VertexNodeDB("cr+");
  CBlockIndex *pindexRescan = pindexBest;

  if(GetBoolArg("-rescan") || GetBoolArg("-xscan") || GetBoolArg("-upgradewallet"))
  {
    pindexRescan = pindexGenesisBlock;
  }
  else
  {
    __wx__DB walletdb(strWalletFileName);
    CBlockLocator locator;

    if (walletdb.ReadBestBlock(locator))
    {
      pindexRescan = locator.GetBlockIndex();
    }
  }

  if (pindexBest != pindexRescan && pindexBest && pindexRescan && pindexBest->nHeight > pindexRescan->nHeight)
  {
    uiInterface.InitMessage(_("Rescanning..."));

    if(GetBoolArg("-rescan"))
    {
      printf("Rescanning last %i blocks (from block %i)...\n", pindexBest->nHeight - pindexRescan->nHeight, pindexRescan->nHeight);
      nStart = GetTimeMillis();
      this->pwalletMain_->ScanForWalletTransactions(pindexRescan, true);
      printf(" rescan      %15" PRId64 "ms\n", GetTimeMillis() - nStart);
    }

    if(GetBoolArg("-xscan") || GetBoolArg("-upgradewallet"))
    {
      xsc(pindexGenesisBlock);
    }
  }

  if (mapArgs.count("-loadblock"))
  {
    uiInterface.InitMessage(_("Importing blockchain data file."));
    BOOST_FOREACH(string strFile, mapMultiArgs["-loadblock"])
    {
      FILE *file = fopen(strFile.c_str(), "rb");

      if (file)
      {
        LoadExternalBlockFile(file);
      }
    }
    exit(0);
  }

  boost::filesystem::path pathBootstrap = GetDataDir() / "bootstrap.dat";

  if (boost::filesystem::exists(pathBootstrap))
  {
    uiInterface.InitMessage(_("Importing bootstrap blockchain data file."));
    FILE *file = fopen(pathBootstrap.string().c_str(), "rb");

    if (file)
    {
      boost::filesystem::path pathBootstrapOld = GetDataDir() / "bootstrap.dat.old";
      LoadExternalBlockFile(file);
      RenameOver(pathBootstrap, pathBootstrapOld);
    }
  }

  uiInterface.InitMessage(_("Loading addresses..."));
  printf("Loading addresses...\n");
  nStart = GetTimeMillis();
  {
    CAddrDB adb;

    if (!adb.Read(addrman))
    {
      printf("Invalid or missing peers.dat; recreating\n");
    }
  }
  printf("Loaded %i addresses from peers.dat  %" PRId64 "ms\n",
         addrman.size(), GetTimeMillis() - nStart);

  if (!CheckDiskSpace())
  {
    return false;
  }

  RandAddSeedPerfmon();
  printf("mapBlockIndex.size() = %" PRIszu "\n", mapBlockIndex.size());
  printf("nBestHeight = %d\n", nBestHeight);
  printf("setKeyPool.size() = %" PRIszu "\n", this->pwalletMain_->setKeyPool.size());
  printf("mapWallet.size() = %" PRIszu "\n", this->pwalletMain_->mapWallet.size());
  printf("mapAddressBook.size() = %" PRIszu "\n", this->pwalletMain_->mapAddressBook.size());

  if (!NewThread(StartNode, NULL))
  {
    InitError_(_("Error: could not start node"));
  }

  if (fServer)
  {
    auto dionsPtr = new Dions();
    auto miningPtr = new Mining();
    auto blockChainPtr = new BlockChain();
    auto rawTransactionPtr = new RawTransaction();
    auto reflectPtr = new Reflect();
    this->rpcServer_.reset(new GenericServer<DionsFace,WalletFace,MiningFace,BlockChainFace,RawTransactionFace,ReflectFace,NetworkFace>(dionsPtr,this->pwalletMain_,miningPtr,blockChainPtr,rawTransactionPtr,reflectPtr,this));
    this->rpcServer_->start(this->argsMap_);
  }

  uiInterface.InitMessage(_("Done loading"));
  printf("Done loading\n");

        this->miner_.sleep(GetArg("-minersleep", 500));
        this->miner_.wallet(this->pwalletMain_);
        this->miner_.client(&this->client_);
	this->miner_.start();

  if (!strErrors.str().empty())
  {
    return InitError_(strErrors.str());
  }

  this->pwalletMain_->ReacceptWalletTransactions();
#if !defined(QT_GUI)

  while (1)
  {
    MilliSleep(5000);
  }

#endif
  return true;
}

Value NetworkNode::getconnectioncount(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 0)
    throw runtime_error(
      "getconnectioncount\n"
      "Returns the number of connections to other nodes.");

  LOCK(cs_vNodes);
  return (int)vNodes.size();
}
static void CopyNodeStats(std::vector<CNodeStats>& vstats)
{
  vstats.clear();
  LOCK(cs_vNodes);
  vstats.reserve(vNodes.size());
  BOOST_FOREACH(CNode* pnode, vNodes)
  {
    CNodeStats stats;
    pnode->copyStats(stats);
    vstats.push_back(stats);
  }
}
Value NetworkNode::getpeerinfo(const Array& params, bool fHelp)
{
  if (fHelp || params.size() != 0)
    throw runtime_error(
      "getpeerinfo\n"
      "Returns data about each connected network node.");

  vector<CNodeStats> vstats;
  CopyNodeStats(vstats);
  Array ret;
  BOOST_FOREACH(const CNodeStats& stats, vstats)
  {
    Object obj;
    obj.push_back(Pair("addr", stats.addrName));
    obj.push_back(Pair("services", strprintf("%08" PRIx64, stats.nServices)));
    obj.push_back(Pair("lastsend", (int64_t)stats.nLastSend));
    obj.push_back(Pair("lastrecv", (int64_t)stats.nLastRecv));
    obj.push_back(Pair("conntime", (int64_t)stats.nTimeConnected));
    obj.push_back(Pair("version", stats.nVersion));
    obj.push_back(Pair("subver", stats.strSubVer));
    obj.push_back(Pair("inbound", stats.fInbound));
    obj.push_back(Pair("startingheight", stats.nStartingHeight));
    obj.push_back(Pair("banscore", stats.nMisbehavior));
    ret.push_back(obj);
  }
  return ret;
}
Value NetworkNode::sendalert(const Array& params, bool fHelp)
{
  if (fHelp || params.size() < 6)
    throw runtime_error(
      "sendalert <message> <privatekey> <minver> <maxver> <priority> <id> [cancelupto] [expiredelta]\n"
      "<message> is the alert text message\n"
      "<privatekey> is hex string of alert master private key\n"
      "<minver> is the minimum applicable internal client version\n"
      "<maxver> is the maximum applicable internal client version\n"
      "<priority> is integer priority number\n"
      "<id> is the alert id\n"
      "[cancelupto] cancels all alert id's up to this number\n"
      "[expiredelta] after this many seconds, the alert will expire\n"
      "Returns true or false.");

  CAlert alert;
  CKey key;
  alert.strStatusBar = params[0].get_str();
  alert.nMinVer = params[2].get_int();
  alert.nMaxVer = params[3].get_int();
  alert.nPriority = params[4].get_int();
  alert.nID = params[5].get_int();

  if (params.size() > 6)
  {
    alert.nCancel = params[6].get_int();
  }

  alert.nVersion = PROTOCOL_VERSION;
  alert.nRelayUntil = GetAdjustedTime() + 365*24*60*60;
  int64_t expireDelta = 365*24*60*60;

  if (params.size() > 7)
  {
    expireDelta = params[7].get_int();
  }

  alert.nExpiration = GetAdjustedTime() + expireDelta;
  CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
  sMsg << (CUnsignedAlert)alert;
  alert.vchMsg = vector<unsigned char>(sMsg.begin(), sMsg.end());
  vector<unsigned char> vchPrivKey = ParseHex(params[1].get_str());
  key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end()));

  if (!key.Sign(Hash(alert.vchMsg.begin(), alert.vchMsg.end()), alert.vchSig))
    throw runtime_error(
      "Unable to sign alert, check private key?\n");

  if(!alert.ProcessAlert())
    throw runtime_error(
      "Failed to process alert.\n");

  {
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    alert.RelayTo(pnode);
  }
  Object result;
  result.push_back(Pair("strStatusBar", alert.strStatusBar));
  result.push_back(Pair("nVersion", alert.nVersion));
  result.push_back(Pair("nMinVer", alert.nMinVer));
  result.push_back(Pair("nMaxVer", alert.nMaxVer));
  result.push_back(Pair("nPriority", alert.nPriority));
  result.push_back(Pair("nID", alert.nID));

  if (alert.nCancel > 0)
  {
    result.push_back(Pair("nCancel", alert.nCancel));
  }

  return result;
}
