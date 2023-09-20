#include "json_req.h"
#include "client.h"

using namespace rpc_util;
using namespace json_spirit;

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
