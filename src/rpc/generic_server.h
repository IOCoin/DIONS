#ifndef GENERIC_SERVER_H
#define GENERIC_SERVER_H

#include <stdlib.h>
#include <string>
#include <tuple>
#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

//#include "client.h"
#include "ui_interface.h"
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
#include "core/base58.h"

#include "rpc/json_req.h"
#include "rpc/util_proto.h"

class AcceptedConnection
{
public:
  virtual ~AcceptedConnection() {}
  virtual std::iostream& stream() = 0;
  virtual std::string peer_address_to_string() const = 0;
  virtual void close() = 0;
};
template <typename Protocol>
class SSLIOStreamDevice : public boost::iostreams::device<boost::iostreams::bidirectional>
{
public:
  SSLIOStreamDevice(boost::asio::ssl::stream<typename Protocol::socket> &streamIn, bool fUseSSLIn) : stream(streamIn)
  {
    fUseSSL = fUseSSLIn;
    fNeedHandshake = fUseSSLIn;
  }
  void handshake(boost::asio::ssl::stream_base::handshake_type role)
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
    handshake(boost::asio::ssl::stream_base::server);

    if (fUseSSL)
    {
      return stream.read_some(boost::asio::buffer(s, n));
    }

    return stream.next_layer().read_some(boost::asio::buffer(s, n));
  }
  std::streamsize write(const char* s, std::streamsize n)
  {
    handshake(boost::asio::ssl::stream_base::client);

    if (fUseSSL)
    {
      return boost::asio::write(stream, boost::asio::buffer(s, n));
    }

    return boost::asio::write(stream.next_layer(), boost::asio::buffer(s, n));
  }
  bool connect(const std::string& server, const std::string& port)
  {
    boost::asio::ip::tcp::resolver resolver(stream.get_executor());
    boost::asio::ip::tcp::resolver::query query(server.c_str(), port.c_str());
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    boost::system::error_code error = boost::asio::error::host_not_found;

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
  boost::asio::ssl::stream<typename Protocol::socket>& stream;
};
template <typename Protocol>
class AcceptedConnectionImpl : public AcceptedConnection
{
public:
  AcceptedConnectionImpl(
    boost::asio::io_context& io_context,
    boost::asio::ssl::context &context,
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
  boost::asio::ssl::stream<typename Protocol::socket> sslStream;
private:
  SSLIOStreamDevice<Protocol> _d;
  boost::iostreams::stream< SSLIOStreamDevice<Protocol> > _stream;
};

template <class I> using AbstractMethodPointer = void(I::*)(json_spirit::Array const& _parameter, json_spirit::Value& _result);

template <class I>
class RPCMethod
{
public:
  AbstractMethodPointer<I> mptr;
  bool okSafeMode;
  bool unlocked;
};

  static CCriticalSection cs_THREAD_RPCHANDLER;
template<class... Is>
class GenericServer
{
public:
  GenericServer() { }

  GenericServer(CClientUIInterface* uiFace)
  {
    uiFace_=uiFace;
  }

  virtual void handleMethodCall(std::string name,json_spirit::Array params, json_spirit::Value& res)
  {
    if (name == "rpc_modules")
    {
      modules();
    }
  }
  inline virtual void modules()
  {
  }
  bool HTTPAuthorized(map<string, string>& mapHeaders)
  {
    string strAuth = mapHeaders["authorization"];

    if (strAuth.substr(0,6) != "Basic ")
    {
      return false;
    }

    string strUserPass64 = strAuth.substr(6);
    boost::trim(strUserPass64);
    string strUserPass = DecodeBase64(strUserPass64);
    return TimingResistantEqual(strUserPass, strRPCUserColonPass);
  }

  template <typename Protocol, typename SocketAcceptorService>
  void RPCListen(boost::shared_ptr< boost::asio::basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
                                       boost::asio::io_context& io_context,
                                       boost::asio::ssl::context& context,
                                       const bool fUseSSL)
  {
    AcceptedConnectionImpl<Protocol>* conn = new AcceptedConnectionImpl<Protocol>(io_context, context, fUseSSL);
    acceptor->async_accept(
      conn->sslStream.lowest_layer(),
      conn->peer,
      boost::bind(&GenericServer<Is...>::RPCAcceptHandler<Protocol, SocketAcceptorService>,this,
                  acceptor,
                  boost::ref(io_context),
                  boost::ref(context),
                  fUseSSL,
                  conn,
                  boost::asio::placeholders::error));
  }
  template <typename Protocol, typename SocketAcceptorService>
  void RPCAcceptHandler(boost::shared_ptr< boost::asio::basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
      boost::asio::io_context& io_context,
      boost::asio::ssl::context& context,
      const bool fUseSSL,
      AcceptedConnection* conn,
      const boost::system::error_code& error)
  {
    vnThreadsRunning[THREAD_RPCLISTENER]++;

    if (error != boost::asio::error::operation_aborted
        && acceptor->is_open())
    {
      RPCListen(acceptor, io_context, context, fUseSSL);
    }

    AcceptedConnectionImpl<boost::asio::ip::tcp>* tcp_conn = dynamic_cast< AcceptedConnectionImpl<boost::asio::ip::tcp>* >(conn);

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
    else
    {
      try
      {
        boost::thread(boost::bind(&GenericServer<Is...>::Reactor, this,conn));
      }
      catch(boost::thread_resource_error &e)
      {
        printf("Failed to create RPC server client thread\n");
        delete conn;
      }
    }

    vnThreadsRunning[THREAD_RPCLISTENER]--;
  }
  std::string trans(const char* psz)
  {
    boost::optional<std::string> rv = uiInterface.Translate(psz);
    return rv ? (*rv) : psz;
  }
  void Reactor(void* parg)
  {
    RenameThread("iocoin-rpcreact");
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
        printf("RPC Reactor incorrect password attempt from %s\n", conn->peer_address_to_string().c_str());

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

      rpc_util::JSONRequest jreq;

      try
      {
	      json_spirit::Value valRequest;

        if (!read_string(strRequest, valRequest))
        {
          throw JSONRPCError(RPC_PARSE_ERROR, "Parse error");
        }

        string strReply;

        if (valRequest.type() == json_spirit::obj_type)
        {
          jreq.parse(valRequest);
          std::cout << "execute 1" << std::endl;
	  json_spirit::Value result;
          this->handleMethodCall(jreq.strMethod,jreq.params,result);
          strReply = JSONRPCReply(result, json_spirit::Value::null, jreq.id);
        }
        else if (valRequest.type() == json_spirit::array_type)
        {
          strReply = JSONRPCExecBatch(valRequest.get_array());
        }
        else
        {
          throw JSONRPCError(RPC_PARSE_ERROR, "Top-level object parse error");
        }

        conn->stream() << HTTPReply(HTTP_OK, strReply, fRun) << std::flush;
      }
      catch (json_spirit::Object& objError)
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

  bool start(map<string,string> args,CClientUIInterface* uiFace)
  {
    this->uiFace_  = uiFace;
    this->argsMap_ = args;

    try
    {
      boost::thread(boost::bind(&GenericServer<Is...>::ThreadRPCServer, this));
    }
    catch(boost::thread_resource_error &e)
    {
      printf("Error creating thread: %s\n", e.what());
      return false;
    }

    return true;
  }
protected:
  std::string strRPCUserColonPass;
  void ThreadRPCServer()
  {
    RenameThread("iocoin-rpclist");

    try
    {
      vnThreadsRunning[THREAD_RPCLISTENER]++;
      GenericServer<Is...>::Acceptor();
      vnThreadsRunning[THREAD_RPCLISTENER]--;
    }
    catch (std::exception& e)
    {
      vnThreadsRunning[THREAD_RPCLISTENER]--;
      PrintException(&e, "ThreadGenericServer()");
    }
    catch (...)
    {
      vnThreadsRunning[THREAD_RPCLISTENER]--;
      PrintException(NULL, "ThreadGenericServer()");
    }

    printf("ThreadGenericServer exited\n");
  }
  void Acceptor()
  {
    printf("Acceptor started\n");
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
        printf("RPC Acceptor ERROR: missing server certificate file %s\n", pathCertFile.string().c_str());
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
        printf("RPC Acceptor ERROR: missing server private key file %s\n", pathPKFile.string().c_str());
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
      acceptor->listen(boost::asio::socket_base::max_connections);
      RPCListen(acceptor, io_context, context, fUseSSL);
      StopRequests.connect(boost::signals2::slot<void ()>(
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
        acceptor->listen(boost::asio::socket_base::max_connections);
        RPCListen(acceptor, io_context, context, fUseSSL);
        StopRequests.connect(boost::signals2::slot<void ()>(
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
  std::map<string,string> argsMap_;
  CClientUIInterface* uiFace_;

  std::map<std::string, std::string> implementedModules_;
};

template<class I, class... Is>
class GenericServer<I, Is...> : public GenericServer<Is...>
{
public:
  GenericServer<I, Is...>(I* _i, Is*... _is): GenericServer<Is...>(_is...), interface_(_i)
  {
    if (!interface_)
    {
      return;
    }

    for (auto const& method: interface_->methods())
    {
      methods_[std::get<0>(method)] = std::get<1>(method);
    }

    for (auto const& module: interface_->implementedModules())
    {
      this->implementedModules_[module.name] = module.version;
    }
  }
  virtual void handleMethodCall(std::string name,json_spirit::Array params, json_spirit::Value& res)
  {
    auto pointer = methods_.find(name);

    if (pointer != methods_.end())
    {
      auto pointer = methods_.find(name);

      try
      {
        (interface_.get()->*(pointer->second.mptr))(params,res);
      }
      catch (std::exception const& ex)
      {
      }
    }
    else
    {
      GenericServer<Is...>::handleMethodCall(name,params,res);
    }
  }

private:
  std::unique_ptr<I> interface_;
  std::map<std::string, RPCMethod<I>> methods_;
};

template <class I>
class ServerInterface
{
public:
  using MethodBinding = std::tuple<std::string, RPCMethod<I>>;
  using Methods = std::vector<MethodBinding>;
  struct RPCModule
  {
    std::string name;
    std::string version;
  };
  using RPCModules = std::vector<RPCModule>;

  virtual ~ServerInterface() {}

  Methods const& methods() const
  {
    return methods_;
  }
  virtual RPCModules implementedModules() const = 0;

protected:
  void bindMethod(std::string name, RPCMethod<I> mptr)
  {
    methods_.emplace_back(name, mptr);
  }

private:
  Methods methods_;
};

class DionsFace : public ServerInterface<DionsFace>
{
public:
  DionsFace()
  {
    this->bindMethod("aliasList", RPCMethod<DionsFace> {&DionsFace::aliasList, "true","false"});
  }
  virtual RPCModules implementedModules() const override
  {
    return RPCModules{RPCModule{"DIONS", "dvm.1.0"}};
  }

  inline virtual void aliasList(const json_spirit::Array &req, json_spirit::Value &res)
  {
    (void)req;
    res = this->aliasList(req);
  }

  virtual json_spirit::Value aliasList(const json_spirit::Array) = 0;
};

#endif
