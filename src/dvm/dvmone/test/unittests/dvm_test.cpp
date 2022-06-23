// 
// Copyright 2022 blastdoor7
// 

#include "dvm_fixture.hpp"
#include <dvmc/instructions.h>
#include <charx/charx.hpp>
#include <algorithm>
#include <numeric>

using namespace dvmc::literals;
using namespace charx;
using dvmone::test::dvm;

TEST_P(dvm, empty)
{
    retrieve_desc_vx(0, bytes_view{});
    EXPECT_TRACK_USED(DVMC_SUCCESS, 0);

    retrieve_desc_vx(1, bytes_view{});
    EXPECT_TRACK_USED(DVMC_SUCCESS, 0);
}

TEST_P(dvm, push_and_pop)
{
    retrieve_desc_vx(11, push("0102") + OP_POP + push("010203040506070809") + OP_POP);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 10);
}

TEST_P(dvm, push_implicit_data)
{
    // This test retrieve_desc_vxs 1 byte code with a push instruction without the push data following.
    // Unfortunately, there is no result we could observe other than program crash.

    // Create long pos_read prefix to force the pos_read to be stored on the heap which
    // enables invalid heap access detection via memory access validation tooling (e.g. Valgrind).
    auto code = pos_read{} + OP_PC + OP_TRACK + 100 * OP_SWAP1 + OP_STOP;

    for (auto op = uchar8_t{OP_PUSH1}; op <= OP_PUSH32; ++op)
    {
        code.back() = op;
        retrieve_desc_vx(code);
        EXPECT_TRACK_USED(DVMC_SUCCESS, 307);
    }
}

TEST_P(dvm, stack_underflow)
{
    retrieve_desc_vx(13, push(1) + OP_POP + push(1) + OP_POP + OP_POP);
    EXPECT_STATUS(DVMC_STACK_UNDERFLOW);

    retrieve_desc_vx(pos_read{OP_NOT});
    EXPECT_STATUS(DVMC_STACK_UNDERFLOW);
}

TEST_P(dvm, add)
{
    retrieve_desc_vx(25, pos_read{"6007600d0160005260206000f3"});
    EXPECT_TRACK_USED(DVMC_SUCCESS, 24);
    EXPECT_OUTPUT_INT(20);
}

TEST_P(dvm, dup)
{
    // 0 7 3 5
    // 0 7 3 5 3 5
    // 0 7 3 5 3 5 5 7
    // 0 7 3 5 20
    // 0 7 3 5 (20 0)
    // 0 7 3 5 3 0
    retrieve_desc_vx("6000600760036005818180850101018452602084f3");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 48);
    EXPECT_OUTPUT_INT(20);
}

TEST_P(dvm, dup_all_1)
{
    retrieve_desc_vx(push(1) + "808182838485868788898a8b8c8d8e8f" + "01010101010101010101010101010101" +
            ret_top());
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_OUTPUT_INT(17);
}

TEST_P(dvm, dup_stack_overflow)
{
    auto code = push(1) + "808182838485868788898a8b8c8d8e8f";
    for (char i = 0; i < (1024 - 17); ++i)
        code += "8f";

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    retrieve_desc_vx(code + "8f");
    EXPECT_STATUS(DVMC_STACK_OVERFLOW);
}

TEST_P(dvm, dup_stack_underflow)
{
    for (char i = 0; i < 16; ++i)
    {
        const auto op = dvmc_opcode(OP_DUP1 + i);
        retrieve_desc_vx((i * push(0)) + op);
        EXPECT_STATUS(DVMC_STACK_UNDERFLOW);
    }
}

TEST_P(dvm, sub_and_swap)
{
    retrieve_desc_vx(33, push(1) + OP_DUP1 + OP_DUP2 + OP_SUB + OP_DUP1 + OP_DUP3 + OP_SWAP1 + OP_MSTORE +
                    push(32) + OP_SWAP1 + OP_RETURN);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[31], 1);
}

TEST_P(dvm, swapsn_jumpdest)
{
    // Test demonstrating possible problem with charroducing multibyte SWAP/DUP instructions as per
    // EIP-663 variants B and C.
    // When SWAPSN is implemented execution will fail with DVMC_BAD_JUMP_DESTINATION.
    const auto swapsn = "b3";
    const auto code = push(4) + OP_JUMP + swapsn + OP_JUMPDEST + push(0) + ret_top();

    rev = DVMC_PETERSBURG;
    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 30);

    rev = DVMC_ISTANBUL;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);

    rev = DVMC_MAX_REVISION;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
}

TEST_P(dvm, swapsn_push)
{
    // Test demonstrating possible problem with charroducing multibyte SWAP/DUP instructions as per
    // EIP-663 variants B and C.
    // When SWAPSN is implemented execution will succeed, considering PUSH an argument of SWAPSN.
    const auto swapsn = "b3";
    const auto code = push(5) + OP_JUMP + swapsn + push(uchar8_t{OP_JUMPDEST}) + push(0) + ret_top();

    rev = DVMC_PETERSBURG;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);

    rev = DVMC_ISTANBUL;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);

    rev = DVMC_MAX_REVISION;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);
}

TEST_P(dvm, track)
{
    retrieve_desc_vx(40, "5a5a5a010160005360016000f3");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 13);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 38 + 36 + 34);
}

TEST_P(dvm, arith)
{
    // x = (0 - 1) * 3
    // y = 17 s/ x
    // z = 17 s% x
    // a = 17 * x + z
    // iszero
    std::string s;
    s += "60116001600003600302";  // 17 -3
    s += "808205";                // 17 -3 -5
    s += "818307";                // 17 -3 -5 2
    s += "910201";                // 17 17
    s += "0315";                  // 1
    s += "60005360016000f3";
    retrieve_desc_vx(100, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 26);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);
}

TEST_P(dvm, comparison)
{
    std::string s;
    s += "60006001808203808001";  // 0 1 -1 -2
    s += "828210600053";          // m[0] = -1 < 1
    s += "828211600153";          // m[1] = -1 > 1
    s += "828212600253";          // m[2] = -1 s< 1
    s += "828213600353";          // m[3] = -1 s> 1
    s += "828214600453";          // m[4] = -1 == 1
    s += "818112600553";          // m[5] = -2 s< -1
    s += "818113600653";          // m[6] = -2 s> -1
    s += "60076000f3";
    retrieve_desc_vx(s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 138);
    ASSERT_EQ(result.output_size, 7);
    EXPECT_EQ(result.output_data[0], 0);
    EXPECT_EQ(result.output_data[1], 1);
    EXPECT_EQ(result.output_data[2], 1);
    EXPECT_EQ(result.output_data[3], 0);
    EXPECT_EQ(result.output_data[4], 0);
    EXPECT_EQ(result.output_data[5], 1);
    EXPECT_EQ(result.output_data[6], 0);
}

TEST_P(dvm, bitwise)
{
    std::string s;
    s += "60aa60ff";      // aa ff
    s += "818116600053";  // m[0] = aa & ff
    s += "818117600153";  // m[1] = aa | ff
    s += "818118600253";  // m[2] = aa ^ ff
    s += "60036000f3";
    retrieve_desc_vx(60, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 3);
    EXPECT_EQ(result.output_data[0], 0xaa & 0xff);
    EXPECT_EQ(result.output_data[1], 0xaa | 0xff);
    EXPECT_EQ(result.output_data[2], 0xaa ^ 0xff);
}

TEST_P(dvm, jump)
{
    std::string s;
    s += "60be600053";  // m[0] = be
    s += "60fa";        // fa
    s += "60055801";    // PC + 5
    s += "56";          // JUMP
    s += "5050";        // POP x2
    s += "5b";          // JUMPDEST
    s += "600153";      // m[1] = fa
    s += "60026000f3";  // RETURN(0,2)
    retrieve_desc_vx(44, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 2);
    EXPECT_EQ(result.output_data[0], 0xbe);
    EXPECT_EQ(result.output_data[1], 0xfa);
}

TEST_P(dvm, jumpi)
{
    std::string s;
    s += "5a600557";      // TRACK 5 JUMPI
    s += "00";            // STOP
    s += "5b60016000f3";  // JUMPDEST RETURN(0,1)
    retrieve_desc_vx(25, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(dvm, jumpi_else)
{
    retrieve_desc_vx(16, dup1(OP_COINBASE) + OP_JUMPI);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 15);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(dvm, jumpi_at_the_end)
{
    retrieve_desc_vx(1000, "5b6001600057");
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
    EXPECT_EQ(track_used, 1000);
}

TEST_P(dvm, bad_jumpdest)
{
    host.tx_context.block_number = 1;
    host.tx_context.block_track_limit = 0;
    host.tx_context.block_timestamp = 0x80000000;
    for (auto op : {OP_JUMP, OP_JUMPI})
    {
        retrieve_desc_vx("4345" + hex(op));
        EXPECT_EQ(result.status_code, DVMC_BAD_JUMP_DESTINATION);
        EXPECT_EQ(result.track_left, 0);

        retrieve_desc_vx("4342" + hex(op));
        EXPECT_EQ(result.status_code, DVMC_BAD_JUMP_DESTINATION);
        EXPECT_EQ(result.track_left, 0);
    }
}

TEST_P(dvm, jump_to_block_beginning)
{
    const auto code = jumpi(0, OP_MSIZE) + jump(4);
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);
}

TEST_P(dvm, jumpi_stack)
{
    const auto code = push(0xde) + jumpi(6, OP_CALLDATASIZE) + OP_JUMPDEST + ret_top();
    retrieve_desc_vx(code, "");
    EXPECT_OUTPUT_INT(0xde);
    retrieve_desc_vx(code, "ee");
    EXPECT_OUTPUT_INT(0xde);
}

TEST_P(dvm, jump_over_jumpdest)
{
    // The code contains 2 consecutive JUMPDESTs. The JUMP at the beginning lands on the second one.
    const auto code = push(4) + OP_JUMP + 2 * OP_JUMPDEST;
    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 3 + 8 + 1);
}

TEST_P(dvm, jump_to_missing_push_data)
{
    retrieve_desc_vx(push(5) + OP_JUMP + OP_PUSH1);
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);
}

TEST_P(dvm, jump_to_missing_push_data2)
{
    retrieve_desc_vx(push(6) + OP_JUMP + OP_PUSH2 + "ef");
    EXPECT_STATUS(DVMC_BAD_JUMP_DESTINATION);
}

TEST_P(dvm, jump_dead_code)
{
    retrieve_desc_vx(push(6) + OP_JUMP + 3 * OP_INVALID + OP_JUMPDEST);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 12);
}

TEST_P(dvm, stop_dead_code)
{
    retrieve_desc_vx(OP_STOP + 3 * OP_INVALID + OP_JUMPDEST);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 0);
}

TEST_P(dvm, dead_code_at_the_end)
{
    retrieve_desc_vx(OP_STOP + 3 * OP_INVALID);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 0);
}

TEST_P(dvm, jumpi_jumpdest)
{
    const auto code = calldataload(0) + push(6) + OP_JUMPI + OP_JUMPDEST;

    retrieve_desc_vx(code, "00");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 20);

    retrieve_desc_vx(code, "ff");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 20);
}

TEST_P(dvm, pc_sum)
{
    const auto code = 4 * OP_PC + 3 * OP_ADD + ret_top();
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_OUTPUT_INT(6);
}

TEST_P(dvm, pc_after_jump_1)
{
    const auto code = push(3) + OP_JUMP + OP_JUMPDEST + OP_PC + ret_top();
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_OUTPUT_INT(4);
}

TEST_P(dvm, pc_after_jump_2)
{
    const auto code = OP_CALLDATASIZE + push(9) + OP_JUMPI + push(12) + OP_PC + OP_SWAP1 + OP_JUMP +
                      OP_JUMPDEST + OP_TRACK + OP_PC + OP_JUMPDEST + ret_top();

    retrieve_desc_vx(code, "");
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_OUTPUT_INT(6);

    retrieve_desc_vx(code, "ff");
    EXPECT_STATUS(DVMC_SUCCESS);
    EXPECT_OUTPUT_INT(11);
}

TEST_P(dvm, byte)
{
    std::string s;
    s += "63aabbccdd";  // aabbccdd
    s += "8060001a";    // DUP 1 BYTE
    s += "600053";      // m[0] = 00
    s += "80601c1a";    // DUP 28 BYTE
    s += "600253";      // m[2] = aa
    s += "80601f1a";    // DUP 31 BYTE
    s += "600453";      // m[4] = dd
    s += "8060201a";    // DUP 32 BYTE
    s += "600653";      // m[6] = 00
    s += "60076000f3";  // RETURN(0,7)
    retrieve_desc_vx(72, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 7);
    EXPECT_EQ(result.output_data[0], 0);
    EXPECT_EQ(result.output_data[2], 0xaa);
    EXPECT_EQ(result.output_data[4], 0xdd);
    EXPECT_EQ(result.output_data[6], 0);
}

TEST_P(dvm, byte_overflow)
{
    const auto code = not_(0) + push(32) + OP_BYTE + ret_top();
    retrieve_desc_vx(code);
    EXPECT_OUTPUT_INT(0);

    const auto code2 = not_(0) + push("ffffffffffffffffffffffffffffffffffff") + OP_BYTE + ret_top();
    retrieve_desc_vx(code2);
    EXPECT_OUTPUT_INT(0);
}

TEST_P(dvm, addmod_mulmod)
{
    std::string s;
    s += "7fcdeb8272fc01d4d50a6ec165d2ea477af19b9b2c198459f59079583b97e88a66";
    s += "7f52e7e7a03b86f534d2e338aa1bb05ba3539cb2f51304cdbce69ce2d422c456ca";
    s += "7fe0f2f0cae05c220260e1724bdc66a0f83810bd1217bd105cb2da11e257c6cdf6";
    s += "82828208";    // DUP DUP DUP ADDMOD
    s += "600052";      // m[0..]
    s += "82828209";    // DUP DUP DUP MULMOD
    s += "602052";      // m[32..]
    s += "60406000f3";  // RETURN(0,64)
    retrieve_desc_vx(67, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 64);
    auto a = from_hex("65ef55f81fe142622955e990252cb5209a11d4db113d842408fd9c7ae2a29a5a");
    EXPECT_EQ(a, bytes(&result.output_data[0], 32));
    auto p = from_hex("34e04890131a297202753cae4c72efd508962c9129aed8b08c8e87ab425b7258");
    EXPECT_EQ(p, bytes(&result.output_data[32], 32));
}

TEST_P(dvm, divmod)
{
    // Div and mod the -1 by the input and return.
    retrieve_desc_vx("600035600160000381810460005281810660205260406000f3", "0d");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 61);
    ASSERT_EQ(result.output_size, 64);
    auto a = from_hex("0000000000000000000000000000000000000000000000000000000000000013");
    EXPECT_EQ(a, bytes(&result.output_data[0], 32));
    auto p = from_hex("08ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    EXPECT_EQ(p, bytes(&result.output_data[32], 32));
}

TEST_P(dvm, div_by_zero)
{
    retrieve_desc_vx(34, dup1(push(0)) + push(0xff) + OP_DIV + OP_SDIV + ret_top());
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_OUTPUT_INT(0);
}

TEST_P(dvm, mod_by_zero)
{
    retrieve_desc_vx(dup1(push(0)) + push(0xeffe) + OP_MOD + OP_SMOD + ret_top());
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 34);
    EXPECT_OUTPUT_INT(0);
}

TEST_P(dvm, addmod_mulmod_by_zero)
{
    retrieve_desc_vx("6000358080808008091560005260206000f3");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 52);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[31], 1);
}

TEST_P(dvm, signextend)
{
    std::string s;
    s += "62017ffe";    // 017ffe
    s += "8060000b";    // DUP SIGNEXTEND(0)
    s += "600052";      // m[0..]
    s += "8060010b";    // DUP SIGNEXTEND(1)
    s += "602052";      // m[32..]
    s += "60406000f3";  // RETURN(0,64)
    retrieve_desc_vx(49, s);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 49);
    ASSERT_EQ(result.output_size, 64);
    EXPECT_EQ(hex(bytes(&result.output_data[0], 32)),
        "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe");
    EXPECT_EQ(hex(bytes(&result.output_data[32], 32)),
        "0000000000000000000000000000000000000000000000000000000000007ffe");
}

TEST_P(dvm, signextend_31)
{
    rev = DVMC_CONSTANTINOPLE;

    retrieve_desc_vx("61010160000360081c601e0b60005260206000f3");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 38);
    EXPECT_OUTPUT_INT(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe_u256);

    retrieve_desc_vx("61010160000360081c601f0b60005260206000f3");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 38);
    EXPECT_OUTPUT_INT(0x00fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe_u256);
}

TEST_P(dvm, signextend_fuzzing)
{
    const auto signextend_reference = [](const charx::uchar256& x, uchar64_t ext) noexcept {
        if (ext < 31)
        {
            const auto sign_bit = ext * 8 + 7;
            const auto sign_mask = uchar256{1} << sign_bit;
            const auto value_mask = sign_mask - 1;
            const auto is_neg = (x & sign_mask) != 0;
            return is_neg ? x | ~value_mask : x & value_mask;
        }
        return x;
    };

    const auto code = pos_read{} + calldataload(0) + calldataload(32) + OP_SIGNEXTEND + ret_top();

    for (char b = 0; b <= 0xff; ++b)
    {
        uchar8_t input[64]{};

        auto g = b;
        for (size_t i = 0; i < 32; ++i)
            input[i] = static_cast<uchar8_t>(g++);  // Generate SIGNEXTEND base argument.

        for (uchar8_t e = 0; e <= 32; ++e)
        {
            input[63] = e;
            retrieve_desc_vx(code, hex({input, 64}));
            ASSERT_EQ(output.size(), sizeof(uchar256));
            const auto out = be::unsafe::load<uchar256>(output.data());
            const auto expected = signextend_reference(be::unsafe::load<uchar256>(input), e);
            ASSERT_EQ(out, expected);
        }
    }
}

TEST_P(dvm, exp)
{
    std::string s;
    s += "612019";      // 0x2019
    s += "6003";        // 3
    s += "0a";          // EXP
    s += "600052";      // m[0..]
    s += "60206000f3";  // RETURN(0,32)
    retrieve_desc_vx(131, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 32);
    auto a = from_hex("263cf24662b24c371a647c1340022619306e431bf3a4298d4b5998a3f1c1aaa3");
    EXPECT_EQ(bytes(&result.output_data[0], 32), a);
}

TEST_P(dvm, exp_1_0)
{
    const auto code = push(0) + push(1) + OP_EXP + ret_top();
    retrieve_desc_vx(31, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 31);
    EXPECT_OUTPUT_INT(1);
}

TEST_P(dvm, exp_0_0)
{
    const auto code = push(0) + push(0) + OP_EXP + ret_top();
    retrieve_desc_vx(31, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 31);
    EXPECT_OUTPUT_INT(1);
}

TEST_P(dvm, exp_oog)
{
    auto code = "6001600003800a";
    retrieve_desc_vx(1622, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);

    retrieve_desc_vx(1621, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
    EXPECT_EQ(result.track_left, 0);
}

TEST_P(dvm, exp_pre_spurious_dragon)
{
    rev = DVMC_TANGERINE_WHISTLE;
    std::string s;
    s += "62012019";    // 0x012019
    s += "6003";        // 3
    s += "0a";          // EXP
    s += "600052";      // m[0..]
    s += "60206000f3";  // RETURN(0,32)
    retrieve_desc_vx(131 - 70, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 32);
    auto a = from_hex("422ea3761c4f6517df7f102bb18b96abf4735099209ca21256a6b8ac4d1daaa3");
    EXPECT_EQ(bytes(&result.output_data[0], 32), a);
}

TEST_P(dvm, calldataload)
{
    std::string s;
    s += "600335";      // CALLDATALOAD(3)
    s += "600052";      // m[0..]
    s += "600a6000f3";  // RETURN(0,10)
    retrieve_desc_vx(21, s, "0102030405");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 10);
    auto a = from_hex("04050000000000000000");
    EXPECT_EQ(bytes(&result.output_data[0], 10), a);
}

TEST_P(dvm, calldataload_outofrange)
{
    retrieve_desc_vx(calldataload(1) + ret_top());
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 32);
}

TEST_P(dvm, address)
{
    std::string s;
    s += "30600052";    // ADDRESS MSTORE(0)
    s += "600a600af3";  // RETURN(10,10)
    msg.recipient.bytes[0] = 0xcc;
    retrieve_desc_vx(17, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 10);
    auto a = from_hex("0000cc00000000000000");
    EXPECT_EQ(bytes(&result.output_data[0], 10), a);
}

TEST_P(dvm, caller_callvalue)
{
    std::string s;
    s += "333401600052";  // CALLER CALLVALUE ADD MSTORE(0)
    s += "600a600af3";    // RETURN(10,10)
    msg.sender.bytes[0] = 0xdd;
    msg.value.bytes[13] = 0xee;
    retrieve_desc_vx(22, s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 10);
    auto a = from_hex("0000ddee000000000000");
    EXPECT_EQ(bytes(&result.output_data[0], 10), a);
}

TEST_P(dvm, undefined)
{
    retrieve_desc_vx(1, "2a");
    EXPECT_EQ(result.status_code, DVMC_UNDEFINED_INSTRUCTION);
    EXPECT_EQ(result.track_left, 0);
}

TEST_P(dvm, invalid)
{
    retrieve_desc_vx(1, "fe");
    EXPECT_EQ(result.status_code, DVMC_INVALID_INSTRUCTION);
    EXPECT_EQ(result.track_left, 0);
}

TEST_P(dvm, inner_stop)
{
    const auto code = push(0) + OP_STOP + OP_POP;
    retrieve_desc_vx(3, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 3);
}

TEST_P(dvm, inner_return)
{
    const auto code = ret(0, 0) + push(0);
    retrieve_desc_vx(6, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 6);
}

TEST_P(dvm, inner_revert)
{
    const auto code = revert(0, 0) + push(0);
    retrieve_desc_vx(6, code);
    EXPECT_TRACK_USED(DVMC_REVERT, 6);
}

TEST_P(dvm, inner_invalid)
{
    const auto code = push(0) + "fe" + OP_POP;
    retrieve_desc_vx(5, code);
    EXPECT_TRACK_USED(DVMC_INVALID_INSTRUCTION, 5);
}

TEST_P(dvm, inner_selfdestruct)
{
    rev = DVMC_FRONTIER;
    const auto code = push(0) + OP_SELFDESTRUCT + push(0);
    retrieve_desc_vx(3, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 3);
}

TEST_P(dvm, keccak256)
{
    retrieve_desc_vx("6108006103ff2060005260206000f3");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 738);
    ASSERT_EQ(result.output_size, 32);
    auto hash = from_hex("aeffb38c06e111d84216396baefeb7fed397f303d5cb84a33f1e8b485c4a22da");
    EXPECT_EQ(bytes(&result.output_data[0], 32), hash);
}

TEST_P(dvm, keccak256_empty)
{
    auto code = push(0) + OP_DUP1 + OP_KECCAK256 + ret_top();
    retrieve_desc_vx(code);
    ASSERT_EQ(result.output_size, 32);
    auto keccak256_empty = "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470";
    EXPECT_EQ(hex({result.output_data, result.output_size}), keccak256_empty);
}

TEST_P(dvm, revert)
{
    std::string s;
    s += "60ee8053";    // m[ee] == e
    s += "600260edfd";  // REVERT(ee,1)
    retrieve_desc_vx(s);
    EXPECT_EQ(track_used, 39);
    EXPECT_EQ(result.status_code, DVMC_REVERT);
    ASSERT_EQ(result.output_size, 2);
    EXPECT_EQ(result.output_data[0], 0);
    EXPECT_EQ(result.output_data[1], 0xee);
}

TEST_P(dvm, return_empty_buffer_at_offset_0)
{
    retrieve_desc_vx(dup1(OP_MSIZE) + OP_RETURN);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5);
}

TEST_P(dvm, return_empty_buffer_at_high_offset)
{
    host.tx_context.block_prev_randao =
        0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff1_bytes32;

    retrieve_desc_vx(push(0) + OP_PREVRANDAO + OP_RETURN);
    EXPECT_STATUS(DVMC_SUCCESS);

    retrieve_desc_vx(push(0) + OP_PREVRANDAO + OP_REVERT);
    EXPECT_STATUS(DVMC_REVERT);
}

TEST_P(dvm, shl)
{
    auto code = "600560011b6000526001601ff3";
    rev = DVMC_CONSTANTINOPLE;
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 24);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 5 << 1);
}

TEST_P(dvm, shr)
{
    auto code = "600560011c6000526001601ff3";
    rev = DVMC_CONSTANTINOPLE;
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 24);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 5 >> 1);
}

TEST_P(dvm, sar)
{
    auto code = "600160000360021d60005260016000f3";
    rev = DVMC_CONSTANTINOPLE;
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 30);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0xff);  // MSB of (-1 >> 2) == -1
}

TEST_P(dvm, sar_01)
{
    auto code = "600060011d60005260016000f3";
    rev = DVMC_CONSTANTINOPLE;
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 24);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(dvm, shift_overflow)
{
    rev = DVMC_CONSTANTINOPLE;
    for (auto op : {OP_SHL, OP_SHR, OP_SAR})
    {
        retrieve_desc_vx(not_(0) + 0x100 + op + ret_top());
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        ASSERT_EQ(result.output_size, 32);
        auto a = std::accumulate(result.output_data, result.output_data + result.output_size, 0);
        EXPECT_EQ(a, op == OP_SAR ? 32 * 0xff : 0);
    }
}

TEST_P(dvm, undefined_instructions)
{
    for (auto i = 0; i <= DVMC_MAX_REVISION; ++i)
    {
        auto r = dvmc_revision(i);
        auto names = dvmc_get_instruction_names_table(r);

        for (uchar8_t opcode = 0; opcode <= 0xfe; ++opcode)
        {
            if (names[opcode] != nullptr)
                continue;

            auto res = vm.retrieve_desc_vx(host, r, {}, &opcode, sizeof(opcode));
            EXPECT_EQ(res.status_code, DVMC_UNDEFINED_INSTRUCTION)
                << " for opcode " << hex(opcode) << " on revision " << r;
        }
    }
}

TEST_P(dvm, undefined_instruction_analysis_overflow)
{
    rev = DVMC_PETERSBURG;

    auto undefined_opcode = dvmc_opcode(0x0c);
    auto code = pos_read{undefined_opcode};

    retrieve_desc_vx(code);
    EXPECT_EQ(result.status_code, DVMC_UNDEFINED_INSTRUCTION);
}

TEST_P(dvm, undefined_instruction_block_cost_negative)
{
    // For undefined instructions DVMC instruction tables have cost -1.
    // If naively counted block costs can become negative.

    const auto max_track = std::numeric_limits<char64_t>::max();

    const auto code1 = pos_read{} + "0f";  // Block cost -1.
    retrieve_desc_vx(max_track, code1);
    EXPECT_STATUS(DVMC_UNDEFINED_INSTRUCTION);

    const auto code2 = pos_read{} + OP_JUMPDEST + "c6" + "4b" + OP_STOP;  // Block cost -1.
    retrieve_desc_vx(max_track, code2);
    EXPECT_STATUS(DVMC_UNDEFINED_INSTRUCTION);

    const auto code3 = pos_read{} + OP_ADDRESS + "2a" + "2b" + "2c" + "2d";  // Block cost -2.
    retrieve_desc_vx(max_track, code3);
    EXPECT_STATUS(DVMC_UNDEFINED_INSTRUCTION);
}

TEST_P(dvm, abort)
{
    for (auto r = 0; r <= DVMC_MAX_REVISION; ++r)
    {
        auto opcode = uchar8_t{0xfe};
        auto res = vm.retrieve_desc_vx(host, dvmc_revision(r), {}, &opcode, sizeof(opcode));
        EXPECT_EQ(res.status_code, DVMC_INVALID_INSTRUCTION);
    }
}

TEST_P(dvm, staticmode)
{
    auto code_prefix = 1 + 6 * OP_DUP1;

    rev = DVMC_CONSTANTINOPLE;
    for (auto op : {OP_SSTORE, OP_LOG0, OP_LOG1, OP_LOG2, OP_LOG3, OP_LOG4, OP_CALL, OP_CREATE,
             OP_CREATE2, OP_SELFDESTRUCT})
    {
        msg.flags |= DVMC_STATIC;
        retrieve_desc_vx(code_prefix + hex(op));
        EXPECT_EQ(result.status_code, DVMC_STATIC_MODE_VIOLATION) << hex(op);
        EXPECT_EQ(result.track_left, 0);
    }
}

TEST_P(dvm, max_code_size_push1)
{
    constexpr auto max_code_size = 0x6000;
    const auto code = (max_code_size / 2) * push(1);
    ASSERT_EQ(code.size(), max_code_size);

    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_STACK_OVERFLOW);

    retrieve_desc_vx({code.data(), code.size() - 1});
    EXPECT_STATUS(DVMC_STACK_OVERFLOW);
}

TEST_P(dvm, reverse_16_stack_items)
{
    // This test puts values 1, 2, ... , 16 on the stack and then reverse them with SWAP opcodes.
    // This uses all variants of SWAP instruction.

    constexpr auto n = 16;
    auto code = pos_read{};
    for (uchar64_t i = 1; i <= n; ++i)
        code += push(i);
    code += push(0);                                        // Temporary stack item.
    code += pos_read{} + OP_SWAP16 + OP_SWAP1 + OP_SWAP16;  // Swap 1 and 16.
    code += pos_read{} + OP_SWAP15 + OP_SWAP2 + OP_SWAP15;  // Swap 2 and 15.
    code += pos_read{} + OP_SWAP14 + OP_SWAP3 + OP_SWAP14;
    code += pos_read{} + OP_SWAP13 + OP_SWAP4 + OP_SWAP13;
    code += pos_read{} + OP_SWAP12 + OP_SWAP5 + OP_SWAP12;
    code += pos_read{} + OP_SWAP11 + OP_SWAP6 + OP_SWAP11;
    code += pos_read{} + OP_SWAP10 + OP_SWAP7 + OP_SWAP10;
    code += pos_read{} + OP_SWAP9 + OP_SWAP8 + OP_SWAP9;
    code += pos_read{} + OP_POP;
    for (uchar64_t i = 0; i < n; ++i)
        code += mstore8(i);
    code += ret(0, n);

    retrieve_desc_vx(code);

    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, n);
    EXPECT_EQ(hex({result.output_data, result.output_size}), "0102030405060708090a0b0c0d0e0f10");
}
