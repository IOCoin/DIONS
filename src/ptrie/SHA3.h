#include "ptrie/FixedHash.h"
#include "ptrie/vector_ref.h"

#include <dvmash/keccak.hpp>

#include <string>

namespace dev
{





bool sha3(bytesConstRef _input, bytesRef o_output) noexcept;


inline h256 sha3(bytesConstRef _input) noexcept
{
  h256 ret;
  sha3(_input, ret.ref());
  return ret;
}

inline SecureFixedHash<32> sha3Secure(bytesConstRef _input) noexcept
{
  SecureFixedHash<32> ret;
  sha3(_input, ret.writable().ref());
  return ret;
}


inline h256 sha3(bytes const& _input) noexcept
{
  return sha3(bytesConstRef(&_input));
}

inline SecureFixedHash<32> sha3Secure(bytes const& _input) noexcept
{
  return sha3Secure(bytesConstRef(&_input));
}


inline h256 sha3(std::string const& _input) noexcept
{
  return sha3(bytesConstRef(_input));
}

inline SecureFixedHash<32> sha3Secure(std::string const& _input) noexcept
{
  return sha3Secure(bytesConstRef(_input));
}


inline h256 sha3(h256 const& _input) noexcept
{
  dvmash::hash256 hash = dvmash::keccak256_32(_input.data());
  return h256{hash.bytes, h256::ConstructFromPointer};
}


template <unsigned N>
inline h256 sha3(FixedHash<N> const& _input) noexcept
{
  return sha3(_input.ref());
}

template <unsigned N>
inline SecureFixedHash<32> sha3Secure(FixedHash<N> const& _input) noexcept
{
  return sha3Secure(_input.ref());
}


inline SecureFixedHash<32> sha3(bytesSec const& _input) noexcept
{
  return sha3Secure(_input.ref());
}

inline SecureFixedHash<32> sha3Secure(bytesSec const& _input) noexcept
{
  return sha3Secure(_input.ref());
}

template <unsigned N>
inline SecureFixedHash<32> sha3(SecureFixedHash<N> const& _input) noexcept
{
  return sha3Secure(_input.ref());
}

template <unsigned N>
inline SecureFixedHash<32> sha3Secure(SecureFixedHash<N> const& _input) noexcept
{
  return sha3Secure(_input.ref());
}


inline std::string sha3(std::string const& _input, bool _isNibbles)
{
  return asString((_isNibbles ? sha3(fromHex(_input)) : sha3(bytesConstRef(&_input))).asBytes());
}


inline void sha3mac(bytesConstRef _secret, bytesConstRef _plain, bytesRef _output)
{
  sha3(_secret.toBytes() + _plain.toBytes()).ref().populate(_output);
}

extern h256 const EmptySHA3;

extern h256 const EmptyListSHA3;

}
