// Aldvm: Ethereum C++ client, tools and libraries.
// Copyright 2014-2019 Aldvm Authors.
// Licensed under the GNU General Public License, Version 3.
#include "DVMC.h"

#include <libdevcore/Log.h>
#include <libdvm/VMFactory.h>

namespace dev
{
namespace dvm
{
namespace
{
dvmc_revision toRevision(DVMSchedule const& _schedule) noexcept
{
    if (_schedule.haveChainID)
        return DVMC_ISTANBUL;
    if (_schedule.haveCreate2 && !_schedule.eip1283Mode)
        return DVMC_PETERSBURG;
    if (_schedule.haveCreate2 && _schedule.eip1283Mode)
        return DVMC_CONSTANTINOPLE;
    if (_schedule.haveRevert)
        return DVMC_BYZANTIUM;
    if (_schedule.eip158Mode)
        return DVMC_SPURIOUS_DRAGON;
    if (_schedule.eip150Mode)
        return DVMC_TANGERINE_WHISTLE;
    if (_schedule.haveDelegateCall)
        return DVMC_HOMESTEAD;
    return DVMC_FRONTIER;
}
}  // namespace

DVMC::DVMC(dvmc_vm* _vm, std::vector<std::pair<std::string, std::string>> const& _options) noexcept
  : dvmc::VM(_vm)
{
    assert(_vm != nullptr);
    assert(is_abi_compatible());

    // Set the options.
    for (auto& pair : _options)
    {
        auto result = set_option(pair.first.c_str(), pair.second.c_str());
        switch (result)
        {
        case DVMC_SET_OPTION_SUCCESS:
            break;
        case DVMC_SET_OPTION_INVALID_NAME:
            cwarn << "Unknown DVMC option '" << pair.first << "'";
            break;
        case DVMC_SET_OPTION_INVALID_VALUE:
            cwarn << "Invalid value '" << pair.second << "' for DVMC option '" << pair.first << "'";
            break;
        default:
            cwarn << "Unknown error when setting DVMC option '" << pair.first << "'";
        }
    }
}

owning_bytes_ref DVMC::exec(u256& io_gas, ExtVMFace& _ext, const OnOpFunc& _onOp)
{
    assert(_ext.envInfo().number() >= 0);
    assert(_ext.envInfo().timestamp() >= 0);

    constexpr int64_t int64max = std::numeric_limits<int64_t>::max();

    // TODO: The following checks should be removed by changing the types
    //       used for gas, block number and timestamp.
    (void)int64max;
    assert(io_gas <= int64max);
    assert(_ext.envInfo().gasLimit() <= int64max);
    assert(_ext.depth <= static_cast<size_t>(std::numeric_limits<int32_t>::max()));

    auto gas = static_cast<int64_t>(io_gas);

    auto mode = toRevision(_ext.dvmSchedule());
    dvmc_call_kind kind = _ext.isCreate ? DVMC_CREATE : DVMC_CALL;
    uint32_t flags = _ext.staticCall ? DVMC_STATIC : 0;
    assert(flags != DVMC_STATIC || kind == DVMC_CALL);  // STATIC implies a CALL.
    dvmc_message msg = {kind, flags, static_cast<int32_t>(_ext.depth), gas, toDvmC(_ext.myAddress),
        toDvmC(_ext.caller), _ext.data.data(), _ext.data.size(), toDvmC(_ext.value),
        toDvmC(0x0_cppui256)};
    DvmCHost host{_ext};
    auto r = execute(host, mode, msg, _ext.code.data(), _ext.code.size());
    // FIXME: Copy the output for now, but copyless version possible.
    auto output = owning_bytes_ref{{&r.output_data[0], &r.output_data[r.output_size]}, 0, r.output_size};

    switch (r.status_code)
    {
    case DVMC_SUCCESS:
        io_gas = r.gas_left;
        return output;

    case DVMC_REVERT:
        io_gas = r.gas_left;
        throw RevertInstruction{std::move(output)};

    case DVMC_OUT_OF_GAS:
    case DVMC_FAILURE:
        BOOST_THROW_EXCEPTION(OutOfGas());

    case DVMC_INVALID_INSTRUCTION:  // NOTE: this could have its own exception
    case DVMC_UNDEFINED_INSTRUCTION:
        BOOST_THROW_EXCEPTION(BadInstruction());

    case DVMC_BAD_JUMP_DESTINATION:
        BOOST_THROW_EXCEPTION(BadJumpDestination());

    case DVMC_STACK_OVERFLOW:
        BOOST_THROW_EXCEPTION(OutOfStack());

    case DVMC_STACK_UNDERFLOW:
        BOOST_THROW_EXCEPTION(StackUnderflow());

    case DVMC_INVALID_MEMORY_ACCESS:
        BOOST_THROW_EXCEPTION(BufferOverrun());

    case DVMC_STATIC_MODE_VIOLATION:
        BOOST_THROW_EXCEPTION(DisallowedStateChange());

    case DVMC_REJECTED:
        cwarn << "Execution rejected by DVMC, executing with default VM implementation";
        return VMFactory::create(VMKind::Legacy)->exec(io_gas, _ext, _onOp);

    case DVMC_INTERNAL_ERROR:
    default:
        if (r.status_code <= DVMC_INTERNAL_ERROR)
            BOOST_THROW_EXCEPTION(InternalVMError{} << errinfo_dvmcStatusCode(r.status_code));
        else
            // These cases aren't really internal errors, just more specific
            // error codes returned by the VM. Map all of them to OOG.
            BOOST_THROW_EXCEPTION(OutOfGas());
    }
}
}  // namespace dvm
}  // namespace dev
