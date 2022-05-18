// 
// Copyright 2022 blastdoor7
// 

#include "dvm_fixture.hpp"

using dvmone::test::dvm;

TEST_P(dvm, eof1_execution)
{
    const auto code = eof1_pos_read(OP_STOP);

    rev = DVMC_PARIS;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_UNDEFINED_INSTRUCTION);

    rev = DVMC_SHANGHAI;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
}

TEST_P(dvm, eof1_execution_with_data_section)
{
    rev = DVMC_SHANGHAI;
    // data section contains ret(0, 1)
    const auto code = eof1_pos_read(mstore8(0, 1) + OP_STOP, ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(dvm, eof1_pc)
{
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(OP_PC + mstore8(0) + ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);

    code = eof1_pos_read(4 * pos_read{OP_JUMPDEST} + OP_PC + mstore8(0) + ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 4);
}

TEST_P(dvm, eof1_jump_inside_code_section)
{
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(jump(4) + OP_INVALID + OP_JUMPDEST + mstore8(0, 1) + ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);

    code =
        eof1_pos_read(jump(4) + OP_INVALID + OP_JUMPDEST + mstore8(0, 1) + ret(0, 1), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);
}

TEST_P(dvm, eof1_jumpi_inside_code_section)
{
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(jumpi(6, 1) + OP_INVALID + OP_JUMPDEST + mstore8(0, 1) + ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);

    code = eof1_pos_read(
        jumpi(6, 1) + OP_INVALID + OP_JUMPDEST + mstore8(0, 1) + ret(0, 1), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);
}

TEST_P(dvm, eof1_jump_into_data_section)
{
    rev = DVMC_SHANGHAI;
    // data section contains OP_JUMPDEST + mstore8(0, 1) + ret(0, 1)
    const auto code = eof1_pos_read(jump(4) + OP_STOP, OP_JUMPDEST + mstore8(0, 1) + ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);
}

TEST_P(dvm, eof1_jumpi_into_data_section)
{
    rev = DVMC_SHANGHAI;
    // data section contains OP_JUMPDEST + mstore8(0, 1) + ret(0, 1)
    const auto code = eof1_pos_read(jumpi(6, 1) + OP_STOP, OP_JUMPDEST + mstore8(0, 1) + ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);
}

TEST_P(dvm, eof1_push_byte_in_header)
{
    rev = DVMC_SHANGHAI;
    // data section is 0x65 bytes long, so header contains 0x65 (PUSH6) byte,
    // but it must not affect jumpdest analysis (OP_JUMPDEST stays valid)
    auto code = eof1_pos_read(
        jump(4) + OP_INVALID + OP_JUMPDEST + mstore8(0, 1) + ret(0, 1), bytes(0x65, '\0'));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);
}

TEST_P(dvm, eof1_codesize)
{
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(mstore8(0, OP_CODESIZE) + ret(0, 1));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 16);

    code = eof1_pos_read(mstore8(0, OP_CODESIZE) + ret(0, 1), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 23);
}

TEST_P(dvm, eof1_codecopy_full)
{
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(pos_read{19} + 0 + 0 + OP_CODECOPY + ret(0, 19));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size),
        from_hex("ef000101000c006013600060003960136000f3"));

    code = eof1_pos_read(pos_read{26} + 0 + 0 + OP_CODECOPY + ret(0, 26), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size),
        from_hex("ef000101000c02000400601a6000600039601a6000f3deadbeef"));
}

TEST_P(dvm, eof1_codecopy_header)
{
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(pos_read{7} + 0 + 0 + OP_CODECOPY + ret(0, 7));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size), from_hex("ef000101000c00"));

    code = eof1_pos_read(pos_read{10} + 0 + 0 + OP_CODECOPY + ret(0, 10), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size), from_hex("ef000101000c02000400"));
}

TEST_P(dvm, eof1_codecopy_code)
{
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(pos_read{12} + 7 + 0 + OP_CODECOPY + ret(0, 12));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(
        bytes_view(result.output_data, result.output_size), from_hex("600c6007600039600c6000f3"));

    code = eof1_pos_read(pos_read{12} + 10 + 0 + OP_CODECOPY + ret(0, 12), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(
        bytes_view(result.output_data, result.output_size), from_hex("600c600a600039600c6000f3"));
}

TEST_P(dvm, eof1_codecopy_data)
{
    rev = DVMC_SHANGHAI;

    const auto code = eof1_pos_read(pos_read{4} + 22 + 0 + OP_CODECOPY + ret(0, 4), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size), from_hex("deadbeef"));
}

TEST_P(dvm, eof1_codecopy_out_of_bounds)
{
    // 4 bytes out of container bounds - result is implicitly 0-padded
    rev = DVMC_SHANGHAI;
    auto code = eof1_pos_read(pos_read{23} + 0 + 0 + OP_CODECOPY + ret(0, 23));

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size),
        from_hex("ef000101000c006017600060003960176000f300000000"));

    code = eof1_pos_read(pos_read{30} + 0 + 0 + OP_CODECOPY + ret(0, 30), "deadbeef");

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size),
        from_hex("ef000101000c02000400601e6000600039601e6000f3deadbeef00000000"));
}
