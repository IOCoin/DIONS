#include "ptrie/Exceptions.h"
#include "ptrie/FixedHash.h"
#include "ptrie/vector_ref.h"

#include <array>
#include <exception>
#include <iomanip>
#include <iosfwd>
#include <vector>

namespace dev
{

class RLP;

template <class _T> struct intTraits
{
  static const unsigned maxSize = sizeof(_T);
};
template <> struct intTraits<u160>
{
  static const unsigned maxSize = 20;
};
template <> struct intTraits<u256>
{
  static const unsigned maxSize = 32;
};
template <> struct intTraits<bigint>
{
  static const unsigned maxSize = ~(unsigned)0;
};

static const byte c_rlpMaxLengthBytes = 8;
static const byte c_rlpDataImmLenStart = 0x80;
static const byte c_rlpListStart = 0xc0;

static const byte c_rlpDataImmLenCount = c_rlpListStart - c_rlpDataImmLenStart - c_rlpMaxLengthBytes;
static const byte c_rlpDataIndLenZero = c_rlpDataImmLenStart + c_rlpDataImmLenCount - 1;
static const byte c_rlpListImmLenCount = 256 - c_rlpListStart - c_rlpMaxLengthBytes;
static const byte c_rlpListIndLenZero = c_rlpListStart + c_rlpListImmLenCount - 1;

template <class T> struct Converter
{
  static T convert(RLP const&, int)
  {
    BOOST_THROW_EXCEPTION(BadCast());
  }
};




class RLP
{
public:

  enum
  {
    AllowNonCanon = 1,
    ThrowOnFail = 4,
    FailIfTooBig = 8,
    FailIfTooSmall = 16,
    Strict = ThrowOnFail | FailIfTooBig,
    VeryStrict = ThrowOnFail | FailIfTooBig | FailIfTooSmall,
    LaissezFaire = AllowNonCanon
  };

  using Strictness = int;


  RLP() {}


  explicit RLP(bytesConstRef _d, Strictness _s = VeryStrict);


  explicit RLP(bytes const& _d, Strictness _s = VeryStrict): RLP(&_d, _s) {}


  RLP(byte const* _b, unsigned _s, Strictness _st = VeryStrict): RLP(bytesConstRef(_b, _s), _st) {}


  explicit RLP(std::string const& _s, Strictness _st = VeryStrict): RLP(bytesConstRef((byte const*)_s.data(), _s.size()), _st) {}


  bytesConstRef data() const
  {
    return m_data;
  }


  explicit operator bool() const
  {
    return !isNull();
  }


  bool isNull() const
  {
    return m_data.size() == 0;
  }


  bool isEmpty() const
  {
    return !isNull() && (m_data[0] == c_rlpDataImmLenStart || m_data[0] == c_rlpListStart);
  }


  bool isData() const
  {
    return !isNull() && m_data[0] < c_rlpListStart;
  }


  bool isList() const
  {
    return !isNull() && m_data[0] >= c_rlpListStart;
  }


  bool isInt() const;


  size_t itemCount() const
  {
    return isList() ? items() : 0;
  }
  size_t itemCountStrict() const
  {
    if (!isList())
    {
      BOOST_THROW_EXCEPTION(BadCast());
    }
    return items();
  }


  size_t size() const
  {
    return isData() ? length() : 0;
  }
  size_t sizeStrict() const
  {
    if (!isData())
    {
      BOOST_THROW_EXCEPTION(BadCast());
    }
    return length();
  }


  bool operator==(char const* _s) const
  {
    return isData() && toString() == _s;
  }
  bool operator!=(char const* _s) const
  {
    return isData() && toString() != _s;
  }
  bool operator==(std::string const& _s) const
  {
    return isData() && toString() == _s;
  }
  bool operator!=(std::string const& _s) const
  {
    return isData() && toString() != _s;
  }
  template <unsigned _N> bool operator==(FixedHash<_N> const& _h) const
  {
    return isData() && toHash<_N>() == _h;
  }
  template <unsigned _N> bool operator!=(FixedHash<_N> const& _s) const
  {
    return isData() && toHash<_N>() != _s;
  }
  bool operator==(unsigned const& _i) const
  {
    return isInt() && toInt<unsigned>() == _i;
  }
  bool operator!=(unsigned const& _i) const
  {
    return isInt() && toInt<unsigned>() != _i;
  }
  bool operator==(u256 const& _i) const
  {
    return isInt() && toInt<u256>() == _i;
  }
  bool operator!=(u256 const& _i) const
  {
    return isInt() && toInt<u256>() != _i;
  }
  bool operator==(bigint const& _i) const
  {
    return isInt() && toInt<bigint>() == _i;
  }
  bool operator!=(bigint const& _i) const
  {
    return isInt() && toInt<bigint>() != _i;
  }




  RLP operator[](size_t _i) const;

  using element_type = RLP;


  class iterator
  {
    friend class RLP;

  public:
    using value_type = RLP;
    using element_type = RLP;

    iterator& operator++();
    iterator operator++(int)
    {
      auto ret = *this;
      operator++();
      return ret;
    }
    RLP operator*() const
    {
      return RLP(m_currentItem);
    }
    bool operator==(iterator const& _cmp) const
    {
      return m_currentItem == _cmp.m_currentItem;
    }
    bool operator!=(iterator const& _cmp) const
    {
      return !operator==(_cmp);
    }

  private:
    iterator() {}
    iterator(RLP const& _parent, bool _begin);

    size_t m_remaining = 0;
    bytesConstRef m_currentItem;
  };


  iterator begin() const
  {
    return iterator(*this, true);
  }


  iterator end() const
  {
    return iterator(*this, false);
  }

  template <class T> inline T convert(int _flags) const;


  explicit operator std::string() const
  {
    return toString();
  }
  explicit operator bytes() const
  {
    return toBytes();
  }
  explicit operator uint8_t() const
  {
    return toInt<uint8_t>();
  }
  explicit operator uint16_t() const
  {
    return toInt<uint16_t>();
  }
  explicit operator uint32_t() const
  {
    return toInt<uint32_t>();
  }
  explicit operator uint64_t() const
  {
    return toInt<uint64_t>();
  }
  explicit operator u160() const
  {
    return toInt<u160>();
  }
  explicit operator u256() const
  {
    return toInt<u256>();
  }
  explicit operator bigint() const
  {
    return toInt<bigint>();
  }
  template <unsigned N> explicit operator FixedHash<N>() const
  {
    return toHash<FixedHash<N>>();
  }
  template <class T, class U> explicit operator std::pair<T, U>() const
  {
    return toPair<T, U>();
  }
  template <class T> explicit operator std::vector<T>() const
  {
    return toVector<T>();
  }
  template <class T> explicit operator std::set<T>() const
  {
    return toSet<T>();
  }
  template <class T, size_t N> explicit operator std::array<T, N>() const
  {
    return toArray<T, N>();
  }


  bytes toBytes(int _flags = LaissezFaire) const
  {
    if (!isData())
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return bytes();
      }
    }
    return bytes(payload().data(), payload().data() + length());
  }

  bytesConstRef toBytesConstRef(int _flags = LaissezFaire) const
  {
    if (!isData())
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return bytesConstRef();
      }
    }
    return payload().cropped(0, length());
  }

  std::string toString(int _flags = LaissezFaire) const
  {
    if (!isData())
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return std::string();
      }
    }
    return payload().cropped(0, length()).toString();
  }

  std::string toStringStrict() const
  {
    return toString(Strict);
  }

  template <class T>
  std::vector<T> toVector(int _flags = LaissezFaire) const
  {
    std::vector<T> ret;
    if (isList())
    {
      ret.reserve(itemCount());
      for (auto const& i: *this)
      {
        ret.push_back(i.convert<T>(_flags));
      }
    }
    else if (_flags & ThrowOnFail)
    {
      BOOST_THROW_EXCEPTION(BadCast());
    }
    return ret;
  }

  template <class T>
  std::set<T> toSet(int _flags = LaissezFaire) const
  {
    std::set<T> ret;
    if (isList())
      for (auto const& i: *this)
      {
        ret.insert(i.convert<T>(_flags));
      }
    else if (_flags & ThrowOnFail)
    {
      BOOST_THROW_EXCEPTION(BadCast());
    }
    return ret;
  }

  template <class T>
  std::unordered_set<T> toUnorderedSet(int _flags = LaissezFaire) const
  {
    std::unordered_set<T> ret;
    if (isList())
      for (auto const& i: *this)
      {
        ret.insert(i.convert<T>(_flags));
      }
    else if (_flags & ThrowOnFail)
    {
      BOOST_THROW_EXCEPTION(BadCast());
    }
    return ret;
  }

  template <class T, class U>
  std::pair<T, U> toPair(int _flags = Strict) const
  {
    std::pair<T, U> ret;
    if (itemCountStrict() != 2)
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return ret;
      }
    }
    ret.first = (*this)[0].convert<T>(_flags);
    ret.second = (*this)[1].convert<U>(_flags);
    return ret;
  }

  template <class T, size_t N>
  std::array<T, N> toArray(int _flags = LaissezFaire) const
  {
    if (itemCount() != N)
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return std::array<T, N>();
      }
    }
    std::array<T, N> ret;
    for (size_t i = 0; i < N; ++i)
    {
      ret[i] = operator[](i).convert<T>(_flags);
    }
    return ret;
  }


  template <class _T = unsigned> _T toInt(int _flags = Strict) const
  {
    requireGood();
    if ((!isInt() && !(_flags & AllowNonCanon)) || isList() || isNull())
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return 0;
      }
    }

    auto p = payload();
    if (p.size() > intTraits<_T>::maxSize && (_flags & FailIfTooBig))
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return 0;
      }
    }

    return fromBigEndian<_T>(p);
  }

  int64_t toPositiveInt64(int _flags = Strict) const
  {
    int64_t i = toInt<int64_t>(_flags);
    if ((_flags & ThrowOnFail) && i < 0)
    {
      BOOST_THROW_EXCEPTION(BadCast());
    }
    return i;
  }

  template <class _N> _N toHash(int _flags = Strict) const
  {
    requireGood();
    auto p = payload();
    auto l = p.size();
    if (!isData() || (l > _N::size && (_flags & FailIfTooBig)) || (l < _N::size && (_flags & FailIfTooSmall)))
    {
      if (_flags & ThrowOnFail)
      {
        BOOST_THROW_EXCEPTION(BadCast());
      }
      else
      {
        return _N();
      }
    }

    _N ret;
    size_t s = std::min<size_t>(_N::size, l);
    memcpy(ret.data() + _N::size - s, p.data(), s);
    return ret;
  }


  bytesConstRef payload() const
  {
    auto l = length();
    if (l > m_data.size())
    {
      BOOST_THROW_EXCEPTION(BadRLP());
    }
    return m_data.cropped(payloadOffset(), l);
  }



  size_t actualSize() const;

private:

  explicit RLP(bytes const&&) {}


  void requireGood() const;


  bool isSingleByte() const
  {
    return !isNull() && m_data[0] < c_rlpDataImmLenStart;
  }


  unsigned lengthSize() const
  {
    if (isData() && m_data[0] > c_rlpDataIndLenZero)
    {
      return m_data[0] - c_rlpDataIndLenZero;
    }
    if (isList() && m_data[0] > c_rlpListIndLenZero)
    {
      return m_data[0] - c_rlpListIndLenZero;
    }
    return 0;
  }


  size_t length() const;


  size_t payloadOffset() const
  {
    return isSingleByte() ? 0 : (1 + lengthSize());
  }


  size_t items() const;


  static size_t sizeAsEncoded(bytesConstRef _data)
  {
    return RLP(_data, ThrowOnFail | FailIfTooSmall).actualSize();
  }


  bytesConstRef m_data;



  mutable size_t m_lastIndex = (size_t)-1;

  mutable size_t m_lastEnd = 0;

  mutable bytesConstRef m_lastItem;
};

template <> struct Converter<std::string>
{
  static std::string convert(RLP const& _r, int _flags)
  {
    return _r.toString(_flags);
  }
};
template <> struct Converter<bytes>
{
  static bytes convert(RLP const& _r, int _flags)
  {
    return _r.toBytes(_flags);
  }
};
template <> struct Converter<uint8_t>
{
  static uint8_t convert(RLP const& _r, int _flags)
  {
    return _r.toInt<uint8_t>(_flags);
  }
};
template <> struct Converter<uint16_t>
{
  static uint16_t convert(RLP const& _r, int _flags)
  {
    return _r.toInt<uint16_t>(_flags);
  }
};
template <> struct Converter<uint32_t>
{
  static uint32_t convert(RLP const& _r, int _flags)
  {
    return _r.toInt<uint32_t>(_flags);
  }
};
template <> struct Converter<uint64_t>
{
  static uint64_t convert(RLP const& _r, int _flags)
  {
    return _r.toInt<uint64_t>(_flags);
  }
};
template <> struct Converter<u160>
{
  static u160 convert(RLP const& _r, int _flags)
  {
    return _r.toInt<u160>(_flags);
  }
};
template <> struct Converter<u256>
{
  static u256 convert(RLP const& _r, int _flags)
  {
    return _r.toInt<u256>(_flags);
  }
};
template <> struct Converter<bigint>
{
  static bigint convert(RLP const& _r, int _flags)
  {
    return _r.toInt<bigint>(_flags);
  }
};
template <unsigned N> struct Converter<FixedHash<N>>
{
  static FixedHash<N> convert(RLP const& _r, int _flags)
  {
    return _r.toHash<FixedHash<N>>(_flags);
  }
};
template <class T, class U> struct Converter<std::pair<T, U>>
{
  static std::pair<T, U> convert(RLP const& _r, int _flags)
  {
    return _r.toPair<T, U>(_flags);
  }
};
template <class T> struct Converter<std::vector<T>>
{
  static std::vector<T> convert(RLP const& _r, int _flags)
  {
    return _r.toVector<T>(_flags);
  }
};
template <class T> struct Converter<std::set<T>>
{
  static std::set<T> convert(RLP const& _r, int _flags)
  {
    return _r.toSet<T>(_flags);
  }
};
template <class T> struct Converter<std::unordered_set<T>>
{
  static std::unordered_set<T> convert(RLP const& _r, int _flags)
  {
    return _r.toUnorderedSet<T>(_flags);
  }
};
template <class T, size_t N> struct Converter<std::array<T, N>>
{
  static std::array<T, N> convert(RLP const& _r, int _flags)
  {
    return _r.toArray<T, N>(_flags);
  }
};

template <class T> inline T RLP::convert(int _flags) const
{
  return Converter<T>::convert(*this, _flags);
}




class RLPStream
{
public:

  RLPStream() {}


  explicit RLPStream(size_t _listItems)
  {
    appendList(_listItems);
  }

  ~RLPStream() {}


  RLPStream& append(unsigned _s)
  {
    return append(bigint(_s));
  }
  RLPStream& append(u160 _s)
  {
    return append(bigint(_s));
  }
  RLPStream& append(u256 _s)
  {
    return append(bigint(_s));
  }
  RLPStream& append(bigint _s);
  RLPStream& append(bytesConstRef _s, bool _compact = false);
  RLPStream& append(bytes const& _s)
  {
    return append(bytesConstRef(&_s));
  }
  RLPStream& append(std::string const& _s)
  {
    return append(bytesConstRef(_s));
  }
  RLPStream& append(char const* _s)
  {
    return append(std::string(_s));
  }
  template <unsigned N> RLPStream& append(FixedHash<N> _s, bool _compact = false, bool _allOrNothing = false)
  {
    return _allOrNothing && !_s ? append(bytesConstRef()) : append(_s.ref(), _compact);
  }


  RLPStream& append(RLP const& _rlp, size_t _itemCount = 1)
  {
    return appendRaw(_rlp.data(), _itemCount);
  }


  template <class _T> RLPStream& append(std::vector<_T> const& _s)
  {
    return appendVector(_s);
  }
  template <class _T> RLPStream& appendVector(std::vector<_T> const& _s)
  {
    appendList(_s.size());
    for (auto const& i: _s)
    {
      append(i);
    }
    return *this;
  }
  template <class _T, size_t S> RLPStream& append(std::array<_T, S> const& _s)
  {
    appendList(_s.size());
    for (auto const& i: _s)
    {
      append(i);
    }
    return *this;
  }
  template <class _T> RLPStream& append(std::set<_T> const& _s)
  {
    appendList(_s.size());
    for (auto const& i: _s)
    {
      append(i);
    }
    return *this;
  }
  template <class _T> RLPStream& append(std::unordered_set<_T> const& _s)
  {
    appendList(_s.size());
    for (auto const& i: _s)
    {
      append(i);
    }
    return *this;
  }
  template <class T, class U> RLPStream& append(std::pair<T, U> const& _s)
  {
    appendList(2);
    append(_s.first);
    append(_s.second);
    return *this;
  }


  RLPStream& appendList(size_t _items);
  RLPStream& appendList(bytesConstRef _rlp);
  RLPStream& appendList(bytes const& _rlp)
  {
    return appendList(&_rlp);
  }
  RLPStream& appendList(RLPStream const& _s)
  {
    return appendList(&_s.out());
  }


  RLPStream& appendRaw(bytesConstRef _rlp, size_t _itemCount = 1);
  RLPStream& appendRaw(bytes const& _rlp, size_t _itemCount = 1)
  {
    return appendRaw(&_rlp, _itemCount);
  }


  template <class T> RLPStream& operator<<(T _data)
  {
    return append(_data);
  }


  void clear()
  {
    m_out.clear();
    m_listStack.clear();
  }


  bytes const& out() const
  {
    if(!m_listStack.empty())
    {
      BOOST_THROW_EXCEPTION(RLPException() << errinfo_comment("listStack is not empty"));
    }
    return m_out;
  }


  bytes&& invalidate()
  {
    if(!m_listStack.empty())
    {
      BOOST_THROW_EXCEPTION(RLPException() << errinfo_comment("listStack is not empty"));
    }
    return std::move(m_out);
  }


  void swapOut(bytes& _dest)
  {
    if(!m_listStack.empty())
    {
      BOOST_THROW_EXCEPTION(RLPException() << errinfo_comment("listStack is not empty"));
    }
    swap(m_out, _dest);
  }

private:
  void noteAppended(size_t _itemCount = 1);



  void pushCount(size_t _count, byte _offset);


  template <class _T> void pushInt(_T _i, size_t _br)
  {
    m_out.resize(m_out.size() + _br);
    byte* b = &m_out.back();
    for (; _i; _i >>= 8)
    {
      *(b--) = (byte)(_i & 0xff);
    }
  }


  bytes m_out;

  std::vector<std::pair<size_t, size_t>> m_listStack;
};

template <class _T> void rlpListAux(RLPStream& _out, _T _t)
{
  _out << _t;
}
template <class _T, class ... _Ts> void rlpListAux(RLPStream& _out, _T _t, _Ts ... _ts)
{
  rlpListAux(_out << _t, _ts...);
}


template <class _T> bytes rlp(_T _t)
{
  return (RLPStream() << _t).out();
}


inline bytes rlpList()
{
  return RLPStream(0).out();
}
template <class ... _Ts> bytes rlpList(_Ts ... _ts)
{
  RLPStream out(sizeof ...(_Ts));
  rlpListAux(out, _ts...);
  return out.out();
}


extern bytes RLPNull;


extern bytes RLPEmptyList;


std::ostream& operator<<(std::ostream& _out, dev::RLP const& _d);

}
