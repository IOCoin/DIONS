#pragma once

#include "ptrie/Account.h"
#include "ptrie/DBFactory.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/TrieDB.h"
#include "ptrie/Address.h"
#include <array>
#include <unordered_map>

struct Change
{
  enum Kind: int
  {
    Balance,
    Storage,
    StorageRoot,
    Nonce,
    Create,
    Code,
    Touch
  };

  Kind kind;
  dev::Address address;
  u256 value;
  u256 key;

  Change(Kind _kind, dev::Address const& _addr, u256 const& _value = 0):
    kind(_kind), address(_addr), value(_value)
  {
  }

  Change(dev::Address const& _addr, u256 const& _key, u256 const& _value):
    kind(Storage), address(_addr), value(_value), key(_key)
  {}

  Change(dev::Address const& _addr, u256 const& _value):
    kind(Nonce), address(_addr), value(_value)
  {}
};

using ChangeLog = std::vector<Change>;

class State
{
public:
  enum class CommitBehaviour
  {
    KeepEmptyAccounts,
    RemoveEmptyAccounts
  };

  using AddressMap = std::map<h256, dev::Address>;

  explicit State(u256 const& _accountStartNonce): State(_accountStartNonce, OverlayDB(), BaseState::Empty) {}

  explicit State(u256 const& _accountStartNonce, OverlayDB const& _db, BaseState _bs = BaseState::PreExisting);

  enum NullType { Null };
  State(NullType): State(Invalid256, OverlayDB(), BaseState::Empty) {}

  State(State const& _s);

  State& operator=(State const& _s);

  OverlayDB const& db() const
  {
    return db_;
  }
  OverlayDB& db()
  {
    return db_;
  }

  void populateFrom(AccountMap const& _map);

  std::unordered_map<dev::Address, u256> addresses() const;

  std::pair<AddressMap, h256> addresses(h256 const& _begin, size_t _maxResults) const;

  std::pair<ExecutionResult, TransactionReceipt> execute(EnvInfo const& _envInfo, SealEngineFace const& _sealEngine, Transaction const& _t, Permanence _p = Permanence::Committed, OnOpFunc const& _onOp = OnOpFunc());

  void executeBlockTransactions(Block const& _block, unsigned _txCount, LastBlockHashesFace const& _lastHashes, SealEngineFace const& _sealEngine);

  bool addressInUse(dev::Address const& _address) const;

  bool accountNonemptyAndExisting(dev::Address const& _address) const;

  bool addressHasCode(dev::Address const& _address) const;

  u256 balance(dev::Address const& _id) const;

  void addBalance(dev::Address const& _id, u256 const& _amount);

  void subBalance(dev::Address const& _addr, u256 const& _value);

  void setBalance(dev::Address const& _addr, u256 const& _value);

  void transferBalance(dev::Address const& _from, dev::Address const& _to, u256 const& _value)
  {
    subBalance(_from, _value);
    addBalance(_to, _value);
  }

  h256 storageRoot(dev::Address const& _contract) const;

  u256 storage(dev::Address const& _contract, u256 const& _memory) const;

  void setStorage(dev::Address const& _contract, u256 const& _location, u256 const& _value);

  u256 originalStorageValue(dev::Address const& _contract, u256 const& _key) const;

  void clearStorage(dev::Address const& _contract);

  void createContract(dev::Address const& _address);

  void setCode(dev::Address const& _address, bytes&& _code, u256 const& _version);

  void kill(dev::Address _a);

  std::map<h256, std::pair<u256, u256>> storage(dev::Address const& _contract) const;

  bytes const& code(dev::Address const& _addr) const;

  h256 codeHash(dev::Address const& _contract) const;

  size_t codeSize(dev::Address const& _contract) const;

  u256 version(dev::Address const& _contract) const;

  void incNonce(dev::Address const& _id);

  void setNonce(dev::Address const& _addr, u256 const& _newNonce);

  u256 getNonce(dev::Address const& _addr) const;

  h256 rootHash() const
  {
    return state_.root();
  }

  void commit(CommitBehaviour _commitBehaviour);

  void setRoot(h256 const& _root);

  u256 const& accountStartNonce() const
  {
    return accountStartNonce_;
  }
  u256 const& requireAccountStartNonce() const;
  void noteAccountStartNonce(u256 const& _actual);

  void unrevertableTouch(dev::Address const& _addr);

  size_t savepoint() const;

  void rollback(size_t _savepoint);

  ChangeLog const& changeLog() const
  {
    return changeLog_;
  }

private:
  void removeEmptyAccounts();

  Account const* account(dev::Address const& _addr) const;

  Account* account(dev::Address const& _addr);

  void clearCacheIfTooLarge() const;

  void createAccount(dev::Address const& _address, Account const&& _account);

  bool executeTransaction(Executive& _e, Transaction const& _t, OnOpFunc const& _onOp);

  OverlayDB db_;
  SecureTrieDB<dev::Address, dev::OverlayDB> state_;
  mutable std::unordered_map<dev::Address, dev::Account> cache_;
  mutable std::vector<dev::Address> unchangedCacheEntries_;
  mutable std::set<dev::Address> nonExistingAccountsCache_;
  AddressHash touched_;

  u256 accountStartNonce_;

  friend std::ostream& operator<<(std::ostream& _out, State const& _s);
  ChangeLog changeLog_;
};

std::ostream& operator<<(std::ostream& _out, State const& _s);

State& createIntermediateState(State& o_s, Block const& _block, unsigned _txIndex, BlockChain const& _bc);

template <class DB>
AddressHash commit(AccountMap const& _cache, SecureTrieDB<dev::Address, DB>& _state);


