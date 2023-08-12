#ifndef BITCOIN_SERIALIZE_H
#define BITCOIN_SERIALIZE_H 

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include <limits>
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <iostream>

#include <boost/type_traits/is_fundamental.hpp>
#include <boost/tuple/tuple.hpp>

#include "allocators.h"
#include "version.h"

class CAutoFile;
class CDataStream;
class CScript;

static const unsigned int MAX_SIZE = 0x02000000;



template<typename T>
inline T& REF(const T& val)
{
  return const_cast<T&>(val);
}







enum
{

  SER_NETWORK = (1 << 0),
  SER_DISK = (1 << 1),
  SER_GETHASH = (1 << 2),


  SER_SKIPSIG = (1 << 16),
  SER_BLOCKHEADERONLY = (1 << 17),
};

#define IMPLEMENT_SERIALIZE(statements) \
    unsigned int GetSerializeSize(int nType, int nVersion) const \
    { \
        CSerActionGetSerializeSize ser_action; \
        const bool fGetSize = true; \
        const bool fWrite = false; \
        const bool fRead = false; \
        unsigned int nSerSize = 0; \
        ser_streamplaceholder s; \
        assert(fGetSize||fWrite||fRead); \
        s.nType = nType; \
        s.nVersion = nVersion; \
        {statements} \
        return nSerSize; \
    } \
    template<typename Stream> \
    void Serialize(Stream& s, int nType, int nVersion) const \
    { \
        CSerActionSerialize ser_action; \
        const bool fGetSize = false; \
        const bool fWrite = true; \
        const bool fRead = false; \
        unsigned int nSerSize = 0; \
        assert(fGetSize||fWrite||fRead); \
        {statements} \
    } \
    template<typename Stream> \
    void Unserialize(Stream& s, int nType, int nVersion) \
    { \
        CSerActionUnserialize ser_action; \
        const bool fGetSize = false; \
        const bool fWrite = false; \
        const bool fRead = true; \
        unsigned int nSerSize = 0; \
        assert(fGetSize||fWrite||fRead); \
        {statements} \
    }

#define READWRITE(obj) (nSerSize += ::SerReadWrite(s, (obj), nType, nVersion, ser_action))
#define WRITEDATA(s,obj) s.write((char*)&(obj), sizeof(obj))
#define READDATA(s,obj) s.read((char*)&(obj), sizeof(obj))

inline unsigned int GetSerializeSize(char a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(signed char a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(unsigned char a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(signed short a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(unsigned short a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(signed int a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(unsigned int a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(signed long a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(unsigned long a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(signed long long a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(unsigned long long a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(float a, int, int=0)
{
  return sizeof(a);
}
inline unsigned int GetSerializeSize(double a, int, int=0)
{
  return sizeof(a);
}

template<typename Stream> inline void Serialize(Stream& s, char a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, signed char a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, unsigned char a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, signed short a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, unsigned short a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, signed int a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, unsigned int a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, signed long a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, unsigned long a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, signed long long a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, unsigned long long a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, float a, int, int=0)
{
  WRITEDATA(s, a);
}
template<typename Stream> inline void Serialize(Stream& s, double a, int, int=0)
{
  WRITEDATA(s, a);
}

template<typename Stream> inline void Unserialize(Stream& s, char& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, signed char& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, unsigned char& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, signed short& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, unsigned short& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, signed int& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, unsigned int& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, signed long& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, unsigned long& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, signed long long& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, unsigned long long& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, float& a, int, int=0)
{
  READDATA(s, a);
}
template<typename Stream> inline void Unserialize(Stream& s, double& a, int, int=0)
{
  READDATA(s, a);
}

inline unsigned int GetSerializeSize(bool a, int, int=0)
{
  return sizeof(char);
}
template<typename Stream> inline void Serialize(Stream& s, bool a, int, int=0)
{
  char f=a;
  WRITEDATA(s, f);
}
template<typename Stream> inline void Unserialize(Stream& s, bool& a, int, int=0)
{
  char f;
  READDATA(s, f);
  a=f;
}
inline unsigned int GetSizeOfCompactSize(uint64_t nSize)
{
  if (nSize < 253)
  {
    return sizeof(unsigned char);
  }
  else if (nSize <= std::numeric_limits<unsigned short>::max())
  {
    return sizeof(unsigned char) + sizeof(unsigned short);
  }
  else if (nSize <= std::numeric_limits<unsigned int>::max())
  {
    return sizeof(unsigned char) + sizeof(unsigned int);
  }
  else
  {
    return sizeof(unsigned char) + sizeof(uint64_t);
  }
}

template<typename Stream>
void WriteCompactSize(Stream& os, uint64_t nSize)
{
  if (nSize < 253)
  {
    unsigned char chSize = nSize;
    WRITEDATA(os, chSize);
  }
  else if (nSize <= std::numeric_limits<unsigned short>::max())
  {
    unsigned char chSize = 253;
    unsigned short xSize = nSize;
    WRITEDATA(os, chSize);
    WRITEDATA(os, xSize);
  }
  else if (nSize <= std::numeric_limits<unsigned int>::max())
  {
    unsigned char chSize = 254;
    unsigned int xSize = nSize;
    WRITEDATA(os, chSize);
    WRITEDATA(os, xSize);
  }
  else
  {
    unsigned char chSize = 255;
    uint64_t xSize = nSize;
    WRITEDATA(os, chSize);
    WRITEDATA(os, xSize);
  }
  return;
}

template<typename Stream>
uint64_t ReadCompactSize(Stream& is)
{
  unsigned char chSize;
  READDATA(is, chSize);
  uint64_t nSizeRet = 0;
  if (chSize < 253)
  {
    nSizeRet = chSize;
  }
  else if (chSize == 253)
  {
    unsigned short xSize;
    READDATA(is, xSize);
    nSizeRet = xSize;
    if (nSizeRet < 253)
    {
      throw std::ios_base::failure("non-canonical ReadCompactSize()");
    }
  }
  else if (chSize == 254)
  {
    unsigned int xSize;
    READDATA(is, xSize);
    nSizeRet = xSize;
    if (nSizeRet < 0x10000u)
    {
      throw std::ios_base::failure("non-canonical ReadCompactSize()");
    }
  }
  else
  {
    uint64_t xSize;
    READDATA(is, xSize);
    nSizeRet = xSize;
    if (nSizeRet < 0x100000000LLu)
    {
      throw std::ios_base::failure("non-canonical ReadCompactSize()");
    }
  }
  if (nSizeRet > (uint64_t)MAX_SIZE)
  {
    throw std::ios_base::failure("ReadCompactSize() : size too large");
  }
  return nSizeRet;
}



#define FLATDATA(obj) REF(CFlatData((char*)&(obj), (char*)&(obj) + sizeof(obj)))



class CFlatData
{
protected:
  char* pbegin;
  char* pend;
public:
  CFlatData(void* pbeginIn, void* pendIn) : pbegin((char*)pbeginIn), pend((char*)pendIn) { }
  char* begin()
  {
    return pbegin;
  }
  const char* begin() const
  {
    return pbegin;
  }
  char* end()
  {
    return pend;
  }
  const char* end() const
  {
    return pend;
  }

  unsigned int GetSerializeSize(int, int=0) const
  {
    return pend - pbegin;
  }

  template<typename Stream>
  void Serialize(Stream& s, int, int=0) const
  {
    s.write(pbegin, pend - pbegin);
  }

  template<typename Stream>
  void Unserialize(Stream& s, int, int=0)
  {
    s.read(pbegin, pend - pbegin);
  }
};






template<typename C> unsigned int GetSerializeSize(const std::basic_string<C>& str, int, int=0);
template<typename Stream, typename C> void Serialize(Stream& os, const std::basic_string<C>& str, int, int=0);
template<typename Stream, typename C> void Unserialize(Stream& is, std::basic_string<C>& str, int, int=0);


template<typename T, typename A> unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&);
template<typename T, typename A> unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&);
template<typename T, typename A> inline unsigned int GetSerializeSize(const std::vector<T, A>& v, int nType, int nVersion);
template<typename Stream, typename T, typename A> void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&);
template<typename Stream, typename T, typename A> void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&);
template<typename Stream, typename T, typename A> inline void Serialize(Stream& os, const std::vector<T, A>& v, int nType, int nVersion);
template<typename Stream, typename T, typename A> void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&);
template<typename Stream, typename T, typename A> void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&);
template<typename Stream, typename T, typename A> inline void Unserialize(Stream& is, std::vector<T, A>& v, int nType, int nVersion);


extern inline unsigned int GetSerializeSize(const CScript& v, int nType, int nVersion);
template<typename Stream> void Serialize(Stream& os, const CScript& v, int nType, int nVersion);
template<typename Stream> void Unserialize(Stream& is, CScript& v, int nType, int nVersion);


template<typename K, typename T> unsigned int GetSerializeSize(const std::pair<K, T>& item, int nType, int nVersion);
template<typename Stream, typename K, typename T> void Serialize(Stream& os, const std::pair<K, T>& item, int nType, int nVersion);
template<typename Stream, typename K, typename T> void Unserialize(Stream& is, std::pair<K, T>& item, int nType, int nVersion);


template<typename T0, typename T1, typename T2> unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2>& item, int nType, int nVersion);
template<typename Stream, typename T0, typename T1, typename T2> void Serialize(Stream& os, const boost::tuple<T0, T1, T2>& item, int nType, int nVersion);
template<typename Stream, typename T0, typename T1, typename T2> void Unserialize(Stream& is, boost::tuple<T0, T1, T2>& item, int nType, int nVersion);


template<typename T0, typename T1, typename T2, typename T3> unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion);
template<typename Stream, typename T0, typename T1, typename T2, typename T3> void Serialize(Stream& os, const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion);
template<typename Stream, typename T0, typename T1, typename T2, typename T3> void Unserialize(Stream& is, boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion);


template<typename K, typename T, typename Pred, typename A> unsigned int GetSerializeSize(const std::map<K, T, Pred, A>& m, int nType, int nVersion);
template<typename Stream, typename K, typename T, typename Pred, typename A> void Serialize(Stream& os, const std::map<K, T, Pred, A>& m, int nType, int nVersion);
template<typename Stream, typename K, typename T, typename Pred, typename A> void Unserialize(Stream& is, std::map<K, T, Pred, A>& m, int nType, int nVersion);


template<typename K, typename Pred, typename A> unsigned int GetSerializeSize(const std::set<K, Pred, A>& m, int nType, int nVersion);
template<typename Stream, typename K, typename Pred, typename A> void Serialize(Stream& os, const std::set<K, Pred, A>& m, int nType, int nVersion);
template<typename Stream, typename K, typename Pred, typename A> void Unserialize(Stream& is, std::set<K, Pred, A>& m, int nType, int nVersion);
template<typename T>
inline unsigned int GetSerializeSize(const T& a)
{
  return a.GetSerializeSize();
}

template<typename Stream, typename T>
inline void Serialize(Stream& os, const T& a, long nType, int nVersion)
{

  a.Serialize(os, (int)nType, nVersion);
}

template<typename Stream, typename T>
inline void Unserialize(Stream& is, T& a, long nType, int nVersion)
{
  a.Unserialize(is, (int)nType, nVersion);
}
template<typename C>
unsigned int GetSerializeSize(const std::basic_string<C>& str, int, int)
{
  return GetSizeOfCompactSize(str.size()) + str.size() * sizeof(str[0]);
}

template<typename Stream, typename C>
void Serialize(Stream& os, const std::basic_string<C>& str, int, int)
{
  WriteCompactSize(os, str.size());
  if (!str.empty())
  {
    os.write((char*)&str[0], str.size() * sizeof(str[0]));
  }
}

template<typename Stream, typename C>
void Unserialize(Stream& is, std::basic_string<C>& str, int, int)
{
  unsigned int nSize = ReadCompactSize(is);
  str.resize(nSize);
  if (nSize != 0)
  {
    is.read((char*)&str[0], nSize * sizeof(str[0]));
  }
}

template<typename T, typename A>
unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&)
{
  return (GetSizeOfCompactSize(v.size()) + v.size() * sizeof(T));
}

template<typename T, typename A>
unsigned int GetSerializeSize_impl(const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&)
{
  unsigned int nSize = GetSizeOfCompactSize(v.size());
  for (typename std::vector<T, A>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
  {
    nSize += GetSerializeSize((*vi), nType, nVersion);
  }
  return nSize;
}

template<typename T, typename A>
inline unsigned int GetSerializeSize(const std::vector<T, A>& v, int nType, int nVersion)
{
  return GetSerializeSize_impl(v, nType, nVersion, boost::is_fundamental<T>());
}

template<typename T>
inline unsigned int GetSerializeSize(const T& a, long nType, int nVersion=1)
{
  return a.GetSerializeSize((int)nType, nVersion);
}


template<typename Stream, typename T, typename A>
void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&)
{
  WriteCompactSize(os, v.size());
  if (!v.empty())
  {
    os.write((char*)&v[0], v.size() * sizeof(T));
  }
}

template<typename Stream, typename T, typename A>
void Serialize_impl(Stream& os, const std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&)
{

  WriteCompactSize(os, v.size());
  for (typename std::vector<T, A>::const_iterator vi = v.begin(); vi != v.end(); ++vi)
  {
    ::Serialize(os, (*vi), nType, nVersion);
  }
}

template<typename Stream, typename T, typename A>
inline void Serialize(Stream& os, const std::vector<T, A>& v, int nType, int nVersion)
{
  Serialize_impl(os, v, nType, nVersion, boost::is_fundamental<T>());
}


template<typename Stream, typename T, typename A>
void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::true_type&)
{

  v.clear();
  unsigned int nSize = ReadCompactSize(is);
  unsigned int i = 0;
  while (i < nSize)
  {
    unsigned int blk = std::min(nSize - i, (unsigned int)(1 + 4999999 / sizeof(T)));
    v.resize(i + blk);
    is.read((char*)&v[i], blk * sizeof(T));
    i += blk;
  }
}

template<typename Stream, typename T, typename A>
void Unserialize_impl(Stream& is, std::vector<T, A>& v, int nType, int nVersion, const boost::false_type&)
{
  v.clear();
  unsigned int nSize = ReadCompactSize(is);
  unsigned int i = 0;
  unsigned int nMid = 0;
  while (nMid < nSize)
  {
    nMid += 5000000 / sizeof(T);
    if (nMid > nSize)
    {
      nMid = nSize;
    }
    v.resize(nMid);
    for (; i < nMid; i++)
    {
      Unserialize(is, v[i], nType, nVersion);
    }
  }
}

template<typename Stream, typename T, typename A>
inline void Unserialize(Stream& is, std::vector<T, A>& v, int nType, int nVersion)
{
  Unserialize_impl(is, v, nType, nVersion, boost::is_fundamental<T>());
}






inline unsigned int GetSerializeSize(const CScript& v, int nType, int nVersion)
{
  return GetSerializeSize((const std::vector<unsigned char>&)v, nType, nVersion);
}

template<typename Stream>
void Serialize(Stream& os, const CScript& v, int nType, int nVersion)
{
  Serialize(os, (const std::vector<unsigned char>&)v, nType, nVersion);
}

template<typename Stream>
void Unserialize(Stream& is, CScript& v, int nType, int nVersion)
{
  Unserialize(is, (std::vector<unsigned char>&)v, nType, nVersion);
}






template<typename K, typename T>
unsigned int GetSerializeSize(const std::pair<K, T>& item, int nType, int nVersion)
{
  return GetSerializeSize(item.first, nType, nVersion) + GetSerializeSize(item.second, nType, nVersion);
}

template<typename Stream, typename K, typename T>
void Serialize(Stream& os, const std::pair<K, T>& item, int nType, int nVersion)
{
  Serialize(os, item.first, nType, nVersion);
  Serialize(os, item.second, nType, nVersion);
}

template<typename Stream, typename K, typename T>
void Unserialize(Stream& is, std::pair<K, T>& item, int nType, int nVersion)
{
  Unserialize(is, item.first, nType, nVersion);
  Unserialize(is, item.second, nType, nVersion);
}






template<typename T0, typename T1, typename T2>
unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2>& item, int nType, int nVersion)
{
  unsigned int nSize = 0;
  nSize += GetSerializeSize(boost::get<0>(item), nType, nVersion);
  nSize += GetSerializeSize(boost::get<1>(item), nType, nVersion);
  nSize += GetSerializeSize(boost::get<2>(item), nType, nVersion);
  return nSize;
}

template<typename Stream, typename T0, typename T1, typename T2>
void Serialize(Stream& os, const boost::tuple<T0, T1, T2>& item, int nType, int nVersion)
{
  Serialize(os, boost::get<0>(item), nType, nVersion);
  Serialize(os, boost::get<1>(item), nType, nVersion);
  Serialize(os, boost::get<2>(item), nType, nVersion);
}

template<typename Stream, typename T0, typename T1, typename T2>
void Unserialize(Stream& is, boost::tuple<T0, T1, T2>& item, int nType, int nVersion)
{
  Unserialize(is, boost::get<0>(item), nType, nVersion);
  Unserialize(is, boost::get<1>(item), nType, nVersion);
  Unserialize(is, boost::get<2>(item), nType, nVersion);
}






template<typename T0, typename T1, typename T2, typename T3>
unsigned int GetSerializeSize(const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion)
{
  unsigned int nSize = 0;
  nSize += GetSerializeSize(boost::get<0>(item), nType, nVersion);
  nSize += GetSerializeSize(boost::get<1>(item), nType, nVersion);
  nSize += GetSerializeSize(boost::get<2>(item), nType, nVersion);
  nSize += GetSerializeSize(boost::get<3>(item), nType, nVersion);
  return nSize;
}

template<typename Stream, typename T0, typename T1, typename T2, typename T3>
void Serialize(Stream& os, const boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion)
{
  Serialize(os, boost::get<0>(item), nType, nVersion);
  Serialize(os, boost::get<1>(item), nType, nVersion);
  Serialize(os, boost::get<2>(item), nType, nVersion);
  Serialize(os, boost::get<3>(item), nType, nVersion);
}

template<typename Stream, typename T0, typename T1, typename T2, typename T3>
void Unserialize(Stream& is, boost::tuple<T0, T1, T2, T3>& item, int nType, int nVersion)
{
  Unserialize(is, boost::get<0>(item), nType, nVersion);
  Unserialize(is, boost::get<1>(item), nType, nVersion);
  Unserialize(is, boost::get<2>(item), nType, nVersion);
  Unserialize(is, boost::get<3>(item), nType, nVersion);
}






template<typename K, typename T, typename Pred, typename A>
unsigned int GetSerializeSize(const std::map<K, T, Pred, A>& m, int nType, int nVersion)
{
  unsigned int nSize = GetSizeOfCompactSize(m.size());
  for (typename std::map<K, T, Pred, A>::const_iterator mi = m.begin(); mi != m.end(); ++mi)
  {
    nSize += GetSerializeSize((*mi), nType, nVersion);
  }
  return nSize;
}

template<typename Stream, typename K, typename T, typename Pred, typename A>
void Serialize(Stream& os, const std::map<K, T, Pred, A>& m, int nType, int nVersion)
{
  WriteCompactSize(os, m.size());
  for (typename std::map<K, T, Pred, A>::const_iterator mi = m.begin(); mi != m.end(); ++mi)
  {
    Serialize(os, (*mi), nType, nVersion);
  }
}

template<typename Stream, typename K, typename T, typename Pred, typename A>
void Unserialize(Stream& is, std::map<K, T, Pred, A>& m, int nType, int nVersion)
{
  m.clear();
  unsigned int nSize = ReadCompactSize(is);
  typename std::map<K, T, Pred, A>::iterator mi = m.begin();
  for (unsigned int i = 0; i < nSize; i++)
  {
    std::pair<K, T> item;
    Unserialize(is, item, nType, nVersion);
    mi = m.insert(mi, item);
  }
}






template<typename K, typename Pred, typename A>
unsigned int GetSerializeSize(const std::set<K, Pred, A>& m, int nType, int nVersion)
{
  unsigned int nSize = GetSizeOfCompactSize(m.size());
  for (typename std::set<K, Pred, A>::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    nSize += GetSerializeSize((*it), nType, nVersion);
  }
  return nSize;
}

template<typename Stream, typename K, typename Pred, typename A>
void Serialize(Stream& os, const std::set<K, Pred, A>& m, int nType, int nVersion)
{
  WriteCompactSize(os, m.size());
  for (typename std::set<K, Pred, A>::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    Serialize(os, (*it), nType, nVersion);
  }
}

template<typename Stream, typename K, typename Pred, typename A>
void Unserialize(Stream& is, std::set<K, Pred, A>& m, int nType, int nVersion)
{
  m.clear();
  unsigned int nSize = ReadCompactSize(is);
  typename std::set<K, Pred, A>::iterator it = m.begin();
  for (unsigned int i = 0; i < nSize; i++)
  {
    K key;
    Unserialize(is, key, nType, nVersion);
    it = m.insert(it, key);
  }
}






class CSerActionGetSerializeSize { };
class CSerActionSerialize { };
class CSerActionUnserialize { };

template<typename Stream, typename T>
inline unsigned int SerReadWrite(Stream& s, const T& obj, int nType, int nVersion, CSerActionGetSerializeSize ser_action)
{
  return ::GetSerializeSize(obj, nType, nVersion);
}

template<typename Stream, typename T>
inline unsigned int SerReadWrite(Stream& s, const T& obj, int nType, int nVersion, CSerActionSerialize ser_action)
{

  ::Serialize(s, obj, nType, nVersion);
  return 0;
}

template<typename Stream, typename T>
inline unsigned int SerReadWrite(Stream& s, T& obj, int nType, int nVersion, CSerActionUnserialize ser_action)
{
  ::Unserialize(s, obj, nType, nVersion);
  return 0;
}

struct ser_streamplaceholder
{
  int nType;
  int nVersion;
};
typedef std::vector<char, zero_after_free_allocator<char> > CSerializeData;






class CDataStream
{
protected:
  typedef CSerializeData vector_type;
  vector_type vch;
  unsigned int nReadPos;
  short state;
  short exceptmask;
public:
  int nType;
  int nVersion;

  typedef vector_type::allocator_type allocator_type;
  typedef vector_type::size_type size_type;
  typedef vector_type::difference_type difference_type;
  typedef vector_type::reference reference;
  typedef vector_type::const_reference const_reference;
  typedef vector_type::value_type value_type;
  typedef vector_type::iterator iterator;
  typedef vector_type::const_iterator const_iterator;
  typedef vector_type::reverse_iterator reverse_iterator;

  explicit CDataStream(int nTypeIn, int nVersionIn)
  {
    Init(nTypeIn, nVersionIn);
  }

  CDataStream(const_iterator pbegin, const_iterator pend, int nTypeIn, int nVersionIn) : vch(pbegin, pend)
  {
    Init(nTypeIn, nVersionIn);
  }

#if !defined(_MSC_VER) || _MSC_VER >= 1300
  CDataStream(const char* pbegin, const char* pend, int nTypeIn, int nVersionIn) : vch(pbegin, pend)
  {
    Init(nTypeIn, nVersionIn);
  }
#endif

  CDataStream(const vector_type& vchIn, int nTypeIn, int nVersionIn) : vch(vchIn.begin(), vchIn.end())
  {
    Init(nTypeIn, nVersionIn);
  }

  CDataStream(const std::vector<char>& vchIn, int nTypeIn, int nVersionIn) : vch(vchIn.begin(), vchIn.end())
  {
    Init(nTypeIn, nVersionIn);
  }

  CDataStream(const std::vector<unsigned char>& vchIn, int nTypeIn, int nVersionIn) : vch((char*)&vchIn.begin()[0], (char*)&vchIn.end()[0])
  {
    Init(nTypeIn, nVersionIn);
  }

  void Init(int nTypeIn, int nVersionIn)
  {
    nReadPos = 0;
    nType = nTypeIn;
    nVersion = nVersionIn;
    state = 0;
    exceptmask = std::ios::badbit | std::ios::failbit;
  }

  CDataStream& operator+=(const CDataStream& b)
  {
    vch.insert(vch.end(), b.begin(), b.end());
    return *this;
  }

  friend CDataStream operator+(const CDataStream& a, const CDataStream& b)
  {
    CDataStream ret = a;
    ret += b;
    return (ret);
  }

  std::string str() const
  {
    return (std::string(begin(), end()));
  }





  const_iterator begin() const
  {
    return vch.begin() + nReadPos;
  }
  iterator begin()
  {
    return vch.begin() + nReadPos;
  }
  const_iterator end() const
  {
    return vch.end();
  }
  iterator end()
  {
    return vch.end();
  }
  size_type size() const
  {
    return vch.size() - nReadPos;
  }
  bool empty() const
  {
    return vch.size() == nReadPos;
  }
  void resize(size_type n, value_type c=0)
  {
    vch.resize(n + nReadPos, c);
  }
  void reserve(size_type n)
  {
    vch.reserve(n + nReadPos);
  }
  const_reference operator[](size_type pos) const
  {
    return vch[pos + nReadPos];
  }
  reference operator[](size_type pos)
  {
    return vch[pos + nReadPos];
  }
  void clear()
  {
    vch.clear();
    nReadPos = 0;
  }
  iterator insert(iterator it, const char& x=char())
  {
    return vch.insert(it, x);
  }
  void insert(iterator it, size_type n, const char& x)
  {
    vch.insert(it, n, x);
  }

  void insert(iterator it, std::vector<char>::const_iterator first, std::vector<char>::const_iterator last)
  {
    assert(last - first >= 0);
    if (it == vch.begin() + nReadPos && (unsigned int)(last - first) <= nReadPos)
    {

      nReadPos -= (last - first);
      memcpy(&vch[nReadPos], &first[0], last - first);
    }
    else
    {
      vch.insert(it, first, last);
    }
  }

#if !defined(_MSC_VER) || _MSC_VER >= 1300
  void insert(iterator it, const char* first, const char* last)
  {
    assert(last - first >= 0);
    if (it == vch.begin() + nReadPos && (unsigned int)(last - first) <= nReadPos)
    {

      nReadPos -= (last - first);
      memcpy(&vch[nReadPos], &first[0], last - first);
    }
    else
    {
      vch.insert(it, first, last);
    }
  }
#endif

  iterator erase(iterator it)
  {
    if (it == vch.begin() + nReadPos)
    {

      if (++nReadPos >= vch.size())
      {

        nReadPos = 0;
        return vch.erase(vch.begin(), vch.end());
      }
      return vch.begin() + nReadPos;
    }
    else
    {
      return vch.erase(it);
    }
  }

  iterator erase(iterator first, iterator last)
  {
    if (first == vch.begin() + nReadPos)
    {

      if (last == vch.end())
      {
        nReadPos = 0;
        return vch.erase(vch.begin(), vch.end());
      }
      else
      {
        nReadPos = (last - vch.begin());
        return last;
      }
    }
    else
    {
      return vch.erase(first, last);
    }
  }

  inline void Compact()
  {
    vch.erase(vch.begin(), vch.begin() + nReadPos);
    nReadPos = 0;
  }

  bool Rewind(size_type n)
  {

    if (n > nReadPos)
    {
      return false;
    }
    nReadPos -= n;
    return true;
  }





  void setstate(short bits, const char* psz)
  {
    state |= bits;
    if (state & exceptmask)
    {
      throw std::ios_base::failure(psz);
    }
  }

  bool eof() const
  {
    return size() == 0;
  }
  bool fail() const
  {
    return state & (std::ios::badbit | std::ios::failbit);
  }
  bool good() const
  {
    return !eof() && (state == 0);
  }
  void clear(short n)
  {
    state = n;
  }
  short exceptions()
  {
    return exceptmask;
  }
  short exceptions(short mask)
  {
    short prev = exceptmask;
    exceptmask = mask;
    setstate(0, "CDataStream");
    return prev;
  }
  CDataStream* rdbuf()
  {
    return this;
  }
  int in_avail()
  {
    return size();
  }

  void SetType(int n)
  {
    nType = n;
  }
  int GetType()
  {
    return nType;
  }
  void SetVersion(int n)
  {
    nVersion = n;
  }
  int GetVersion()
  {
    return nVersion;
  }
  void ReadVersion()
  {
    *this >> nVersion;
  }
  void WriteVersion()
  {
    *this << nVersion;
  }

  CDataStream& read(char* pch, int nSize)
  {

    assert(nSize >= 0);
    unsigned int nReadPosNext = nReadPos + nSize;
    if (nReadPosNext >= vch.size())
    {
      if (nReadPosNext > vch.size())
      {
        setstate(std::ios::failbit, "CDataStream::read() : end of data");
        memset(pch, 0, nSize);
        nSize = vch.size() - nReadPos;
      }
      memcpy(pch, &vch[nReadPos], nSize);
      nReadPos = 0;
      vch.clear();
      return (*this);
    }
    memcpy(pch, &vch[nReadPos], nSize);
    nReadPos = nReadPosNext;
    return (*this);
  }

  CDataStream& ignore(int nSize)
  {

    assert(nSize >= 0);
    unsigned int nReadPosNext = nReadPos + nSize;
    if (nReadPosNext >= vch.size())
    {
      if (nReadPosNext > vch.size())
      {
        setstate(std::ios::failbit, "CDataStream::ignore() : end of data");
      }
      nReadPos = 0;
      vch.clear();
      return (*this);
    }
    nReadPos = nReadPosNext;
    return (*this);
  }

  CDataStream& write(const char* pch, int nSize)
  {

    assert(nSize >= 0);
    vch.insert(vch.end(), pch, pch + nSize);
    return (*this);
  }

  template<typename Stream>
  void Serialize(Stream& s, int nType, int nVersion) const
  {

    if (!vch.empty())
    {
      s.write((char*)&vch[0], vch.size() * sizeof(vch[0]));
    }
  }

  template<typename T>
  unsigned int GetSerializeSize(const T& obj)
  {

    return ::GetSerializeSize(obj, nType, nVersion);
  }

  template<typename T>
  CDataStream& operator<<(const T& obj)
  {

    ::Serialize(*this, obj, nType, nVersion);
    return (*this);
  }

  template<typename T>
  CDataStream& operator>>(T& obj)
  {

    ::Unserialize(*this, obj, nType, nVersion);
    return (*this);
  }

  void GetAndClear(CSerializeData &data)
  {
    data.insert(data.end(), begin(), end());
    clear();
  }
};
class CAutoFile
{
protected:
  FILE* file;
  short state;
  short exceptmask;
public:
  int nType;
  int nVersion;

  CAutoFile(FILE* filenew, int nTypeIn, int nVersionIn)
  {
    file = filenew;
    nType = nTypeIn;
    nVersion = nVersionIn;
    state = 0;
    exceptmask = std::ios::badbit | std::ios::failbit;
  }

  ~CAutoFile()
  {
    fclose();
  }

  void fclose()
  {
    if (file != NULL && file != stdin && file != stdout && file != stderr)
    {
      ::fclose(file);
    }
    file = NULL;
  }

  FILE* release()
  {
    FILE* ret = file;
    file = NULL;
    return ret;
  }
  operator FILE*()
  {
    return file;
  }
  FILE* operator->()
  {
    return file;
  }
  FILE& operator*()
  {
    return *file;
  }
  FILE** operator&()
  {
    return &file;
  }
  FILE* operator=(FILE* pnew)
  {
    return file = pnew;
  }
  bool operator!()
  {
    return (file == NULL);
  }





  void setstate(short bits, const char* psz)
  {
    state |= bits;
    if (state & exceptmask)
    {
      throw std::ios_base::failure(psz);
    }
  }

  bool fail() const
  {
    return state & (std::ios::badbit | std::ios::failbit);
  }
  bool good() const
  {
    return state == 0;
  }
  void clear(short n = 0)
  {
    state = n;
  }
  short exceptions()
  {
    return exceptmask;
  }
  short exceptions(short mask)
  {
    short prev = exceptmask;
    exceptmask = mask;
    setstate(0, "CAutoFile");
    return prev;
  }

  void SetType(int n)
  {
    nType = n;
  }
  int GetType()
  {
    return nType;
  }
  void SetVersion(int n)
  {
    nVersion = n;
  }
  int GetVersion()
  {
    return nVersion;
  }
  void ReadVersion()
  {
    *this >> nVersion;
  }
  void WriteVersion()
  {
    *this << nVersion;
  }

  CAutoFile& read(char* pch, size_t nSize)
  {
    if (!file)
    {
      throw std::ios_base::failure("CAutoFile::read : file handle is NULL");
    }
    if (fread(pch, 1, nSize, file) != nSize)
    {
      setstate(std::ios::failbit, feof(file) ? "CAutoFile::read : end of file" : "CAutoFile::read : fread failed");
    }
    return (*this);
  }

  CAutoFile& write(const char* pch, size_t nSize)
  {
    if (!file)
    {
      throw std::ios_base::failure("CAutoFile::write : file handle is NULL");
    }
    if (fwrite(pch, 1, nSize, file) != nSize)
    {
      setstate(std::ios::failbit, "CAutoFile::write : write failed");
    }
    return (*this);
  }

  template<typename T>
  unsigned int GetSerializeSize(const T& obj)
  {

    return ::GetSerializeSize(obj, nType, nVersion);
  }

  template<typename T>
  CAutoFile& operator<<(const T& obj)
  {

    if (!file)
    {
      throw std::ios_base::failure("CAutoFile::operator<< : file handle is NULL");
    }
    ::Serialize(*this, obj, nType, nVersion);
    return (*this);
  }

  template<typename T>
  CAutoFile& operator>>(T& obj)
  {

    if (!file)
    {
      throw std::ios_base::failure("CAutoFile::operator>> : file handle is NULL");
    }
    ::Unserialize(*this, obj, nType, nVersion);
    return (*this);
  }
};

#endif
