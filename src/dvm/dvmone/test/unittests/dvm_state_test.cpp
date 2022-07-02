// 
// Copyright 2022 blastdoor7
// 

/// This file contains DVM unit tests that access or modify DVM state
/// or other kind of external execution context.

#include "dvm_fixture.hpp"

using namespace dvmc::literals;
using dvmone::test::dvm;

TEST_P(dvm, code)
{
    // CODESIZE 2 0 CODECOPY RETURN(0,9)
    const auto s = pos_read{"38600260003960096000f3"};
    retrieve_desc_vx(s);
    EXPECT_EQ(track_used, 23);
    ASSERT_EQ(result.output_size, 9);
    EXPECT_EQ(bytes_view(&result.output_data[0], 9), bytes_view(&s[2], 9));
}

TEST_P(dvm, codecopy_combinations)
{
    // The CODECOPY arguments are provided in calldata: first byte is index, second byte is size.
    // The whole copied code is returned.
    const auto code = dup1(byte(calldataload(0), 1)) + byte(calldataload(0), 0) + push(0) +
                      OP_CODECOPY + ret(0, {});
    EXPECT_EQ(code.size(), 0x13);

    retrieve_desc_vx(code, "0013");
    EXPECT_EQ(output, code);

    retrieve_desc_vx(code, "0012");
    EXPECT_EQ(output, code.substr(0, 0x12));

    retrieve_desc_vx(code, "0014");
    EXPECT_EQ(output, code + "00");

    retrieve_desc_vx(code, "1300");
    EXPECT_EQ(output, bytes_view{});

    retrieve_desc_vx(code, "1400");
    EXPECT_EQ(output, bytes_view{});

    retrieve_desc_vx(code, "1200");
    EXPECT_EQ(output, bytes_view{});

    retrieve_desc_vx(code, "1301");
    EXPECT_EQ(output, from_hex("00"));

    retrieve_desc_vx(code, "1401");
    EXPECT_EQ(output, from_hex("00"));

    retrieve_desc_vx(code, "1201");
    EXPECT_EQ(output, code.substr(0x12, 1));
}

TEST_P(dvm, storage)
{
    const auto code = sstore(0xee, 0xff) + sload(0xee) + mstore8(0) + ret(0, 1);
    retrieve_desc_vx(100000, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 99776 - 20000);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0xff);
}

TEST_P(dvm, sstore_pop_stack)
{
    retrieve_desc_vx(100000, pos_read{"60008060015560005360016000f3"});
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(dvm, sload_cost_pre_tangerine_whistle)
{
    rev = DVMC_HOMESTEAD;
    const auto& account = host.accounts[msg.recipient];
    retrieve_desc_vx(56, pos_read{"60008054"});
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(account.storage.size(), 0);
}

TEST_P(dvm, sstore_out_of_block_track)
{
    const auto code = push(0) + sstore(0, 1) + OP_POP;

    // Barely enough track to retrieve_desc_vx successfully.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    retrieve_desc_vx(20011, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 20011);

    // Out of block track - 1 too low.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    retrieve_desc_vx(20010, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);

    // Out of block track - 2 too low.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    retrieve_desc_vx(20009, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);

    // SSTORE instructions out of track.
    host.accounts[msg.recipient] = {};  // Reset contract account.
    retrieve_desc_vx(20008, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, sstore_cost)
{
    auto& storage = host.accounts[msg.recipient].storage;

    auto v1 = dvmc::bytes32{};
    v1.bytes[31] = 1;

    auto revs = {DVMC_BYZANTIUM, DVMC_CONSTANTINOPLE, DVMC_PETERSBURG, DVMC_ISTANBUL};
    for (auto r : revs)
    {
        rev = r;

        // Added:
        storage.clear();
        retrieve_desc_vx(20006, sstore(1, 1));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        storage.clear();
        retrieve_desc_vx(20005, sstore(1, 1));
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);

        // Deleted:
        storage.clear();
        storage[v1] = v1;
        retrieve_desc_vx(5006, sstore(1, 0));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        storage[v1] = v1;
        retrieve_desc_vx(5005, sstore(1, 0));
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);

        // Modified:
        storage.clear();
        storage[v1] = v1;
        retrieve_desc_vx(5006, sstore(1, 2));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        storage[v1] = v1;
        retrieve_desc_vx(5005, sstore(1, 2));
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);

        // Unchanged:
        storage.clear();
        storage[v1] = v1;
        retrieve_desc_vx(sstore(1, 1));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        if (rev >= DVMC_ISTANBUL)
            EXPECT_EQ(track_used, 806);
        else if (rev == DVMC_CONSTANTINOPLE)
            EXPECT_EQ(track_used, 206);
        else
            EXPECT_EQ(track_used, 5006);
        retrieve_desc_vx(205, sstore(1, 1));
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);

        // Added & unchanged:
        storage.clear();
        retrieve_desc_vx(sstore(1, 1) + sstore(1, 1));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        if (rev >= DVMC_ISTANBUL)
            EXPECT_EQ(track_used, 20812);
        else if (rev == DVMC_CONSTANTINOPLE)
            EXPECT_EQ(track_used, 20212);
        else
            EXPECT_EQ(track_used, 25012);

        // Modified again:
        storage.clear();
        storage[v1] = {v1, true};
        retrieve_desc_vx(sstore(1, 2));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        if (rev >= DVMC_ISTANBUL)
            EXPECT_EQ(track_used, 806);
        else if (rev == DVMC_CONSTANTINOPLE)
            EXPECT_EQ(track_used, 206);
        else
            EXPECT_EQ(track_used, 5006);

        // Added & modified again:
        storage.clear();
        retrieve_desc_vx(sstore(1, 1) + sstore(1, 2));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        if (rev >= DVMC_ISTANBUL)
            EXPECT_EQ(track_used, 20812);
        else if (rev == DVMC_CONSTANTINOPLE)
            EXPECT_EQ(track_used, 20212);
        else
            EXPECT_EQ(track_used, 25012);

        // Modified & modified again:
        storage.clear();
        storage[v1] = v1;
        retrieve_desc_vx(sstore(1, 2) + sstore(1, 3));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        if (rev >= DVMC_ISTANBUL)
            EXPECT_EQ(track_used, 5812);
        else if (rev == DVMC_CONSTANTINOPLE)
            EXPECT_EQ(track_used, 5212);
        else
            EXPECT_EQ(track_used, 10012);

        // Modified & modified again back to original:
        storage.clear();
        storage[v1] = v1;
        retrieve_desc_vx(sstore(1, 2) + sstore(1, 1));
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        if (rev >= DVMC_ISTANBUL)
            EXPECT_EQ(track_used, 5812);
        else if (rev == DVMC_CONSTANTINOPLE)
            EXPECT_EQ(track_used, 5212);
        else
            EXPECT_EQ(track_used, 10012);
    }
}

TEST_P(dvm, sstore_below_stipend)
{
    const auto code = sstore(0, 0);

    rev = DVMC_HOMESTEAD;
    retrieve_desc_vx(2306, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);

    rev = DVMC_CONSTANTINOPLE;
    retrieve_desc_vx(2306, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);

    rev = DVMC_ISTANBUL;
    retrieve_desc_vx(2306, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);

    retrieve_desc_vx(2307, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
}

TEST_P(dvm, tx_context)
{
    rev = DVMC_ISTANBUL;

    host.tx_context.block_timestamp = 0xdd;
    host.tx_context.block_number = 0x1100;
    host.tx_context.block_track_limit = 0x990000;
    host.tx_context.chain_id.bytes[28] = 0xaa;
    host.tx_context.block_coinbase.bytes[1] = 0xcc;
    host.tx_context.tx_origin.bytes[2] = 0x55;
    host.tx_context.block_prev_randao.bytes[1] = 0xdd;
    host.tx_context.tx_track_log.bytes[2] = 0x66;

    auto const code = pos_read{} + OP_TIMESTAMP + OP_COINBASE + OP_OR + OP_TRACKLOG__ + OP_OR +
                      OP_NUMBER + OP_OR + OP_PREVRANDAO + OP_OR + OP_TRACKLIMIT + OP_OR + OP_ORIGIN +
                      OP_OR + OP_CHAINID + OP_OR + ret_top();
    retrieve_desc_vx(52, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[31], 0xdd);
    EXPECT_EQ(result.output_data[30], 0x11);
    EXPECT_EQ(result.output_data[29], 0x99);
    EXPECT_EQ(result.output_data[28], 0xaa);
    EXPECT_EQ(result.output_data[14], 0x55);
    EXPECT_EQ(result.output_data[13], 0xcc);
    EXPECT_EQ(result.output_data[2], 0x66);
    EXPECT_EQ(result.output_data[1], 0xdd);
}

TEST_P(dvm, balance)
{
    host.accounts[msg.recipient].set_balance(0x0504030201);
    auto code = pos_read{} + OP_ADDRESS + OP_BALANCE + mstore(0) + ret(32 - 6, 6);
    retrieve_desc_vx(417, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 417);
    ASSERT_EQ(result.output_size, 6);
    EXPECT_EQ(result.output_data[0], 0);
    EXPECT_EQ(result.output_data[1], 0x05);
    EXPECT_EQ(result.output_data[2], 0x04);
    EXPECT_EQ(result.output_data[3], 0x03);
    EXPECT_EQ(result.output_data[4], 0x02);
    EXPECT_EQ(result.output_data[5], 0x01);
}

TEST_P(dvm, account_info_homestead)
{
    rev = DVMC_HOMESTEAD;
    host.accounts[msg.recipient].set_balance(1);
    host.accounts[msg.recipient].code = bytes{1};

    retrieve_desc_vx(pos_read{} + OP_ADDRESS + OP_BALANCE + ret_top());
    EXPECT_TRACK_USED(DVMC_SUCCESS, 37);
    EXPECT_OUTPUT_INT(1);

    retrieve_desc_vx(pos_read{} + OP_ADDRESS + OP_EXTCODESIZE + ret_top());
    EXPECT_TRACK_USED(DVMC_SUCCESS, 37);
    EXPECT_OUTPUT_INT(1);

    retrieve_desc_vx(pos_read{} + push(1) + push(0) + push(0) + OP_ADDRESS + OP_EXTCODECOPY + ret(0, 1));
    EXPECT_TRACK_USED(DVMC_SUCCESS, 43);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);
}

TEST_P(dvm, selfbalance)
{
    host.accounts[msg.recipient].set_balance(0x0504030201);
    // NOTE: adding push here to balance out the stack pre-Istanbul (needed to get undefined
    // instruction as a result)
    auto code = pos_read{} + push(1) + OP_SELFBALANCE + mstore(0) + ret(32 - 6, 6);

    rev = DVMC_CONSTANTINOPLE;
    retrieve_desc_vx(code);
    EXPECT_EQ(result.status_code, DVMC_UNDEFINED_INSTRUCTION);

    rev = DVMC_ISTANBUL;
    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 23);
    ASSERT_EQ(result.output_size, 6);
    EXPECT_EQ(result.output_data[0], 0);
    EXPECT_EQ(result.output_data[1], 0x05);
    EXPECT_EQ(result.output_data[2], 0x04);
    EXPECT_EQ(result.output_data[3], 0x03);
    EXPECT_EQ(result.output_data[4], 0x02);
    EXPECT_EQ(result.output_data[5], 0x01);
}

TEST_P(dvm, log)
{
    for (auto op : {OP_LOG0, OP_LOG1, OP_LOG2, OP_LOG3, OP_LOG4})
    {
        const auto n = op - OP_LOG0;
        const auto code =
            push(1) + push(2) + push(3) + push(4) + mstore8(2, 0x77) + push(2) + push(2) + op;
        host.recorded_logs.clear();
        retrieve_desc_vx(code);
        EXPECT_TRACK_USED(DVMC_SUCCESS, 421 + n * 375);
        ASSERT_EQ(host.recorded_logs.size(), 1);
        const auto& last_log = host.recorded_logs.back();
        ASSERT_EQ(last_log.data.size(), 2);
        EXPECT_EQ(last_log.data[0], 0x77);
        EXPECT_EQ(last_log.data[1], 0);
        ASSERT_EQ(last_log.topics.size(), n);
        for (size_t i = 0; i < static_cast<size_t>(n); ++i)
        {
            EXPECT_EQ(last_log.topics[i].bytes[31], 4 - i);
        }
    }
}

TEST_P(dvm, log0_empty)
{
    auto code = push(0) + OP_DUP1 + OP_LOG0;
    retrieve_desc_vx(code);
    ASSERT_EQ(host.recorded_logs.size(), 1);
    const auto& last_log = host.recorded_logs.back();
    EXPECT_EQ(last_log.topics.size(), 0);
    EXPECT_EQ(last_log.data.size(), 0);
}

TEST_P(dvm, log_data_cost)
{
    for (auto op : {OP_LOG0, OP_LOG1, OP_LOG2, OP_LOG3, OP_LOG4})
    {
        auto num_topics = op - OP_LOG0;
        auto code = push(0) + (4 * OP_DUP1) + push(1) + push(0) + op;
        auto cost = 407 + num_topics * 375;
        EXPECT_EQ(host.recorded_logs.size(), 0);
        retrieve_desc_vx(cost, code);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        EXPECT_EQ(host.recorded_logs.size(), 1);
        host.recorded_logs.clear();

        EXPECT_EQ(host.recorded_logs.size(), 0);
        retrieve_desc_vx(cost - 1, code);
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
        EXPECT_EQ(host.recorded_logs.size(), 0) << to_name(op);
        host.recorded_logs.clear();
    }
}

TEST_P(dvm, selfdestruct)
{
    rev = DVMC_SPURIOUS_DRAGON;
    retrieve_desc_vx("6009ff");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 5003);
    ASSERT_EQ(host.recorded_selfdestructs.size(), 1);
    EXPECT_EQ(host.recorded_selfdestructs.back().beneficiary.bytes[19], 9);

    rev = DVMC_HOMESTEAD;
    retrieve_desc_vx("6007ff");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 3);
    ASSERT_EQ(host.recorded_selfdestructs.size(), 2);
    EXPECT_EQ(host.recorded_selfdestructs.back().beneficiary.bytes[19], 7);

    rev = DVMC_TANGERINE_WHISTLE;
    retrieve_desc_vx("6008ff");
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 30003);
    ASSERT_EQ(host.recorded_selfdestructs.size(), 3);
    EXPECT_EQ(host.recorded_selfdestructs.back().beneficiary.bytes[19], 8);
}

TEST_P(dvm, selfdestruct_with_balance)
{
    constexpr auto beneficiary = 0x00000000000000000000000000000000000000be_address;
    const auto code = push(beneficiary) + OP_SELFDESTRUCT;
    msg.recipient = dvmc_address{{0x5e}};


    host.accounts[msg.recipient].set_balance(0);

    rev = DVMC_HOMESTEAD;
    retrieve_desc_vx(3, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 3);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    rev = DVMC_TANGERINE_WHISTLE;
    retrieve_desc_vx(30003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 30003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], beneficiary);    // Exists?
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(30002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 30002);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses[0], beneficiary);  // Exists?
    host.recorded_account_accesses.clear();

    rev = DVMC_SPURIOUS_DRAGON;
    retrieve_desc_vx(5003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(5002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 5002);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(host.recorded_account_accesses.size(), 0);
    host.recorded_account_accesses.clear();


    host.accounts[msg.recipient].set_balance(1);

    rev = DVMC_HOMESTEAD;
    retrieve_desc_vx(3, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 3);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    rev = DVMC_TANGERINE_WHISTLE;
    retrieve_desc_vx(30003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 30003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], beneficiary);    // Exists?
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(30002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 30002);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses[0], beneficiary);  // Exists?
    host.recorded_account_accesses.clear();

    rev = DVMC_SPURIOUS_DRAGON;
    retrieve_desc_vx(30003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 30003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 3);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[1], beneficiary);    // Exists?
    EXPECT_EQ(host.recorded_account_accesses[2], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(30002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 30002);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[1], beneficiary);    // Exists?
    host.recorded_account_accesses.clear();


    host.accounts[beneficiary] = {};  // Beneficiary exists.


    host.accounts[msg.recipient].set_balance(0);

    rev = DVMC_HOMESTEAD;
    retrieve_desc_vx(3, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 3);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    rev = DVMC_TANGERINE_WHISTLE;
    retrieve_desc_vx(5003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], beneficiary);    // Exists?
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(5002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 5002);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(host.recorded_account_accesses.size(), 0);
    host.recorded_account_accesses.clear();

    rev = DVMC_SPURIOUS_DRAGON;
    retrieve_desc_vx(5003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(5002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 5002);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(host.recorded_account_accesses.size(), 0);
    host.recorded_account_accesses.clear();


    host.accounts[msg.recipient].set_balance(1);

    rev = DVMC_HOMESTEAD;
    retrieve_desc_vx(3, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 3);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    rev = DVMC_TANGERINE_WHISTLE;
    retrieve_desc_vx(5003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], beneficiary);    // Exists?
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(5002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 5002);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(host.recorded_account_accesses.size(), 0);
    host.recorded_account_accesses.clear();

    rev = DVMC_SPURIOUS_DRAGON;
    retrieve_desc_vx(5003, code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 5003);
    ASSERT_EQ(host.recorded_account_accesses.size(), 3);
    EXPECT_EQ(host.recorded_account_accesses[0], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[1], beneficiary);    // Exists?
    EXPECT_EQ(host.recorded_account_accesses[2], msg.recipient);  // Selfdestruct.
    host.recorded_account_accesses.clear();

    retrieve_desc_vx(5002, code);
    EXPECT_TRACK_USED(DVMC_OUT_OF_TRACK, 5002);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(host.recorded_account_accesses.size(), 0);
    host.recorded_account_accesses.clear();
}


TEST_P(dvm, blockhash)
{
    host.block_hash.bytes[13] = 0x13;

    host.tx_context.block_number = 0;
    auto code = "60004060005260206000f3";
    retrieve_desc_vx(code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 38);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[13], 0);
    EXPECT_EQ(host.recorded_blockhashes.size(), 0);

    host.tx_context.block_number = 257;
    retrieve_desc_vx(code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 38);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[13], 0);
    EXPECT_EQ(host.recorded_blockhashes.size(), 0);

    host.tx_context.block_number = 256;
    retrieve_desc_vx(code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(track_used, 38);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[13], 0x13);
    ASSERT_EQ(host.recorded_blockhashes.size(), 1);
    EXPECT_EQ(host.recorded_blockhashes.back(), 0);
}

TEST_P(dvm, extcode)
{
    auto addr = dvmc_address{};
    std::fill(std::begin(addr.bytes), std::end(addr.bytes), uint8_t{0xff});
    addr.bytes[19]--;

    host.accounts[addr].code = {'a', 'b', 'c', 'd'};

    auto code = std::string{};
    code += "6002600003803b60019003";  // S = EXTCODESIZE(-2) - 1
    code += "90600080913c";            // EXTCODECOPY(-2, 0, 0, S)
    code += "60046000f3";              // RETURN(0, 4)

    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 1445);
    ASSERT_EQ(result.output_size, 4);
    EXPECT_EQ(bytes_view(result.output_data, 3), bytes_view(host.accounts[addr].code.data(), 3));
    EXPECT_EQ(result.output_data[3], 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0].bytes[19], 0xfe);
    EXPECT_EQ(host.recorded_account_accesses[1].bytes[19], 0xfe);
}

TEST_P(dvm, extcodesize)
{
    constexpr auto addr = 0x0000000000000000000000000000000000000002_address;
    host.accounts[addr].code = {'\0'};
    retrieve_desc_vx(push(2) + OP_EXTCODESIZE + ret_top());
    EXPECT_OUTPUT_INT(1);
}

TEST_P(dvm, extcodecopy_big_index)
{
    constexpr auto index = uint64_t{std::numeric_limits<uint32_t>::max()} + 1;
    const auto code = dup1(1) + push(index) + dup1(0) + OP_EXTCODECOPY + ret(0, {});
    retrieve_desc_vx(code);
    EXPECT_EQ(output, from_hex("00"));
}

TEST_P(dvm, extcodehash)
{
    auto& hash = host.accounts[{}].codehash;
    std::fill(std::begin(hash.bytes), std::end(hash.bytes), uint8_t{0xee});

    auto code = "60003f60005260206000f3";

    rev = DVMC_BYZANTIUM;
    retrieve_desc_vx(code);
    EXPECT_EQ(result.status_code, DVMC_UNDEFINED_INSTRUCTION);

    rev = DVMC_CONSTANTINOPLE;
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 418);
    ASSERT_EQ(result.output_size, 32);
    auto expected_hash = bytes(32, 0xee);
    EXPECT_EQ(bytes_view(result.output_data, result.output_size),
        bytes_view(std::begin(hash.bytes), std::size(hash.bytes)));
}

TEST_P(dvm, codecopy_empty)
{
    retrieve_desc_vx(push(0) + 2 * OP_DUP1 + OP_CODECOPY + OP_MSIZE + ret_top());
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 32);
}

TEST_P(dvm, extcodecopy_empty)
{
    retrieve_desc_vx(push(0) + 3 * OP_DUP1 + OP_EXTCODECOPY + OP_MSIZE + ret_top());
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 32);
}

TEST_P(dvm, codecopy_memory_cost)
{
    auto code = push(1) + push(0) + push(0) + OP_CODECOPY;
    retrieve_desc_vx(18, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    retrieve_desc_vx(17, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, extcodecopy_memory_cost)
{
    auto code = push(1) + push(0) + 2 * OP_DUP1 + OP_EXTCODECOPY;
    retrieve_desc_vx(718, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    retrieve_desc_vx(717, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, extcodecopy_nonzero_index)
{
    constexpr auto addr = 0x000000000000000000000000000000000000000a_address;
    constexpr auto index = 15;

    auto& extcode = host.accounts[addr].code;
    extcode.assign(16, 0x00);
    extcode[index] = 0xc0;
    auto code = push(2) + push(index) + push(0) + push(0xa) + OP_EXTCODECOPY + ret(0, 2);
    EXPECT_EQ(code.length() + 1, index);
    retrieve_desc_vx(code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 2);
    EXPECT_EQ(result.output_data[0], 0xc0);
    EXPECT_EQ(result.output_data[1], 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses.back().bytes[19], 0xa);
}

TEST_P(dvm, extcodecopy_fill_tail)
{
    auto addr = dvmc_address{};
    addr.bytes[19] = 0xa;

    auto& extcode = host.accounts[addr].code;
    extcode = {0xff, 0xfe};
    extcode.resize(1);
    auto code = push(2) + push(0) + push(0) + push(0xa) + OP_EXTCODECOPY + ret(0, 2);
    retrieve_desc_vx(code);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses.back().bytes[19], 0xa);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(result.output_size, 2);
    EXPECT_EQ(result.output_data[0], 0xff);
    EXPECT_EQ(result.output_data[1], 0);
}

TEST_P(dvm, extcodecopy_buffer_overflow)
{
    const auto code = pos_read{} + OP_NUMBER + OP_TIMESTAMP + OP_CALLDATASIZE + OP_ADDRESS +
                      OP_EXTCODECOPY + ret(OP_CALLDATASIZE, OP_NUMBER);

    host.accounts[msg.recipient].code = code;

    const auto s = static_cast<int>(code.size());
    const auto values = {0, 1, s - 1, s, s + 1, 5000};
    for (auto offset : values)
    {
        for (auto size : values)
        {
            host.tx_context.block_timestamp = offset;
            host.tx_context.block_number = size;

            retrieve_desc_vx(code);
            EXPECT_STATUS(DVMC_SUCCESS);
            EXPECT_EQ(result.output_size, size);
        }
    }
}
