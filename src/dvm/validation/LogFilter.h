// Licensed under the GNU General Public License, Version 3.

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdvmcore/Common.h>
#include "TransactionReceipt.h"

#ifdef __INTEL_COMPILER
#pragma warning(disable:1098) //the qualifier on this friend declaration is ignored
#endif

namespace dev
{

namespace dvm
{
class LogFilter;
}

namespace dvm
{

/// Simple stream output for the StateDiff.
std::ostream& operator<<(std::ostream& _out, dev::dvm::LogFilter const& _s);

class State;
class Block;

class LogFilter
{
public:
	LogFilter(h256 _earliest = EarliestBlockHash, h256 _latest = PendingBlockHash): m_earliest(_earliest), m_latest(_latest) {}

	void streamRLP(RLPStream& _s) const;
	h256 sha3() const;

	/// hash of earliest block which should be filtered
	h256 earliest() const { return m_earliest; }

	/// hash of latest block which should be filtered
	h256 latest() const { return m_latest; }

	/// Range filter is a filter which doesn't care about addresses or topics
	/// Matches are all entries from earliest to latest
	/// @returns true if addresses and topics are unspecified
	bool isRangeFilter() const;

	/// @returns bloom possibilities for all addresses and topics
	std::vector<LogBloom> bloomPossibilities() const;

	bool matches(LogBloom _bloom) const;
	bool matches(Block const& _b, unsigned _i) const;
	LogEntries matches(TransactionReceipt const& _r) const;

	LogFilter address(Address _a) { m_addresses.insert(_a); return *this; }
	LogFilter topic(unsigned _index, h256 const& _t) { if (_index < 4) m_topics[_index].insert(_t); return *this; }
	LogFilter withEarliest(h256 _e) { m_earliest = _e; return *this; }
	LogFilter withLatest(h256 _e) { m_latest = _e; return *this; }

	friend std::ostream& dev::dvm::operator<<(std::ostream& _out, dev::dvm::LogFilter const& _s);

private:
	AddressHash m_addresses;
	std::array<h256Hash, 4> m_topics;
	h256 m_earliest = EarliestBlockHash;
	h256 m_latest = PendingBlockHash;
};

}

}
