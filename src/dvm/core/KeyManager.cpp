// Licensed under the GNU General Public License, Version 3.


#include "KeyManager.h"
#include <thread>
#include <mutex>
#include <boost/filesystem.hpp>
#include <json_spirit/JsonSpiritHeaders.h>
#include <libdevcore/Log.h>
#include <libdevcore/Guards.h>
#include <libdevcore/RLP.h>
#include <libdevcore/SHA3.h>
using namespace std;
using namespace dev;
using namespace dvm;
namespace js = json_spirit;
namespace fs = boost::filesystem;

KeyManager::KeyManager(fs::path const& _keysFile, fs::path const& _secretsPath):
	m_keysFile(_keysFile), m_store(_secretsPath)
{
	for (auto const& uuid: m_store.keys())
	{
		auto addr = m_store.address(uuid);
		m_addrLookup[addr] = uuid;
		m_uuidLookup[uuid] = addr;
	}
}

KeyManager::~KeyManager()
{}

bool KeyManager::exists() const
{
	return !contents(appendToFilename(m_keysFile, ".salt")).empty() && !contents(m_keysFile).empty();
}

void KeyManager::create(string const& _pass)
{
	m_defaultPasswordDeprecated = asString(h256::random().asBytes());
	write(_pass, m_keysFile);
}

bool KeyManager::recode(Address const& _address, string const& _newPass, string const& _hint, function<string()> const& _pass, KDF _kdf)
{
	noteHint(_newPass, _hint);
	h128 u = uuid(_address);
	if (!store().recode(u, _newPass, [&](){ return getPassword(u, _pass); }, _kdf))
		return false;

	m_keyInfo[_address].passHash = hashPassword(_newPass);
	write();
	return true;
}

bool KeyManager::load(string const& _pass)
{
	try
	{
		bytes salt = contents(appendToFilename(m_keysFile, ".salt"));
		bytes encKeys = contents(m_keysFile);
		if (encKeys.empty())
			return false;
		m_keysFileKey = SecureFixedHash<16>(pbkdf2(_pass, salt, 262144, 16));
		bytesSec bs = decryptSymNoAuth(m_keysFileKey, h128(), &encKeys);
		RLP s(bs.ref());
		unsigned version = unsigned(s[0]);
		if (version == 1)
		{
			bool saveRequired = false;
			for (auto const& i: s[1])
			{
				h128 uuid(i[1]);
				Address addr(i[0]);
				if (uuid)
				{
					if (m_store.contains(uuid))
					{
						m_addrLookup[addr] = uuid;
						m_uuidLookup[uuid] = addr;
						m_keyInfo[addr] = KeyInfo(h256(i[2]), string(i[3]), i.itemCount() > 4 ? string(i[4]) : "");
						if (m_store.noteAddress(uuid, addr))
							saveRequired = true;
					}
					else
						cwarn << "Missing key:" << uuid << addr;
				}
				else
					m_keyInfo[addr] = KeyInfo(h256(i[2]), string(i[3]), i.itemCount() > 4 ? string(i[4]) : "");
//				cdebug << toString(addr) << toString(uuid) << toString((h256)i[2]) << (string)i[3];
			}
			if (saveRequired)
				m_store.save();

			for (auto const& i: s[2])
				m_passwordHint[h256(i[0])] = string(i[1]);
			m_defaultPasswordDeprecated = string(s[3]);
		}
//		cdebug << hashPassword(m_password) << toHex(m_password);
		cachePassword(m_defaultPasswordDeprecated);
//		cdebug << hashPassword(asString(m_key.ref())) << m_key.hex();
		cachePassword(asString(m_keysFileKey.ref()));
//		cdebug << hashPassword(_pass) << _pass;
		m_master = hashPassword(_pass);
		cachePassword(_pass);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

Secret KeyManager::secret(Address const& _address, function<string()> const& _pass, bool _usePasswordCache) const
{
	return secret(m_addrLookup.at(_address), _pass, _usePasswordCache);
}

Secret KeyManager::secret(h128 const& _uuid, function<string()> const& _pass, bool _usePasswordCache) const
{
	if (_usePasswordCache)
		return Secret(m_store.secret(_uuid, [&](){ return getPassword(_uuid, _pass); }, _usePasswordCache));
	else
		return Secret(m_store.secret(_uuid, _pass, _usePasswordCache));
}

string KeyManager::getPassword(h128 const& _uuid, function<string()> const& _pass) const
{
	h256 ph;
	auto ait = m_uuidLookup.find(_uuid);
	if (ait != m_uuidLookup.end())
	{
		auto kit = m_keyInfo.find(ait->second);
		if (kit != m_keyInfo.end())
			ph = kit->second.passHash;
	}
	return getPassword(ph, _pass);
}

string KeyManager::getPassword(h256 const& _passHash, function<string()> const& _pass) const
{
	auto it = m_cachedPasswords.find(_passHash);
	if (it != m_cachedPasswords.end())
		return it->second;
	for (unsigned i = 0; i < 10; ++i)
	{
		string p = _pass();
		if (p.empty())
			break;
		if (_passHash == UnknownPassword || hashPassword(p) == _passHash)
		{
			cachePassword(p);
			return p;
		}
	}
	return string();
}

h128 KeyManager::uuid(Address const& _a) const
{
	auto it = m_addrLookup.find(_a);
	if (it == m_addrLookup.end())
		return h128();
	return it->second;
}

Address KeyManager::address(h128 const& _uuid) const
{
	auto it = m_uuidLookup.find(_uuid);
	if (it == m_uuidLookup.end())
		return Address();
	return it->second;
}

h128 KeyManager::import(Secret const& _s, string const& _accountName, string const& _pass, string const& _passwordHint)
{
	Address addr = KeyPair(_s).address();
	auto passHash = hashPassword(_pass);
	cachePassword(_pass);
	m_passwordHint[passHash] = _passwordHint;
	auto uuid = m_store.importSecret(_s.asBytesSec(), _pass);
	m_keyInfo[addr] = KeyInfo{passHash, _accountName, ""};
	m_addrLookup[addr] = uuid;
	m_uuidLookup[uuid] = addr;
	write(m_keysFile);
	return uuid;
}

void KeyManager::importExisting(h128 const& _uuid, string const& _info, string const& _pass, string const& _passwordHint)
{
	bytesSec key = m_store.secret(_uuid, [&](){ return _pass; });
	if (key.empty())
		return;
	Address a = KeyPair(Secret(key)).address();
	auto passHash = hashPassword(_pass);
	if (!m_cachedPasswords.count(passHash))
		cachePassword(_pass);
	importExisting(_uuid, _info, a, passHash, _passwordHint);
}

void KeyManager::importExisting(h128 const& _uuid, string const& _accountName, Address const& _address, h256 const& _passHash, string const& _passwordHint)
{
	if (!m_passwordHint.count(_passHash))
		m_passwordHint[_passHash] = _passwordHint;
	m_uuidLookup[_uuid] = _address;
	m_addrLookup[_address] = _uuid;
	m_keyInfo[_address].passHash = _passHash;
	m_keyInfo[_address].accountName = _accountName;
	write(m_keysFile);
}

void KeyManager::kill(Address const& _a)
{
	auto id = m_addrLookup[_a];
	m_uuidLookup.erase(id);
	m_addrLookup.erase(_a);
	m_keyInfo.erase(_a);
	m_store.kill(id);
	write(m_keysFile);
}

KeyPair KeyManager::presaleSecret(std::string const& _json, function<string(bool)> const& _password)
{
	js::mValue val;
	json_spirit::read_string(_json, val);
	auto obj = val.get_obj();
	string p = _password(true);
	if (obj["encseed"].type() == js::str_type)
	{
		auto encseed = fromHex(obj["encseed"].get_str());
		while (true)
		{
			KeyPair k = KeyPair::fromEncryptedSeed(&encseed, p);
			if (obj["dvmaddr"].type() == js::str_type)
			{
				Address a(obj["dvmaddr"].get_str());
				Address b = k.address();
				if (a != b)
				{
					if ((p = _password(false)).empty())
						BOOST_THROW_EXCEPTION(PasswordUnknown());
					continue;
				}
			}
			return k;
		}
	}
	else
		BOOST_THROW_EXCEPTION(Exception() << errinfo_comment("encseed type is not js::str_type"));
}

Addresses KeyManager::accounts() const
{
	set<Address> addresses;
	for (auto const& i: m_keyInfo)
		addresses.insert(i.first);
	for (auto const& key: m_store.keys())
		addresses.insert(m_store.address(key));
	// Remove the zero address if present
	return Addresses{addresses.upper_bound(Address()), addresses.end()};
}

bool KeyManager::hasAccount(Address const& _address) const
{
	if (!_address)
		return false;
	if (m_keyInfo.count(_address))
		return true;
	for (auto const& key: m_store.keys())
		if (m_store.address(key) == _address)
			return true;
	return false;
}

string const& KeyManager::accountName(Address const& _address) const
{
	try
	{
		return m_keyInfo.at(_address).accountName;
	}
	catch (...)
	{
		return EmptyString;
	}
}

string const& KeyManager::passwordHint(Address const& _address) const
{
	try
	{
		auto& info = m_keyInfo.at(_address);
		if (info.passwordHint.size())
			return info.passwordHint;
		return m_passwordHint.at(info.passHash);
	}
	catch (...)
	{
		return EmptyString;
	}
}

h256 KeyManager::hashPassword(string const& _pass) const
{
	// TODO SECURITY: store this a bit more securely; Scrypt perhaps?
	return h256(pbkdf2(_pass, asBytes(m_defaultPasswordDeprecated), 262144, 32).makeInsecure());
}

void KeyManager::cachePassword(string const& _password) const
{
	m_cachedPasswords[hashPassword(_password)] = _password;
}

bool KeyManager::write(fs::path const& _keysFile) const
{
	if (!m_keysFileKey)
		return false;
	write(m_keysFileKey, _keysFile);
	return true;
}

void KeyManager::write(string const& _pass, fs::path const& _keysFile) const
{
	bytes salt = h256::random().asBytes();
	writeFile(appendToFilename(_keysFile, ".salt"), salt, true);
	auto key = SecureFixedHash<16>(pbkdf2(_pass, salt, 262144, 16));

	cachePassword(_pass);
	m_master = hashPassword(_pass);
	write(key, _keysFile);
}

void KeyManager::write(SecureFixedHash<16> const& _key, fs::path const& _keysFile) const
{
	RLPStream s(4);
	s << 1; // version

	s.appendList(m_keyInfo.size());
	for (auto const& info: m_keyInfo)
	{
		h128 id = uuid(info.first);
		auto const& ki = info.second;
		s.appendList(5) << info.first << id << ki.passHash << ki.accountName << ki.passwordHint;
	}

	s.appendList(m_passwordHint.size());
	for (auto const& i: m_passwordHint)
		s.appendList(2) << i.first << i.second;
	s.append(m_defaultPasswordDeprecated);

	writeFile(_keysFile, encryptSymNoAuth(_key, h128(), &s.out()), true);
	m_keysFileKey = _key;
	cachePassword(defaultPassword());
}

KeyPair KeyManager::newKeyPair(KeyManager::NewKeyType _type)
{
	KeyPair p = KeyPair::create();
	bool keepGoing = true;
	unsigned done = 0;
	auto f = [&]() {
		KeyPair lp = KeyPair::create();
		while (keepGoing)
		{
			done++;
			if (done % 1000 == 0)
				cnote << "Tried" << done << "keys";
			lp = KeyPair::create();
			auto a = lp.address();
			if (_type == NewKeyType::NoVanity ||
				(_type == NewKeyType::DirectICAP && !a[0]) ||
				(_type == NewKeyType::FirstTwo && a[0] == a[1]) ||
				(_type == NewKeyType::FirstTwoNextTwo && a[0] == a[1] && a[2] == a[3]) ||
				(_type == NewKeyType::FirstThree && a[0] == a[1] && a[1] == a[2]) ||
				(_type == NewKeyType::FirstFour && a[0] == a[1] && a[1] == a[2] && a[2] == a[3])
			)
				break;
		}
		if (keepGoing)
			p = lp;
		keepGoing = false;
	};

	vector<std::thread*> ts;
	for (unsigned t = 0; t < std::thread::hardware_concurrency() - 1; ++t)
		ts.push_back(new std::thread(f));
	f();

	for (std::thread* t: ts)
	{
		t->join();
		delete t;
	}
	return p;
}
