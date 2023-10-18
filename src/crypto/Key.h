#ifndef BITCOIN_KEY_H
#define BITCOIN_KEY_H 

#include <stdexcept>
#include <vector>

#include "Allocators.h"
#include "Serialize.h"
#include "core/uint256.h"
#include "core/Util.h"
#include "RayShade.h"

#include <openssl/ec.h>
class key_error : public std::runtime_error
{
public:
  explicit key_error(const std::string& str) : std::runtime_error(str) {}
};


class CKeyID : public uint160
{
public:
  CKeyID() : uint160(0) { }
  CKeyID(const uint160 &in) : uint160(in) { }
};
typedef vector<unsigned char> __im__;

class CScriptID : public uint160
{
public:
  CScriptID() : uint160(0) { }
  CScriptID(const uint160 &in) : uint160(in) { }
};


class CPubKey
{
private:
  std::vector<unsigned char> vchPubKey;
  friend class CKey;

public:
  uint160 sync_;
  CPubKey() { }
  CPubKey(const std::vector<unsigned char> &vchPubKeyIn) : vchPubKey(vchPubKeyIn) { }
  friend bool operator==(const CPubKey &a, const CPubKey &b)
  {
    return a.vchPubKey == b.vchPubKey;
  }
  friend bool operator!=(const CPubKey &a, const CPubKey &b)
  {
    return a.vchPubKey != b.vchPubKey;
  }
  friend bool operator<(const CPubKey &a, const CPubKey &b)
  {
    return a.vchPubKey < b.vchPubKey;
  }

  IMPLEMENT_SERIALIZE(
    READWRITE(vchPubKey);
  )

  CKeyID GetID() const
  {
    return CKeyID(Hash160(vchPubKey));
  }

  uint256 GetHash() const
  {
    return Hash(vchPubKey.begin(), vchPubKey.end());
  }

  bool IsValid() const
  {
    return vchPubKey.size() == 33 || vchPubKey.size() == 65;
  }

  bool IsCompressed() const
  {
    return vchPubKey.size() == 33;
  }

  std::vector<unsigned char> Raw() const
  {
    return vchPubKey;
  }
};




typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CSecret;

int reflection(__pq__&);
int invert(__inv__&);

int __synth_piv__conv71__intern(__im__&,__im__&,__im__&,__im__&);
int __synth_piv__conv71__outer(__im__&,__im__&,__im__&,__im__&);
int __synth_piv__conv77(__im__&,__im__&,__im__&);

class CKey
{

public:

  void Reset();

  CKey();
  CKey(const CKey& b);

  CKey& operator=(const CKey& b);

  ~CKey();

  bool IsNull() const;
  bool IsCompressed() const;

  void MakeNewKey(bool fCompressed);
  bool SetPrivKey(const CPrivKey& vchPrivKey);
  bool SetSecret(const CSecret& vchSecret, bool fCompressed = false);
  CSecret GetSecret(bool &fCompressed) const;
  CPrivKey GetPrivKey() const;
  bool SetPubKey(const CPubKey& vchPubKey);
  CPubKey GetPubKey() const;

  bool Sign(uint256 hash, std::vector<unsigned char>& vchSig);





  bool SignCompact(uint256 hash, std::vector<unsigned char>& vchSig);





  bool SetCompactSignature(uint256 hash, const std::vector<unsigned char>& vchSig);

  bool Verify(uint256 hash, const std::vector<unsigned char>& vchSig);

  bool IsValid();


  static bool CheckSignatureElement(const unsigned char *vch, int len, bool half);
protected:
  EC_KEY* pkey;
  bool fSet;
  bool fCompressedPubKey;

  void SetCompressedPubKey();

};


bool ECC_InitSanityCheck(void);

#endif
