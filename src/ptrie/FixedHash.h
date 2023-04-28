#include <array>
#include <cstdint>
#include <algorithm>
#include <random>
#include <boost/functional/hash.hpp>
#include "serialize.h"
#include "ptrie/CommonData.h"

namespace dev
{


template <unsigned N> struct StaticLog2
{
  enum { result = 1 + StaticLog2<N/2>::result };
};
template <> struct StaticLog2<1>
{
  enum { result = 0 };
};

extern std::random_device s_fixedHashEngine;




template <unsigned N>
class FixedHash
{
public:

  using Arith = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<N * 8, N * 8, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;


  enum { size = N };


  enum ConstructFromPointerType { ConstructFromPointer };


  enum ConstructFromStringType { FromHex, FromBinary };


  enum ConstructFromHashType { AlignLeft, AlignRight, FailIfDifferent };


  FixedHash()
  {
    m_data.fill(0);
  }


  template <unsigned M> explicit FixedHash(FixedHash<M> const& _h, ConstructFromHashType _t = AlignLeft)
  {
    m_data.fill(0);
    unsigned c = std::min(M, N);
    for (unsigned i = 0; i < c; ++i)
    {
      m_data[_t == AlignRight ? N - 1 - i : i] = _h[_t == AlignRight ? M - 1 - i : i];
    }
  }


  FixedHash(Arith const& _arith)
  {
    toBigEndian(_arith, m_data);
  }


  explicit FixedHash(unsigned _u)
  {
    toBigEndian(_u, m_data);
  }


  explicit FixedHash(bytes const& _b, ConstructFromHashType _t = FailIfDifferent)
  {
    if (_b.size() == N)
    {
      memcpy(m_data.data(), _b.data(), std::min<unsigned>(_b.size(), N));
    }
    else
    {
      m_data.fill(0);
      if (_t != FailIfDifferent)
      {
        auto c = std::min<unsigned>(_b.size(), N);
        for (unsigned i = 0; i < c; ++i)
        {
          m_data[_t == AlignRight ? N - 1 - i : i] = _b[_t == AlignRight ? _b.size() - 1 - i : i];
        }
      }
    }
  }


  explicit FixedHash(bytesConstRef _b, ConstructFromHashType _t = FailIfDifferent)
  {
    if (_b.size() == N)
    {
      memcpy(m_data.data(), _b.data(), std::min<unsigned>(_b.size(), N));
    }
    else
    {
      m_data.fill(0);
      if (_t != FailIfDifferent)
      {
        auto c = std::min<unsigned>(_b.size(), N);
        for (unsigned i = 0; i < c; ++i)
        {
          m_data[_t == AlignRight ? N - 1 - i : i] = _b[_t == AlignRight ? _b.size() - 1 - i : i];
        }
      }
    }
  }


  explicit FixedHash(byte const* _bs, ConstructFromPointerType)
  {
    memcpy(m_data.data(), _bs, N);
  }


  explicit FixedHash(std::string const& _s, ConstructFromStringType _t = FromHex, ConstructFromHashType _ht = FailIfDifferent): FixedHash(_t == FromHex ? fromHex(_s, WhenError::Throw) : dev::asBytes(_s), _ht) {}


  operator Arith() const
  {
    return fromBigEndian<Arith>(m_data);
  }


  explicit operator bool() const
  {
    return std::any_of(m_data.begin(), m_data.end(), [](byte _b)
    {
      return _b != 0;
    });
  }


  bool operator==(FixedHash const& _c) const
  {
    return m_data == _c.m_data;
  }
  bool operator!=(FixedHash const& _c) const
  {
    return m_data != _c.m_data;
  }
  bool operator<(FixedHash const& _c) const
  {
    for (unsigned i = 0; i < N; ++i) if (m_data[i] < _c.m_data[i])
      {
        return true;
      }
      else if (m_data[i] > _c.m_data[i])
      {
        return false;
      }
    return false;
  }
  bool operator>=(FixedHash const& _c) const
  {
    return !operator<(_c);
  }
  bool operator<=(FixedHash const& _c) const
  {
    return operator==(_c) || operator<(_c);
  }
  bool operator>(FixedHash const& _c) const
  {
    return !operator<=(_c);
  }


  FixedHash& operator^=(FixedHash const& _c)
  {
    for (unsigned i = 0; i < N; ++i)
    {
      m_data[i] ^= _c.m_data[i];
    }
    return *this;
  }
  FixedHash operator^(FixedHash const& _c) const
  {
    return FixedHash(*this) ^= _c;
  }
  FixedHash& operator|=(FixedHash const& _c)
  {
    for (unsigned i = 0; i < N; ++i)
    {
      m_data[i] |= _c.m_data[i];
    }
    return *this;
  }
  FixedHash operator|(FixedHash const& _c) const
  {
    return FixedHash(*this) |= _c;
  }
  FixedHash& operator&=(FixedHash const& _c)
  {
    for (unsigned i = 0; i < N; ++i)
    {
      m_data[i] &= _c.m_data[i];
    }
    return *this;
  }
  FixedHash operator&(FixedHash const& _c) const
  {
    return FixedHash(*this) &= _c;
  }
  FixedHash operator~() const
  {
    FixedHash ret;
    for (unsigned i = 0; i < N; ++i)
    {
      ret[i] = ~m_data[i];
    }
    return ret;
  }


  FixedHash& operator++()
  {
    for (unsigned i = size; i > 0 && !++m_data[--i]; ) {} return *this;
  }


  bool contains(FixedHash const& _c) const
  {
    return (*this & _c) == _c;
  }


  byte& operator[](unsigned _i)
  {
    return m_data[_i];
  }

  byte operator[](unsigned _i) const
  {
    return m_data[_i];
  }


  std::string abridged() const
  {
    return toHex(ref().cropped(0, 4)) + "\342\200\246";
  }


  std::string abridgedMiddle() const
  {
    return toHex(ref().cropped(0, 4)) + "\342\200\246" + toHex(ref().cropped(N - 4));
  }


  std::string hex() const
  {
    return toHex(ref());
  }


  bytesRef ref()
  {
    return bytesRef(m_data.data(), N);
  }


  bytesConstRef ref() const
  {
    return bytesConstRef(m_data.data(), N);
  }


  byte* data()
  {
    return m_data.data();
  }


  byte const* data() const
  {
    return m_data.data();
  }


  auto begin() const -> typename std::array<byte, N>::const_iterator
  {
    return m_data.begin();
  }


  auto end() const -> typename std::array<byte, N>::const_iterator
  {
    return m_data.end();
  }


  bytes asBytes() const
  {
    return bytes(data(), data() + N);
  }


  std::array<byte, N>& asArray()
  {
    return m_data;
  }


  std::array<byte, N> const& asArray() const
  {
    return m_data;
  }


  template <class Engine>
  void randomize(Engine& _eng)
  {
    for (auto& i: m_data)
    {
      i = (uint8_t)std::uniform_int_distribution<uint16_t>(0, 255)(_eng);
    }
  }


  static FixedHash random()
  {
    FixedHash ret;
    ret.randomize(s_fixedHashEngine);
    return ret;
  }

  struct hash
  {

    size_t operator()(FixedHash const& _value) const
    {
      return boost::hash_range(_value.m_data.cbegin(), _value.m_data.cend());
    }
  };

  template <unsigned P, unsigned M> inline FixedHash& shiftBloom(FixedHash<M> const& _h)
  {
    return (*this |= _h.template bloomPart<P, N>());
  }

  template <unsigned P, unsigned M> inline bool containsBloom(FixedHash<M> const& _h)
  {
    return contains(_h.template bloomPart<P, N>());
  }

  template <unsigned P, unsigned M> inline FixedHash<M> bloomPart() const
  {
    unsigned const c_bloomBits = M * 8;
    unsigned const c_mask = c_bloomBits - 1;
    unsigned const c_bloomBytes = (StaticLog2<c_bloomBits>::result + 7) / 8;

    static_assert((M & (M - 1)) == 0, "M must be power-of-two");
    static_assert(P * c_bloomBytes <= N, "out of range");

    FixedHash<M> ret;
    byte const* p = data();
    for (unsigned i = 0; i < P; ++i)
    {
      unsigned index = 0;
      for (unsigned j = 0; j < c_bloomBytes; ++j, ++p)
      {
        index = (index << 8) | *p;
      }
      index &= c_mask;
      ret[M - 1 - index / 8] |= (1 << (index % 8));
    }
    return ret;
  }


  inline unsigned firstBitSet() const
  {
    unsigned ret = 0;
    for (auto d: m_data)
      if (d)
      {
        for (;; ++ret, d <<= 1)
        {
          if (d & 0x80)
          {
            return ret;
          }
        }
      }
      else
      {
        ret += 8;
      }
    return ret;
  }

  void clear()
  {
    m_data.fill(0);
  }


  unsigned int GetSerializeSize(int nType, int nVersion) const
  {
    return sizeof(m_data.data());
  }

  template<typename Stream>
  void Serialize(Stream& s, int nType, int nVersion) const
  {
    s.write((char*)m_data.data(), sizeof(m_data.data()));
  }

  template<typename Stream>
  void Unserialize(Stream& s, int nType, int nVersion)
  {
    s.read((char*)m_data.data(), sizeof(m_data.data()));
  }


private:
  std::array<byte, N> m_data;
};

template <unsigned T>
class SecureFixedHash: private FixedHash<T>
{
public:
  using ConstructFromHashType = typename FixedHash<T>::ConstructFromHashType;
  using ConstructFromStringType = typename FixedHash<T>::ConstructFromStringType;
  using ConstructFromPointerType = typename FixedHash<T>::ConstructFromPointerType;
  SecureFixedHash() = default;
  SecureFixedHash(SecureFixedHash const&) = default;
  explicit SecureFixedHash(bytes const& _b, ConstructFromHashType _t = FixedHash<T>::FailIfDifferent): FixedHash<T>(_b, _t) {}
  explicit SecureFixedHash(bytesConstRef _b, ConstructFromHashType _t = FixedHash<T>::FailIfDifferent): FixedHash<T>(_b, _t) {}
  explicit SecureFixedHash(bytesSec const& _b, ConstructFromHashType _t = FixedHash<T>::FailIfDifferent): FixedHash<T>(_b.ref(), _t) {}
  template <unsigned M> explicit SecureFixedHash(FixedHash<M> const& _h, ConstructFromHashType _t = FixedHash<T>::AlignLeft): FixedHash<T>(_h, _t) {}
  template <unsigned M> explicit SecureFixedHash(SecureFixedHash<M> const& _h, ConstructFromHashType _t = FixedHash<T>::AlignLeft): FixedHash<T>(_h.makeInsecure(), _t) {}
  explicit SecureFixedHash(std::string const& _s, ConstructFromStringType _t = FixedHash<T>::FromHex, ConstructFromHashType _ht = FixedHash<T>::FailIfDifferent): FixedHash<T>(_s, _t, _ht) {}
  explicit SecureFixedHash(byte const* _d, ConstructFromPointerType _t): FixedHash<T>(_d, _t) {}
  ~SecureFixedHash()
  {
    ref().cleanse();
  }

  SecureFixedHash<T>& operator=(SecureFixedHash<T> const& _c)
  {
    if (&_c == this)
    {
      return *this;
    }
    ref().cleanse();
    FixedHash<T>::operator=(static_cast<FixedHash<T> const&>(_c));
    return *this;
  }

  using FixedHash<T>::size;

  bytesSec asBytesSec() const
  {
    return bytesSec(ref());
  }

  FixedHash<T> const& makeInsecure() const
  {
    return static_cast<FixedHash<T> const&>(*this);
  }
  FixedHash<T>& writable()
  {
    clear();
    return static_cast<FixedHash<T>&>(*this);
  }

  using FixedHash<T>::operator bool;


  bool operator==(SecureFixedHash const& _c) const
  {
    return static_cast<FixedHash<T> const&>(*this).operator==(static_cast<FixedHash<T> const&>(_c));
  }
  bool operator!=(SecureFixedHash const& _c) const
  {
    return static_cast<FixedHash<T> const&>(*this).operator!=(static_cast<FixedHash<T> const&>(_c));
  }
  bool operator<(SecureFixedHash const& _c) const
  {
    return static_cast<FixedHash<T> const&>(*this).operator<(static_cast<FixedHash<T> const&>(_c));
  }
  bool operator>=(SecureFixedHash const& _c) const
  {
    return static_cast<FixedHash<T> const&>(*this).operator>=(static_cast<FixedHash<T> const&>(_c));
  }
  bool operator<=(SecureFixedHash const& _c) const
  {
    return static_cast<FixedHash<T> const&>(*this).operator<=(static_cast<FixedHash<T> const&>(_c));
  }
  bool operator>(SecureFixedHash const& _c) const
  {
    return static_cast<FixedHash<T> const&>(*this).operator>(static_cast<FixedHash<T> const&>(_c));
  }

  using FixedHash<T>::operator==;
  using FixedHash<T>::operator!=;
  using FixedHash<T>::operator<;
  using FixedHash<T>::operator>=;
  using FixedHash<T>::operator<=;
  using FixedHash<T>::operator>;


  SecureFixedHash& operator^=(FixedHash<T> const& _c)
  {
    static_cast<FixedHash<T>&>(*this).operator^=(_c);
    return *this;
  }
  SecureFixedHash operator^(FixedHash<T> const& _c) const
  {
    return SecureFixedHash(*this) ^= _c;
  }
  SecureFixedHash& operator|=(FixedHash<T> const& _c)
  {
    static_cast<FixedHash<T>&>(*this).operator^=(_c);
    return *this;
  }
  SecureFixedHash operator|(FixedHash<T> const& _c) const
  {
    return SecureFixedHash(*this) |= _c;
  }
  SecureFixedHash& operator&=(FixedHash<T> const& _c)
  {
    static_cast<FixedHash<T>&>(*this).operator^=(_c);
    return *this;
  }
  SecureFixedHash operator&(FixedHash<T> const& _c) const
  {
    return SecureFixedHash(*this) &= _c;
  }

  SecureFixedHash& operator^=(SecureFixedHash const& _c)
  {
    static_cast<FixedHash<T>&>(*this).operator^=(static_cast<FixedHash<T> const&>(_c));
    return *this;
  }
  SecureFixedHash operator^(SecureFixedHash const& _c) const
  {
    return SecureFixedHash(*this) ^= _c;
  }
  SecureFixedHash& operator|=(SecureFixedHash const& _c)
  {
    static_cast<FixedHash<T>&>(*this).operator^=(static_cast<FixedHash<T> const&>(_c));
    return *this;
  }
  SecureFixedHash operator|(SecureFixedHash const& _c) const
  {
    return SecureFixedHash(*this) |= _c;
  }
  SecureFixedHash& operator&=(SecureFixedHash const& _c)
  {
    static_cast<FixedHash<T>&>(*this).operator^=(static_cast<FixedHash<T> const&>(_c));
    return *this;
  }
  SecureFixedHash operator&(SecureFixedHash const& _c) const
  {
    return SecureFixedHash(*this) &= _c;
  }
  SecureFixedHash operator~() const
  {
    auto r = ~static_cast<FixedHash<T> const&>(*this);
    return static_cast<SecureFixedHash const&>(r);
  }

  using FixedHash<T>::abridged;
  using FixedHash<T>::abridgedMiddle;

  bytesConstRef ref() const
  {
    return FixedHash<T>::ref();
  }
  byte const* data() const
  {
    return FixedHash<T>::data();
  }

  static SecureFixedHash<T> random()
  {
    SecureFixedHash<T> ret;
    ret.randomize(s_fixedHashEngine);
    return ret;
  }
  using FixedHash<T>::firstBitSet;

  void clear()
  {
    ref().cleanse();
  }
};


template<> inline bool FixedHash<32>::operator==(FixedHash<32> const& _other) const
{
  const uint64_t* hash1 = (const uint64_t*)data();
  const uint64_t* hash2 = (const uint64_t*)_other.data();
  return (hash1[0] == hash2[0]) && (hash1[1] == hash2[1]) && (hash1[2] == hash2[2]) && (hash1[3] == hash2[3]);
}


template<> inline size_t FixedHash<32>::hash::operator()(FixedHash<32> const& value) const
{
  uint64_t const* data = reinterpret_cast<uint64_t const*>(value.data());
  return boost::hash_range(data, data + 4);
}


template <unsigned N>
inline std::ostream& operator<<(std::ostream& _out, FixedHash<N> const& _h)
{
  _out << toHex(_h);
  return _out;
}

template <unsigned N>
inline std::istream& operator>>(std::istream& _in, FixedHash<N>& o_h)
{
  std::string s;
  _in >> s;
  o_h = FixedHash<N>(s, FixedHash<N>::FromHex, FixedHash<N>::AlignRight);
  return _in;
}


template <unsigned N>
inline std::ostream& operator<<(std::ostream& _out, SecureFixedHash<N> const& _h)
{
  _out << "SecureFixedHash#" << std::hex << typename FixedHash<N>::hash()(_h.makeInsecure()) << std::dec;
  return _out;
}


using h2048 = FixedHash<256>;
using h1024 = FixedHash<128>;
using h520 = FixedHash<65>;
using h512 = FixedHash<64>;
using h256 = FixedHash<32>;
using h160 = FixedHash<20>;
using h128 = FixedHash<16>;
using h64 = FixedHash<8>;
using h512s = std::vector<h512>;
using h256s = std::vector<h256>;
using h160s = std::vector<h160>;
using h256Set = std::set<h256>;
using h160Set = std::set<h160>;
using h256Hash = std::unordered_set<h256>;
using h160Hash = std::unordered_set<h160>;


inline h160 right160(h256 const& _t)
{
  h160 ret;
  memcpy(ret.data(), _t.data() + 12, 20);
  return ret;
}

h128 fromUUID(std::string const& _uuid);

std::string toUUID(h128 const& _uuid);

inline std::string toString(h256s const& _bs)
{
  std::ostringstream out;
  out << "[ ";
  for (h256 const& i: _bs)
  {
    out << i.abridged() << ", ";
  }
  out << "]";
  return out.str();
}

}

namespace std
{

template<> struct hash<dev::h64>: dev::h64::hash {};
template<> struct hash<dev::h128>: dev::h128::hash {};
template<> struct hash<dev::h160>: dev::h160::hash {};
template<> struct hash<dev::h256>: dev::h256::hash {};
template<> struct hash<dev::h512>: dev::h512::hash {};
}
