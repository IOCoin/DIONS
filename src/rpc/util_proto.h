#ifndef UTIL_PROTO_H
#define UTIL_PROTO_H

#include <stdlib.h>
#include <string>
#include <tuple>
#include <map>
#include <memory>
#include <vector>
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
bool ClientAllowed(const boost::asio::ip::address& address);
void StartShutdown();
unsigned short GetDefaultRPCPort();
std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);
std::string HTTPReply(int nStatus, const std::string& strMsg, bool keepalive);
int ReadHTTPStatus(std::basic_istream<char>& stream, int &proto);
int ReadHTTPHeader(std::basic_istream<char>& stream, std::map<std::string, std::string>& mapHeadersRet);
int ReadHTTP(std::basic_istream<char>& stream, std::map<std::string, std::string>& mapHeadersRet, std::string& strMessageRet);
int ReadHTTP(std::basic_istream<char>& stream, std::map<std::string, std::string>& mapHeadersRet, std::string& strMessageRet);
json_spirit::Object JSONRPCReplyObj(const json_spirit::Value& result, const json_spirit::Value& error, const json_spirit::Value& id);
json_spirit::Object JSONRPCExecOne(const json_spirit::Value& req);
std::string JSONRPCExecBatch(const json_spirit::Array& vReq);
json_spirit::Object JSONRPCExecOne(const json_spirit::Value& req);
int ReadHTTPStatus(std::basic_istream<char>& stream, int &proto);
int ReadHTTPHeader(std::basic_istream<char>& stream, std::map<std::string, std::string>& mapHeadersRet);
int ReadHTTP(std::basic_istream<char>& stream, std::map<std::string, std::string>& mapHeadersRet, std::string& strMessageRet);
std::string JSONRPCExecBatch(const json_spirit::Array& vReq);
std::string rfc1123Time();
std::string HTTPReply(int nStatus, const std::string& strMsg, bool keepalive);
std::string JSONRPCRequest(const std::string& strMethod, const json_spirit::Array& params, const json_spirit::Value& id);
  json_spirit::Object JSONRPCReplyObj(const json_spirit::Value& result, const json_spirit::Value& error, const json_spirit::Value& id);
  std::string JSONRPCReply(const json_spirit::Value& result, const json_spirit::Value& error, const json_spirit::Value& id);
  void ErrorReply(std::ostream& stream, const json_spirit::Object& objError, const json_spirit::Value& id);
  std::string rfc1123Time();

#endif
