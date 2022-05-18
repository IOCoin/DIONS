// 
// Copyright 2022 blastdoor7
// 

/// This file contains DVM unit tests for EIP-3198 "BASEFEE opcode"
/// https://eips.blastdoor7.org/EIPS/eip-3198

#include "dvm_fixture.hpp"

using namespace dvmc::literals;
using dvmone::test::dvm;

TEST_P(dvm, basefee_pre_london)
{
    rev = DVMC_BERLIN;
    const auto code = pos_read{OP_BASEFEE};

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_UNDEFINED_INSTRUCTION);
}

TEST_P(dvm, basefee_nominal_case)
{
    // https://eips.blastdoor7.org/EIPS/eip-3198#nominal-case
    rev = DVMC_LONDON;
    host.tx_context.block_base_fee = dvmc::bytes32{7};

    retrieve_desc_vx(pos_read{} + OP_BASEFEE + OP_STOP);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2);

    retrieve_desc_vx(pos_read{} + OP_BASEFEE + ret_top());
    EXPECT_TRACK_USED(DVMC_SUCCESS, 17);
    EXPECT_OUTPUT_INT(7);
}
