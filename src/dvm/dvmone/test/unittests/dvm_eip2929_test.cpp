// 
// Copyright 2022 blastdoor7
// 

/// This file contains DVM unit tests for EIP-2929 "Gas cost increases for state access opcodes"
/// https://eips.blastdoor7.org/EIPS/eip-2929

#include "dvm_fixture.hpp"

using namespace dvmc::literals;
using dvmone::test::dvm;

TEST_P(dvm, eip2929_case1)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-1
    rev = DVMC_BERLIN;
    msg.sender = 0x0000000000000000000000000000000000000000_address;
    msg.recipient = 0x000000000000000000000000636F6E7472616374_address;
    const auto code =
        "0x60013f5060023b506003315060f13f5060f23b5060f3315060f23f5060f33b5060f1315032315030315000";

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 8653);
    EXPECT_EQ(result.output_size, 0);

    const auto& r = host.recorded_account_accesses;
    ASSERT_EQ(r.size(), 24);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], 0x0000000000000000000000000000000000000001_address);
    EXPECT_EQ(r[3], 0x0000000000000000000000000000000000000001_address);
    EXPECT_EQ(r[4], 0x0000000000000000000000000000000000000002_address);
    EXPECT_EQ(r[5], 0x0000000000000000000000000000000000000002_address);
    EXPECT_EQ(r[6], 0x0000000000000000000000000000000000000003_address);
    EXPECT_EQ(r[7], 0x0000000000000000000000000000000000000003_address);
    EXPECT_EQ(r[8], 0x00000000000000000000000000000000000000f1_address);
    EXPECT_EQ(r[9], 0x00000000000000000000000000000000000000f1_address);
    EXPECT_EQ(r[10], 0x00000000000000000000000000000000000000f2_address);
    EXPECT_EQ(r[11], 0x00000000000000000000000000000000000000f2_address);
    EXPECT_EQ(r[12], 0x00000000000000000000000000000000000000f3_address);
    EXPECT_EQ(r[13], 0x00000000000000000000000000000000000000f3_address);
    EXPECT_EQ(r[14], 0x00000000000000000000000000000000000000f2_address);
    EXPECT_EQ(r[15], 0x00000000000000000000000000000000000000f2_address);
    EXPECT_EQ(r[16], 0x00000000000000000000000000000000000000f3_address);
    EXPECT_EQ(r[17], 0x00000000000000000000000000000000000000f3_address);
    EXPECT_EQ(r[18], 0x00000000000000000000000000000000000000f1_address);
    EXPECT_EQ(r[19], 0x00000000000000000000000000000000000000f1_address);
    EXPECT_EQ(r[20], 0x0000000000000000000000000000000000000000_address);
    EXPECT_EQ(r[21], 0x0000000000000000000000000000000000000000_address);
    EXPECT_EQ(r[22], msg.recipient);
    EXPECT_EQ(r[23], msg.recipient);
}

TEST_P(dvm, eip2929_case2)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-2
    rev = DVMC_BERLIN;
    msg.sender = 0x0000000000000000000000000000000000000000_address;
    msg.recipient = 0x000000000000000000000000636F6E7472616374_address;
    const auto code = "0x60006000600060ff3c60006000600060ff3c600060006000303c00";

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2835);
    EXPECT_EQ(result.output_size, 0);

    const auto& r = host.recorded_account_accesses;
    ASSERT_EQ(r.size(), 5);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], 0x00000000000000000000000000000000000000ff_address);
    EXPECT_EQ(r[3], 0x00000000000000000000000000000000000000ff_address);
    EXPECT_EQ(r[4], msg.recipient);
}

TEST_P(dvm, eip2929_case3)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-3
    rev = DVMC_BERLIN;
    msg.sender = 0x0000000000000000000000000000000000000000_address;
    msg.recipient = 0x000000000000000000000000636F6E7472616374_address;
    const auto code = "0x60015450601160015560116002556011600255600254600154";

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 44529);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(dvm, eip2929_case4)
{
    // https://gist.github.com/holiman/174548cad102096858583c6fbbb0649a#case-4
    rev = DVMC_BERLIN;
    msg.sender = 0x0000000000000000000000000000000000000000_address;
    msg.recipient = 0x000000000000000000000000636F6E7472616374_address;
    const auto code = "0x60008080808060046000f15060008080808060ff6000f15060008080808060ff6000fa50";

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2869);
    EXPECT_EQ(result.output_size, 0);
}

TEST_P(dvm, eip2929_balance_oog)
{
    rev = DVMC_BERLIN;
    const auto code = push(0x0a) + OP_BALANCE;

    retrieve_desc_vx(2603, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2603);

    host.recorded_account_accesses.clear();
    retrieve_desc_vx(2602, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 2602);
}

TEST_P(dvm, eip2929_extcodesize_oog)
{
    rev = DVMC_BERLIN;
    const auto code = push(0x0a) + OP_EXTCODESIZE;

    retrieve_desc_vx(2603, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2603);

    host.recorded_account_accesses.clear();
    retrieve_desc_vx(2602, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 2602);
}

TEST_P(dvm, eip2929_extcodecopy_oog)
{
    rev = DVMC_BERLIN;
    const auto code = push(0) + OP_DUP1 + OP_DUP1 + push(0x0a) + OP_EXTCODECOPY;

    retrieve_desc_vx(2612, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2612);

    host.recorded_account_accesses.clear();
    retrieve_desc_vx(2611, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 2611);
}

TEST_P(dvm, eip2929_extcodehash_oog)
{
    rev = DVMC_BERLIN;
    const auto code = push(0x0a) + OP_EXTCODEHASH;

    retrieve_desc_vx(2603, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2603);

    host.recorded_account_accesses.clear();
    retrieve_desc_vx(2602, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 2602);
}

TEST_P(dvm, eip2929_sload_cold)
{
    rev = DVMC_BERLIN;
    const auto code = push(1) + OP_SLOAD;

    const dvmc::bytes32 key{1};
    host.accounts[msg.recipient].storage[key] = dvmc::bytes32{2};
    ASSERT_EQ(host.accounts[msg.recipient].storage[key].access_status, DVMC_ACCESS_COLD);
    retrieve_desc_vx(2103, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2103);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, DVMC_ACCESS_WARM);

    host.accounts[msg.recipient].storage[key].access_status = DVMC_ACCESS_COLD;
    retrieve_desc_vx(2102, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 2102);
}

TEST_P(dvm, eip2929_sload_two_slots)
{
    rev = DVMC_BERLIN;
    const dvmc::bytes32 key0{0};
    const dvmc::bytes32 key1{1};
    const auto code = push(key0) + OP_SLOAD + OP_POP + push(key1) + OP_SLOAD + OP_POP;

    retrieve_desc_vx(30000, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 4210);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key0].access_status, DVMC_ACCESS_WARM);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key1].access_status, DVMC_ACCESS_WARM);
}

TEST_P(dvm, eip2929_sload_warm)
{
    rev = DVMC_BERLIN;
    const auto code = push(1) + OP_SLOAD;

    const dvmc::bytes32 key{1};
    host.accounts[msg.recipient].storage[key] = {dvmc::bytes32{2}, DVMC_ACCESS_WARM};
    ASSERT_EQ(host.accounts[msg.recipient].storage[key].access_status, DVMC_ACCESS_WARM);
    retrieve_desc_vx(103, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 103);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, DVMC_ACCESS_WARM);

    retrieve_desc_vx(102, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 102);
}

TEST_P(dvm, eip2929_sstore_modify_cold)
{
    rev = DVMC_BERLIN;
    const auto code = sstore(1, 3);

    const dvmc::bytes32 key{1};
    host.accounts[msg.recipient].storage[key] = dvmc::bytes32{2};
    retrieve_desc_vx(5006, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5006);
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].value, dvmc::bytes32{3});
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, DVMC_ACCESS_WARM);

    host.accounts[msg.recipient].storage[key] = dvmc::bytes32{2};
    retrieve_desc_vx(5005, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 5005);
    // The storage will be modified anyway, because the cost is checked after.
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].value, dvmc::bytes32{3});
    EXPECT_EQ(host.accounts[msg.recipient].storage[key].access_status, DVMC_ACCESS_WARM);
}

TEST_P(dvm, eip2929_selfdestruct_cold_beneficiary)
{
    rev = DVMC_BERLIN;
    const auto code = push(0xbe) + OP_SELFDESTRUCT;

    retrieve_desc_vx(7603, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 7603);

    host.recorded_account_accesses.clear();
    retrieve_desc_vx(7602, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 7602);
}

TEST_P(dvm, eip2929_selfdestruct_warm_beneficiary)
{
    rev = DVMC_BERLIN;
    const auto code = push(0xbe) + OP_SELFDESTRUCT;

    host.access_account(0x00000000000000000000000000000000000000be_address);
    retrieve_desc_vx(5003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5003);

    host.recorded_account_accesses.clear();
    host.access_account(0x00000000000000000000000000000000000000be_address);
    retrieve_desc_vx(5002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 5002);
}

TEST_P(dvm, eip2929_delegatecall_cold)
{
    rev = DVMC_BERLIN;
    const auto code = delegatecall(0xde);
    auto& r = host.recorded_account_accesses;

    retrieve_desc_vx(2618, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 2618);
    ASSERT_EQ(r.size(), 4);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], 0x00000000000000000000000000000000000000de_address);
    EXPECT_EQ(r[3], msg.sender);

    r.clear();
    retrieve_desc_vx(2617, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 2617);
    ASSERT_EQ(r.size(), 3);
    EXPECT_EQ(r[0], msg.sender);
    EXPECT_EQ(r[1], msg.recipient);
    EXPECT_EQ(r[2], 0x00000000000000000000000000000000000000de_address);
}
