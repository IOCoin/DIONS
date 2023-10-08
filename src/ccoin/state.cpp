#include "state.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace fs = boost::filesystem;

State::State(dev::u256 const& _accountStartNonce, dev::OverlayDB const& _db, BaseState _bs):
  db_(_db),
  state_(&db_),
  accountStartNonce_(_accountStartNonce)
{
  if (_bs != BaseState::PreExisting)
  {
    state_.init();
  }
}

State::State(State const& _s):
  db_(_s.db_),
  state_(&db_, _s.state_.root(), Verification::Skip),
  cache_(_s.cache_),
  unchangedCacheEntries_(_s.unchangedCacheEntries_),
  nonExistingAccountsCache_(_s.nonExistingAccountsCache_),
  touched_(_s.touched_),
  unrevertablyTouched_(_s.unrevertablyTouched_),
  accountStartNonce_(_s.accountStartNonce_)
{}

dev::OverlayDB State::openDB(fs::path const& _basePath, dev::h256 const& _genesisHash, WithExisting _we)
{
  DatabasePaths const dbPaths{_basePath, _genesisHash};

  if (db::isDiskDatabase())
  {
    if (_we == WithExisting::Kill)
    {
      clog(VerbosityInfo, "statedb") << "Deleting state database: " << dbPaths.statePath();
      fs::remove_all(dbPaths.statePath());
    }

    clog(VerbosityDebug, "statedb")
        << "Verifying path exists (and creating if not present): " << dbPaths.chainPath();
    fs::create_directories(dbPaths.chainPath());
    clog(VerbosityDebug, "statedb")
        << "Ensuring permissions are set for path: " << dbPaths.chainPath();
    DEV_IGNORE_EXCEPTIONS(fs::permissions(dbPaths.chainPath(), fs::owner_all));
  }

  try
  {
    clog(VerbosityTrace, "statedb") << "Opening state database";
    std::unique_ptr<db::DatabaseFace> db = db::DBFactory::create(dbPaths.statePath());
    return dev::OverlayDB(std::move(db));
  }
  catch (boost::exception const& ex)
  {
    if (db::isDiskDatabase())
    {
      clog(VerbosityError, "statedb")
          << "Error opening state database: " << dbPaths.statePath();
      db::DatabaseStatus const dbStatus =
        *boost::get_error_info<db::errinfo_dbStatusCode>(ex);

      if (fs::space(dbPaths.statePath()).available < 1024)
      {
        clog(VerbosityError, "statedb")
            << "Not enough available space found on hard drive. Please free some up and "
            "then re-run. Bailing.";
        BOOST_THROW_EXCEPTION(NotEnoughAvailableSpace());
      }
      else if (dbStatus == db::DatabaseStatus::Corruption)
      {
        clog(VerbosityError, "statedb")
            << "Database corruption detected. Please see the exception for corruption "
            "details. Exception: "
            << boost::diagnostic_information(ex);
        BOOST_THROW_EXCEPTION(DatabaseCorruption());
      }
      else if (dbStatus == db::DatabaseStatus::IOError)
      {
        clog(VerbosityError, "statedb") << "Database already open. You appear to have "
                                        "another instance of Aleth running.";
        BOOST_THROW_EXCEPTION(DatabaseAlreadyOpen());
      }
    }

    clog(VerbosityError, "statedb")
        << "Unknown error encountered when opening state database. Exception details: "
        << boost::diagnostic_information(ex);
    throw;
  }
}

void State::populateFrom(AccountMap const& _map)
{
  eth::commit(_map, state_);
  commit(State::CommitBehaviour::KeepEmptyAccounts);
}

dev::u256 const& State::requireAccountStartNonce() const
{
  if (accountStartNonce_ == Invalid256)
  {
    BOOST_THROW_EXCEPTION(InvalidAccountStartNonceInState());
  }

  return accountStartNonce_;
}

void State::noteAccountStartNonce(dev::u256 const& _actual)
{
  if (accountStartNonce_ == Invalid256)
  {
    accountStartNonce_ = _actual;
  }
  else if (accountStartNonce_ != _actual)
  {
    BOOST_THROW_EXCEPTION(IncorrectAccountStartNonceInState());
  }
}

void State::removeEmptyAccounts()
{
  for (auto& i: this->cache_)
    if (i.second.isDirty() && i.second.isEmpty())
    {
      i.second.kill();
    }

  for (auto const& _address : unrevertablyTouched_)
  {
    Account* a = account(_address);

    if (a && a->isEmpty())
    {
      a->kill();
    }
  }
}

State& State::operator=(State const& _s)
{
  if (&_s == this)
  {
    return *this;
  }

  db_ = _s.db_;
  state_.open(&db_, _s.state_.root(), Verification::Skip);
  this->cache_ = _s.cache_;
  unchangedCacheEntries_ = _s.unchangedCacheEntries_;
  nonExistingAccountsCache_ = _s.nonExistingAccountsCache_;
  touched_ = _s.touched_;
  unrevertablyTouched_ = _s.unrevertablyTouched_;
  accountStartNonce_ = _s.accountStartNonce_;
  return *this;
}

Account const* State::account(dev::Address const& _a) const
{
  return const_cast<State*>(this)->account(_a);
}

Account* State::account(dev::Address const& _addr)
{
  auto it = this->cache_.find(_addr);

  if (it != this->cache_.end())
  {
    return &it->second;
  }

  if (nonExistingAccountsCache_.count(_addr))
  {
    return nullptr;
  }

  string stateBack = state_.at(_addr);

  if (stateBack.empty())
  {
    nonExistingAccountsCache_.insert(_addr);
    return nullptr;
  }

  clearCacheIfTooLarge();
  RLP state(stateBack);
  auto const nonce = state[0].toInt<dev::u256>();
  auto const balance = state[1].toInt<dev::u256>();
  auto const storageRoot = state[2].toHash<dev::h256>();
  auto const codeHash = state[3].toHash<dev::h256>();
  auto const version = state[4] ? state[4].toInt<dev::u256>() : 0;
  auto i = this->cache_.emplace(piecewise_construct, forward_as_tuple(_addr),
                           forward_as_tuple(nonce, balance, storageRoot, codeHash, version, Account::Unchanged));
  unchangedCacheEntries_.push_back(_addr);
  return &i.first->second;
}

void State::clearCacheIfTooLarge() const
{
  while (unchangedCacheEntries_.size() > 1000)
  {
    size_t const randomIndex = std::uniform_int_distribution<size_t>(0, unchangedCacheEntries_.size() - 1)(dev::s_fixedHashEngine);
    dev::Address const addr = unchangedCacheEntries_[randomIndex];
    swap(unchangedCacheEntries_[randomIndex], unchangedCacheEntries_.back());
    unchangedCacheEntries_.pop_back();
    auto cacheEntry = this->cache_.find(addr);

    if (cacheEntry != this->cache_.end() && !cacheEntry->second.isDirty())
    {
      this->cache_.erase(cacheEntry);
    }
  }
}

void State::commit(CommitBehaviour _commitBehaviour)
{
  if (_commitBehaviour == CommitBehaviour::RemoveEmptyAccounts)
  {
    removeEmptyAccounts();
  }

  touched_ += dev::eth::commit(this->cache_, state_);
changeLog_.clear();
  this->cache_.clear();
  unchangedCacheEntries_.clear();
}

unordered_map<dev::Address, dev::u256> State::addresses() const
{
#if ETH_FATDB
  unordered_map<dev::Address, dev::u256> ret;

  for (auto& i: this->cache_)
    if (i.second.isAlive())
    {
      ret[i.first] = i.second.balance();
    }

  for (auto const& i: state_)
    if (this->cache_.find(i.first) == this->cache_.end())
    {
      ret[i.first] = RLP(i.second)[1].toInt<dev::u256>();
    }

  return ret;
#else
  BOOST_THROW_EXCEPTION(InterfaceNotSupported() << errinfo_interface("State::addresses()"));
#endif
}

std::pair<State::AddressMap, dev::h256> State::addresses(
  dev::h256 const& _beginHash, size_t _maxResults) const
{
  AddressMap addresses;
  dev::h256 nextKey;
#if ETH_FATDB

  for (auto it = state_.hashedLowerBound(_beginHash); it != state_.hashedEnd(); ++it)
  {
    auto const address = dev::Address(it.key());
    auto const itCachedAddress = this->cache_.find(address);

    if (itCachedAddress != this->cache_.end() && itCachedAddress->second.isDirty() &&
        !itCachedAddress->second.isAlive())
    {
      continue;
    }

    if (addresses.size() == _maxResults)
    {
      nextKey = dev::h256((*it).first);
      break;
    }

    dev::h256 const hashedAddress((*it).first);
    addresses[hashedAddress] = address;
  }

#endif
  AddressMap cacheAddresses;

  for (auto const& addressAndAccount : this->cache_)
  {
    auto const& address = addressAndAccount.first;
    auto const addressHash = sha3(address);
    auto const& account = addressAndAccount.second;

    if (account.isDirty() && account.isAlive() && addressHash >= _beginHash)
    {
      cacheAddresses.emplace(addressHash, address);
    }
  }

  addresses.insert(cacheAddresses.begin(), cacheAddresses.end());

  if (addresses.size() > _maxResults)
  {
    auto itEnd = std::next(addresses.begin(), _maxResults);
    nextKey = itEnd->first;
    addresses.erase(itEnd, addresses.end());
  }

  return {addresses, nextKey};
}


void State::setRoot(dev::h256 const& _r)
{
  this->cache_.clear();
  unchangedCacheEntries_.clear();
  nonExistingAccountsCache_.clear();
  state_.setRoot(_r);
}

bool State::addressInUse(Address const& _id) const
{
  return !!account(_id);
}

bool State::accountNonemptyAndExisting(Address const& _address) const
{
  if (Account const* a = account(_address))
  {
    return !a->isEmpty();
  }
  else
  {
    return false;
  }
}

bool State::addressHasCode(Address const& _id) const
{
  if (auto a = account(_id))
  {
    return a->codeHash() != EmptySHA3;
  }
  else
  {
    return false;
  }
}

dev::u256 State::balance(Address const& _id) const
{
  if (auto a = account(_id))
  {
    return a->balance();
  }
  else
  {
    return 0;
  }
}

void State::incNonce(Address const& _addr)
{
  if (Account* a = account(_addr))
  {
    auto oldNonce = a->nonce();
    a->incNonce();
    changeLog_.emplace_back(_addr, oldNonce);
  }
  else
  {
    createAccount(_addr, Account(requireAccountStartNonce() + 1, 0));
  }
}

void State::setNonce(Address const& _addr, dev::u256 const& _newNonce)
{
  if (Account* a = account(_addr))
  {
    auto oldNonce = a->nonce();
    a->setNonce(_newNonce);
    changeLog_.emplace_back(_addr, oldNonce);
  }
  else
  {
    createAccount(_addr, Account(_newNonce, 0));
  }
}

void State::addBalance(Address const& _id, dev::u256 const& _amount)
{
  if (Account* a = account(_id))
  {
    if (!a->isDirty() && a->isEmpty())
    {
      changeLog_.emplace_back(Change::Touch, _id);
    }

    a->addBalance(_amount);
  }
  else
    createAccount(_id, {requireAccountStartNonce(), _amount});

  if (_amount)
  {
    changeLog_.emplace_back(Change::Balance, _id, _amount);
  }
}

void State::subBalance(Address const& _addr, dev::u256 const& _value)
{
  if (_value == 0)
  {
    return;
  }

  Account* a = account(_addr);

  if (!a || a->balance() < _value)
  {
    BOOST_THROW_EXCEPTION(NotEnoughCash());
  }

  addBalance(_addr, 0 - _value);
}

void State::setBalance(Address const& _addr, dev::u256 const& _value)
{
  Account* a = account(_addr);
  dev::u256 original = a ? a->balance() : 0;
  addBalance(_addr, _value - original);
}

void State::createContract(Address const& _address)
{
  createAccount(_address, {requireAccountStartNonce(), 0});
}

void State::createAccount(Address const& _address, Account const&& _account)
{
  assert(!addressInUse(_address) && "Account already exists");
  this->cache_[_address] = std::move(_account);
  nonExistingAccountsCache_.erase(_address);
  changeLog_.emplace_back(Change::Create, _address);
}

void State::kill(Address _addr)
{
  if (auto a = account(_addr))
  {
    a->kill();
  }
}

dev::u256 State::getNonce(Address const& _addr) const
{
  if (auto a = account(_addr))
  {
    return a->nonce();
  }
  else
  {
    return accountStartNonce_;
  }
}

dev::u256 State::storage(Address const& _id, dev::u256 const& _key) const
{
  if (Account const* a = account(_id))
  {
    return a->storageValue(_key, db_);
  }
  else
  {
    return 0;
  }
}

void State::setStorage(Address const& _contract, dev::u256 const& _key, dev::u256 const& _value)
{
  changeLog_.emplace_back(_contract, _key, storage(_contract, _key));
  this->cache_[_contract].setStorage(_key, _value);
}

dev::u256 State::originalStorageValue(Address const& _contract, dev::u256 const& _key) const
{
  if (Account const* a = account(_contract))
  {
    return a->originalStorageValue(_key, db_);
  }
  else
  {
    return 0;
  }
}

void State::clearStorage(Address const& _contract)
{
	dev::h256 const& oldHash{this->cache_[_contract].baseRoot()};

  if (oldHash == EmptyTrie)
  {
    return;
  }

  changeLog_.emplace_back(Change::StorageRoot, _contract, oldHash);
  this->cache_[_contract].clearStorage();
}

map<dev::h256, pair<dev::u256, dev::u256>> State::storage(Address const& _id) const
{
#if ETH_FATDB
  map<dev::h256, pair<dev::u256, dev::u256>> ret;

  if (Account const* a = account(_id))
  {
    if (dev::h256 root = a->baseRoot())
    {
      for (auto it = memdb.hashedBegin(); it != memdb.hashedEnd(); ++it)
      {
	      dev::h256 const hashedKey((*it).first);
	      dev::u256 const key = dev::h256(it.key());
	      dev::u256 const value = RLP((*it).second).toInt<dev::u256>();
        ret[hashedKey] = make_pair(key, value);
      }
    }

    for (auto const& i : a->storageOverlay())
    {
	    dev::h256 const key = i.first;
	    dev::h256 const hashedKey = sha3(key);

      if (i.second)
      {
        ret[hashedKey] = i;
      }
      else
      {
        ret.erase(hashedKey);
      }
    }
  }

  return ret;
#else
  (void) _id;
  BOOST_THROW_EXCEPTION(InterfaceNotSupported() << errinfo_interface("State::storage(Address const& _id)"));
#endif
}

dev::h256 State::storageRoot(Address const& _id) const
{
  string s = state_.at(_id);

  if (s.size())
  {
    RLP r(s);
    return r[2].toHash<dev::h256>();
  }

  return EmptyTrie;
}

bytes const& State::code(Address const& _addr) const
{
  Account const* a = account(_addr);

  if (!a || a->codeHash() == EmptySHA3)
  {
    return NullBytes;
  }

  if (a->code().empty())
  {
    Account* mutableAccount = const_cast<Account*>(a);
    mutableAccount->noteCode(db_.lookup(a->codeHash()));
    CodeSizeCache::instance().store(a->codeHash(), a->code().size());
  }

  return a->code();
}

void State::setCode(Address const& _address, bytes&& _code, dev::u256 const& _version)
{
  assert(!addressHasCode(_address));
  changeLog_.emplace_back(Change::Code, _address);
  this->cache_[_address].setCode(move(_code), _version);
}

dev::h256 State::codeHash(Address const& _a) const
{
  if (Account const* a = account(_a))
  {
    return a->codeHash();
  }
  else
  {
    return EmptySHA3;
  }
}

size_t State::codeSize(Address const& _a) const
{
  if (Account const* a = account(_a))
  {
    if (a->hasNewCode())
    {
      return a->code().size();
    }

    auto& codeSizeCache = CodeSizeCache::instance();
    dev::h256 codeHash = a->codeHash();

    if (codeSizeCache.contains(codeHash))
    {
      return codeSizeCache.get(codeHash);
    }
    else
    {
      size_t size = code(_a).size();
      codeSizeCache.store(codeHash, size);
      return size;
    }
  }
  else
  {
    return 0;
  }
}

dev::u256 State::version(Address const& _a) const
{
  Account const* a = account(_a);
  return a ? a->version() : 0;
}

void State::unrevertableTouch(Address const& _address)
{
  unrevertablyTouched_.insert(_address);
}

size_t State::savepoint() const
{
  return changeLog_.size();
}

void State::rollback(size_t _savepoint)
{
  while (_savepoint != changeLog_.size())
  {
    auto& change = changeLog_.back();
    auto& account = this->cache_[change.address];

    switch (change.kind)
    {
    case Change::Storage:
      account.setStorage(change.key, change.value);
      break;

    case Change::StorageRoot:
      account.setStorageRoot(change.value);
      break;

    case Change::Balance:
      account.addBalance(0 - change.value);
      break;

    case Change::Nonce:
      account.setNonce(change.value);
      break;

    case Change::Create:
      this->cache_.erase(change.address);
      break;

    case Change::Code:
      account.resetCode();
      break;

    case Change::Touch:
      account.untouch();
      unchangedCacheEntries_.emplace_back(change.address);
      break;
    }

    changeLog_.pop_back();
  }
}

std::pair<ExecutionResult, TransactionReceipt> State::execute(EnvInfo const& _envInfo, Transaction const& _t, Permanence _p, OnOpFunc const& _onOp)
{
  Executive e(*this, _envInfo);
  ExecutionResult res;
  e.setResultRecipient(res);
  auto onOp = _onOp;

  if (isVmTraceEnabled() && !onOp)
  {
    onOp = e.simpleTrace();
  }

  dev::u256 const startGasUsed = _envInfo.gasUsed();
  bool const statusCode = executeTransaction(e, _t, onOp);
  bool removeEmptyAccounts = false;

  switch (_p)
  {
  case Permanence::Reverted:
    this->cache_.clear();
    break;

  case Permanence::Committed:
    commit(removeEmptyAccounts ? State::CommitBehaviour::RemoveEmptyAccounts : State::CommitBehaviour::KeepEmptyAccounts);
    break;

  case Permanence::Uncommitted:
    break;
  }

  TransactionReceipt receipt; 
  return make_pair(res, receipt);
}

void State::executeBlockTransactions(Block const& _block, unsigned _txCount,
                                     LastBlockHashesFace const& _lastHashes)
{
	dev::u256 gasUsed = 0;

  for (unsigned i = 0; i < _txCount; ++i)
  {
    EnvInfo envInfo(_block.info(), _lastHashes, gasUsed);
    Executive e(*this, envInfo);
    executeTransaction(e, _block.pending()[i], OnOpFunc());
    gasUsed += e.gasUsed();
  }
}

bool State::executeTransaction(Executive& _e, Transaction const& _t, OnOpFunc const& _onOp)
{
  size_t const savept = savepoint();

  try
  {
    _e.initialize(_t);

    if (!_e.execute())
    {
      _e.go(_onOp);
    }

    return _e.finalize();
  }
  catch (Exception const&)
  {
    rollback(savept);
    throw;
  }
}

std::ostream& dev::eth::operator<<(std::ostream& _out, State const& _s)
{
  _out << "--- " << _s.rootHash() << std::endl;
  std::set<Address> d;
  std::set<Address> dtr;
  auto trie = SecureTrieDB<Address, dev::OverlayDB>(const_cast<dev::OverlayDB*>(&_s.db_), _s.rootHash());

  for (auto i: trie)
  {
    d.insert(i.first), dtr.insert(i.first);
  }

  for (auto i: _s.cache_)
  {
    d.insert(i.first);
  }

  for (auto i: d)
  {
    auto it = _s.cache_.find(i);
    Account* cache = it != _s.cache_.end() ? &it->second : nullptr;
    string rlpString = dtr.count(i) ? trie.at(i) : "";
    RLP r(rlpString);
    assert(cache || r);

    if (cache && !cache->isAlive())
    {
      _out << "XXX  " << i << std::endl;
    }
    else
    {
      string lead = (cache ? r ? " *   " : " +   " : "     ");

      if (cache && r && cache->nonce() == r[0].toInt<dev::u256>() && cache->balance() == r[1].toInt<dev::u256>())
      {
        lead = " .   ";
      }

      stringstream contout;

      if ((cache && cache->codeHash() == EmptySHA3) || (!cache && r && (dev::h256)r[3] != EmptySHA3))
      {
        std::map<dev::u256, u256> mem;
        std::set<dev::u256> back;
        std::set<dev::u256> delta;
        std::set<dev::u256> cached;

        if (r)
        {
		dev::SecureTrieDB<dev::h256, dev::OverlayDB> memdb(const_cast<dev::OverlayDB*>(&_s.db_), r[2].toHash<dev::h256>());

          for (auto const& j: memdb)
          {
            mem[j.first] = RLP(j.second).toInt<dev::u256>(), back.insert(j.first);
          }
        }

        if (cache)
          for (auto const& j: cache->storageOverlay())
          {
            if ((!mem.count(j.first) && j.second) || (mem.count(j.first) && mem.at(j.first) != j.second))
            {
              mem[j.first] = j.second, delta.insert(j.first);
            }
            else if (j.second)
            {
              cached.insert(j.first);
            }
          }

        if (!delta.empty())
        {
          lead = (lead == " .   ") ? "*.*  " : "***  ";
        }

        contout << " @:";

        if (!delta.empty())
        {
          contout << "???";
        }
        else
        {
          contout << r[2].toHash<dev::h256>();
        }

        if (cache && cache->hasNewCode())
        {
          contout << " $" << toHex(cache->code());
        }
        else
        {
          contout << " $" << (cache ? cache->codeHash() : r[3].toHash<dev::h256>());
        }

        for (auto const& j: mem)
          if (j.second)
          {
            contout << std::endl << (delta.count(j.first) ? back.count(j.first) ? " *     " : " +     " : cached.count(j.first) ? " .     " : "       ") << std::hex << nouppercase << std::setw(64) << j.first << ": " << std::setw(0) << j.second ;
          }
          else
          {
            contout << std::endl << "XXX    " << std::hex << nouppercase << std::setw(64) << j.first << "";
          }
      }
      else
      {
        contout << " [SIMPLE]";
      }

      _out << lead << i << ": " << std::dec << (cache ? cache->nonce() : r[0].toInt<dev::u256>()) << " #:" << (cache ? cache->balance() : r[1].toInt<dev::u256>()) << contout.str() << std::endl;
    }
  }

  return _out;
}

State& dev::eth::createIntermediateState(State& o_s, Block const& _block, unsigned _txIndex, BlockChain const& _bc)
{
  o_s = _block.state();
  dev::u256 const rootHash = _block.stateRootBeforeTx(_txIndex);

  if (rootHash)
  {
    o_s.setRoot(rootHash);
  }
  else
  {
    o_s.setRoot(_block.stateRootBeforeTx(0));
  }

  return o_s;
}

template <class DB>
AddressHash dev::eth::commit(AccountMap const& _cache, SecureTrieDB<Address, DB>& _state)
{
  AddressHash ret;

  for (auto const& i: _cache)
    if (i.second.isDirty())
    {
      if (!i.second.isAlive())
      {
        _state.remove(i.first);
      }
      else
      {
        auto const version = i.second.version();
        RLPStream s(version != 0 ? 5 : 4);
        s << i.second.nonce() << i.second.balance();

        if (i.second.storageOverlay().empty())
        {
          assert(i.second.baseRoot());
          s.append(i.second.baseRoot());
        }
        else
        {
          SecureTrieDB<dev::h256, DB> storageDB(_state.db(), i.second.baseRoot());

          for (auto const& j: i.second.storageOverlay())
            if (j.second)
            {
              storageDB.insert(j.first, rlp(j.second));
            }
            else
            {
              storageDB.remove(j.first);
            }

          assert(storageDB.root());
          s.append(storageDB.root());
        }

        if (i.second.hasNewCode())
        {
		dev::h256 ch = i.second.codeHash();
          CodeSizeCache::instance().store(ch, i.second.code().size());
          _state.db()->insert(ch, &i.second.code());
          s << ch;
        }
        else
        {
          s << i.second.codeHash();
        }

        if (version != 0)
        {
          s << i.second.version();
        }

        _state.insert(i.first, &s.out());
      }

      ret.insert(i.first);
    }

  return ret;
}


template AddressHash dev::eth::commit<dev::OverlayDB>(AccountMap const& _cache, SecureTrieDB<Address, dev::OverlayDB>& _state);
template AddressHash dev::eth::commit<StateCacheDB>(AccountMap const& _cache, SecureTrieDB<Address, StateCacheDB>& _state);
