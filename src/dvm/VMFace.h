// Aldvm: Ethereum C++ client, tools and libraries.
// Copyright 2014-2019 Aldvm Authors.
// Licensed under the GNU General Public License, Version 3.
#pragma once

#include "ExtVMFace.h"
#include <libdevcore/Exceptions.h>
#include <memory>

namespace dev
{
namespace dvm
{

struct VMException: Exception {};
#define DVM_SIMPLE_EXCEPTION_VM(X) struct X: VMException { const char* what() const noexcept override { return #X; } }
DVM_SIMPLE_EXCEPTION_VM(InvalidInstruction);
DVM_SIMPLE_EXCEPTION_VM(BadInstruction);
DVM_SIMPLE_EXCEPTION_VM(BadJumpDestination);
DVM_SIMPLE_EXCEPTION_VM(OutOfGas);
DVM_SIMPLE_EXCEPTION_VM(OutOfStack);
DVM_SIMPLE_EXCEPTION_VM(StackUnderflow);
DVM_SIMPLE_EXCEPTION_VM(DisallowedStateChange);
DVM_SIMPLE_EXCEPTION_VM(BufferOverrun);

/// Reports VM internal error. This is not based on VMException because it must be handled
/// differently than defined consensus exceptions.
struct InternalVMError : Exception {};

/// Error info for DVMC status code.
using errinfo_dvmcStatusCode = boost::error_info<struct tag_dvmcStatusCode, dvmc_status_code>;

struct RevertInstruction: VMException
{
	explicit RevertInstruction(owning_bytes_ref&& _output) : m_output(std::move(_output)) {}
	RevertInstruction(RevertInstruction const&) = delete;
	RevertInstruction(RevertInstruction&&) = default;
	RevertInstruction& operator=(RevertInstruction const&) = delete;
	RevertInstruction& operator=(RevertInstruction&&) = default;

	char const* what() const noexcept override { return "Revert instruction"; }

	owning_bytes_ref&& output() { return std::move(m_output); }

private:
	owning_bytes_ref m_output;
};


/// DVM Virtual Machine interface
class VMFace
{
public:
	VMFace() = default;
	virtual ~VMFace() = default;
	VMFace(VMFace const&) = delete;
	VMFace& operator=(VMFace const&) = delete;

	/// VM implementation
	virtual owning_bytes_ref exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp) = 0;
};

/// Helpers:

// Convert from a 256-bit integer stack/memory entry into a 160-bit Address hash.
// Currently we just pull out the right (low-order in BE) 160-bits.
inline Address asAddress(u256 _item)
{
	return right160(h256(_item));
}

inline u256 fromAddress(Address _a)
{
	return (u160)_a;
}

// Checks whdvmer address is in the address range for precompiles according to EIP-1352
inline bool isPrecompiledContract(Address const& _addr) noexcept
{
    static Address const c_maxPrecompiledAddress{0xffff};
    return _addr <= c_maxPrecompiledAddress;
}
}
}
