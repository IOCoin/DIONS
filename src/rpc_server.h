#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include "bitcoinrpc.h"
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
class JSONRequest
{
public:
  json_spirit::Value id;
  string strMethod;
  json_spirit::Array params;
  JSONRequest()
  {
    id = json_spirit::Value::null;
  }
  void parse(const json_spirit::Value& valRequest);
};

class RPCServer
{
public:
  RPCServer() = default;
  RPCServer(CClientUIInterface* uiFace)
  {
    uiFace_=uiFace;
  }
  ~RPCServer() = default;

  bool start(std::map<string,string>);

private:
  std::string trans(const char* psz);
  std::string strRPCUserColonPass;
  void ThreadRPCServer();
  void ThreadRPCServer2();
  void ThreadRPCServer3(void* parg);

  template <typename Protocol, typename SocketAcceptorService>
  void RPCAcceptHandler(boost::shared_ptr< boost::asio::basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
                        boost::asio::io_context& io_context,
                        boost::asio::ssl::context& context,
                        const bool fUseSSL,
                        AcceptedConnection* conn,
                        const boost::system::error_code& error);
template <typename Protocol, typename SocketAcceptorService>
void RPCListen(boost::shared_ptr< boost::asio::basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
                      boost::asio::io_context& io_context,
                      boost::asio::ssl::context& context,
                      const bool fUseSSL);

  bool HTTPAuthorized(map<string, string>& mapHeaders);
  std::map<string,string> argsMap_;
  CClientUIInterface* uiFace_;

};

#endif
