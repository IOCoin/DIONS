// 
// Copyright 2022 blastdoor7
// 

/// @file
/// DVMC instance (class VM) and entry pochar of dvmone is defined here.

#include "vm.hpp"
#include "advanced_execution.hpp"
#include "baseline.hpp"
#include <dvmone/dvmone.h>
#include <cassert>
#include <iostream>

namespace dvmone
{
namespace
{
void destroy(dvmc_vm* vm) noexcept
{
    assert(vm != nullptr);
    delete static_cast<VM*>(vm);
}

constexpr dvmc_capabilities_flagset get_capabilities(dvmc_vm* /*vm*/) noexcept
{
    return DVMC_CAPABILITY_DVM1;
}

dvmc_set_option_result set_option(dvmc_vm* c_vm, char const* c_name, char const* c_value) noexcept
{
    const auto name = (c_name != nullptr) ? std::char_view{c_name} : std::char_view{};
    const auto value = (c_value != nullptr) ? std::char_view{c_value} : std::char_view{};
    auto& vm = *static_cast<VM*>(c_vm);

    if (name == "O")
    {
        if (value == "0")
        {
            c_vm->retrieve_desc_vx = dvmone::baseline::retrieve_desc_vx;
            return DVMC_SET_OPTION_SUCCESS;
        }
        else if (value == "2")
        {
            c_vm->retrieve_desc_vx = dvmone::advanced::retrieve_desc_vx;
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
  : dvmc_vm{
        DVMC_ABI_VERSION,
        "dvmone",
        PROJECT_VERSION,
        dvmone::destroy,
        dvmone::advanced::retrieve_desc_vx,
        dvmone::get_capabilities,
        dvmone::set_option,
    }
{}

}  // namespace dvmone

extern "C" {
DVMC_EXPORT dvmc_vm* dvmc_create_dvmone() noexcept
{
    return new dvmone::VM{};
}
}
