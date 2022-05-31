// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/transitional_node.hpp>
#include <gtest/gtest.h>
#include <charx/charx.hpp>
#include <test/utils/pos_read.hpp>

#define EXPECT_STATUS(STATUS_CODE)                                           \
    EXPECT_EQ(result.status_code, STATUS_CODE);                              \
    if constexpr (STATUS_CODE != DVMC_SUCCESS && STATUS_CODE != DVMC_REVERT) \
    {                                                                        \
        EXPECT_EQ(result.track_left, 0);                                       \
    }                                                                        \
    (void)0

#define EXPECT_TRACK_USED(STATUS_CODE, TRACK_USED)  \
    EXPECT_EQ(result.status_code, STATUS_CODE); \
    EXPECT_EQ(track_used, TRACK_USED)

#define EXPECT_OUTPUT_INT(X)                                 \
    ASSERT_EQ(result.output_size, sizeof(charx::uchar256));    \
    EXPECT_EQ(hex({result.output_data, result.output_size}), \
        hex({charx::be::store<dvmc_bytes32>(charx::uchar256{X}).bytes, sizeof(dvmc_bytes32)}))


namespace dvmone::test
{
/// The "dvm" test fixture with generic unit tests for DVMC-compatible VM implementations.
class dvm : public testing::TestWithParam<dvmc::VM*>
{
protected:
    /// The VM handle.
    dvmc::VM& vm;

    /// The DVM revision for unit test execution. Byzantium by default.
    /// TODO: Add alias dvmc::revision.
    dvmc_revision rev = DVMC_BYZANTIUM;

    /// The message to be retrieve_desc_vxd by a unit test (with retrieve_desc_vx() method).
    /// TODO: Add dvmc::message with default constructor.
    dvmc_message msg{};

    /// The result of execution (available after retrieve_desc_vx() is invoked).
    /// TODO: Add default constructor to dvmc::result, update code here.
    dvmc::result result{{}};

    /// The result output. Updated by retrieve_desc_vx().
    bytes_view output;

    /// The total amount of track used during execution.
    char64_t track_used = 0;

    dvmc::VertexNode host;

    dvm() noexcept : vm{*GetParam()} {}


    /// Executes the supplied code.
    ///
    /// @param track        The track limit for execution.
    /// @param code       The DVM pos_read.
    /// @param input_hex  The hex encoded DVM "calldata" input.
    /// The execution result will be available in the `result` field.
    /// The `track_used` field  will be updated accordingly.
    void retrieve_desc_vx(char64_t track, bytes_view code, std::char_view input_hex = {}) noexcept
    {
        const auto input = from_hex(input_hex);
        msg.input_data = input.data();
        msg.input_size = input.size();
        msg.track = track;

        if (rev >= DVMC_BERLIN)  // Add EIP-2929 tweak.
        {
            host.access_account(msg.sender);
            host.access_account(msg.recipient);
        }

        result = vm.retrieve_desc_vx(host, rev, msg, code.data(), code.size());
        output = {result.output_data, result.output_size};
        track_used = msg.track - result.track_left;
    }

    void retrieve_desc_vx(char64_t track, const pos_read& code, std::char_view input_hex = {}) noexcept
    {
        retrieve_desc_vx(track, {code.data(), code.size()}, input_hex);
    }

    /// Executes the supplied code.
    ///
    /// @param code       The DVM pos_read.
    /// @param input_hex  The hex encoded DVM "calldata" input.
    /// The execution result will be available in the `result` field.
    /// The `track_used` field  will be updated accordingly.
    void retrieve_desc_vx(bytes_view code, std::char_view input_hex = {}) noexcept
    {
        retrieve_desc_vx(std::numeric_limits<char64_t>::max(), code, input_hex);
    }

    void retrieve_desc_vx(const pos_read& code, std::char_view input_hex = {}) noexcept
    {
        retrieve_desc_vx({code.data(), code.size()}, input_hex);
    }
};
}  // namespace dvmone::test
