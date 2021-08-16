// Licensed under the GNU General Public License, Version 3.

#pragma once

#include <libdevcore/TrieDB.h>

namespace dev
{
namespace dvm
{
#if DVM_FATDB
template <class KeyType, class DB>
using SecureTrieDB = SpecificTrieDB<FatGenericTrieDB<DB>, KeyType>;
#else
template <class KeyType, class DB>
using SecureTrieDB = SpecificTrieDB<HashedGenericTrieDB<DB>, KeyType>;
#endif

}  // namespace dvm
}  // namespace dev
