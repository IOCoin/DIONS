
#include "init.h"
#include "util.h"
#include "sync.h"
#include "ui_interface.h"
#include "base58.h"
#include "bitcoinrpc.h"
#include "db.h"
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
static std::string strRPCUserColonPass;
const Object emptyobj;
static inline unsigned short GetDefaultRPCPort()
{
  return GetBoolArg("-testnet", false) ? 43765 : 33765;
}
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
string CRPCTable::help(string strCommand) const
{
  string strRet;
  set<rpcfn_type> setDone;

  for (map<string, const CRPCCommand*>::const_iterator mi = mapCommands.begin(); mi != mapCommands.end(); ++mi)
  {
    const CRPCCommand *pcmd = mi->second;
    string strMethod = mi->first;

    if (strMethod.find("label") != string::npos)
    {
      continue;
    }

    if (strCommand != "" && strMethod != strCommand)
    {
      continue;
    }

    try
    {
      Array params;
      rpcfn_type pfn = pcmd->actor;

      if (setDone.insert(pfn).second)
      {
        (*pfn)(params, true);
      }
    }
    catch (std::exception& e)
    {
      string strHelp = string(e.what());

      if (strCommand == "")
        if (strHelp.find('\n') != string::npos)
        {
          strHelp = strHelp.substr(0, strHelp.find('\n'));
        }

      strRet += strHelp + "\n";
    }
  }

  if (strRet == "")
  {
    strRet = strprintf("help: unknown command: %s\n", strCommand.c_str());
  }

  strRet = strRet.substr(0,strRet.size()-1);
  return strRet;
}
Value help(const Array& params, bool fHelp)
{
  if (params.size() > 1)
    throw runtime_error(
      "help [command]\n"
      "List commands, or get help for a command.");

  if(params.size() == 1)
  {
    string strCommand = params[0].get_str();
    return tableRPC.help(strCommand);
  }

  return tableRPC();
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
static const CRPCCommand vRPCCommands[] =
{
  { "help", &help, true, true },
  { "stop", &stop, true, true },
  { "getbestblockhash", &getbestblockhash, true, false },
  { "getblockcount", &getblockcount, true, false },
  { "getpowblocks", &getpowblocks, true, false },
  { "getpowblocksleft", &getpowblocksleft, true, false },
  { "getpowtimeleft", &getpowtimeleft, true, false },
  { "getconnectioncount", &getconnectioncount, true, false },
  { "getnumblocksofpeers", &getnumblocksofpeers, true, false },
  { "getpeerinfo", &getpeerinfo, true, false },
  { "getdifficulty", &getdifficulty, true, false },
  { "gw1", &gw1, true, false },
  { "vertexScan", &vertexScan, true, false },
  { "integratedTest1", &integratedTest1, true, false },
  { "integratedTest2", &integratedTest2, true, false },
  { "integratedTest3", &integratedTest3, true, false },
  { "integratedTest4", &integratedTest4, true, false },
  { "integratedTest5", &integratedTest5, true, false },
  { "integratedTest6", &integratedTest6, true, false },
  { "getnetworkmhashps", &getnetworkmhashps, true, false },
  { "getinfo", &getinfo, true, false },
  { "getsubsidy", &getsubsidy, true, false },
  { "getmininginfo", &getmininginfo, true, false },
  { "center__base__0", &center__base__0, true, false },
  { "getstakinginfo", &getstakinginfo, true, false },
  { "getnewaddress", &getnewaddress, true, false },
  { "sectionlog", &sectionlog, true, false },
  { "xtu_url", &xtu_url, false, false },
  { "getnewpubkey", &getnewpubkey, true, false },
  { "getaccountaddress", &getaccountaddress, true, false },
  { "setaccount", &setaccount, true, false },
  { "sa", &sa, true, false },
  { "shade", &shade, true, false },
  { "shadeK", &shadeK, true, false },
  { "shadesend", &shadesend, false, false },
  { "__vtx_s", &__vtx_s, false, false },
  { "ydwiWhldw_base_diff", &ydwiWhldw_base_diff, true, false },
  { "getaccount", &getaccount, false, false },
  { "getaddressesbyaccount", &getaddressesbyaccount, true, false },
  { "sendtoaddress", &sendtoaddress, false, false },
  { "sendtodion", &sendtodion, false, false },
  { "addresstodion", &addresstodion, false, false },
  { "getreceivedbyaddress", &getreceivedbyaddress, false, false },
  { "gra", &gra, false, false },
  { "getreceivedbyaccount", &getreceivedbyaccount, false, false },
  { "listreceivedbyaddress", &listreceivedbyaddress, false, false },
  { "listreceivedbyaccount", &listreceivedbyaccount, false, false },
  { "backupwallet", &backupwallet, true, false },
  { "keypoolrefill", &keypoolrefill, true, false },
  { "walletpassphrase", &walletpassphrase, true, false },
  { "walletpassphrasechange", &walletpassphrasechange, false, false },
  { "walletlock", &walletlock, true, false },
  { "walletlockstatus", &walletlockstatus, true, false },
  { "encryptwallet", &encryptwallet, false, false },
  { "getencryptionstatus", &getencryptionstatus, true, false },
  { "validateaddress", &validateaddress, true, false },
  { "validate", &validate, false, false },
  { "transform", &transform, false, false },
  { "validateLocator", &validateLocator, true, false },
  { "validatepubkey", &validatepubkey, true, false },
  { "pending", &pending, false, false },
  { "getbalance", &getbalance, false, false },
  { "move", &movecmd, false, false },
  { "sublimateYdwi", &sublimateYdwi, false, false },
  { "sendfrom", &sendfrom, false, false },
  { "sendmany", &sendmany, false, false },
  { "addmultisigaddress", &addmultisigaddress, false, false },
  { "addredeemscript", &addredeemscript, false, false },
  { "getrawmempool", &getrawmempool, true, false },
  { "gettxout", &gettxout, true, false },
  { "getblock", &getblock, false, false },
  { "getblockbynumber", &getblockbynumber, false, false },
  { "getblockhash", &getblockhash, false, false },
  { "gettransaction", &gettransaction, false, false },
  { "listtransactions", &listtransactions, false, false },
  { "listtransactions__", &listtransactions__, false, false },
  { "listaddressgroupings", &listaddressgroupings, false, false },
  { "signmessage", &signmessage, false, false },
  { "verifymessage", &verifymessage, false, false },
  { "getwork", &getwork, true, false },
  { "tmpTest", &tmpTest, true, false },
  { "getworkex", &getworkex, true, false },
  { "listaccounts", &listaccounts, false, false },
  { "settxfee", &settxfee, false, false },
  { "getblocktemplate", &getblocktemplate, true, false },
  { "submitblock", &submitblock, false, false },
  { "listsinceblock", &listsinceblock, false, false },
  { "dumpprivkey", &dumpprivkey, false, false },
  { "dumpwalletRT", &dumpwalletRT, true, false },
  { "dumpwallet", &dumpwallet, true, false },
  { "importwalletRT", &importwalletRT, false, false },
  { "importwallet", &importwallet, false, false },
  { "crawgen", &crawgen, false, false },
  { "rmtx", &rmtx, false, false },
  { "importprivkey", &importprivkey, false, false },
  { "listunspent", &listunspent, false, false },
  { "getrawtransaction", &getrawtransaction, false, false },
  { "trcbase", &trcbase, false, false },
  { "createrawtransaction", &createrawtransaction, false, false },
  { "trc", &trc, false, false },
  { "decoderawtransaction", &decoderawtransaction, false, false },
  { "sr71", &sr71, false, false },
  { "decodescript", &decodescript, false, false },
  { "signrawtransaction", &signrawtransaction, false, false },
  { "sendPlainMessage", &sendPlainMessage, false, false },
  { "vtx", &vtx, false, false },
  { "sendMessage", &sendMessage, false, false },
  { "publicKey", &publicKey, false, false },
  { "sendPublicKey", &sendPublicKey, false, false },
  { "sendSymmetric", &sendSymmetric, false, false },
  { "uC", &uC, false, false },
  { "registerAlias", &registerAlias, false, false },
  { "xstat", &xstat, false, false },
  { "mapProject", &mapProject, false, false },
  { "mapVertex", &mapVertex, false, false },
  { "registerAliasGenerate", &registerAliasGenerate, false, false },
  { "alias", &alias, false, false },
  { "statusList", &statusList, false, false },
  { "downloadDecrypt", &downloadDecrypt, false, false },
  { "downloadDecryptEPID", &downloadDecryptEPID, false, false },
  { "extract", &extract, false, false },
  { "updateEncrypt", &updateEncrypt, false, false },
  { "ioget", &ioget, false, false },
  { "simplexU", &simplexU, false, false },
  { "decryptAlias", &decryptAlias, false, false },
  { "transferAlias", &transferAlias, false, false },
  { "updateAlias", &updateAlias, false, false },
  { "updateAlias_executeContractPayload", &updateAlias_executeContractPayload, false, false },
  { "updateAliasFile", &updateAliasFile, false, false },
  { "primaryCXValidate", &primaryCXValidate, false, false },
  { "updateEncryptedAlias", &updateEncryptedAlias, false, false },
  { "psimplex", &psimplex, false, false },
  { "updateEncryptedAliasFile", &updateEncryptedAliasFile, false, false },
  { "transferAlias", &transferAlias, false, false },
  { "transferEncryptedAlias", &transferEncryptedAlias, false, false },
  { "transferEncryptedExtPredicate", &transferEncryptedExtPredicate, false, false },
  { "transientStatus__", &transientStatus__, false, false },
  { "transientStatus__C", &transientStatus__C, false, false },
  { "decryptedMessageList", &decryptedMessageList, false, false },
  { "plainTextMessageList", &plainTextMessageList, false, false },
  { "aliasOut", &aliasOut, false, false },
  { "externFrame__", &externFrame__, false, false },
  { "internFrame__", &internFrame__, false, false },
  { "aliasOut", &aliasOut, false, false },
  { "getNodeRecord", &getNodeRecord, false, false },
  { "projection", &projection, false, false },
  { "nodeRetrieve", &nodeRetrieve, false, false },
  { "nodeValidate", &nodeValidate, false, false },
  { "aliasList", &aliasList, true, false },
  { "aliasList__", &aliasList__, true, false },
  { "publicKeys", &publicKeys, false, false },
  { "publicKeyExports", &publicKeyExports, false, false },
  { "myRSAKeys", &myRSAKeys, false, false },
  { "nodeDebug", &nodeDebug, false, false },
  { "nodeDebug1", &nodeDebug1, false, false },
  { "vtxtrace", &vtxtrace, false, false },
  { "svtx", &svtx, false, false },
  { "sendrawtransaction", &sendrawtransaction, false, false },
  { "getcheckpoint", &getcheckpoint, true, false },
  { "reservebalance", &reservebalance, false, true},
  { "checkwallet", &checkwallet, false, true},
  { "repairwallet", &repairwallet, false, true},
  { "resendtx", &resendtx, false, true},
  { "makekeypair", &makekeypair, false, true},
  { "sendalert", &sendalert, false, false},
};
CRPCTable::CRPCTable()
{
  unsigned int vcidx;

  for (vcidx = 0; vcidx < (sizeof(vRPCCommands) / sizeof(vRPCCommands[0])); vcidx++)
  {
    const CRPCCommand *pcmd;
    pcmd = &vRPCCommands[vcidx];
    mapCommands[pcmd->name] = pcmd;
  }
}
const CRPCCommand* CRPCTable::operator[](string name) const
{
  map<string, const CRPCCommand*>::const_iterator it = mapCommands.find(name);

  if (it == mapCommands.end())
  {
    return NULL;
  }

  return (*it).second;
}
const json_spirit::Value CRPCTable::operator()() const
{
  Array v;
  map<string, const CRPCCommand*>::const_iterator it = mapCommands.begin();

  while(it != mapCommands.end())
  {
    Value o;
    o = (*it).first;
    v.push_back(o);
    it++;
  }

  return v;
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
static int ReadHTTPStatus(std::basic_istream<char>& stream, int &proto)
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
static int ReadHTTPHeader(std::basic_istream<char>& stream, map<string, string>& mapHeadersRet)
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
static int ReadHTTP(std::basic_istream<char>& stream, map<string, string>& mapHeadersRet, string& strMessageRet)
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
static string JSONRPCRequest(const string& strMethod, const Array& params, const Value& id)
{
  Object request;
  request.push_back(Pair("method", strMethod));
  request.push_back(Pair("params", params));
  request.push_back(Pair("id", id));
  return write_string(Value(request), false) + "\n";
}
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
class AcceptedConnection
{
public:
  virtual ~AcceptedConnection() {}
  virtual std::iostream& stream() = 0;
  virtual std::string peer_address_to_string() const = 0;
  virtual void close() = 0;
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
static void RPCAcceptHandler(boost::shared_ptr< basic_socket_acceptor<Protocol, SocketAcceptorService> > acceptor,
                             boost::asio::io_context& io_context,
                             ssl::context& context,
                             bool fUseSSL,
                             AcceptedConnection* conn,
                             const boost::system::error_code& error);
class JSONRequest
{
public:
  Value id;
  string strMethod;
  Array params;
  JSONRequest()
  {
    id = Value::null;
  }
  void parse(const Value& valRequest);
};
void JSONRequest::parse(const Value& valRequest)
{
  if (valRequest.type() != obj_type)
  {
    throw JSONRPCError(RPC_INVALID_REQUEST, "Invalid Request object");
  }

  const Object& request = valRequest.get_obj();
  id = find_value(request, "id");
  Value valMethod = find_value(request, "method");

  if (valMethod.type() == null_type)
  {
    throw JSONRPCError(RPC_INVALID_REQUEST, "Missing method");
  }

  if (valMethod.type() != str_type)
  {
    throw JSONRPCError(RPC_INVALID_REQUEST, "Method must be a string");
  }

  strMethod = valMethod.get_str();
  Value valParams = find_value(request, "params");

  if (valParams.type() == array_type)
  {
    params = valParams.get_array();
  }
  else if (valParams.type() == null_type)
  {
    params = Array();
  }
  else
  {
    throw JSONRPCError(RPC_INVALID_REQUEST, "Params must be an array");
  }
}
static CCriticalSection cs_THREAD_RPCHANDLER;
json_spirit::Value CRPCTable::execute(const std::string &strMethod, const json_spirit::Array &params) const
{
  const CRPCCommand *pcmd = tableRPC[strMethod];

  if (!pcmd)
  {
    throw JSONRPCError(RPC_METHOD_NOT_FOUND, "Method not found");
  }

  string strWarning = GetWarnings("rpc");

  if (strWarning != "" && !GetBoolArg("-disablesafemode") &&
      !pcmd->okSafeMode)
  {
    throw JSONRPCError(RPC_FORBIDDEN_BY_SAFE_MODE, string("Safe mode: ") + strWarning);
  }

  try
  {
    Value result;
    {
      if (pcmd->unlocked)
      {
        result = pcmd->actor(params, false);
      }
      else
      {
        LOCK2(cs_main, pwalletMainId->cs_wallet);
        result = pcmd->actor(params, false);
      }
    }
    return result;
  }
  catch (std::exception& e)
  {
    throw JSONRPCError(RPC_MISC_ERROR, e.what());
  }
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
const CRPCTable tableRPC;
