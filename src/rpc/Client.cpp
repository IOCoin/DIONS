
#include "core/Util.h"
#include "Sync.h"
#include "UIInterface.h"
#include "core/Base58.h"
#include "Client.h"
#include "wallet/DB.h"
#include "wallet/Wallet.h"
#undef printf
#include <boost/asio.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#define printf OutputDebugStringF
using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace json_spirit;
extern __wx__* pwalletMainId;
void StartShutdown();
static std::string strRPCUserColonPass;
const Object emptyobj;
  string JSONRPCRequest(const string& strMethod, const json_spirit::Array& params, const json_spirit::Value& id);
Object JSONRPCError(int code, const string& message)
{
  Object error;
  error.push_back(Pair("code", code));
  error.push_back(Pair("message", message));
  return error;
}
void RPCTypeCheck(const Array& params,
                  const list<Value_type>& typesExpected,
                  bool fAllowNull)
{
  unsigned int i = 0;
  BOOST_FOREACH(Value_type t, typesExpected)
  {
    if (params.size() <= i)
    {
      break;
    }

    const Value& v = params[i];

    if (!((v.type() == t) || (fAllowNull && (v.type() == null_type))))
    {
      string err = strprintf("Expected type %s, got %s",
                             Value_type_name[t], Value_type_name[v.type()]);
      throw JSONRPCError(RPC_TYPE_ERROR, err);
    }

    i++;
  }
}
void RPCTypeCheck(const Object& o,
                  const map<string, Value_type>& typesExpected,
                  bool fAllowNull)
{
  BOOST_FOREACH(const PAIRTYPE(string, Value_type)& t, typesExpected)
  {
    const Value& v = find_value(o, t.first);

    if (!fAllowNull && v.type() == null_type)
    {
      throw JSONRPCError(RPC_TYPE_ERROR, strprintf("Missing %s", t.first.c_str()));
    }

    if (!((v.type() == t.second) || (fAllowNull && (v.type() == null_type))))
    {
      string err = strprintf("Expected type %s for %s, got %s",
                             Value_type_name[t.second], t.first.c_str(), Value_type_name[v.type()]);
      throw JSONRPCError(RPC_TYPE_ERROR, err);
    }
  }
}
int64_t AmountFromValue(const Value& value)
{
  double dAmount = value.get_real();

  if (dAmount <= 0.0 || dAmount > ConfigurationState::MAX_MONEY)
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount");
  }

  int64_t nAmount = roundint64(dAmount * COIN);

  if (!MoneyRange(nAmount))
  {
    throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount");
  }

  return nAmount;
}
Value ValueFromAmount(int64_t amount)
{
  return (double)amount / (double)COIN;
}
uint256 ParseHashV(const Value& v, string aliasStr)
{
  string strHex;

  if (v.type() == str_type)
  {
    strHex = v.get_str();
  }

  if (!IsHex(strHex))
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, aliasStr+" must be hexadecimal string (not '"+strHex+"')");
  }

  uint256 result;
  result.SetHex(strHex);
  return result;
}
uint256 ParseHashO(const Object& o, string strKey)
{
  return ParseHashV(find_value(o, strKey), strKey);
}
vector<unsigned char> ParseHexV(const Value& v, string aliasStr)
{
  string strHex;

  if (v.type() == str_type)
  {
    strHex = v.get_str();
  }

  if (!IsHex(strHex))
  {
    throw JSONRPCError(RPC_INVALID_PARAMETER, aliasStr+" must be hexadecimal string (not '"+strHex+"')");
  }

  return ParseHex(strHex);
}
vector<unsigned char> ParseHexO(const Object& o, string strKey)
{
  return ParseHexV(find_value(o, strKey), strKey);
}
Value stop(const Array& params, bool fHelp)
{
  if (fHelp || params.size() > 1)
    throw runtime_error(
      "stop\n"
      "Stop I/OCoin server.");

  StartShutdown();
  return "I/OCoin server stopping";
}
string HTTPPost(const string& strMsg, const map<string,string>& mapRequestHeaders)
{
  ostringstream s;
  s << "POST / HTTP/1.1\r\n"
    << "User-Agent: iocoin-json-rpc/" << FormatFullVersion() << "\r\n"
    << "Host: 127.0.0.1\r\n"
    << "Content-Type: application/json\r\n"
    << "Content-Length: " << strMsg.size() << "\r\n"
    << "Connection: close\r\n"
    << "Accept: application/json\r\n";
  BOOST_FOREACH(const PAIRTYPE(string, string)& item, mapRequestHeaders)
  s << item.first << ": " << item.second << "\r\n";
  s << "\r\n" << strMsg;
  return s.str();
}
Object CallRPC(const string& strMethod, const Array& params)
{
  if (mapArgs["-rpcuser"] == "" && mapArgs["-rpcpassword"] == "")
    throw runtime_error(strprintf(
                          _("You must set rpcpassword=<password> in the configuration file:\n%s\n"
                            "If the file does not exist, create it with owner-readable-only file permissions."),
                          GetConfigFile().string().c_str()));

  bool fUseSSL = GetBoolArg("-rpcssl");
  asio::io_context io_context;
  ssl::context context(ssl::context::sslv23);
  context.set_options(ssl::context::no_sslv2);
  asio::ssl::stream<asio::ip::tcp::socket> sslStream(io_context, context);
  SSLIOStreamDevice<asio::ip::tcp> d(sslStream, fUseSSL);
  iostreams::stream< SSLIOStreamDevice<asio::ip::tcp> > stream(d);

  if (!d.connect(GetArg("-rpcconnect", "127.0.0.1"), GetArg("-rpcport", itostr(GetDefaultRPCPort()))))
  {
    throw runtime_error("couldn't connect to server");
  }

  string strUserPass64 = EncodeBase64(mapArgs["-rpcuser"] + ":" + mapArgs["-rpcpassword"]);
  map<string, string> mapRequestHeaders;
  mapRequestHeaders["Authorization"] = string("Basic ") + strUserPass64;
  string strRequest = JSONRPCRequest(strMethod, params, 1);
  string strPost = HTTPPost(strRequest, mapRequestHeaders);
  stream << strPost << std::flush;
  map<string, string> mapHeaders;
  string strReply;
  int nStatus = ReadHTTP(stream, mapHeaders, strReply);

  if (nStatus == HTTP_UNAUTHORIZED)
  {
    throw runtime_error("incorrect rpcuser or rpcpassword (authorization failed)");
  }
  else if (nStatus >= 400 && nStatus != HTTP_BAD_REQUEST && nStatus != HTTP_NOT_FOUND && nStatus != HTTP_INTERNAL_SERVER_ERROR)
  {
    throw runtime_error(strprintf("server returned HTTP error %d", nStatus));
  }
  else if (strReply.empty())
  {
    throw runtime_error("no response from server");
  }

  Value valReply;

  if (!read_string(strReply, valReply))
  {
    throw runtime_error("couldn't parse reply from server");
  }

  const Object& reply = valReply.get_obj();

  if (reply.empty())
  {
    throw runtime_error("expected reply to have result, error and id properties");
  }

  return reply;
}
template<typename T>
void ConvertTo(Value& value, bool fAllowNull=false)
{
  if (fAllowNull && value.type() == null_type)
  {
    return;
  }

  if (value.type() == str_type)
  {
    Value value2;
    string strJSON = value.get_str();

    if (!read_string(strJSON, value2))
    {
      throw runtime_error(string("Error parsing JSON:")+strJSON);
    }

    ConvertTo<T>(value2, fAllowNull);
    value = value2;
  }
  else
  {
    value = value.get_value<T>();
  }
}
Array RPCConvertValues(const std::string &strMethod, const std::vector<std::string> &strParams)
{
  Array params;
  BOOST_FOREACH(const std::string &param, strParams)
  params.push_back(param);
  int n = params.size();

  if (strMethod == "stop" && n > 0)
  {
    ConvertTo<bool>(params[0]);
  }

  if (strMethod == "sendtoaddress" && n > 1)
  {
    ConvertTo<double>(params[1]);
  }

  if (strMethod == "sendtodion" && n > 1)
  {
    ConvertTo<double>(params[1]);
  }

  if (strMethod == "settxfee" && n > 0)
  {
    ConvertTo<double>(params[0]);
  }

  if (strMethod == "getreceivedbyaddress" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "getreceivedbyaccount" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "listreceivedbyaddress" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "shadesend" && n > 1)
  {
    ConvertTo<double>(params[1]);
  }

  if (strMethod == "__vtx_s" && n > 1)
  {
    ConvertTo<double>(params[1]);
  }

  if (strMethod == "listreceivedbyaddress" && n > 1)
  {
    ConvertTo<bool>(params[1]);
  }

  if (strMethod == "listreceivedbyaccount" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "listreceivedbyaccount" && n > 1)
  {
    ConvertTo<bool>(params[1]);
  }

  if (strMethod == "getbalance" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "getblock" && n > 1)
  {
    ConvertTo<bool>(params[1]);
  }

  if (strMethod == "getpowblocks" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "getpowblocksleft" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "getpowtimeleft" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "getblockbynumber" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "getblockbynumber" && n > 1)
  {
    ConvertTo<bool>(params[1]);
  }

  if (strMethod == "getblockhash" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "gettxout" && n == 2)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "gettxout" && n == 3)
  {
    ConvertTo<int64_t>(params[1]);
    ConvertTo<bool>(params[2]);
  }

  if (strMethod == "getnetworkmhashps" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "getnetworkmhashps" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "move" && n > 2)
  {
    ConvertTo<double>(params[2]);
  }

  if (strMethod == "move" && n > 3)
  {
    ConvertTo<int64_t>(params[3]);
  }

  if (strMethod == "sendfrom" && n > 2)
  {
    ConvertTo<double>(params[2]);
  }

  if (strMethod == "sendfrom" && n > 3)
  {
    ConvertTo<int64_t>(params[3]);
  }

  if (strMethod == "listtransactions" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "listtransactions" && n > 2)
  {
    ConvertTo<int64_t>(params[2]);
  }

  if (strMethod == "listaccounts" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "walletpassphrase" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "walletpassphrase" && n > 2)
  {
    ConvertTo<bool>(params[2]);
  }

  if (strMethod == "getblocktemplate" && n > 0)
  {
    ConvertTo<Object>(params[0]);
  }

  if (strMethod == "listsinceblock" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "sendalert" && n > 2)
  {
    ConvertTo<int64_t>(params[2]);
  }

  if (strMethod == "sendalert" && n > 3)
  {
    ConvertTo<int64_t>(params[3]);
  }

  if (strMethod == "sendalert" && n > 4)
  {
    ConvertTo<int64_t>(params[4]);
  }

  if (strMethod == "sendalert" && n > 5)
  {
    ConvertTo<int64_t>(params[5]);
  }

  if (strMethod == "sendalert" && n > 6)
  {
    ConvertTo<int64_t>(params[6]);
  }

  if (strMethod == "sendalert" && n > 7)
  {
    ConvertTo<int64_t>(params[7]);
  }

  if (strMethod == "sendmany" && n > 1)
  {
    ConvertTo<Object>(params[1]);
  }

  if (strMethod == "sendmany" && n > 2)
  {
    ConvertTo<int64_t>(params[2]);
  }

  if (strMethod == "reservebalance" && n > 0)
  {
    ConvertTo<double>(params[0]);
  }

  if (strMethod == "addmultisigaddress" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "addmultisigaddress" && n > 1)
  {
    ConvertTo<Array>(params[1]);
  }

  if (strMethod == "listunspent" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  if (strMethod == "listunspent" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "listunspent" && n > 2)
  {
    ConvertTo<Array>(params[2]);
  }

  if (strMethod == "crawgen")
  {
    ConvertTo<double>(params[0]);
    ConvertTo<Object>(params[1]);
  }

  if (strMethod == "getrawtransaction" && n > 1)
  {
    ConvertTo<int64_t>(params[1]);
  }

  if (strMethod == "createrawtransaction" && n > 0)
  {
    ConvertTo<Array>(params[0]);
  }

  if (strMethod == "createrawtransaction" && n > 1)
  {
    ConvertTo<Object>(params[1]);
  }

  if (strMethod == "signrawtransaction" && n > 1)
  {
    ConvertTo<Array>(params[1], true);
  }

  if (strMethod == "signrawtransaction" && n > 2)
  {
    ConvertTo<Array>(params[2], true);
  }

  if (strMethod == "keypoolrefill" && n > 0)
  {
    ConvertTo<int64_t>(params[0]);
  }

  return params;
}
int CommandLineRPC(int argc, char *argv[])
{
  string strPrint;
  int nRet = 0;

  try
  {
    while (argc > 1 && IsSwitchChar(argv[1][0]))
    {
      argc--;
      argv++;
    }

    if (argc < 2)
    {
      throw runtime_error("too few parameters");
    }

    string strMethod = argv[1];
    std::vector<std::string> strParams(&argv[2], &argv[argc]);
    Array params = RPCConvertValues(strMethod, strParams);
    Object reply = CallRPC(strMethod, params);
    const Value& result = find_value(reply, "result");
    const Value& error = find_value(reply, "error");

    if (error.type() != null_type)
    {
      strPrint = "error: " + write_string(error, false);
      int code = find_value(error.get_obj(), "code").get_int();
      nRet = abs(code);
    }
    else
    {
      if (result.type() == null_type)
      {
        strPrint = "";
      }
      else if (result.type() == str_type)
      {
        strPrint = result.get_str();
      }
      else
      {
        strPrint = write_string(result, true);
      }
    }
  }
  catch (std::exception& e)
  {
    strPrint = string("error: ") + e.what();
    nRet = 87;
  }
  catch (...)
  {
    PrintException(NULL, "CommandLineRPC()");
  }

  if (strPrint != "")
  {
    fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
  }

  return nRet;
}
