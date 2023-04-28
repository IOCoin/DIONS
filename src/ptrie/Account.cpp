#include "ptrie/Account.h"
#include <ptrie/OverlayDB.h>
#include <ptrie/TrieDB.h>
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

using namespace std;
using namespace dev;
using namespace dev::dvm;

namespace fs = boost::filesystem;

template <class KeyType, class DB>
using SecureTrieDB = SpecificTrieDB<FatGenericTrieDB<DB>, KeyType>;

void Account::setCode(bytes&& _code, u256 const& _version)
{
  auto const newHash = sha3(_code);
  if (newHash != m_codeHash)
  {
    m_codeCache = std::move(_code);
    m_hasNewCode = true;
    m_codeHash = newHash;
  }
  m_version = _version;
}

void Account::resetCode()
{
  m_codeCache.clear();
  m_hasNewCode = false;
  m_codeHash = EmptySHA3;

  m_version = 0;
}

u256 Account::originalStorageValue(u256 const& _key, OverlayDB const& _db) const
{
  auto it = m_storageOriginal.find(_key);
  if (it != m_storageOriginal.end())
  {
    return it->second;
  }


  SecureTrieDB<h256, OverlayDB> const memdb(const_cast<OverlayDB*>(&_db), m_storageRoot);
  std::string const payload = memdb.at(_key);
  auto const value = payload.size() ? RLP(payload).toInt<u256>() : 0;
  m_storageOriginal[_key] = value;
  return value;
}

namespace js = json_spirit;


AccountMap dev::dvm::jsonToAccountMap(std::string const& _json, u256 const& _defaultNonce,
                                      AccountMaskMap* o_mask, const fs::path& _configPath)
{
  auto u256Safe = [](std::string const& s) -> u256
  {
    bigint ret(s);
    if (ret >= bigint(1) << 256)
      BOOST_THROW_EXCEPTION(
        ValueTooLarge() << errinfo_comment("State value is equal or greater than 2**256"));
    return (u256)ret;
  };

  std::unordered_map<Address, Account> ret;

  js::mValue val;
  json_spirit::read_string_or_throw(_json, val);

  for (auto const& account : val.get_obj())
  {
    Address a(fromHex(account.first));
    auto const& accountMaskJson = account.second.get_obj();
  }

  return ret;
}
