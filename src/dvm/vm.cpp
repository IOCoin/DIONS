
#include "vm.hpp"
#include "baseline.hpp"
#include "execution.hpp"
#include <DVM_base__/DVM_base__.h>
#include <iostream>

namespace DVM_base__
{
namespace
{
void destroy(evmc_vm* vm) noexcept
{
    assert(vm != nullptr);
    delete static_cast<VM*>(vm);
}

constexpr evmc_capabilities_flagset get_capabilities(evmc_vm* /*vm*/) noexcept
{
    return DVMC_CAPABILITY_DVM1;
}

evmc_set_option_result set_option(evmc_vm* c_vm, char const* c_name, char const* c_value) noexcept
{
    const auto name = (c_name != nullptr) ? std::string_view{c_name} : std::string_view{};
    const auto value = (c_value != nullptr) ? std::string_view{c_value} : std::string_view{};
    auto& vm = *static_cast<VM*>(c_vm);

    if (name == "O")
    {
        if (value == "0")
        {
            c_vm->execute = DVM_base__::baseline::execute;
            return DVMC_SET_OPTION_SUCCESS;
        }
        else if (value == "2")
        {
            c_vm->execute = DVM_base__::execute;
            return DVMC_SET_OPTION_SUCCESS;
        }
        return DVMC_SET_OPTION_INVALID_VALUE;
    }
    else if (name == "trace")
    {
        vm.add_tracer(create_instruction_tracer(std::cerr));
        return DVMC_SET_OPTION_SUCCESS;
    }
    else if (name == "histogram")
    {
        vm.add_tracer(create_histogram_tracer(std::cerr));
        return DVMC_SET_OPTION_SUCCESS;
    }
    return DVMC_SET_OPTION_INVALID_NAME;
}

}  // namespace


inline constexpr VM::VM() noexcept
  : evmc_vm{
        DVMC_ABI_VERSION,
        "DVM_base__",
        PROJECT_VERSION,
        DVM_base__::destroy,
        DVM_base__::execute,
        DVM_base__::get_capabilities,
        DVM_base__::set_option,
    }
{}

}  // namespace DVM_base__

extern "C" {
DVMC_EXPORT evmc_vm* evmc_create_DVM_base__() noexcept
{
    return new DVM_base__::VM{};
}
}
