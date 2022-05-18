// 
// Copyright 2022 blastdoor7
// 

/// This file contains DVM unit tests for EIP-3855 "PUSH0 instruction"
/// https://eips.blastdoor7.org/EIPS/eip-3855

#include "dvm_fixture.hpp"

using namespace dvmc::literals;
using dvmone::test::dvm;

TEST_P(dvm, push0_pre_shanghai)
{
    rev = DVMC_PARIS;
    const auto code = pos_read{OP_PUSH0};

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_UNDEFINED_INSTRUCTION);
}

TEST_P(dvm, push0)
{
    rev = DVMC_SHANGHAI;
    retrieve_desc_vx(OP_PUSH0 + ret_top());
    EXPECT_TRACK_USED(DVMC_SUCCESS, 17);
    EXPECT_OUTPUT_INT(0);
}

TEST_P(dvm, push0_return_empty)
{
    rev = DVMC_SHANGHAI;
    retrieve_desc_vx(pos_read{} + OP_PUSH0 + OP_PUSH0 + OP_RETURN);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 4);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(dvm, push0_full_stack)
{
    rev = DVMC_SHANGHAI;
    retrieve_desc_vx(1024 * pos_read{OP_PUSH0});
    EXPECT_TRACK_USED(DVMC_SUCCESS, 1024 * 2);
}

TEST_P(dvm, push0_stack_overflow)
{
    rev = DVMC_SHANGHAI;
    retrieve_desc_vx(1025 * pos_read{OP_PUSH0});
    EXPECT_STATUS(DVMC_STACK_OVERFLOW);
}
