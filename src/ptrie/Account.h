#include <ptrie/Common.h>
#include <ptrie/Address.h>
#include <ptrie/SHA3.h>
#include <ptrie/TrieCommon.h>

#include <boost/filesystem/path.hpp>

namespace dev
{
class OverlayDB;

namespace dvm
{
# 40 "Account.h"
class Account
{
public:

  enum Changedness
  {

    Changed,

    Unchanged
  };


  Account() {}



  Account(u256 _nonce, u256 _balance, Changedness _c = Changed): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance) {}


  Account(u256 const& _nonce, u256 const& _balance, h256 const& _contractRoot,
          h256 const& _codeHash, u256 const& _version, Changedness _c)
    : m_isAlive(true),
      m_isUnchanged(_c == Unchanged),
      m_nonce(_nonce),
      m_balance(_balance),
      m_storageRoot(_contractRoot),
      m_codeHash(_codeHash),
      m_version(_version)
  {
    assert(_contractRoot);
  }




  void kill()
  {
    m_isAlive = false;
    m_storageOverlay.clear();
    m_storageOriginal.clear();
    m_codeHash = EmptySHA3;
    m_storageRoot = EmptyTrie;
    m_balance = 0;
    m_nonce = 0;
    m_version = 0;
    changed();
  }




  bool isAlive() const
  {
    return m_isAlive;
  }


  bool isDirty() const
  {
    return !m_isUnchanged;
  }

  void untouch()
  {
    m_isUnchanged = true;
  }



  bool isEmpty() const
  {
    return nonce() == 0 && balance() == 0 && codeHash() == EmptySHA3;
  }


  u256 const& balance() const
  {
    return m_balance;
  }


  void addBalance(u256 _value)
  {
    m_balance += _value;
    changed();
  }


  u256 nonce() const
  {
    return m_nonce;
  }


  void incNonce()
  {
    ++m_nonce;
    changed();
  }



  void setNonce(u256 const& _nonce)
  {
    m_nonce = _nonce;
    changed();
  }



  h256 baseRoot() const
  {
    assert(m_storageRoot);
    return m_storageRoot;
  }



  u256 storageValue(u256 const& _key, OverlayDB const& _db) const
  {
    auto mit = m_storageOverlay.find(_key);
    if (mit != m_storageOverlay.end())
    {
      return mit->second;
    }

    return originalStorageValue(_key, _db);
  }



  u256 originalStorageValue(u256 const& _key, OverlayDB const& _db) const;


  std::unordered_map<u256, u256> const& storageOverlay() const
  {
    return m_storageOverlay;
  }



  void setStorage(u256 _p, u256 _v)
  {
    m_storageOverlay[_p] = _v;
    changed();
  }


  void clearStorage()
  {
    m_storageOverlay.clear();
    m_storageOriginal.clear();
    m_storageRoot = EmptyTrie;
    changed();
  }


  void setStorageRoot(h256 const& _root)
  {
    m_storageOverlay.clear();
    m_storageOriginal.clear();
    m_storageRoot = _root;
    changed();
  }


  h256 codeHash() const
  {
    return m_codeHash;
  }

  bool hasNewCode() const
  {
    return m_hasNewCode;
  }


  void setCode(bytes&& _code, u256 const& _version);


  void resetCode();



  void noteCode(bytesConstRef _code)
  {
    assert(sha3(_code) == m_codeHash);
    m_codeCache = _code.toBytes();
  }


  bytes const& code() const
  {
    return m_codeCache;
  }

  u256 version() const
  {
    return m_version;
  }

private:

  void changed()
  {
    m_isUnchanged = false;
  }


  bool m_isAlive = false;


  bool m_isUnchanged = false;


  bool m_hasNewCode = false;


  u256 m_nonce;


  u256 m_balance = 0;



  h256 m_storageRoot = EmptyTrie;







  h256 m_codeHash = EmptySHA3;


  u256 m_version = 0;


  mutable std::unordered_map<u256, u256> m_storageOverlay;


  mutable std::unordered_map<u256, u256> m_storageOriginal;



  bytes m_codeCache;


  static const h256 c_contractConceptionCodeHash;
};

class AccountMask
{
public:
  AccountMask(bool _all = false):
    m_hasBalance(_all),
    m_hasNonce(_all),
    m_hasCode(_all),
    m_hasStorage(_all)
  {}

  AccountMask(
    bool _hasBalance,
    bool _hasNonce,
    bool _hasCode,
    bool _hasStorage,
    bool _shouldNotExist = false
  ):
    m_hasBalance(_hasBalance),
    m_hasNonce(_hasNonce),
    m_hasCode(_hasCode),
    m_hasStorage(_hasStorage),
    m_shouldNotExist(_shouldNotExist)
  {}

  bool allSet() const
  {
    return m_hasBalance && m_hasNonce && m_hasCode && m_hasStorage;
  }
  bool hasBalance() const
  {
    return m_hasBalance;
  }
  bool hasNonce() const
  {
    return m_hasNonce;
  }
  bool hasCode() const
  {
    return m_hasCode;
  }
  bool hasStorage() const
  {
    return m_hasStorage;
  }
  bool shouldExist() const
  {
    return !m_shouldNotExist;
  }

private:
  bool m_hasBalance;
  bool m_hasNonce;
  bool m_hasCode;
  bool m_hasStorage;
  bool m_shouldNotExist = false;
};

using AccountMap = std::unordered_map<Address, Account>;
using AccountMaskMap = std::unordered_map<Address, AccountMask>;

AccountMap jsonToAccountMap(std::string const& _json, u256 const& _defaultNonce = 0,
                            AccountMaskMap* o_mask = nullptr, const boost::filesystem::path& _configPath = {});
}
}
