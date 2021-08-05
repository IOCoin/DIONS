// Licensed under the GNU General Public License, Version 3.


#pragma once

#include <unordered_map>
#include <functional>
#include <libdevcore/CommonData.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace dvm
{
struct ChainOperationParams;

using PrecompiledExecutor = std::function<std::pair<bool, bytes>(bytesConstRef _in)>;
using PrecompiledPricer = std::function<bigint(
    bytesConstRef _in, ChainOperationParams const& _chainParams, u256 const& _blockNumber)>;

DEV_SIMPLE_EXCEPTION(ExecutorNotFound);
DEV_SIMPLE_EXCEPTION(PricerNotFound);

class PrecompiledRegistrar
{
public:
    /// Get the executor object for @a _name function or @throw ExecutorNotFound if not found.
    static PrecompiledExecutor const& executor(std::string const& _name);

    /// Get the price calculator object for @a _name function or @throw PricerNotFound if not found.
    static PrecompiledPricer const& pricer(std::string const& _name);

    /// Register an executor. In general just use DVM_REGISTER_PRECOMPILED.
    static PrecompiledExecutor registerExecutor(std::string const& _name, PrecompiledExecutor const& _exec) { return (get()->m_execs[_name] = _exec); }
    /// Unregister an executor. Shouldn't generally be necessary.
    static void unregisterExecutor(std::string const& _name) { get()->m_execs.erase(_name); }

    /// Register a pricer. In general just use DVM_REGISTER_PRECOMPILED_PRICER.
    static PrecompiledPricer registerPricer(std::string const& _name, PrecompiledPricer const& _exec) { return (get()->m_pricers[_name] = _exec); }
    /// Unregister a pricer. Shouldn't generally be necessary.
    static void unregisterPricer(std::string const& _name) { get()->m_pricers.erase(_name); }

private:
    static PrecompiledRegistrar* get() { if (!s_this) s_this = new PrecompiledRegistrar; return s_this; }

    std::unordered_map<std::string, PrecompiledExecutor> m_execs;
    std::unordered_map<std::string, PrecompiledPricer> m_pricers;
    static PrecompiledRegistrar* s_this;
};

// TODO: unregister on unload with a static object.
#define DVM_REGISTER_PRECOMPILED(Name) static std::pair<bool, bytes> __dvm_registerPrecompiledFunction ## Name(bytesConstRef _in); static PrecompiledExecutor __dvm_registerPrecompiledFactory ## Name = ::dev::dvm::PrecompiledRegistrar::registerExecutor(#Name, &__dvm_registerPrecompiledFunction ## Name); static std::pair<bool, bytes> __dvm_registerPrecompiledFunction ## Name
#define DVM_REGISTER_PRECOMPILED_PRICER(Name)                                                   \
    static bigint __dvm_registerPricerFunction##Name(                                           \
        bytesConstRef _in, ChainOperationParams const& _chainParams, u256 const& _blockNumber); \
    static PrecompiledPricer __dvm_registerPricerFactory##Name =                                \
        ::dev::dvm::PrecompiledRegistrar::registerPricer(                                       \
            #Name, &__dvm_registerPricerFunction##Name);                                        \
    static bigint __dvm_registerPricerFunction##Name
}
}
