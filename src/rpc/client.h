#ifndef _CLIENTRPC_H_
#define _CLIENTRPC_H_ 1

#include <string>
#include <list>
#include <map>

class CBlockIndex;

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include "core/util.h"
#include "checkpoints.h"

enum HTTPStatusCode
{
  HTTP_OK = 200,
  HTTP_BAD_REQUEST = 400,
  HTTP_UNAUTHORIZED = 401,
  HTTP_FORBIDDEN = 403,
  HTTP_NOT_FOUND = 404,
  HTTP_INTERNAL_SERVER_ERROR = 500,
};

enum RPCErrorCode
{

  RPC_INVALID_REQUEST = -32600,
  RPC_METHOD_NOT_FOUND = -32601,
  RPC_INVALID_PARAMS = -32602,
  RPC_INTERNAL_ERROR = -32603,
  RPC_PARSE_ERROR = -32700,


  RPC_MISC_ERROR = -1,
  RPC_FORBIDDEN_BY_SAFE_MODE = -2,
  RPC_TYPE_ERROR = -3,
  RPC_INVALID_ADDRESS_OR_KEY = -5,
  RPC_OUT_OF_MEMORY = -7,
  RPC_INVALID_PARAMETER = -8,
  RPC_DATABASE_ERROR = -20,
  RPC_DESERIALIZATION_ERROR = -22,


  RPC_CLIENT_NOT_CONNECTED = -9,
  RPC_CLIENT_IN_INITIAL_DOWNLOAD = -10,


  RPC_WALLET_ERROR = -4,
  RPC_WALLET_INSUFFICIENT_FUNDS = -6,
  RPC_WALLET_INVALID_ACCOUNT_NAME = -11,
  RPC_WALLET_KEYPOOL_RAN_OUT = -12,
  RPC_WALLET_UNLOCK_NEEDED = -13,
  RPC_WALLET_PASSPHRASE_INCORRECT = -14,
  RPC_WALLET_WRONG_ENC_STATE = -15,
  RPC_WALLET_ENCRYPTION_FAILED = -16,
  RPC_WALLET_ALREADY_UNLOCKED = -17,
};

json_spirit::Object JSONRPCError(int code, const std::string& message);

void ThreadRPCServer(void* parg);
int CommandLineRPC(int argc, char *argv[]);

json_spirit::Array RPCConvertValues(const std::string &strMethod, const std::vector<std::string> &strParams);

void RPCTypeCheck(const json_spirit::Array& params,
                  const std::list<json_spirit::Value_type>& typesExpected, bool fAllowNull=false);
void RPCTypeCheck(const json_spirit::Object& o,
                  const std::map<std::string, json_spirit::Value_type>& typesExpected, bool fAllowNull=false);

extern int64_t nWalletUnlockTime;
extern int64_t AmountFromValue(const json_spirit::Value& value);
extern json_spirit::Value ValueFromAmount(int64_t amount);
extern double GetDifficulty(const CBlockIndex* blockindex = NULL);

extern double GetPoWMHashPS(int nBlocks = 15, int nHeight = -1);
extern double GetPoSKernelPS(int nHeight = -1);

extern std::string HelpRequiringPassphrase();
extern void EnsureWalletIsUnlocked();

extern uint256 ParseHashV(const json_spirit::Value& v, std::string aliasStr);
extern uint256 ParseHashO(const json_spirit::Object& o, std::string strKey);
extern std::vector<unsigned char> ParseHexV(const json_spirit::Value& v, std::string aliasStr);
extern std::vector<unsigned char> ParseHexO(const json_spirit::Object& o, std::string strKey);
#endif
