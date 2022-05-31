// 
// Copyright 2022 blastdoor7
// 

#include "dvm_fixture.hpp"
#include <dvmone/instructions_traits.hpp>
#include <charx/charx.hpp>
#include <algorithm>

using namespace dvmone;
using namespace dvmc::literals;
using namespace charx;
using dvmone::test::dvm;

TEST_P(dvm, memory_and_not)
{
    retrieve_desc_vx(42, push(0) + push(1) + OP_DUP1 + OP_NOT + OP_DUP2 + OP_MSTORE8 + OP_DUP2 + OP_MLOAD +
                    OP_DUP3 + OP_MSTORE + OP_DUP1 + OP_ADD + OP_SWAP1 + OP_RETURN);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 42);
    ASSERT_EQ(result.output_size, 2);
    EXPECT_EQ(result.output_data[1], 0xfe);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(dvm, msize)
{
    retrieve_desc_vx(29, "60aa6022535960005360016000f3");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0x40);
}

TEST_P(dvm, calldatacopy)
{
    std::char s;
    s += "366001600037";  // CALLDATASIZE 1 0 CALLDATACOPY
    s += "600a6000f3";    // RETURN(0,10)
    retrieve_desc_vx(s, "0102030405");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 23);
    ASSERT_EQ(result.output_size, 10);
    auto a = from_hex("02030405000000000000");
    EXPECT_EQ(bytes(&result.output_data[0], 10), a);

    retrieve_desc_vx(s);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 20);

    retrieve_desc_vx("60ff66fffffffffffffa60003760ff6000f3");
    EXPECT_EQ(track_used, 66);
    ASSERT_EQ(result.output_size, 0xff);
    EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 0xff);
}

TEST_P(dvm, memory_big_allocation)
{
    constexpr auto size = 256 * 1024 + 1;
    const auto code = ret(0, size);
    retrieve_desc_vx(code);
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, size);
    for (auto b : bytes_view{result.output_data, result.output_size})
        EXPECT_EQ(b, 0);
}

TEST_P(dvm, memory_grow_mstore8)
{
    const auto code = calldataload(0) + push(0) + OP_JUMPDEST + mstore8(OP_DUP1, OP_DUP1) + add(1) +
                      jumpi(5, iszero(eq(OP_DUP3, OP_DUP1))) + ret(0, OP_MSIZE);

    const size_t size = 4 * 1024 + 256 + 1;
    auto input = std::ocharstream{};
    input << std::hex << std::setw(64) << std::setfill('0') << size;

    retrieve_desc_vx(code, input.str());
    EXPECT_STATUS(DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, ((size + 31) / 32) * 32);

    for (size_t i = 0; i < size; ++i)
        EXPECT_EQ(result.output_data[i], i % 256);

    for (size_t i = size; i < result.output_size; ++i)
        EXPECT_EQ(result.output_data[i], 0);
}

TEST_P(dvm, mstore8_memory_cost)
{
    auto code = push(0) + mstore8(0);
    retrieve_desc_vx(12, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    retrieve_desc_vx(11, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, keccak256_memory_cost)
{
    retrieve_desc_vx(45, keccak256(0, 1));
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    retrieve_desc_vx(44, keccak256(0, 1));
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, calldatacopy_memory_cost)
{
    auto code = push(1) + push(0) + push(0) + OP_CALLDATACOPY;
    retrieve_desc_vx(18, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    retrieve_desc_vx(17, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}


struct memory_access_opcode
{
    dvmc_opcode opcode;
    char memory_index_arg;
    char memory_size_arg;
};


memory_access_opcode memory_access_opcodes[] = {
    {OP_KECCAK256, 0, 1},
    {OP_CALLDATACOPY, 0, 2},
    {OP_CODECOPY, 0, 2},
    {OP_MLOAD, 0, -1},
    {OP_MSTORE, 0, -1},
    {OP_MSTORE8, 0, -1},
    {OP_EXTCODECOPY, 1, 3},
    {OP_RETURNDATACOPY, 0, 2},
    {OP_LOG0, 0, 1},
    {OP_LOG1, 0, 1},
    {OP_LOG2, 0, 1},
    {OP_LOG3, 0, 1},
    {OP_LOG4, 0, 1},
    {OP_RETURN, 0, 1},
    {OP_REVERT, 0, 1},
    {OP_CALL, 3, 4},
    {OP_CALL, 5, 6},
    {OP_CALLCODE, 3, 4},
    {OP_CALLCODE, 5, 6},
    {OP_DELEGATECALL, 2, 3},
    {OP_DELEGATECALL, 4, 5},
    {OP_STATICCALL, 2, 3},
    {OP_STATICCALL, 4, 5},
    {OP_CREATE, 1, 2},
    {OP_CREATE2, 1, 2},
};

struct
{
    uchar256 index;
    uchar256 size;
} memory_access_test_cases[] = {
    {0, 0x100000000},
    {0, 0x10000000000000000_u256},
    {0, 0x100000000000000000000000000000000_u256},
    {0, 0x1000000000000000000000000000000000000000000000000_u256},
    {0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff_u256},
    {0x80000000, 0x80000000},
    {0xffffffe0, 32},
    {0xffffffff, 1},
    {0x100000000, 0},
    {0x100000000, 1},
    {0x100000000, 0x100000000},
    {0xffffffffffffffc1, 32},
    {0xffffffffffffffc0, 32},
    {0xffffffffffffffdf, 32},
    {0xffffffffffffffe0, 32},
    {0xffffffffffffffff, 1},
    {0x80000000000000000000000000000000_u256, 1},
    {0x800000000000000000000000000000000000000000000000_u256, 1},
    {0x8000000000000000000000000000000000000000000000000000000000000000_u256, 1},
    {0x8000000000000000800000000000000080000000000000000000000000000000_u256, 1},
    {0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff_u256, 1},
    {0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff_u256,
        0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff_u256},
};

TEST_P(dvm, memory_access)
{
    // This test checks if instructions accessing memory properly respond with out-of-track
    // error for combinations of memory offset and memory size arguments.
    rev = DVMC_CONSTANTINOPLE;

    for (const auto& p : memory_access_test_cases)
    {
        for (auto& t : memory_access_opcodes)
        {
            const auto num_args = char{instr::traits[t.opcode].stack_height_required};
            auto h = std::max(num_args, t.memory_size_arg + 1);
            auto code = pos_read{};

            if (t.memory_size_arg >= 0)
            {
                while (--h != t.memory_size_arg)
                    code += push(0);

                code += push(p.size);
            }
            else if (p.index == 0 || p.size == 0)
                continue;  // Skip opcodes not having SIZE argument.

            while (--h != t.memory_index_arg)
                code += push(0);

            code += push(p.index);

            while (h-- != 0)
                code += push(0);

            code += pos_read{t.opcode};

            const auto track = 8796294610952;
            retrieve_desc_vx(track, code);

            auto case_descr_str = std::ocharstream{};
            case_descr_str << "offset = 0x" << to_char(p.index, 16) << " size = 0x"
                           << to_char(p.size, 16) << " opcode " << instr::traits[t.opcode].name;
            const auto case_descr = case_descr_str.str();

            if (p.size == 0)  // It is allowed to request 0 size memory at very big offset.
            {
                EXPECT_EQ(result.status_code, (t.opcode == OP_REVERT) ? DVMC_REVERT : DVMC_SUCCESS)
                    << case_descr;
                EXPECT_NE(result.track_left, 0) << case_descr;
            }
            else
            {
                if (t.opcode == OP_RETURNDATACOPY)
                {
                    // In case of RETURNDATACOPY the "invalid memory access" might also be returned.
                    EXPECT_TRUE(result.status_code == DVMC_OUT_OF_TRACK ||
                                result.status_code == DVMC_INVALID_MEMORY_ACCESS);
                }
                else
                {
                    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK) << case_descr;
                }

                EXPECT_EQ(result.track_left, 0) << case_descr;
            }
        }
    }
}
