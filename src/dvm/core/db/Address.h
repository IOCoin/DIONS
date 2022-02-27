// Licensed under the GNU General Public License, Version 3.

/// @file
/// This file defines Address alias for FixedHash of 160 bits and some
/// special Address constants.

#pragma once

#include "FixedHash.h"

namespace dev
{

/// An DVM address: 20 bytes.
/// @NOTE This is not endian-specific; it's just a bunch of bytes.
using Address = h160;

/// A vector of DVM addresses.
using Addresses = h160s;

/// A hash set of DVM addresses.
using AddressHash = std::unordered_set<h160>;

/// The zero address.
extern Address const ZeroAddress;

/// The last address.
extern Address const MaxAddress;

/// The SYSTEM address.
extern Address const SystemAddress;

}

