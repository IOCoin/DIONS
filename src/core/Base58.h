#pragma once

#include <string>
#include <vector>
#include "BigNum.h"
#include "crypto/Key.h"
#include "Script.h"
#include "ccoin/Process.h"
#include "dions/Dions.h"
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

inline unsigned char GetAddressVersion()
{
  return((unsigned char)(fTestNet ? 111 : 103));
}

inline std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
  CAutoBN_CTX pctx;
  CBigNum bn58 ;
  std::ostringstream os;
  os << 58;
  BN_dec2bn(&bn58.bn_, os.str().c_str());

  CBigNum bn0;
  BN_zero(bn0.bn_);



  std::vector<unsigned char> vchTmp(pend-pbegin+1, 0);
  reverse_copy(pbegin, pend, vchTmp.begin());


  CBigNum bn;
  bn.setvch(vchTmp);


  std::string str;


  str.reserve((pend - pbegin) * 500 / 100 + 1);
  CBigNum dv;
  CBigNum rem;
  while (bn > bn0)
  {
    if (!BN_div(dv.bn_, rem.bn_, bn.bn_, bn58.bn_, pctx))
    {
      throw bignum_error("EncodeBase58 : BN_div failed");
    }
    bn = dv;
    unsigned int c = rem.getulong();
    str += pszBase58[c];
  }


  for (const unsigned char* p = pbegin; p < pend && *p == 0; p++)
  {
    str += pszBase58[0];
  }


  reverse(str.begin(), str.end());
  return str;
}


inline std::string EncodeBase58(const std::vector<unsigned char>& vch)
{
  return EncodeBase58(&vch[0], &vch[0] + vch.size());
}



inline bool DecodeBase58(const char* psz, std::vector<unsigned char>& vchRet)
{
  CAutoBN_CTX pctx;
  vchRet.clear();
  CBigNum bn58 = 58;
  CBigNum bn = 0;
  CBigNum bnChar;
  while (isspace(*psz))
  {
    psz++;
  }


  for (const char* p = psz; *p; p++)
  {
    const char* p1 = strchr(pszBase58, *p);
    if (p1 == NULL)
    {
      while (isspace(*p))
      {
        p++;
      }
      if (*p != '\0')
      {
        return false;
      }
      break;
    }
    bnChar.setulong(p1 - pszBase58);
    if (!BN_mul(bn.bn_, bn.bn_, bn58.bn_, pctx))
    {
      throw bignum_error("DecodeBase58 : BN_mul failed");
    }
    bn += bnChar;
  }


  std::vector<unsigned char> vchTmp = bn.getvch();


  if (vchTmp.size() >= 2 && vchTmp.end()[-1] == 0 && vchTmp.end()[-2] >= 0x80)
  {
    vchTmp.erase(vchTmp.end()-1);
  }


  int nLeadingZeros = 0;
  for (const char* p = psz; *p == pszBase58[0]; p++)
  {
    nLeadingZeros++;
  }
  vchRet.assign(nLeadingZeros + vchTmp.size(), 0);


  reverse_copy(vchTmp.begin(), vchTmp.end(), vchRet.end() - vchTmp.size());
  return true;
}



inline bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet)
{
  return DecodeBase58(str.c_str(), vchRet);
}





inline std::string EncodeBase58Check(const std::vector<unsigned char>& vchIn)
{

  std::vector<unsigned char> vch(vchIn);
  uint256 hash = Hash(vch.begin(), vch.end());
  vch.insert(vch.end(), (unsigned char*)&hash, (unsigned char*)&hash + 4);
  return EncodeBase58(vch);
}



inline bool DecodeBase58Check(const char* psz, std::vector<unsigned char>& vchRet)
{
  if (!DecodeBase58(psz, vchRet))
  {
    return false;
  }
  if (vchRet.size() < 4)
  {
    vchRet.clear();
    return false;
  }
  uint256 hash = Hash(vchRet.begin(), vchRet.end()-4);
  if (memcmp(&hash, &vchRet.end()[-4], 4) != 0)
  {
    vchRet.clear();
    return false;
  }
  vchRet.resize(vchRet.size()-4);
  return true;
}



inline bool DecodeBase58Check(const std::string& str, std::vector<unsigned char>& vchRet)
{
  return DecodeBase58Check(str.c_str(), vchRet);
}






class CBase58Data
{
protected:

  unsigned char nVersion;


  std::vector<unsigned char> vchData;

  CBase58Data()
  {
    nVersion = 0;
    vchData.clear();
  }

  ~CBase58Data()
  {

    if (!vchData.empty())
    {
      memset(&vchData[0], 0, vchData.size());
    }
  }

  void SetData(int nVersionIn, const void* pdata, size_t nSize)
  {
    nVersion = nVersionIn;
    vchData.resize(nSize);
    if (!vchData.empty())
    {
      memcpy(&vchData[0], pdata, nSize);
    }
  }

  void SetData(int nVersionIn, const unsigned char *pbegin, const unsigned char *pend)
  {
    SetData(nVersionIn, (void*)pbegin, pend - pbegin);
  }

public:
  bool SetString(const char* psz)
  {
    std::vector<unsigned char> vchTemp;
    DecodeBase58Check(psz, vchTemp);
    if (vchTemp.empty())
    {
      vchData.clear();
      nVersion = 0;
      return false;
    }
    nVersion = vchTemp[0];
    vchData.resize(vchTemp.size() - 1);
    if (!vchData.empty())
    {
      memcpy(&vchData[0], &vchTemp[1], vchData.size());
    }
    memset(&vchTemp[0], 0, vchTemp.size());
    return true;
  }

  bool SetString(const std::string& str)
  {
    return SetString(str.c_str());
  }

  std::string ToString() const
  {
    std::vector<unsigned char> vch(1, nVersion);
    vch.insert(vch.end(), vchData.begin(), vchData.end());
    return EncodeBase58Check(vch);
  }

  int CompareTo(const CBase58Data& b58) const
  {
    if (nVersion < b58.nVersion)
    {
      return -1;
    }
    if (nVersion > b58.nVersion)
    {
      return 1;
    }
    if (vchData < b58.vchData)
    {
      return -1;
    }
    if (vchData > b58.vchData)
    {
      return 1;
    }
    return 0;
  }

  bool operator==(const CBase58Data& b58) const
  {
    return CompareTo(b58) == 0;
  }
  bool operator<=(const CBase58Data& b58) const
  {
    return CompareTo(b58) <= 0;
  }
  bool operator>=(const CBase58Data& b58) const
  {
    return CompareTo(b58) >= 0;
  }
  bool operator< (const CBase58Data& b58) const
  {
    return CompareTo(b58) < 0;
  }
  bool operator> (const CBase58Data& b58) const
  {
    return CompareTo(b58) > 0;
  }
};







class cba;
class cbaVisitor : public boost::static_visitor<bool>
{
private:
  cba *addr;
public:
  cbaVisitor(cba *addrIn) : addr(addrIn) { }
  bool operator()(const CKeyID &id) const;
  bool operator()(const CScriptID &id) const;
  bool operator()(const CNoDestination &no) const;
};

class cba : public CBase58Data
{
public:
  enum
  {

    PUBKEY_ADDRESS = 103,
    SCRIPT_ADDRESS = 85,
    PUBKEY_ADDRESS_TEST = 111,
    SCRIPT_ADDRESS_TEST = 196,
  };

  bool Set(const CKeyID &id)
  {
    SetData(fTestNet ? PUBKEY_ADDRESS_TEST : PUBKEY_ADDRESS, &id, 20);
    return true;
  }

  bool Set(const CScriptID &id)
  {
    SetData(fTestNet ? SCRIPT_ADDRESS_TEST : SCRIPT_ADDRESS, &id, 20);
    return true;
  }

  bool Set(const CTxDestination &dest)
  {
    return boost::apply_visitor(cbaVisitor(this), dest);
  }

  bool IsValid() const
  {
    unsigned int nExpectedSize = 20;
    bool fExpectTestNet = false;
    switch(nVersion)
    {
    case PUBKEY_ADDRESS:
      nExpectedSize = 20;
      fExpectTestNet = false;
      break;
    case SCRIPT_ADDRESS:
      nExpectedSize = 20;
      fExpectTestNet = false;
      break;

    case PUBKEY_ADDRESS_TEST:
      nExpectedSize = 20;
      fExpectTestNet = true;
      break;
    case SCRIPT_ADDRESS_TEST:
      nExpectedSize = 20;
      fExpectTestNet = true;
      break;

    default:
      return false;
    }
    return fExpectTestNet == fTestNet && vchData.size() == nExpectedSize;
  }

  cba()
  {
  }

  cba(const CTxDestination &dest)
  {
    Set(dest);
  }

  cba(const std::string& strAddress)
  {
    SetString(strAddress);
  }

  cba(const char* pszAddress)
  {
    SetString(pszAddress);
  }

  CTxDestination Get() const
  {
    if (!IsValid())
    {
      return CNoDestination();
    }
    switch (nVersion)
    {
    case PUBKEY_ADDRESS:
    case PUBKEY_ADDRESS_TEST:
    {
      uint160 id;
      memcpy(&id, &vchData[0], 20);
      return CKeyID(id);
    }
    case SCRIPT_ADDRESS:
    case SCRIPT_ADDRESS_TEST:
    {
      uint160 id;
      memcpy(&id, &vchData[0], 20);
      return CScriptID(id);
    }
    }
    return CNoDestination();
  }

  bool GetKeyID(CKeyID &keyID) const
  {
    if (!IsValid())
    {
      return false;
    }
    switch (nVersion)
    {
    case PUBKEY_ADDRESS:
    case PUBKEY_ADDRESS_TEST:
    {
      uint160 id;
      memcpy(&id, &vchData[0], 20);
      keyID = CKeyID(id);
      return true;
    }
    default:
      return false;
    }
  }

  bool IsScript() const
  {
    if (!IsValid())
    {
      return false;
    }
    switch (nVersion)
    {
    case SCRIPT_ADDRESS:
    case SCRIPT_ADDRESS_TEST:
    {
      return true;
    }
    default:
      return false;
    }
  }
};

bool inline cbaVisitor::operator()(const CKeyID &id) const
{
  return addr->Set(id);
}
bool inline cbaVisitor::operator()(const CScriptID &id) const
{
  return addr->Set(id);
}
bool inline cbaVisitor::operator()(const CNoDestination &id) const
{
  return false;
}


class CBitcoinSecret : public CBase58Data
{
public:
  void SetSecret(const CSecret& vchSecret, bool fCompressed)
  {
    assert(vchSecret.size() == 32);
    SetData(128 + (fTestNet ? cba::PUBKEY_ADDRESS_TEST : cba::PUBKEY_ADDRESS), &vchSecret[0], vchSecret.size());
    if (fCompressed)
    {
      vchData.push_back(1);
    }
  }

  CSecret GetSecret(bool &fCompressedOut)
  {
    CSecret vchSecret;
    vchSecret.resize(32);
    memcpy(&vchSecret[0], &vchData[0], 32);
    fCompressedOut = vchData.size() == 33;
    return vchSecret;
  }

  bool IsValid() const
  {
    bool fExpectTestNet = false;
    switch(nVersion)
    {
    case (128 + cba::PUBKEY_ADDRESS):
      break;

    case (128 + cba::PUBKEY_ADDRESS_TEST):
      fExpectTestNet = true;
      break;

    default:
      return false;
    }
    return fExpectTestNet == fTestNet && (vchData.size() == 32 || (vchData.size() == 33 && vchData[32] == 1));
  }

  bool SetString(const char* pszSecret)
  {
    return CBase58Data::SetString(pszSecret) && IsValid();
  }

  bool SetString(const std::string& strSecret)
  {
    return SetString(strSecret.c_str());
  }

  CBitcoinSecret(const CSecret& vchSecret, bool fCompressed)
  {
    SetSecret(vchSecret, fCompressed);
  }

  CBitcoinSecret()
  {
  }
};

inline bool AddressToHash160(const char* psz, uint160& hash160Ret)
{
  std::vector<unsigned char> vch;
  if (!DecodeBase58Check(psz, vch))
  {
    return false;
  }
  if (vch.empty())
  {
    return false;
  }
  unsigned char nVersion = vch[0];
  if (vch.size() != sizeof(hash160Ret) + 1)
  {
    return false;
  }
  memcpy(&hash160Ret, &vch[1], sizeof(hash160Ret));
  return (nVersion == GetAddressVersion());
}

inline bool AddressToHash160(const std::string& str, uint160& hash160Ret)
{
  return AddressToHash160(str.c_str(), hash160Ret);
}

inline std::string Hash160ToAddress(uint160 hash160)
{

  std::vector<unsigned char> vch(1, GetAddressVersion());
  vch.insert(vch.end(), UBEGIN(hash160), UEND(hash160));
  return EncodeBase58Check(vch);
}
