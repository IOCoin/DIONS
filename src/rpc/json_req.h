#ifndef JSON_REQ_H
#define JSON_REQ_H

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

namespace rpc_util {
class JSONRequest
{
public:
  json_spirit::Value id;
  std::string strMethod;
  json_spirit::Array params;
  JSONRequest()
  {
    id = json_spirit::Value::null;
  }
  void parse(const json_spirit::Value& valRequest);
};
}

#endif
