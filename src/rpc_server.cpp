#include "rpc_server.h"
#include "base58.h"
#include "util.h"
#include <boost/asio.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <list>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace json_spirit;

extern void StartShutdown();
static inline unsigned short GetDefaultRPCPort()
{
  return GetBoolArg("-testnet", false) ? 43765 : 33765;
}
string rfc1123Time()
{
  char buffer[64];
  time_t now;
  time(&now);
  struct tm* now_gmt = gmtime(&now);
  string locale(setlocale(LC_TIME, NULL));
  setlocale(LC_TIME, "C");
  strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S +0000", now_gmt);
  setlocale(LC_TIME, locale.c_str());
  return string(buffer);
}
static string HTTPReply(int nStatus, const string& strMsg, bool keepalive)
{
  if (nStatus == HTTP_UNAUTHORIZED)
    return strprintf("HTTP/1.0 401 Authorization Required\r\n"
                     "Date: %s\r\n"
                     "Server: iocoin-json-rpc/%s\r\n"
                     "WWW-Authenticate: Basic realm=\"jsonrpc\"\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: 296\r\n"
                     "\r\n"
                     "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\r\n"
                     "\"http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd\">\r\n"
                     "<HTML>\r\n"
                     "<HEAD>\r\n"
                     "<TITLE>Error</TITLE>\r\n"
                     "<META HTTP-EQUIV='Content-Type' CONTENT='text/html; charset=ISO-8859-1'>\r\n"
                     "</HEAD>\r\n"
                     "<BODY><H1>401 Unauthorized.</H1></BODY>\r\n"
                     "</HTML>\r\n", rfc1123Time().c_str(), FormatFullVersion().c_str());

  const char *cStatus;

  if (nStatus == HTTP_OK)
  {
    cStatus = "OK";
  }
  else if (nStatus == HTTP_BAD_REQUEST)
  {
    cStatus = "Bad Request";
  }
  else if (nStatus == HTTP_FORBIDDEN)
  {
    cStatus = "Forbidden";
  }
  else if (nStatus == HTTP_NOT_FOUND)
  {
    cStatus = "Not Found";
  }
  else if (nStatus == HTTP_INTERNAL_SERVER_ERROR)
  {
    cStatus = "Internal Server Error";
  }
  else
  {
    cStatus = "";
  }

  return strprintf(
           "HTTP/1.1 %d %s\r\n"
           "Date: %s\r\n"
           "Connection: %s\r\n"
           "Content-Length: %" PRIszu "\r\n"
           "Content-Type: application/json\r\n"
           "Server: iocoin-json-rpc/%s\r\n"
           "\r\n"
           "%s",
           nStatus,
           cStatus,
           rfc1123Time().c_str(),
           keepalive ? "keep-alive" : "close",
           strMsg.size(),
           FormatFullVersion().c_str(),
           strMsg.c_str());
}

bool ClientAllowed(const boost::asio::ip::address& address)
{
  if (address.is_v6()
      && (address.to_v6().is_v4_compatible()
          || address.to_v6().is_v4_mapped()))
  {
    return ClientAllowed(address.to_v6().to_v4());
  }

  if (address == asio::ip::address_v4::loopback()
      || address == asio::ip::address_v6::loopback()
      || (address.is_v4()
          && (address.to_v4().to_ulong() & 0xff000000) == 0x7f000000))
  {
    return true;
  }

  const string strAddress = address.to_string();
  const vector<string>& vAllow = mapMultiArgs["-rpcallowip"];
  BOOST_FOREACH(string strAllow, vAllow)

  if (WildcardMatch(strAddress, strAllow))
  {
    return true;
  }

  return false;
}
class AcceptedConnection
{
public:
  virtual ~AcceptedConnection() {}
  virtual std::iostream& stream() = 0;
  virtual std::string peer_address_to_string() const = 0;
  virtual void close() = 0;
};

template <typename Protocol, typename SocketAcceptorService>
static void RPCAcceptHandler(boost::shared_ptr< basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
                             boost::asio::io_context& io_context,
                             ssl::context& context,
                             bool fUseSSL,
                             AcceptedConnection* conn,
                             const boost::system::error_code& error);
template <typename Protocol>
class SSLIOStreamDevice : public iostreams::device<iostreams::bidirectional>
{
public:
  SSLIOStreamDevice(asio::ssl::stream<typename Protocol::socket> &streamIn, bool fUseSSLIn) : stream(streamIn)
  {
    fUseSSL = fUseSSLIn;
    fNeedHandshake = fUseSSLIn;
  }
  void handshake(ssl::stream_base::handshake_type role)
  {
    if (!fNeedHandshake)
    {
      return;
    }

    fNeedHandshake = false;
    stream.handshake(role);
  }
  std::streamsize read(char* s, std::streamsize n)
  {
    handshake(ssl::stream_base::server);

    if (fUseSSL)
    {
      return stream.read_some(asio::buffer(s, n));
    }

    return stream.next_layer().read_some(asio::buffer(s, n));
  }
  std::streamsize write(const char* s, std::streamsize n)
  {
    handshake(ssl::stream_base::client);

    if (fUseSSL)
    {
      return asio::write(stream, asio::buffer(s, n));
    }

    return asio::write(stream.next_layer(), asio::buffer(s, n));
  }
  bool connect(const std::string& server, const std::string& port)
  {
    ip::tcp::resolver resolver(stream.get_executor());
    ip::tcp::resolver::query query(server.c_str(), port.c_str());
    ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    ip::tcp::resolver::iterator end;
    boost::system::error_code error = asio::error::host_not_found;

    while (error && endpoint_iterator != end)
    {
      stream.lowest_layer().close();
      stream.lowest_layer().connect(*endpoint_iterator++, error);
    }

    if (error)
    {
      return false;
    }

    return true;
  }
private:
  bool fNeedHandshake;
  bool fUseSSL;
  asio::ssl::stream<typename Protocol::socket>& stream;
};
template <typename Protocol>
class AcceptedConnectionImpl : public AcceptedConnection
{
public:
  AcceptedConnectionImpl(
    asio::io_context& io_context,
    ssl::context &context,
    bool fUseSSL) :
    sslStream(io_context, context),
    _d(sslStream, fUseSSL),
    _stream(_d)
  {
  }
  virtual std::iostream& stream()
  {
    return _stream;
  }
  virtual std::string peer_address_to_string() const
  {
    return peer.address().to_string();
  }
  virtual void close()
  {
    _stream.close();
  }
  typename Protocol::endpoint peer;
  asio::ssl::stream<typename Protocol::socket> sslStream;
private:
  SSLIOStreamDevice<Protocol> _d;
  iostreams::stream< SSLIOStreamDevice<Protocol> > _stream;
};
template <typename Protocol, typename SocketAcceptorService>
static void RPCListen(boost::shared_ptr< basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
                      asio::io_context& io_context,
                      ssl::context& context,
                      const bool fUseSSL)
{
  AcceptedConnectionImpl<Protocol>* conn = new AcceptedConnectionImpl<Protocol>(io_context, context, fUseSSL);
  acceptor->async_accept(
    conn->sslStream.lowest_layer(),
    conn->peer,
    boost::bind(&RPCAcceptHandler<Protocol, SocketAcceptorService>,
                acceptor,
                boost::ref(io_context),
                boost::ref(context),
                fUseSSL,
                conn,
                boost::asio::placeholders::error));
}
template <typename Protocol, typename SocketAcceptorService>
static void RPCAcceptHandler(boost::shared_ptr< basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
                             boost::asio::io_context& io_context,
                             ssl::context& context,
                             const bool fUseSSL,
                             AcceptedConnection* conn,
                             const boost::system::error_code& error)
{
  vnThreadsRunning[THREAD_RPCLISTENER]++;

  if (error != asio::error::operation_aborted
      && acceptor->is_open())
  {
    RPCListen(acceptor, io_context, context, fUseSSL);
  }

  AcceptedConnectionImpl<ip::tcp>* tcp_conn = dynamic_cast< AcceptedConnectionImpl<ip::tcp>* >(conn);

  if (error)
  {
    delete conn;
  }
  else if (tcp_conn
           && !ClientAllowed(tcp_conn->peer.address()))
  {
    if (!fUseSSL)
    {
      conn->stream() << HTTPReply(HTTP_FORBIDDEN, "", false) << std::flush;
    }

    delete conn;
  }
  //XXXX else if (!NewThread(ThreadRPCServer3, conn))
  else
  {
    try
    {
      boost::thread(boost::bind(&RPCServer::ThreadRPCServer3, this,conn));
    }
    catch(boost::thread_resource_error &e)
    {
      printf("Failed to create RPC server client thread\n");
      delete conn;
    }
  }

  vnThreadsRunning[THREAD_RPCLISTENER]--;
}
bool RPCServer::start(map<string,string> args)
{
  this->argsMap_ = args;

  //XXXX NewThread(this->ThreadRPCServer, NULL);
  try
  {
    //XXXX boost::thread(pfn, parg);
    boost::thread(boost::bind(&RPCServer::ThreadRPCServer, this));
  }
  catch(boost::thread_resource_error &e)
  {
    printf("Error creating thread: %s\n", e.what());
    return false;
  }

  return true;
}

void RPCServer::ThreadRPCServer()
{
  RenameThread("iocoin-rpclist");

  try
  {
    vnThreadsRunning[THREAD_RPCLISTENER]++;
    RPCServer::ThreadRPCServer2();
    vnThreadsRunning[THREAD_RPCLISTENER]--;
  }
  catch (std::exception& e)
  {
    vnThreadsRunning[THREAD_RPCLISTENER]--;
    PrintException(&e, "ThreadRPCServer()");
  }
  catch (...)
  {
    vnThreadsRunning[THREAD_RPCLISTENER]--;
    PrintException(NULL, "ThreadRPCServer()");
  }

  printf("ThreadRPCServer exited\n");
}
void RPCServer::ThreadRPCServer2()
{
  printf("ThreadRPCServer started\n");
  strRPCUserColonPass = mapArgs["-rpcuser"] + ":" + mapArgs["-rpcpassword"];

  if ((this->argsMap_["-rpcpassword"] == "") ||
      (this->argsMap_["-rpcuser"] == this->argsMap_["-rpcpassword"]))
  {
    unsigned char rand_pwd[32];
    RAND_bytes(rand_pwd, 32);
    string strWhatAmI = "To use iocoind";

    if (this->argsMap_.count("-server"))
    {
      strWhatAmI = strprintf(trans("To use the %s option"), "\"-server\"");
    }
    else if (this->argsMap_.count("-daemon"))
    {
      strWhatAmI = strprintf(trans("To use the %s option"), "\"-daemon\"");
    }

    uiInterface.ThreadSafeMessageBox(strprintf(
                                       trans("%s, you must set a rpcpassword in the configuration file:\n %s\n"
                                           "It is recommended you use the following random password:\n"
                                           "rpcuser=iocoinrpc\n"
                                           "rpcpassword=%s\n"
                                           "(you do not need to remember this password)\n"
                                           "The username and password MUST NOT be the same.\n"
                                           "If the file does not exist, create it with owner-readable-only file permissions.\n"
                                           "It is also recommended to set alertnotify so you are notified of problems;\n"
                                           "for example: alertnotify=echo %%s | mail -s \"I/OCoin Alert\" admin@foo.com\n"),
                                       strWhatAmI.c_str(),
                                       GetConfigFile().string().c_str(),
                                       EncodeBase58(&rand_pwd[0],&rand_pwd[0]+32).c_str()),
                                     trans("Error"), CClientUIInterface::OK | CClientUIInterface::MODAL);
    StartShutdown();
    return;
  }

  const bool fUseSSL = GetBoolArg("-rpcssl");
  boost::asio::io_context io_context;
  boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);

  if (fUseSSL)
  {
    context.set_options(boost::asio::ssl::context::no_sslv2);
    boost::filesystem::path pathCertFile(GetArg("-rpcsslcertificatechainfile", "server.cert"));

    if (!pathCertFile.is_complete())
    {
      pathCertFile = boost::filesystem::path(GetDataDir()) / pathCertFile;
    }

    if (boost::filesystem::exists(pathCertFile))
    {
      context.use_certificate_chain_file(pathCertFile.string());
    }
    else
    {
      printf("ThreadRPCServer ERROR: missing server certificate file %s\n", pathCertFile.string().c_str());
    }

    boost::filesystem::path pathPKFile(GetArg("-rpcsslprivatekeyfile", "server.pem"));

    if (!pathPKFile.is_complete())
    {
      pathPKFile = boost::filesystem::path(GetDataDir()) / pathPKFile;
    }

    if (boost::filesystem::exists(pathPKFile))
    {
      context.use_private_key_file(pathPKFile.string(), boost::asio::ssl::context::pem);
    }
    else
    {
      printf("ThreadRPCServer ERROR: missing server private key file %s\n", pathPKFile.string().c_str());
    }

    string strCiphers = GetArg("-rpcsslciphers", "TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH");
    SSL_CTX_set_cipher_list(context.native_handle(), strCiphers.c_str());
  }

  const bool loopback = !mapArgs.count("-rpcallowip");
  boost::asio::ip::address bindAddress = loopback ? boost::asio::ip::address_v6::loopback() : boost::asio::ip::address_v6::any();
  boost::asio::ip::tcp::endpoint endpoint(bindAddress, GetArg("-rpcport", GetDefaultRPCPort()));
  boost::system::error_code v6_only_error;
  boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor(new boost::asio::ip::tcp::acceptor(io_context));
  boost::signals2::signal<void ()> StopRequests;
  bool fListening = false;
  std::string strerr;

  try
  {
    acceptor->open(endpoint.protocol());
    acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor->set_option(boost::asio::ip::v6_only(loopback), v6_only_error);
    acceptor->bind(endpoint);
    acceptor->listen(socket_base::max_connections);
    RPCListen(acceptor, io_context, context, fUseSSL);
    StopRequests.connect(signals2::slot<void ()>(
                           static_cast<void (boost::asio::ip::tcp::acceptor::*)()>(&boost::asio::ip::tcp::acceptor::close), acceptor.get())
                         .track(acceptor));
    fListening = true;
  }
  catch(boost::system::system_error &e)
  {
    strerr = strprintf(trans("An error occurred while setting up the RPC port %u for listening on IPv6, falling back to IPv4: %s"), endpoint.port(), e.what());
  }

  try
  {
    if (!fListening || loopback || v6_only_error)
    {
      bindAddress = loopback ? boost::asio::ip::address_v4::loopback() : boost::asio::ip::address_v4::any();
      endpoint.address(bindAddress);
      acceptor.reset(new boost::asio::ip::tcp::acceptor(io_context));
      acceptor->open(endpoint.protocol());
      acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
      acceptor->bind(endpoint);
      acceptor->listen(socket_base::max_connections);
      RPCListen(acceptor, io_context, context, fUseSSL);
      StopRequests.connect(signals2::slot<void ()>(
                             static_cast<void (boost::asio::ip::tcp::acceptor::*)()>(&boost::asio::ip::tcp::acceptor::close), acceptor.get())
                           .track(acceptor));
      fListening = true;
    }
  }
  catch(boost::system::system_error &e)
  {
    strerr = strprintf(trans("An error occurred while setting up the RPC port %u for listening on IPv4: %s"), endpoint.port(), e.what());
  }

  if (!fListening)
  {
    uiInterface.ThreadSafeMessageBox(strerr, trans("Error"), CClientUIInterface::OK | CClientUIInterface::MODAL);
    StartShutdown();
    return;
  }

  vnThreadsRunning[THREAD_RPCLISTENER]--;

  while (!fShutdown)
  {
    io_context.run_one();
  }

  vnThreadsRunning[THREAD_RPCLISTENER]++;
  StopRequests();
}
std::string RPCServer::trans(const char* psz)
{
  boost::optional<std::string> rv = uiInterface.Translate(psz);
  return rv ? (*rv) : psz;
}
static CCriticalSection cs_THREAD_RPCHANDLER;
void RPCServer::ThreadRPCServer3(void* parg)
{
  RenameThread("iocoin-rpchand");
  {
    LOCK(cs_THREAD_RPCHANDLER);
    vnThreadsRunning[THREAD_RPCHANDLER]++;
  }
  AcceptedConnection *conn = (AcceptedConnection *) parg;
  bool fRun = true;

  while (true)
  {
    if (fShutdown || !fRun)
    {
      conn->close();
      delete conn;
      {
        LOCK(cs_THREAD_RPCHANDLER);
        --vnThreadsRunning[THREAD_RPCHANDLER];
      }
      return;
    }

    map<string, string> mapHeaders;
    string strRequest;
    ReadHTTP(conn->stream(), mapHeaders, strRequest);

    if (mapHeaders.count("authorization") == 0)
    {
      conn->stream() << HTTPReply(HTTP_UNAUTHORIZED, "", false) << std::flush;
      break;
    }

    if (!HTTPAuthorized(mapHeaders))
    {
      printf("ThreadRPCServer incorrect password attempt from %s\n", conn->peer_address_to_string().c_str());

      if (mapArgs["-rpcpassword"].size() < 20)
      {
        MilliSleep(250);
      }

      conn->stream() << HTTPReply(HTTP_UNAUTHORIZED, "", false) << std::flush;
      break;
    }

    if (mapHeaders["connection"] == "close")
    {
      fRun = false;
    }

    JSONRequest jreq;

    try
    {
      Value valRequest;

      if (!read_string(strRequest, valRequest))
      {
        throw JSONRPCError(RPC_PARSE_ERROR, "Parse error");
      }

      string strReply;

      if (valRequest.type() == obj_type)
      {
        jreq.parse(valRequest);
        Value result = tableRPC.execute(jreq.strMethod, jreq.params);
        strReply = JSONRPCReply(result, Value::null, jreq.id);
      }
      else if (valRequest.type() == array_type)
      {
        strReply = JSONRPCExecBatch(valRequest.get_array());
      }
      else
      {
        throw JSONRPCError(RPC_PARSE_ERROR, "Top-level object parse error");
      }

      conn->stream() << HTTPReply(HTTP_OK, strReply, fRun) << std::flush;
    }
    catch (Object& objError)
    {
      ErrorReply(conn->stream(), objError, jreq.id);
      break;
    }
    catch (std::exception& e)
    {
      ErrorReply(conn->stream(), JSONRPCError(RPC_PARSE_ERROR, e.what()), jreq.id);
      break;
    }
  }

  delete conn;
  {
    LOCK(cs_THREAD_RPCHANDLER);
    vnThreadsRunning[THREAD_RPCHANDLER]--;
  }
}
