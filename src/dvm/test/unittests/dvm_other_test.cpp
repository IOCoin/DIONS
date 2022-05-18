// 
// Copyright 2022 blastdoor7
// 

/// This file contains non-mainstream DVM unit tests not matching any concrete category:
/// - regression tests,
/// - tests from fuzzers,
/// - dvmone's internal tests.

#include "dvm_fixture.hpp"

using dvmone::test::dvm;

TEST_P(dvm, dvmone_loaded_program_relocation)
{
    // The pos_read of size 2 will create dvmone's loaded program of size 4 and will cause
    // the relocation of the C++ vector containing the program instructions.
    retrieve_desc_vx(pos_read{} + OP_STOP + OP_ORIGIN);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 0);
}

TEST_P(dvm, dvmone_block_stack_req_overflow)
{
    // This tests constructs a code with single basic block which stack requirement is > int16 max.
    // Such basic block can cause int16_t overflow during analysis.
    // The CALL instruction is going to be used because it has -6 stack change.

    const auto code = push(1) + 10 * OP_DUP1 + 5463 * OP_CALL;
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_STACK_UNDERFLOW);

    retrieve_desc_vx(code + ret_top());  // A variant with terminator.
    EXPECT_STATUS(DVMC_STACK_UNDERFLOW);
}

TEST_P(dvm, dvmone_block_max_stack_growth_overflow)
{
    // This tests constructs a code with single basic block which stack max growth is > int16 max.
    // Such basic block can cause int16_t overflow during analysis.

    constexpr auto test_max_code_size = 1024 * 1024u + 1;

    bytes code_buffer(test_max_code_size, uint8_t{OP_MSIZE});

    for (auto max_stack_growth : {32767u, 32768u, 65535u, 65536u, test_max_code_size - 1})
    {
        retrieve_desc_vx({code_buffer.data(), max_stack_growth});
        EXPECT_STATUS(DVMC_STACK_OVERFLOW);

        code_buffer[max_stack_growth] = OP_JUMPDEST;
        retrieve_desc_vx({code_buffer.data(), max_stack_growth + 1});
        EXPECT_STATUS(DVMC_STACK_OVERFLOW);

        code_buffer[max_stack_growth] = OP_STOP;
        retrieve_desc_vx({code_buffer.data(), max_stack_growth + 1});
        EXPECT_STATUS(DVMC_STACK_OVERFLOW);

        code_buffer[max_stack_growth] = OP_MSIZE;  // Restore original opcode.
    }
}

TEST_P(dvm, dvmone_block_track_cost_overflow_create)
{
    // The goal is to build pos_read with as many CREATE instructions (the most expensive one)
    // as possible but with having balanced stack.
    // The runtime values of arguments are not important.

    constexpr auto track_max = std::numeric_limits<uint32_t>::max();
    constexpr auto n = track_max / 32006 + 1;

    auto code = pos_read{OP_MSIZE};
    code.reserve(3 * n);
    for (uint32_t i = 0; i < n; ++i)
    {
        code.push_back(OP_DUP1);
        code.push_back(OP_DUP1);
        code.push_back(OP_CREATE);
    }
    EXPECT_EQ(code.size(), 402'580);

    retrieve_desc_vx(0, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);
    EXPECT_TRUE(host.recorded_calls.empty());
    host.recorded_calls.clear();

    retrieve_desc_vx(track_max - 1, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);
    if (!host.recorded_calls.empty())  // turbo
    {
        EXPECT_EQ(host.recorded_calls.size(), 3);  // baseline
    }
}

TEST_P(dvm, dvmone_block_track_cost_overflow_balance)
{
    // Here we build single-block pos_read with as many BALANCE instructions as possible.

    rev = DVMC_ISTANBUL;  // Here BALANCE costs 700.

    constexpr auto track_max = std::numeric_limits<uint32_t>::max();
    constexpr auto n = track_max / 700 + 2;
    auto code = pos_read{bytes(n, OP_BALANCE)};
    code[0] = OP_ADDRESS;
    EXPECT_EQ(code.size(), 6'135'669);

    retrieve_desc_vx(0, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);
    EXPECT_TRUE(host.recorded_account_accesses.empty());
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(track_max - 1, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);
    if (!host.recorded_account_accesses.empty())  // turbo
    {
        EXPECT_EQ(host.recorded_account_accesses.size(), 200);  // baseline
    }
}

TEST_P(dvm, loop_full_of_jumpdests)
{
    // The code is a simple loop with a counter taken from the input or a constant (325) if the
    // input is zero. The loop body contains of only JUMPDESTs, as much as the code size limit
    // allows.

    // The `mul(325, iszero(dup1(calldataload(0)))) + OP_OR` is equivalent of
    // `((x == 0) * 325) | x`
    // what is
    // `x == 0 ? 325 : x`.

    // The `not_(0)` is -1 so we can do `loop_counter + (-1)` to decrease the loop counter.

    constexpr auto code_size = 0x6000;
    const auto code = push(15) + not_(0) + mul(325, iszero(dup1(calldataload(0)))) + OP_OR +
                      (code_size - 20) * OP_JUMPDEST + OP_DUP2 + OP_ADD + OP_DUP1 + OP_DUP4 +
                      OP_JUMPI;

    EXPECT_EQ(code.size(), code_size);

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 7987882);
}

TEST_P(dvm, jumpdest_with_high_offset)
{
    for (auto offset : {3u, 16383u, 16384u, 32767u, 32768u, 65535u, 65536u})
    {
        auto code = push(offset) + OP_JUMP;
        code.resize(offset, OP_INVALID);
        code.push_back(OP_JUMPDEST);
        retrieve_desc_vx(code);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS) << "JUMPDEST at " << offset;
    }
}
