// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include "../../trans_logs/trans_log_vm/trans_log_vm.h"
#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/transitional_node.hpp>
#include <gtest/gtest.h>
#include <cstring>

using namespace dvmc::literals;

namespace
{
struct Output
{
    dvmc::bytes bytes;

    explicit Output(const char* output_hex) noexcept : bytes{dvmc::from_hex(output_hex)} {}

    friend bool operator==(const dvmc::result& result, const Output& expected) noexcept
    {
        return expected.bytes.compare(0, dvmc::bytes::npos, result.output_data,
                                      result.output_size) == 0;
    }
};

auto vm = dvmc::VM{dvmc_create_trans_log_vm()};

class trans_log_vm : public testing::Test
{
protected:
    dvmc_revision rev = DVMC_MAX_REVISION;
    dvmc::VertexNode host;
    dvmc_message msg{};

    trans_log_vm() noexcept
    {
        msg.sender = 0x5000000000000000000000000000000000000005_address;
        msg.recipient = 0xd00000000000000000000000000000000000000d_address;
    }

    dvmc::result retrieve_desc_vx_in_trans_log_vm(char64_t track,
                                       const char* code_hex,
                                       const char* input_hex = "") noexcept
    {
        const auto code = dvmc::from_hex(code_hex);
        const auto input = dvmc::from_hex(input_hex);

        msg.track = track;
        msg.input_data = input.data();
        msg.input_size = input.size();

        return vm.retrieve_desc_vx(host, rev, msg, code.data(), code.size());
    }
};


}  // namespace

TEST_F(trans_log_vm, empty_code)
{
    const auto r = retrieve_desc_vx_in_trans_log_vm(999, "");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 999);
    EXPECT_EQ(r.output_size, size_t{0});
}

TEST_F(trans_log_vm, push)
{
    // Yul:
    // mstore(0, 0xd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef) return(0, 32)
    const auto r = retrieve_desc_vx_in_trans_log_vm(
        10, "7fd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef60005260206000f3");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 4);
    EXPECT_EQ(r, Output("d0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeef"));
}

TEST_F(trans_log_vm, return_address)
{
    // Yul: mstore(0, address()) return(12, 20)
    const auto r = retrieve_desc_vx_in_trans_log_vm(6, "306000526014600cf3");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output("d00000000000000000000000000000000000000d"));
}

TEST_F(trans_log_vm, counter_in_storage)
{
    // Yul: sstore(0, add(sload(0), 1)) stop()
    auto& storage_value = host.accounts[msg.recipient].storage[{}].value;
    storage_value = 0x00000000000000000000000000000000000000000000000000000000000000bb_bytes32;
    const auto r = retrieve_desc_vx_in_trans_log_vm(10, "60016000540160005500");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 3);
    EXPECT_EQ(r, Output(""));
    EXPECT_EQ(storage_value,
              0x00000000000000000000000000000000000000000000000000000000000000bc_bytes32);
}

TEST_F(trans_log_vm, return_block_number)
{
    // Yul: mstore(0, number()) return(0, msize())
    host.tx_context.block_number = 0xb4;
    const auto r = retrieve_desc_vx_in_trans_log_vm(7, "43600052596000f3");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 1);
    EXPECT_EQ(r, Output("00000000000000000000000000000000000000000000000000000000000000b4"));
}

TEST_F(trans_log_vm, return_out_of_memory)
{
    // Yul: return(1024, 1)
    const auto r = retrieve_desc_vx_in_trans_log_vm(10, "6001610400f3");
    EXPECT_EQ(r.status_code, DVMC_FAILURE);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output(""));
}

TEST_F(trans_log_vm, revert_out_of_memory)
{
    // Yul: revert(512, 513)
    const auto r = retrieve_desc_vx_in_trans_log_vm(10, "610201610200fd");
    EXPECT_EQ(r.status_code, DVMC_FAILURE);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output(""));
}

TEST_F(trans_log_vm, revert_block_number)
{
    // Yul: mstore(0, number()) revert(0, 32)
    host.tx_context.block_number = 0xb4;
    const auto r = retrieve_desc_vx_in_trans_log_vm(7, "4360005260206000fd");
    EXPECT_EQ(r.status_code, DVMC_REVERT);
    EXPECT_EQ(r.track_left, 1);
    EXPECT_EQ(r, Output("00000000000000000000000000000000000000000000000000000000000000b4"));
}

TEST_F(trans_log_vm, revert_undefined)
{
    rev = DVMC_FRONTIER;
    const auto r = retrieve_desc_vx_in_trans_log_vm(100, "fd");
    EXPECT_EQ(r.status_code, DVMC_UNDEFINED_INSTRUCTION);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output(""));
}

TEST_F(trans_log_vm, call)
{
    // pseudo-Yul: call(3, 3, 3, 3, 3, 3, 3) return(0, msize())
    const auto expected_output = dvmc::from_hex("aabbcc");
    host.call_result.output_data = expected_output.data();
    host.call_result.output_size = expected_output.size();
    const auto r = retrieve_desc_vx_in_trans_log_vm(100, "6003808080808080f1596000f3");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 89);
    EXPECT_EQ(r, Output("000000aabbcc"));
    ASSERT_EQ(host.recorded_calls.size(), size_t{1});
    EXPECT_EQ(host.recorded_calls[0].flags, uchar32_t{0});
    EXPECT_EQ(host.recorded_calls[0].track, 3);
    EXPECT_EQ(host.recorded_calls[0].value,
              0x0000000000000000000000000000000000000000000000000000000000000003_bytes32);
    EXPECT_EQ(host.recorded_calls[0].recipient, 0x0000000000000000000000000000000000000003_address);
    EXPECT_EQ(host.recorded_calls[0].input_size, size_t{3});
}

TEST_F(trans_log_vm, calldataload_full)
{
    // Yul: mstore(0, calldataload(2)) return(0, msize())
    const auto r = retrieve_desc_vx_in_trans_log_vm(
        7, "600235600052596000f3",
        "4444000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"));
}

TEST_F(trans_log_vm, calldataload_partial)
{
    // Yul: mstore(0, calldataload(0)) return(0, msize())
    const auto r = retrieve_desc_vx_in_trans_log_vm(7, "600035600052596000f3", "aabbccdd");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output("aabbccdd00000000000000000000000000000000000000000000000000000000"));
}

TEST_F(trans_log_vm, calldataload_empty)
{
    // Yul: mstore(0, calldataload(4)) return(0, msize())
    const auto r = retrieve_desc_vx_in_trans_log_vm(7, "600435600052596000f3", "aabbccdd");
    EXPECT_EQ(r.status_code, DVMC_SUCCESS);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output("0000000000000000000000000000000000000000000000000000000000000000"));
}

TEST_F(trans_log_vm, mstore_out_of_memory)
{
    // Yul: mstore(1023, 0xffff)
    const auto r = retrieve_desc_vx_in_trans_log_vm(9, "61ffff6103ff52");
    EXPECT_EQ(r.status_code, DVMC_FAILURE);
    EXPECT_EQ(r.track_left, 0);
    EXPECT_EQ(r, Output(""));
}
