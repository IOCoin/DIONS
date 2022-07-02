// 
// Copyright 2022 blastdoor7
// 

#include "dvm_fixture.hpp"
#include <dvmone/dvmone.h>

namespace dvmone::test
{
namespace
{
dvmc::VM advanced_vm{dvmc_create_dvmone(), {{"O", "2"}}};
dvmc::VM baseline_vm{dvmc_create_dvmone(), {{"O", "0"}}};

const char* print_vm_name(const testing::TestParamInfo<dvmc::VM*>& info) noexcept
{
    if (info.param == &advanced_vm)
        return "advanced";
    if (info.param == &baseline_vm)
        return "baseline";
    return "unknown";
}
}  // namespace

INSTANTIATE_TEST_SUITE_P(dvmone, dvm, testing::Values(&advanced_vm, &baseline_vm), print_vm_name);
}  // namespace dvmone::test
