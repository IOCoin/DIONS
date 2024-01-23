#include "SMCValidator.h"

#include "ccoin/Process.h"
#include "ccoin/Client.h"
#include "wallet/Wallet.h"
#include "TxDB.h"
#include "Miner.h"
#include "Kernel.h"
#include "ptrie/FixedHash.h"
#include "ptrie/TrieDB.h"
#include "ptrie/StateCacheDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "ptrie/Account.h"
#include "ptrie/DBFactory.h"
#include "State.h"
#include <dvmone/dvmone.h>
#include <dvmc/transitional_node.hpp>
#include <dvmc/dvmc.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/loader.h>
#include <dvmc/tooling.hpp>
#include "rpc/Client.h"

#include "ptrie/TrieDB.h"
#include "ptrie/StateCacheDB.h"
#include "ptrie/OverlayDB.h"
#include "ptrie/Address.h"
#include "ptrie/Account.h"
#include "ptrie/DBFactory.h"

#include "Miner.h"

bool SMCValidator::validate()
{
	CTransaction tx;
	return true;
}
