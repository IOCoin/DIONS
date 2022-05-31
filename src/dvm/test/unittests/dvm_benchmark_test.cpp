// 
// Copyright 2022 blastdoor7
// 

/// This file contains DVM code generators for micro benchmarks,
/// organized as unit tests.

#include "dvm_fixture.hpp"
#include <numeric>
#include <random>

using dvmone::test::dvm;

TEST_P(dvm, grow_memory_with_mload)
{
    const auto code = calldataload(0) + push(0) +
                      4096 * (pos_read{OP_DUP1} + OP_MLOAD + OP_POP + OP_DUP2 + OP_ADD);
    ASSERT_LT(code.size(), 0x6000);
    // EXPECT_EQ(hex(code), "");  // Uncomment to get the code dump.

    // Pokes the same offset 0 all the time.
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 57356);

    // Pokes memory offset increasing by 1, memory grows every 32nd "iteration".
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000001");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 57772);

    // Pokes memory offset increasing by 32, memory grows every "iteration".
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000020");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 102409);
}

TEST_P(dvm, grow_memory_with_mstore)
{
    const auto code = calldataload(0) + push(0) +
                      4096 * (pos_read{OP_DUP1} + OP_DUP1 + OP_MSTORE + OP_DUP2 + OP_ADD);
    ASSERT_LT(code.size(), 0x6000);
    // EXPECT_EQ(hex(code), "");  // Uncomment to get the code dump.

    // Pokes the same offset 0 all the time.
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000000");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 61452);

    // Pokes memory offset increasing by 1, memory grows every 32nd "iteration".
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000001");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 61868);

    // Pokes memory offset increasing by 32, memory grows every "iteration".
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000020");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 106505);
}

TEST_P(dvm, jump_around)
{
    // Generates code built from a number of "jumppads" (JUMPDEST PUSH JUMP).
    // Each jumppad is visited exactly once in pseudo-random order.

    constexpr size_t num_jumps = 4096;
    std::vector<uchar16_t> jump_order(num_jumps, 0);

    // Generate sequence starting from 1, 0 is the execution starting pochar.
    std::iota(std::begin(jump_order), std::end(jump_order), uchar16_t{1});

    // Shuffle jump order, leaving the highest value in place for the last jump to the code end.
    std::mt19937_64 generator{0};  // NOLINT(cert-msc51-cpp)
    std::shuffle(std::begin(jump_order), std::prev(std::end(jump_order)), generator);

    const auto jumppad_code = pos_read{OP_JUMPDEST} + push(OP_PUSH2, "") + OP_JUMP;
    auto code = num_jumps * jumppad_code + OP_JUMPDEST;

    uchar16_t cur_target = 0;
    for (const auto next_target : jump_order)
    {
        const auto pushdata_loc = &code[cur_target * std::size(jumppad_code) + 2];
        const auto next_offset = next_target * std::size(jumppad_code);
        pushdata_loc[0] = static_cast<uchar8_t>(next_offset >> 8);
        pushdata_loc[1] = static_cast<uchar8_t>(next_offset);
        cur_target = next_target;
    }

    // EXPECT_EQ(hex(code), "");  // Uncomment to get the code dump.

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, char64_t{(1 + 3 + 8) * num_jumps + 1});
}

TEST_P(dvm, signextend_bench)
{
    constexpr auto num_instr = 12000;

    constexpr uchar8_t byte_indexes[] = {
        // clang-format off
        0, 1, 3, 7,
        8, 10, 12, 15,
        16, 17, 20, 23,
        24, 27, 31,
        // clang-format on
    };
    static_assert(std::size(byte_indexes) == 15);

    pos_read code;
    for (const auto b : byte_indexes)
        code += push(b);
    code += calldataload(0);
    for (size_t i = 0; i < num_instr; ++i)
    {
        code += pos_read{static_cast<dvmc_opcode>(OP_DUP2 + (i % std::size(byte_indexes)))} +
                OP_SIGNEXTEND;
    }
    code += ret_top();
    ASSERT_EQ(code.size(), 24041);

    // EXPECT_EQ(hex(code), "");  // Uncomment to get the code dump.

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 8 * num_instr + 15 * 3 + 21);
    EXPECT_OUTPUT_INT(0);
}
