
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
#ifndef WIN32
#include <signal.h>
#endif
using namespace std;
using namespace boost;
extern void xsc(CBlockIndex*);
__wx__* pwalletMain;
CClientUIInterface uiInterface;
bool fConfChange;
bool fEnforceCanonical;
unsigned int nNodeLifespan;
unsigned int nDerivationMethodIndex;
unsigned int nMinerSleep;
string strDNSSeedNode;
bool fUseFastIndex;
enum Checkpoints::CPMode CheckpointsMode;
LocatorNodeDB* ln1Db = NULL;
VertexNodeDB* vertexDB__ = NULL;
dev::OverlayDB* overlayDB__ = NULL;
extern ConfigurationState globalState;
void ExitTimeout(void* parg)
{
#ifdef WIN32
  MilliSleep(5000);
  ExitProcess(0);
#endif
}
void StartShutdown()
{
#ifdef QT_GUI
  uiInterface.QueueShutdown();
#else
  NewThread(Shutdown, NULL);
#endif
}
void Shutdown(void* parg)
{
  static CCriticalSection cs_Shutdown;
  static bool fTaken;
  RenameThread("iocoin-shutoff");
  bool fFirstThread = false;
  {
    TRY_LOCK(cs_Shutdown, lockShutdown);

    if (lockShutdown)
    {
      fFirstThread = !fTaken;
      fTaken = true;
    }
  }
  static bool fExit;

  if (fFirstThread)
  {
    fShutdown = true;
    nTransactionsUpdated++;
    bitdb.Flush(false);
    StopNode();
    bitdb.Flush(true);
    boost::filesystem::remove(GetPidFile());
    UnregisterWallet(pwalletMain);
    delete pwalletMain;
    delete ln1Db;
    delete overlayDB__;
    NewThread(ExitTimeout, NULL);
    MilliSleep(50);
    printf("I/OCoin exited\n\n");
    fExit = true;
#ifndef QT_GUI
    exit(0);
#endif
  }
  else
  {
    while (!fExit)
    {
      MilliSleep(500);
    }

    MilliSleep(100);
    ExitThread(0);
  }
}
void HandleSIGTERM(int)
{
  fRequestShutdown = true;
}
void HandleSIGHUP(int)
{
  fReopenDebugLog = true;
}
#if !defined(QT_GUI)
bool AppInit(int argc, char* argv[])
{
  bool fRet = false;

  try
  {
    ParseParameters(argc, argv);

    if (!boost::filesystem::is_directory(GetDataDir(false)))
    {
      fprintf(stderr, "Error: Specified directory does not exist\n");
      Shutdown(NULL);
    }

    ReadConfigFile(mapArgs, mapMultiArgs);

    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
      std::string strUsage = _("I/OCoin version") + " " + FormatFullVersion() + "\n\n" +
                             _("Usage:") + "\n" +
                             "  iocoind [options]                     " + "\n" +
                             "  iocoind [options] <command> [params]  " + _("Send command to -server or iocoind") + "\n" +
                             "  iocoind [options] help                " + _("List commands") + "\n" +
                             "  iocoind [options] help <command>      " + _("Get help for a command") + "\n";
      strUsage += "\n" + HelpMessage();
      fprintf(stdout, "%s", strUsage.c_str());
      return false;
    }

    for (int i = 1; i < argc; i++)
      if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "iocoin:"))
      {
        fCommandLine = true;
      }

    if (fCommandLine)
    {
      int ret = CommandLineRPC(argc, argv);
      exit(ret);
    }

    fRet = AppInit2();
  }
  catch (std::exception& e)
  {
    PrintException(&e, "AppInit()");
  }
  catch (...)
  {
    PrintException(NULL, "AppInit()");
  }

  if (!fRet)
  {
    Shutdown(NULL);
  }

  return fRet;
}
extern void noui_connect();
ConfigurationState globalState;
int main(int argc, char* argv[])
{
  bool fRet = false;
  noui_connect();
  fRet = AppInit(argc, argv);

  if (fRet && fDaemon)
  {
    return 0;
  }

  return 1;
}
#endif
bool static InitError(const std::string &str)
{
  uiInterface.ThreadSafeMessageBox(str, _("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::MODAL);
  return false;
}
bool static InitWarning(const std::string &str)
{
  uiInterface.ThreadSafeMessageBox(str, _("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
  return true;
}
bool static Bind(const CService &addr, bool fError = true)
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
      return InitError(strError);
    }

    return false;
  }

  return true;
}
std::string HelpMessage()
{
  string strUsage = _("Options:") + "\n" +
                    "  -?                     " + _("This help message") + "\n" +
                    "  -conf=<file>           " + _("Specify configuration file (default: iocoin.conf)") + "\n" +
                    "  -pid=<file>            " + _("Specify pid file (default: iocoind.pid)") + "\n" +
                    "  -datadir=<dir>         " + _("Specify data directory") + "\n" +
                    "  -wallet=<dir>          " + _("Specify wallet file (within data directory)") + "\n" +
                    "  -dbcache=<n>           " + _("Set database cache size in megabytes (default: 25)") + "\n" +
                    "  -dblogsize=<n>         " + _("Set database disk log size in megabytes (default: 100)") + "\n" +
                    "  -timeout=<n>           " + _("Specify connection timeout in milliseconds (default: 5000)") + "\n" +
                    "  -proxy=<ip:port>       " + _("Connect through socks proxy") + "\n" +
                    "  -socks=<n>             " + _("Select the version of socks proxy to use (4-5, default: 5)") + "\n" +
                    "  -tor=<ip:port>         " + _("Use proxy to reach tor hidden services (default: same as -proxy)") + "\n"
                    "  -dns                   " + _("Allow DNS lookups for -addnode, -seednode and -connect") + "\n" +
                    "  -port=<port>           " + _("Listen for connections on <port> (default: 33764 or testnet: 1901)") + "\n" +
                    "  -maxconnections=<n>    " + _("Maintain at most <n> connections to peers (default: 125)") + "\n" +
                    "  -addnode=<ip>          " + _("Add a node to connect to and attempt to keep the connection open") + "\n" +
                    "  -connect=<ip>          " + _("Connect only to the specified node(s)") + "\n" +
                    "  -seednode=<ip>         " + _("Connect to a node to retrieve peer addresses, and disconnect") + "\n" +
                    "  -externalip=<ip>       " + _("Specify your own public address") + "\n" +
                    "  -onlynet=<net>         " + _("Only connect to nodes in network <net> (IPv4, IPv6 or Tor)") + "\n" +
                    "  -discover              " + _("Discover own IP address (default: 1 when listening and no -externalip)") + "\n" +
                    "  -irc                   " + _("Find peers using internet relay chat (default: 0)") + "\n" +
                    "  -listen                " + _("Accept connections from outside (default: 1 if no -proxy or -connect)") + "\n" +
                    "  -bind=<addr>           " + _("Bind to given address. Use [host]:port notation for IPv6") + "\n" +
                    "  -dnsseed               " + _("Find peers using DNS lookup (default: 1)") + "\n" +
                    "  -synctime              " + _("Sync time with other nodes. Disable if time on your system is precise e.g. syncing with NTP (default: 1)") + "\n" +
                    "  -cppolicy              " + _("Sync checkpoints policy (default: strict)") + "\n" +
                    "  -banscore=<n>          " + _("Threshold for disconnecting misbehaving peers (default: 100)") + "\n" +
                    "  -bantime=<n>           " + _("Number of seconds to keep misbehaving peers from reconnecting (default: 86400)") + "\n" +
                    "  -maxreceivebuffer=<n>  " + _("Maximum per-connection receive buffer, <n>*1000 bytes (default: 5000)") + "\n" +
                    "  -maxsendbuffer=<n>     " + _("Maximum per-connection send buffer, <n>*1000 bytes (default: 1000)") + "\n" +
                    "  -paytxfee=<amt>        " + _("Fee per KB to add to transactions you send") + "\n" +
                    "  -mininput=<amt>        " + _("When creating transactions, ignore inputs with value less than this (default: 0.01)") + "\n" +
#ifdef QT_GUI
                    "  -server                " + _("Accept command line and JSON-RPC commands") + "\n" +
#endif
#if !defined(WIN32) && !defined(QT_GUI)
                    "  -daemon                " + _("Run in the background as a daemon and accept commands") + "\n" +
#endif
                    "  -testnet               " + _("Use the test network") + "\n" +
                    "  -viewwallet               " + _("view wallet only") + "\n" +
                    "  -debug                 " + _("Output extra debugging information. Implies all other -debug* options") + "\n" +
                    "  -debugnet              " + _("Output extra network debugging information") + "\n" +
                    "  -logtimestamps         " + _("Prepend debug output with timestamp") + "\n" +
                    "  -shrinkdebugfile       " + _("Shrink debug.log file on client startup (default: 1 when no -debug)") + "\n" +
                    "  -printtoconsole        " + _("Send trace/debug info to console instead of debug.log file") + "\n" +
#ifdef WIN32
                    "  -printtodebugger       " + _("Send trace/debug info to debugger") + "\n" +
#endif
                    "  -rpcuser=<user>        " + _("Username for JSON-RPC connections") + "\n" +
                    "  -rpcpassword=<pw>      " + _("Password for JSON-RPC connections") + "\n" +
                    "  -rpcport=<port>        " + _("Listen for JSON-RPC connections on <port> (default: 33765 or testnet: 43765)") + "\n" +
                    "  -rpcallowip=<ip>       " + _("Allow JSON-RPC connections from specified IP address") + "\n" +
                    "  -rpcconnect=<ip>       " + _("Send commands to node running on <ip> (default: 127.0.0.1)") + "\n" +
                    "  -blocknotify=<cmd>     " + _("Execute command when the best block changes (%s in cmd is replaced by block hash)") + "\n" +
                    "  -walletnotify=<cmd>    " + _("Execute command when a wallet transaction changes (%s in cmd is replaced by TxID)") + "\n" +
                    "  -zapwallettxes=<mode>" + _("Delete all wallet transactions and only recover those parts of the blockchain through -rescan on startup") +
                    "  -confchange            " + _("Require a confirmations for change (default: 0)") + "\n" +
                    "  -enforcecanonical      " + _("Enforce transaction scripts to use canonical PUSH operators (default: 1)") + "\n" +
                    "  -alertnotify=<cmd>     " + _("Execute command when a relevant alert is received (%s in cmd is replaced by message)") + "\n" +
                    "  -upgradewallet         " + _("Upgrade wallet to latest format") + "\n" +
                    "  -keypool=<n>           " + _("Set key pool size to <n> (default: 100)") + "\n" +
                    "  -rescan                " + _("Rescan the block chain for missing wallet transactions") + "\n" +
                    "  -xscan                " + _("Rescan the block chain for aliases") + "\n" +
                    "  -salvagewallet         " + _("Attempt to recover private keys from a corrupt wallet.dat") + "\n" +
                    "  -checkblocks=<n>       " + _("How many blocks to check at startup (default: 2500, 0 = all)") + "\n" +
                    "  -checklevel=<n>        " + _("How thorough the block verification is (0-6, default: 1)") + "\n" +
                    "  -loadblock=<file>      " + _("Imports blocks from external blk000?.dat file") + "\n" +
                    "\n" + _("Block creation options:") + "\n" +
                    "  -blockminsize=<n>      " + _("Set minimum block size in bytes (default: 0)") + "\n" +
                    "  -blockmaxsize=<n>      " + _("Set maximum block size in bytes (default: 250000)") + "\n" +
                    "  -blockprioritysize=<n> " + _("Set maximum size of high-priority/low-fee transactions in bytes (default: 27000)") + "\n" +
                    "\n" + _("SSL options: (see the Bitcoin Wiki for SSL setup instructions)") + "\n" +
                    "  -rpcssl                                  " + _("Use OpenSSL (https) for JSON-RPC connections") + "\n" +
                    "  -rpcsslcertificatechainfile=<file.cert>  " + _("Server certificate file (default: server.cert)") + "\n" +
                    "  -rpcsslprivatekeyfile=<file.pem>         " + _("Server private key (default: server.pem)") + "\n" +
                    "  -rpcsslciphers=<ciphers>                 " + _("Acceptable ciphers (default: TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH)") + "\n";
  return strUsage;
}
bool InitSanityCheck(void)
{
  if(!ECC_InitSanityCheck())
  {
    InitError("OpenSSL appears to lack support for elliptic curve cryptography. For more "
              "information, visit https://en.bitcoin.it/wiki/OpenSSL_and_EC_Libraries");
    return false;
  }

  return true;
}
bool AppInit2()
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
  nMinerSleep = GetArg("-minersleep", 500);
  CheckpointsMode = Checkpoints::STRICT;
  std::string strCpMode = GetArg("-cppolicy", "strict");
  strDNSSeedNode = GetArg("-dnsseednode", "default");

  if(strCpMode == "strict")
  {
    CheckpointsMode = Checkpoints::STRICT;
  }

  if(strCpMode == "advisory")
  {
    CheckpointsMode = Checkpoints::ADVISORY;
  }

  if(strCpMode == "permissive")
  {
    CheckpointsMode = Checkpoints::PERMISSIVE;
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
      return InitError(_("Initialization error. Configured as view wallet but wallet.dat file exists in configuration directory. I/O Coin shutting down."));
    }
  }
  else
  {
    if(boost::filesystem::exists(GetDataDir() / "view.dat"))
    {
      return InitError(_("Initialization error. Configured as default wallet but view.dat file exists in configuration directory. I/O Coin shutting down."));
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
      return InitError(strprintf(_("Invalid amount for -paytxfee=<amount>: '%s'"), mapArgs["-paytxfee"].c_str()));
    }

    if (globalState.nTransactionFee > 0.25 * COIN)
    {
      InitWarning(_("Warning: -paytxfee is set very high! This is the transaction fee you will pay if you send a transaction."));
    }
  }

  fConfChange = GetBoolArg("-confchange", false);
  fEnforceCanonical = GetBoolArg("-enforcecanonical", true);

  if (mapArgs.count("-mininput"))
  {
    if (!ParseMoney(mapArgs["-mininput"], nMinimumInputValue))
    {
      return InitError(strprintf(_("Invalid amount for -mininput=<amount>: '%s'"), mapArgs["-mininput"].c_str()));
    }
  }

  if (!InitSanityCheck())
  {
    return InitError(_("Initialization sanity check failed. I/OCoin is shutting down."));
  }

  std::string strDataDir = GetDataDir().string();
  std::string strWalletFileName = GetArg("-wallet", "wallet.dat");

  if (strWalletFileName != boost::filesystem::basename(strWalletFileName) + boost::filesystem::extension(strWalletFileName))
  {
    return InitError(strprintf(_("Wallet %s resides outside data directory %s."), strWalletFileName.c_str(), strDataDir.c_str()));
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
    return InitError(strprintf(_("Cannot obtain a lock on data directory %s.  I/OCoin is probably already running."), strDataDir.c_str()));
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
    return InitError(msg);
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
      return InitError(_("wallet.dat corrupt, salvage failed"));
    }
  }

  int nSocksVersion = GetArg("-socks", 5);

  if (nSocksVersion != 4 && nSocksVersion != 5)
  {
    return InitError(strprintf(_("Unknown -socks proxy version requested: %i"), nSocksVersion));
  }

  if (mapArgs.count("-onlynet"))
  {
    std::set<enum Network> nets;
    BOOST_FOREACH(std::string snet, mapMultiArgs["-onlynet"])
    {
      enum Network net = ParseNetwork(snet);

      if (net == NET_UNROUTABLE)
      {
        return InitError(strprintf(_("Unknown network specified in -onlynet: '%s'"), snet.c_str()));
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
      return InitError(strprintf(_("Invalid -proxy address: '%s'"), mapArgs["-proxy"].c_str()));
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
      return InitError(strprintf(_("Invalid -tor address: '%s'"), mapArgs["-tor"].c_str()));
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
      BOOST_FOREACH(std::string strBind, mapMultiArgs["-bind"])
      {
        CService addrBind;

        if (!Lookup(strBind.c_str(), addrBind, GetListenPort(), false))
        {
          return InitError(strprintf(_("Cannot resolve -bind address: '%s'"), strBind.c_str()));
        }

        fBound |= Bind(addrBind);
      }
    }
    else
    {
      struct in_addr inaddr_any;
      inaddr_any.s_addr = INADDR_ANY;

      if (!IsLimited(NET_IPV6))
      {
        fBound |= Bind(CService(in6addr_any, GetListenPort()), false);
      }

      if (!IsLimited(NET_IPV4))
      {
        fBound |= Bind(CService(inaddr_any, GetListenPort()), !fBound);
      }
    }

    if (!fBound)
    {
      return InitError(_("Failed to listen on any port. Use -listen=0 if you want this."));
    }
  }

  if (mapArgs.count("-externalip"))
  {
    BOOST_FOREACH(string strAddr, mapMultiArgs["-externalip"])
    {
      CService addrLocal(strAddr, GetListenPort(), fNameLookup);

      if (!addrLocal.IsValid())
      {
        return InitError(strprintf(_("Cannot resolve -externalip address: '%s'"), strAddr.c_str()));
      }

      AddLocal(CService(strAddr, GetListenPort(), fNameLookup), LOCAL_MANUAL);
    }
  }

  if (mapArgs.count("-reservebalance"))
  {
    if (!ParseMoney(mapArgs["-reservebalance"], nReserveBalance))
    {
      InitError(_("Invalid amount for -reservebalance=<amount>"));
      return false;
    }
  }

  if (mapArgs.count("-checkpointkey"))
  {
    if (!Checkpoints::SetCheckpointPrivKey(GetArg("-checkpointkey", "")))
    {
      InitError(_("Unable to sign checkpoint, wrong checkpointkey?\n"));
    }
  }

  BOOST_FOREACH(string strDest, mapMultiArgs["-seednode"])
  AddOneShot(strDest);

  try
  {
    printf("Opening state database...\n");
    std::unique_ptr<dev::db::DatabaseFace> db = dev::db::DBFactory::create(GetDataDir() / "state");
    overlayDB__ = new dev::OverlayDB(std::move(db));
  }
  catch (boost::exception const& ex)
  {
    if (dev::db::isDiskDatabase())
    {
      printf("Error opening state database\n");
      dev::db::DatabaseStatus const dbStatus =
        *boost::get_error_info<dev::db::errinfo_dbStatusCode>(ex);

      if (boost::filesystem::space(GetDataDir() / "state").available < 1024)
      {
        return InitError("Insufficient disk space : "
                         "Not enough available space on hard drive."
                         "Please back up disk first - free up some space and then re run. Exiting.");
      }
      else if (dbStatus == dev::db::DatabaseStatus::Corruption)
      {
        printf("Database corruption detected. Please see the exception for corruption "
               "details. Exception: %s\n", boost::diagnostic_information(ex).c_str());
        throw runtime_error("Database corruption");
        string msg = strprintf(_(
                                 " Database corruption detected. Details : Exception %s\n"
                               ), boost::diagnostic_information(ex));
        return InitError(msg);
      }
      else if (dbStatus == dev::db::DatabaseStatus::IOError)
      {
        return InitError("Database already open. You appear to have "
                         "another instance running on the same data path.");
      }
    }

    string msg = strprintf(_(
                             "statedb: Unknown error encountered when opening state database. Details : Exception %s\n"
                           ), boost::diagnostic_information(ex));
    return InitError(msg);
  }

  if (!bitdb.Open(GetDataDir()))
  {
    string msg = strprintf(_("Error initializing database environment %s!"
                             " To recover, BACKUP THAT DIRECTORY, then remove"
                             " everything from it except for wallet.dat."), strDataDir.c_str());
    return InitError(msg);
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
    return InitError(_("Error loading blkindex.dat"));
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
    pwalletMain = new __wx__("wallet.dat");
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
  pwalletMain = new __wx__(strWalletFileName);
  DBErrors nLoadWalletRet = pwalletMain->LoadWallet(fFirstRun);

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
      return InitError(strErrors.str());
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
      pwalletMain->SetMinVersion(FEATURE_LATEST);
      std::map<CKeyID, int64_t> mk;
      pwalletMain->kt(mk);

      for(std::map<CKeyID, int64_t>::const_iterator it = mk.begin(); it != mk.end(); it++)
      {
        CKeyID ck = it->first;
        pwalletMain->kd[ck].nVersion = CKeyMetadata::CURRENT_VERSION;
      }
    }
    else
    {
      printf("Allowing wallet upgrade up to %i\n", nMaxVersion);
    }

    if (nMaxVersion < pwalletMain->GetVersion())
    {
      strErrors << _("Cannot downgrade wallet") << "\n";
    }

    pwalletMain->SetMaxVersion(nMaxVersion);
  }

  if (fFirstRun && !fViewWallet)
  {
    RandAddSeedPerfmon();
    CPubKey newDefaultKey;

    if (pwalletMain->GetKeyFromPool(newDefaultKey, false))
    {
      pwalletMain->SetDefaultKey(newDefaultKey);

      if (!pwalletMain->SetAddressBookName(pwalletMain->vchDefaultKey.GetID(), ""))
      {
        strErrors << _("Cannot write default address") << "\n";
      }
    }
  }

  printf("%s", strErrors.str().c_str());
  printf(" wallet      %15" PRId64 "ms\n", GetTimeMillis() - nStart);
  RegisterWallet(pwalletMain);

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

  ln1Db = new LocatorNodeDB("cr+");
  vertexDB__ = new VertexNodeDB("cr+");
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
      pwalletMain->ScanForWalletTransactions(pindexRescan, true);
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
  printf("setKeyPool.size() = %" PRIszu "\n", pwalletMain->setKeyPool.size());
  printf("mapWallet.size() = %" PRIszu "\n", pwalletMain->mapWallet.size());
  printf("mapAddressBook.size() = %" PRIszu "\n", pwalletMain->mapAddressBook.size());

  if (!NewThread(StartNode, NULL))
  {
    InitError(_("Error: could not start node"));
  }

  if (fServer)
  {
    NewThread(ThreadRPCServer, NULL);
  }

  uiInterface.InitMessage(_("Done loading"));
  printf("Done loading\n");

  if (!strErrors.str().empty())
  {
    return InitError(strErrors.str());
  }

  pwalletMain->ReacceptWalletTransactions();
#if !defined(QT_GUI)

  while (1)
  {
    MilliSleep(5000);
  }

#endif
  return true;
}
