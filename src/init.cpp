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
#include "network_node.h"
#ifndef WIN32
#include <signal.h>
#endif
using namespace std;
using namespace boost;
extern void xsc(CBlockIndex*);
__wx__* pwalletMainId;
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
    CTxMemPool::nTransactionsUpdated++;
    bitdb.Flush(false);
    StopNode();
    bitdb.Flush(true);
    boost::filesystem::remove(GetPidFile());
    UnregisterWallet(pwalletMainId);
    delete ln1Db;
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
extern void noui_connect();
ConfigurationState globalState;
int main(int argc, char* argv[])
{
  bool fRet = false;
  noui_connect();

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

    NetworkNode networkNode(mapArgs);

    pwalletMainId = networkNode.wallet();
    fRet = networkNode.init();
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

  if (fRet && fDaemon)
  {
    return 0;
  }

  return 1;
}
#endif
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
