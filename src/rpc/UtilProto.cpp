#include "JsonReq.h"
#include "UtilProto.h"

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
#include "Client.h" 

using namespace rpc_util;
using namespace std;

string HTTPReply(int nStatus, const string& strMsg, bool keepalive)
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
  string JSONRPCRequest(const string& strMethod, const json_spirit::Array& params, const json_spirit::Value& id)
  {
	  json_spirit::Object request;
    request.push_back(json_spirit::Pair("method", strMethod));
    request.push_back(json_spirit::Pair("params", params));
    request.push_back(json_spirit::Pair("id", id));
    return write_string(json_spirit::Value(request), false) + "\n";
  }
  json_spirit::Object JSONRPCReplyObj(const json_spirit::Value& result, const json_spirit::Value& error, const json_spirit::Value& id)
  {
	  json_spirit::Object reply;

    if (error.type() != json_spirit::null_type)
    {
      reply.push_back(json_spirit::Pair("result", json_spirit::Value::null));
    }
    else
    {
      reply.push_back(json_spirit::Pair("result", result));
    }

    reply.push_back(json_spirit::Pair("error", error));
    reply.push_back(json_spirit::Pair("id", id));
    return reply;
  }
  string JSONRPCReply(const json_spirit::Value& result, const json_spirit::Value& error, const json_spirit::Value& id)
  {
	  json_spirit::Object reply = JSONRPCReplyObj(result, error, id);
    return write_string(json_spirit::Value(reply), false) + "\n";
  }
  void ErrorReply(std::ostream& stream, const json_spirit::Object& objError, const json_spirit::Value& id)
  {
    int nStatus = HTTP_INTERNAL_SERVER_ERROR;
    int code = find_value(objError, "code").get_int();

    if (code == RPC_INVALID_REQUEST)
    {
      nStatus = HTTP_BAD_REQUEST;
    }
    else if (code == RPC_METHOD_NOT_FOUND)
    {
      nStatus = HTTP_NOT_FOUND;
    }

    string strReply = JSONRPCReply(json_spirit::Value::null, objError, id);
    stream << HTTPReply(nStatus, strReply, false) << std::flush;
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
  bool ClientAllowed(const boost::asio::ip::address& address)
  {
    if (address.is_v6()
        && (address.to_v6().is_v4_compatible()
            || address.to_v6().is_v4_mapped()))
    {
      return ClientAllowed(address.to_v6().to_v4());
    }

    if (address == boost::asio::ip::address_v4::loopback()
        || address == boost::asio::ip::address_v6::loopback()
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
unsigned short GetDefaultRPCPort()
{
  return GetBoolArg("-testnet", false) ? 43765 : 33765;
}
json_spirit::Object JSONRPCExecOne(const json_spirit::Value& req)
{
	json_spirit::Object rpc_result;
  JSONRequest jreq;

  try
  {
    jreq.parse(req);
    json_spirit::Value result;
    rpc_result = JSONRPCReplyObj(result, json_spirit::Value::null, jreq.id);
  }
  catch (json_spirit::Object& objError)
  {
    rpc_result = JSONRPCReplyObj(json_spirit::Value::null, objError, jreq.id);
  }
  catch (std::exception& e)
  {
    rpc_result = JSONRPCReplyObj(json_spirit::Value::null,
                                 JSONRPCError(RPC_PARSE_ERROR, e.what()), jreq.id);
  }

  return rpc_result;
}
int ReadHTTPStatus(std::basic_istream<char>& stream, int &proto)
{
  string str;
  getline(stream, str);
  vector<string> vWords;
  boost::split(vWords, str, boost::is_any_of(" "));

  if (vWords.size() < 2)
  {
    return HTTP_INTERNAL_SERVER_ERROR;
  }

  proto = 0;
  const char *ver = strstr(str.c_str(), "HTTP/1.");

  if (ver != NULL)
  {
    proto = atoi(ver+7);
  }

  return atoi(vWords[1].c_str());
}
int ReadHTTPHeader(std::basic_istream<char>& stream, map<string, string>& mapHeadersRet)
{
  int nLen = 0;

  while (true)
  {
    string str;
    std::getline(stream, str);

    if (str.empty() || str == "\r")
    {
      break;
    }

    string::size_type nColon = str.find(":");

    if (nColon != string::npos)
    {
      string strHeader = str.substr(0, nColon);
      boost::trim(strHeader);
      boost::to_lower(strHeader);
      string strValue = str.substr(nColon+1);
      boost::trim(strValue);
      mapHeadersRet[strHeader] = strValue;

      if (strHeader == "content-length")
      {
        nLen = atoi(strValue.c_str());
      }
    }
  }

  return nLen;
}
int ReadHTTP(std::basic_istream<char>& stream, map<string, string>& mapHeadersRet, string& strMessageRet)
{
  mapHeadersRet.clear();
  strMessageRet = "";
  int nProto = 0;
  int nStatus = ReadHTTPStatus(stream, nProto);
  int nLen = ReadHTTPHeader(stream, mapHeadersRet);

  if (nLen < 0 || nLen > (int)MAX_SIZE)
  {
    return HTTP_INTERNAL_SERVER_ERROR;
  }

  if (nLen > 0)
  {
    vector<char> vch(nLen);
    stream.read(&vch[0], nLen);
    strMessageRet = string(vch.begin(), vch.end());
  }

  string sConHdr = mapHeadersRet["connection"];

  if ((sConHdr != "close") && (sConHdr != "keep-alive"))
  {
    if (nProto >= 1)
    {
      mapHeadersRet["connection"] = "keep-alive";
    }
    else
    {
      mapHeadersRet["connection"] = "close";
    }
  }

  return nStatus;
}
string JSONRPCExecBatch(const json_spirit::Array& vReq)
{
	json_spirit::Array ret;

  for (unsigned int reqIdx = 0; reqIdx < vReq.size(); reqIdx++)
  {
    ret.push_back(JSONRPCExecOne(vReq[reqIdx]));
  }

  return write_string(json_spirit::Value(ret), false) + "\n";
}
