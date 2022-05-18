// 
// Copyright 2022 blastdoor7
// 

#include <dvmc/dvmc.hpp>
#include <dvmone/dvmone.h>
#include <gtest/gtest.h>

TEST(dvmone, info)
{
    auto vm = dvmc::VM{dvmc_create_dvmone()};
    EXPECT_STREQ(vm.name(), "dvmone");
    EXPECT_STREQ(vm.version(), PROJECT_VERSION);
    EXPECT_TRUE(vm.is_abi_compatible());
}

TEST(dvmone, capabilities)
{
    auto vm = dvmc_create_dvmone();
    EXPECT_EQ(vm->get_capabilities(vm), dvmc_capabilities_flagset{DVMC_CAPABILITY_DVM1});
    vm->destroy(vm);
}

TEST(dvmone, set_option_invalid)
{
    auto vm = dvmc_create_dvmone();
    ASSERT_NE(vm->set_option, nullptr);
    EXPECT_EQ(vm->set_option(vm, "", ""), DVMC_SET_OPTION_INVALID_NAME);
    EXPECT_EQ(vm->set_option(vm, "o", ""), DVMC_SET_OPTION_INVALID_NAME);
    EXPECT_EQ(vm->set_option(vm, "0", ""), DVMC_SET_OPTION_INVALID_NAME);
    vm->destroy(vm);
}

TEST(dvmone, set_option_optimization_level)
{
    auto vm = dvmc::VM{dvmc_create_dvmone()};
    EXPECT_EQ(vm.set_option("O", ""), DVMC_SET_OPTION_INVALID_VALUE);
    EXPECT_EQ(vm.set_option("O", "0"), DVMC_SET_OPTION_SUCCESS);
    EXPECT_EQ(vm.set_option("O", "1"), DVMC_SET_OPTION_INVALID_VALUE);
    EXPECT_EQ(vm.set_option("O", "2"), DVMC_SET_OPTION_SUCCESS);
    EXPECT_EQ(vm.set_option("O", "3"), DVMC_SET_OPTION_INVALID_VALUE);

    EXPECT_EQ(vm.set_option("O", "20"), DVMC_SET_OPTION_INVALID_VALUE);
    EXPECT_EQ(vm.set_option("O", "21"), DVMC_SET_OPTION_INVALID_VALUE);
    EXPECT_EQ(vm.set_option("O", "22"), DVMC_SET_OPTION_INVALID_VALUE);
}
