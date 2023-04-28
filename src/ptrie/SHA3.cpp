#include "ptrie/SHA3.h"
#include "ptrie/RLP.h"

#include <dvmash/keccak.hpp>

namespace dev
{
h256 const EmptySHA3 = sha3(bytesConstRef());
h256 const EmptyListSHA3 = sha3(rlpList());

bool sha3(bytesConstRef _input, bytesRef o_output) noexcept
{
  if (o_output.size() != 32)
  {
    return false;
  }
  dvmash::hash256 h = dvmash::keccak256(_input.data(), _input.size());
  bytesConstRef{h.bytes, 32}.copyTo(o_output);
  return true;
}
}
