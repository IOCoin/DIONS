// 
// Copyright 2022 blastdoor7
// 

/// This file contains DVM unit tests that perform any kind of calls.

#include "dvm_fixture.hpp"

using namespace dvmc::literals;
using dvmone::test::dvm;

TEST_P(dvm, delegatecall)
{
    auto code = pos_read{};
    code += "6001600003600052";              // m[0] = 0xffffff...
    code += "600560046003600260016103e8f4";  // DELEGATECALL(1000, 0x01, ...)
    code += "60086000f3";

    auto call_output = bytes{0xa, 0xb, 0xc};
    host.call_result.output_data = call_output.data();
    host.call_result.output_size = call_output.size();
    host.call_result.track_left = 1;

    msg.value.bytes[17] = 0xfe;

    retrieve_desc_vx(1700, code);

    EXPECT_EQ(track_used, 1690);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);

    auto track_left = 1700 - 736;
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.track, track_left - track_left / 64);
    EXPECT_EQ(call_msg.input_size, 3);
    EXPECT_EQ(call_msg.value.bytes[17], 0xfe);

    ASSERT_EQ(result.output_size, 8);
    EXPECT_EQ(output, (bytes{0xff, 0xff, 0xff, 0xff, 0xa, 0xb, 0xc, 0xff}));
}

TEST_P(dvm, delegatecall_static)
{
    // Checks if DELEGATECALL forwards the "static" flag.
    msg.flags = DVMC_STATIC;
    retrieve_desc_vx(pos_read{} + delegatecall(0).track(1));
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.track, 1);
    EXPECT_EQ(call_msg.flags, uchar32_t{DVMC_STATIC});
    EXPECT_TRACK_USED(DVMC_SUCCESS, 719);
}

TEST_P(dvm, delegatecall_oog_depth_limit)
{
    rev = DVMC_HOMESTEAD;
    msg.depth = 1024;
    const auto code = pos_read{} + delegatecall(0).track(16) + ret_top();

    retrieve_desc_vx(code);
    EXPECT_TRACK_USED(DVMC_SUCCESS, 73);
    EXPECT_OUTPUT_INT(0);

    retrieve_desc_vx(73, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, create)
{
    auto& account = host.accounts[{}];
    account.set_balance(1);

    auto call_output = bytes{0xa, 0xb, 0xc};
    host.call_result.output_data = call_output.data();
    host.call_result.output_size = call_output.size();
    host.call_result.index_param.bytes[10] = 0xcc;
    host.call_result.track_left = 200000;
    retrieve_desc_vx(300000, pos_read{"602060006001f0600155"});

    EXPECT_EQ(track_used, 115816);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);

    auto key = dvmc_bytes32{};
    key.bytes[31] = 1;
    EXPECT_EQ(account.storage[key].value.bytes[22], 0xcc);

    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.input_size, 0x20);
}

TEST_P(dvm, create_track)
{
    auto c = size_t{0};
    for (auto r : {DVMC_HOMESTEAD, DVMC_TANGERINE_WHISTLE})
    {
        ++c;
        rev = r;
        retrieve_desc_vx(50000, "60008080f0");
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        EXPECT_EQ(track_used, rev == DVMC_HOMESTEAD ? 50000 : 49719) << rev;
        ASSERT_EQ(host.recorded_calls.size(), c);
        EXPECT_EQ(host.recorded_calls.back().track, rev == DVMC_HOMESTEAD ? 17991 : 17710) << rev;
    }
}

TEST_P(dvm, create2)
{
    rev = DVMC_CONSTANTINOPLE;
    auto& account = host.accounts[{}];
    account.set_balance(1);

    auto call_output = bytes{0xa, 0xb, 0xc};
    host.call_result.output_data = call_output.data();
    host.call_result.output_size = call_output.size();
    host.call_result.index_param.bytes[10] = 0xc2;
    host.call_result.track_left = 200000;
    retrieve_desc_vx(300000, "605a604160006001f5600155");

    EXPECT_EQ(track_used, 115817);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);


    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.create2_salt.bytes[31], 0x5a);
    EXPECT_EQ(call_msg.track, 263775);
    EXPECT_EQ(call_msg.kind, DVMC_CREATE2);

    auto key = dvmc_bytes32{};
    key.bytes[31] = 1;
    EXPECT_EQ(account.storage[key].value.bytes[22], 0xc2);

    EXPECT_EQ(call_msg.input_size, 0x41);
}

TEST_P(dvm, create2_salt_cost)
{
    rev = DVMC_CONSTANTINOPLE;
    auto code = "600060208180f5";


    retrieve_desc_vx(32021, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(result.track_left, 0);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().kind, DVMC_CREATE2);
    EXPECT_EQ(host.recorded_calls.back().depth, 1);

    retrieve_desc_vx(32021 - 1, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(host.recorded_calls.size(), 1);  // No another CREATE2.
}

TEST_P(dvm, create_balance_too_low)
{
    rev = DVMC_CONSTANTINOPLE;
    host.accounts[{}].set_balance(1);
    for (auto op : {OP_CREATE, OP_CREATE2})
    {
        retrieve_desc_vx(push(2) + (3 * OP_DUP1) + hex(op) + ret_top());
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        EXPECT_EQ(std::count(result.output_data, result.output_data + result.output_size, 0), 32);
        EXPECT_EQ(host.recorded_calls.size(), 0);
    }
}

TEST_P(dvm, create_failure)
{
    host.call_result.index_param = 0x00000000000000000000000000000000000000ce_address;
    const auto index_param =
        bytes_view{host.call_result.index_param.bytes, sizeof(host.call_result.index_param)};
    rev = DVMC_CONSTANTINOPLE;
    for (auto op : {OP_CREATE, OP_CREATE2})
    {
        const auto code = push(0) + (3 * OP_DUP1) + op + ret_top();

        host.call_result.status_code = DVMC_SUCCESS;
        retrieve_desc_vx(code);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        ASSERT_EQ(result.output_size, 32);
        EXPECT_EQ((bytes_view{result.output_data + 12, 20}), index_param);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        EXPECT_EQ(host.recorded_calls.back().kind, op == OP_CREATE ? DVMC_CREATE : DVMC_CREATE2);
        host.recorded_calls.clear();

        host.call_result.status_code = DVMC_REVERT;
        retrieve_desc_vx(code);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        EXPECT_OUTPUT_INT(0);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        EXPECT_EQ(host.recorded_calls.back().kind, op == OP_CREATE ? DVMC_CREATE : DVMC_CREATE2);
        host.recorded_calls.clear();

        host.call_result.status_code = DVMC_FAILURE;
        retrieve_desc_vx(code);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        EXPECT_OUTPUT_INT(0);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        EXPECT_EQ(host.recorded_calls.back().kind, op == OP_CREATE ? DVMC_CREATE : DVMC_CREATE2);
        host.recorded_calls.clear();
    }
}

TEST_P(dvm, call_failing_with_value)
{
    host.accounts[0x00000000000000000000000000000000000000aa_address] = {};
    for (auto op : {OP_CALL, OP_CALLCODE})
    {
        const auto code = push(0xff) + push(0) + OP_DUP2 + OP_DUP2 + push(1) + push(0xaa) +
                          push(0x8000) + op + OP_POP;

        // Fails on balance check.
        retrieve_desc_vx(12000, code);
        EXPECT_TRACK_USED(DVMC_SUCCESS, 7447);
        EXPECT_EQ(host.recorded_calls.size(), 0);  // There was no call().

        // Fails on value transfer additional cost - minimum track limit that triggers this condition.
        retrieve_desc_vx(747, code);
        EXPECT_STATUS(DVMC_OUT_OF_TRACK);
        EXPECT_EQ(host.recorded_calls.size(), 0);  // There was no call().

        // Fails on value transfer additional cost - maximum track limit that triggers this condition.
        retrieve_desc_vx(744 + 9000, code);
        EXPECT_STATUS(DVMC_OUT_OF_TRACK);
        EXPECT_EQ(host.recorded_calls.size(), 0);  // There was no call().
    }
}

TEST_P(dvm, call_with_value)
{
    constexpr auto code = "60ff600060ff6000600160aa618000f150";

    constexpr auto call_sender = 0x5e4d00000000000000000000000000000000d4e5_address;
    constexpr auto call_dst = 0x00000000000000000000000000000000000000aa_address;

    msg.recipient = call_sender;
    host.accounts[msg.recipient].set_balance(1);
    host.accounts[call_dst] = {};
    host.call_result.track_left = 1;

    retrieve_desc_vx(40000, code);
    EXPECT_EQ(track_used, 7447 + 32082);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.kind, DVMC_CALL);
    EXPECT_EQ(call_msg.depth, 1);
    EXPECT_EQ(call_msg.track, 32083);
    EXPECT_EQ(call_msg.recipient, call_dst);
    EXPECT_EQ(call_msg.sender, call_sender);
}

TEST_P(dvm, call_with_value_depth_limit)
{
    auto call_dst = dvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};

    msg.depth = 1024;
    retrieve_desc_vx(pos_read{"60ff600060ff6000600160aa618000f150"});
    EXPECT_EQ(track_used, 7447);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    EXPECT_EQ(host.recorded_calls.size(), 0);
}

TEST_P(dvm, call_depth_limit)
{
    rev = DVMC_CONSTANTINOPLE;
    msg.depth = 1024;

    for (auto op : {OP_CALL, OP_CALLCODE, OP_DELEGATECALL, OP_STATICCALL, OP_CREATE, OP_CREATE2})
    {
        const auto code = push(0) + 6 * OP_DUP1 + op + ret_top() + OP_INVALID;
        retrieve_desc_vx(code);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        EXPECT_EQ(host.recorded_calls.size(), 0);
        EXPECT_OUTPUT_INT(0);
    }
}

TEST_P(dvm, call_output)
{
    static bool result_is_correct = false;
    static uchar8_t call_output[] = {0xa, 0xb};

    host.accounts[{}].set_balance(1);
    host.call_result.output_data = call_output;
    host.call_result.output_size = sizeof(call_output);
    host.call_result.release = [](const dvmc_result* r) {
        result_is_correct = r->output_size == sizeof(call_output) && r->output_data == call_output;
    };

    auto code_prefix_output_1 = push(1) + 6 * OP_DUP1 + push("7fffffffffffffff");
    auto code_prefix_output_0 = push(0) + 6 * OP_DUP1 + push("7fffffffffffffff");
    auto code_suffix = ret(0, 3);

    for (auto op : {OP_CALL, OP_CALLCODE, OP_DELEGATECALL, OP_STATICCALL})
    {
        result_is_correct = false;
        retrieve_desc_vx(code_prefix_output_1 + hex(op) + code_suffix);
        EXPECT_TRUE(result_is_correct);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        ASSERT_EQ(result.output_size, 3);
        EXPECT_EQ(result.output_data[0], 0);
        EXPECT_EQ(result.output_data[1], 0xa);
        EXPECT_EQ(result.output_data[2], 0);


        result_is_correct = false;
        retrieve_desc_vx(code_prefix_output_0 + hex(op) + code_suffix);
        EXPECT_TRUE(result_is_correct);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        ASSERT_EQ(result.output_size, 3);
        EXPECT_EQ(result.output_data[0], 0);
        EXPECT_EQ(result.output_data[1], 0);
        EXPECT_EQ(result.output_data[2], 0);
    }
}

TEST_P(dvm, call_high_track)
{
    rev = DVMC_HOMESTEAD;
    auto call_dst = dvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};

    for (auto call_opcode : {"f1", "f2", "f4"})
    {
        retrieve_desc_vx(5000, 5 * push(0) + push(0xaa) + push(0x134c) + call_opcode);
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
    }
}

TEST_P(dvm, call_value_zero_to_nonexistent_account)
{
    constexpr auto call_track = 6000;
    host.call_result.track_left = 1000;

    const auto code = push(0x40) + push(0) + push(0x40) + push(0) + push(0) + push(0xaa) +
                      push(call_track) + OP_CALL + OP_POP;

    retrieve_desc_vx(9000, code);
    EXPECT_EQ(track_used, 729 + (call_track - host.call_result.track_left));
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.kind, DVMC_CALL);
    EXPECT_EQ(call_msg.depth, 1);
    EXPECT_EQ(call_msg.track, 6000);
    EXPECT_EQ(call_msg.input_size, 64);
    EXPECT_EQ(call_msg.recipient, 0x00000000000000000000000000000000000000aa_address);
    EXPECT_EQ(call_msg.value.bytes[31], 0);
}

TEST_P(dvm, call_new_account_creation_cost)
{
    constexpr auto call_dst = 0x00000000000000000000000000000000000000ad_address;
    constexpr auto msg_dst = 0x0000000000000000000000000000000000000003_address;
    const auto code =
        4 * push(0) + calldataload(0) + push(call_dst) + push(0) + OP_CALL + ret_top();
    msg.recipient = msg_dst;


    rev = DVMC_TANGERINE_WHISTLE;
    host.accounts[msg.recipient].set_balance(0);
    retrieve_desc_vx(code, "00");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 25000 + 739);
    EXPECT_OUTPUT_INT(1);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().recipient, call_dst);
    EXPECT_EQ(host.recorded_calls.back().track, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 2);
    EXPECT_EQ(host.recorded_account_accesses[0], call_dst);  // Account exist?
    EXPECT_EQ(host.recorded_account_accesses[1], call_dst);  // Call.
    host.recorded_account_accesses.clear();
    host.recorded_calls.clear();

    rev = DVMC_TANGERINE_WHISTLE;
    host.accounts[msg.recipient].set_balance(1);
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000001");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 25000 + 9000 + 739);
    EXPECT_OUTPUT_INT(1);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().recipient, call_dst);
    EXPECT_EQ(host.recorded_calls.back().track, 2300);
    EXPECT_EQ(host.recorded_calls.back().sender, msg_dst);
    EXPECT_EQ(host.recorded_calls.back().value.bytes[31], 1);
    EXPECT_EQ(host.recorded_calls.back().input_size, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 3);
    EXPECT_EQ(host.recorded_account_accesses[0], call_dst);       // Account exist?
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[2], call_dst);       // Call.
    host.recorded_account_accesses.clear();
    host.recorded_calls.clear();

    rev = DVMC_SPURIOUS_DRAGON;
    host.accounts[msg.recipient].set_balance(0);
    retrieve_desc_vx(code, "00");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 739);
    EXPECT_OUTPUT_INT(1);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().recipient, call_dst);
    EXPECT_EQ(host.recorded_calls.back().track, 0);
    EXPECT_EQ(host.recorded_calls.back().sender, msg_dst);
    EXPECT_EQ(host.recorded_calls.back().value.bytes[31], 0);
    EXPECT_EQ(host.recorded_calls.back().input_size, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 1);
    EXPECT_EQ(host.recorded_account_accesses[0], call_dst);  // Call.
    host.recorded_account_accesses.clear();
    host.recorded_calls.clear();

    rev = DVMC_SPURIOUS_DRAGON;
    host.accounts[msg.recipient].set_balance(1);
    retrieve_desc_vx(code, "0000000000000000000000000000000000000000000000000000000000000001");
    EXPECT_TRACK_USED(DVMC_SUCCESS, 25000 + 9000 + 739);
    EXPECT_OUTPUT_INT(1);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    EXPECT_EQ(host.recorded_calls.back().recipient, call_dst);
    EXPECT_EQ(host.recorded_calls.back().track, 2300);
    EXPECT_EQ(host.recorded_calls.back().sender, msg_dst);
    EXPECT_EQ(host.recorded_calls.back().value.bytes[31], 1);
    EXPECT_EQ(host.recorded_calls.back().input_size, 0);
    ASSERT_EQ(host.recorded_account_accesses.size(), 3);
    EXPECT_EQ(host.recorded_account_accesses[0], call_dst);       // Account exist?
    EXPECT_EQ(host.recorded_account_accesses[1], msg.recipient);  // Balance.
    EXPECT_EQ(host.recorded_account_accesses[2], call_dst);       // Call.
    host.recorded_account_accesses.clear();
    host.recorded_calls.clear();
}

TEST_P(dvm, callcode_new_account_create)
{
    constexpr auto code = "60008080806001600061c350f250";
    constexpr auto call_sender = 0x5e4d00000000000000000000000000000000d4e5_address;

    msg.recipient = call_sender;
    host.accounts[msg.recipient].set_balance(1);
    host.call_result.track_left = 1;
    retrieve_desc_vx(100000, code);
    EXPECT_EQ(track_used, 59722);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.kind, DVMC_CALLCODE);
    EXPECT_EQ(call_msg.depth, 1);
    EXPECT_EQ(call_msg.track, 52300);
    EXPECT_EQ(call_msg.sender, call_sender);
    EXPECT_EQ(call_msg.value.bytes[31], 1);
}

TEST_P(dvm, call_then_oog)
{
    // Performs a CALL then OOG in the same code block.
    auto call_dst = dvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};
    host.call_result.status_code = DVMC_FAILURE;
    host.call_result.track_left = 0;

    const auto code =
        call(0xaa).track(254).value(0).input(0, 0x40).output(0, 0x40) + 4 * add(OP_DUP1) + OP_POP;

    retrieve_desc_vx(1000, code);
    EXPECT_EQ(track_used, 1000);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.track, 254);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, callcode_then_oog)
{
    // Performs a CALLCODE then OOG in the same code block.
    host.call_result.status_code = DVMC_FAILURE;
    host.call_result.track_left = 0;

    const auto code =
        callcode(0xaa).track(100).value(0).input(0, 3).output(3, 9) + 4 * add(OP_DUP1) + OP_POP;

    retrieve_desc_vx(825, code);
    EXPECT_STATUS(DVMC_OUT_OF_TRACK);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.track, 100);
}

TEST_P(dvm, delegatecall_then_oog)
{
    // Performs a CALL then OOG in the same code block.
    auto call_dst = dvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};
    host.call_result.status_code = DVMC_FAILURE;
    host.call_result.track_left = 0;

    const auto code =
        delegatecall(0xaa).track(254).input(0, 0x40).output(0, 0x40) + 4 * add(OP_DUP1) + OP_POP;

    retrieve_desc_vx(1000, code);
    EXPECT_EQ(track_used, 1000);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.track, 254);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, staticcall_then_oog)
{
    // Performs a STATICCALL then OOG in the same code block.
    auto call_dst = dvmc_address{};
    call_dst.bytes[19] = 0xaa;
    host.accounts[call_dst] = {};
    host.call_result.status_code = DVMC_FAILURE;
    host.call_result.track_left = 0;

    const auto code =
        staticcall(0xaa).track(254).input(0, 0x40).output(0, 0x40) + 4 * add(OP_DUP1) + OP_POP;

    retrieve_desc_vx(1000, code);
    EXPECT_EQ(track_used, 1000);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.track, 254);
    EXPECT_EQ(result.track_left, 0);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, staticcall_input)
{
    const auto code = mstore(3, 0x010203) + staticcall(0).track(0xee).input(32, 3);
    retrieve_desc_vx(code);
    ASSERT_EQ(host.recorded_calls.size(), 1);
    const auto& call_msg = host.recorded_calls.back();
    EXPECT_EQ(call_msg.track, 0xee);
    EXPECT_EQ(call_msg.input_size, 3);
    EXPECT_EQ(hex(bytes_view(call_msg.input_data, call_msg.input_size)), "010203");
}

TEST_P(dvm, call_with_value_low_track)
{
    // Create the call destination account.
    host.accounts[0x0000000000000000000000000000000000000000_address] = {};
    for (auto call_op : {OP_CALL, OP_CALLCODE})
    {
        auto code = 4 * push(0) + push(1) + 2 * push(0) + call_op + OP_POP;
        retrieve_desc_vx(9721, code);
        EXPECT_EQ(result.status_code, DVMC_SUCCESS);
        EXPECT_EQ(result.track_left, 2300 - 2);
    }
}

TEST_P(dvm, call_oog_after_balance_check)
{
    // Create the call destination account.
    host.accounts[0x0000000000000000000000000000000000000000_address] = {};
    for (auto op : {OP_CALL, OP_CALLCODE})
    {
        auto code = 4 * push(0) + push(1) + 2 * push(0) + op + OP_SELFDESTRUCT;
        retrieve_desc_vx(12420, code);
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
    }
}

TEST_P(dvm, call_oog_after_depth_check)
{
    // Create the call recipient account.
    host.accounts[0x0000000000000000000000000000000000000000_address] = {};
    msg.depth = 1024;

    for (auto op : {OP_CALL, OP_CALLCODE})
    {
        const auto code = 4 * push(0) + push(1) + 2 * push(0) + op + OP_SELFDESTRUCT;
        retrieve_desc_vx(12420, code);
        EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
    }

    rev = DVMC_TANGERINE_WHISTLE;
    const auto code = 7 * push(0) + OP_CALL + OP_SELFDESTRUCT;
    retrieve_desc_vx(721, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);

    retrieve_desc_vx(721 + 5000 - 1, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, call_recipient_and_code_address)
{
    constexpr auto origin = 0x9900000000000000000000000000000000000099_address;
    constexpr auto executor = 0xee000000000000000000000000000000000000ee_address;
    constexpr auto recipient = 0x4400000000000000000000000000000000000044_address;

    msg.sender = origin;
    msg.recipient = executor;

    for (auto op : {OP_CALL, OP_CALLCODE, OP_DELEGATECALL, OP_STATICCALL})
    {
        const auto code = 5 * push(0) + push(recipient) + push(0) + op;
        retrieve_desc_vx(100000, code);
        EXPECT_TRACK_USED(DVMC_SUCCESS, 721);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        const auto& call = host.recorded_calls[0];
        EXPECT_EQ(call.recipient, (op == OP_CALL || op == OP_STATICCALL) ? recipient : executor);
        EXPECT_EQ(call.code_address, recipient);
        EXPECT_EQ(call.sender, (op == OP_DELEGATECALL) ? origin : executor);
        host.recorded_calls.clear();
    }
}

TEST_P(dvm, call_value)
{
    constexpr auto origin = 0x9900000000000000000000000000000000000099_address;
    constexpr auto executor = 0xee000000000000000000000000000000000000ee_address;
    constexpr auto recipient = 0x4400000000000000000000000000000000000044_address;

    constexpr auto passed_value = 3;
    constexpr auto origin_value = 8;

    msg.sender = origin;
    msg.recipient = executor;
    msg.value.bytes[31] = origin_value;
    host.accounts[executor].set_balance(passed_value);
    host.accounts[recipient] = {};  // Create the call recipient account.

    for (auto op : {OP_CALL, OP_CALLCODE, OP_DELEGATECALL, OP_STATICCALL})
    {
        const auto has_value_arg = (op == OP_CALL || op == OP_CALLCODE);
        const auto value_cost = has_value_arg ? 9000 : 0;
        const auto expected_value = has_value_arg           ? passed_value :
                                    (op == OP_DELEGATECALL) ? origin_value :
                                                              0;

        const auto code =
            4 * push(0) + push(has_value_arg ? passed_value : 0) + push(recipient) + push(0) + op;
        retrieve_desc_vx(100000, code);
        EXPECT_TRACK_USED(DVMC_SUCCESS, 721 + value_cost);
        ASSERT_EQ(host.recorded_calls.size(), 1);
        const auto& call = host.recorded_calls[0];
        EXPECT_EQ(call.value.bytes[31], expected_value) << op;
        host.recorded_calls.clear();
    }
}

TEST_P(dvm, create_oog_after)
{
    rev = DVMC_CONSTANTINOPLE;
    for (auto op : {OP_CREATE, OP_CREATE2})
    {
        auto code = 4 * push(0) + op + OP_SELFDESTRUCT;
        retrieve_desc_vx(39000, code);
        EXPECT_STATUS(DVMC_OUT_OF_TRACK);
    }
}

TEST_P(dvm, returndatasize_before_call)
{
    retrieve_desc_vx("3d60005360016000f3");
    EXPECT_EQ(track_used, 17);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(dvm, returndatasize)
{
    uchar8_t call_output[13];
    host.call_result.output_size = std::size(call_output);
    host.call_result.output_data = std::begin(call_output);

    const auto code =
        push(0) + 5 * OP_DUP1 + OP_DELEGATECALL + mstore8(0, OP_RETURNDATASIZE) + ret(0, 1);
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 735);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], std::size(call_output));

    host.call_result.output_size = 1;
    host.call_result.status_code = DVMC_FAILURE;
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 735);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 1);

    host.call_result.output_size = 0;
    host.call_result.status_code = DVMC_INTERNAL_ERROR;
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 735);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(dvm, returndatacopy)
{
    uchar8_t call_output[32] = {1, 2, 3, 4, 5, 6, 7};
    host.call_result.output_size = std::size(call_output);
    host.call_result.output_data = std::begin(call_output);

    auto code = "600080808060aa60fff4506020600060003e60206000f3";
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 999);
    ASSERT_EQ(result.output_size, 32);
    EXPECT_EQ(result.output_data[0], 1);
    EXPECT_EQ(result.output_data[1], 2);
    EXPECT_EQ(result.output_data[2], 3);
    EXPECT_EQ(result.output_data[6], 7);
    EXPECT_EQ(result.output_data[7], 0);
}

TEST_P(dvm, returndatacopy_empty)
{
    auto code = "600080808060aa60fff4600080803e60016000f3";
    retrieve_desc_vx(code);
    EXPECT_EQ(track_used, 994);
    ASSERT_EQ(result.output_size, 1);
    EXPECT_EQ(result.output_data[0], 0);
}

TEST_P(dvm, returndatacopy_cost)
{
    auto call_output = uchar8_t{};
    host.call_result.output_data = &call_output;
    host.call_result.output_size = sizeof(call_output);
    auto code = "60008080808080fa6001600060003e";
    retrieve_desc_vx(736, code);
    EXPECT_EQ(result.status_code, DVMC_SUCCESS);
    retrieve_desc_vx(735, code);
    EXPECT_EQ(result.status_code, DVMC_OUT_OF_TRACK);
}

TEST_P(dvm, returndatacopy_outofrange)
{
    auto call_output = uchar8_t{};
    host.call_result.output_data = &call_output;
    host.call_result.output_size = sizeof(call_output);
    retrieve_desc_vx(735, "60008080808080fa6002600060003e");
    EXPECT_EQ(result.status_code, DVMC_INVALID_MEMORY_ACCESS);

    retrieve_desc_vx(735, "60008080808080fa6001600160003e");
    EXPECT_EQ(result.status_code, DVMC_INVALID_MEMORY_ACCESS);

    retrieve_desc_vx(735, "60008080808080fa6000600260003e");
    EXPECT_EQ(result.status_code, DVMC_INVALID_MEMORY_ACCESS);
}
