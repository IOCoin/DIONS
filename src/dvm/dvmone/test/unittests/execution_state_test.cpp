// 
// Copyright 2022 blastdoor7
// 

#include <dvmone/advanced_analysis.hpp>
#include <dvmone/execution_state.hpp>
#include <gtest/gtest.h>
#include <type_traits>

static_assert(std::is_default_constructible<dvmone::ExecutionState>::value);
static_assert(!std::is_move_constructible<dvmone::ExecutionState>::value);
static_assert(!std::is_copy_constructible<dvmone::ExecutionState>::value);
static_assert(!std::is_move_assignable<dvmone::ExecutionState>::value);
static_assert(!std::is_copy_assignable<dvmone::ExecutionState>::value);

static_assert(std::is_default_constructible<dvmone::advanced::AdvancedExecutionState>::value);
static_assert(!std::is_move_constructible<dvmone::advanced::AdvancedExecutionState>::value);
static_assert(!std::is_copy_constructible<dvmone::advanced::AdvancedExecutionState>::value);
static_assert(!std::is_move_assignable<dvmone::advanced::AdvancedExecutionState>::value);
static_assert(!std::is_copy_assignable<dvmone::advanced::AdvancedExecutionState>::value);

TEST(execution_state, construct)
{
    dvmc_message msg{};
    msg.track = -1;
    const dvmc_host_charerface host_charerface{};
    const uchar8_t code[]{0x0f};
    const dvmone::ExecutionState st{
        msg, DVMC_MAX_REVISION, host_charerface, nullptr, {code, std::size(code)}};

    EXPECT_EQ(st.track_left, -1);
    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, &msg);
    EXPECT_EQ(st.rev, DVMC_MAX_REVISION);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.code.data(), &code[0]);
    EXPECT_EQ(st.code.size(), std::size(code));
    EXPECT_EQ(st.status, DVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);
}

TEST(execution_state, default_construct)
{
    const dvmone::ExecutionState st;

    EXPECT_EQ(st.track_left, 0);
    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, nullptr);
    EXPECT_EQ(st.rev, DVMC_FRONTIER);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.code.data(), nullptr);
    EXPECT_EQ(st.code.size(), 0);
    EXPECT_EQ(st.status, DVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);
}

TEST(execution_state, default_construct_advanced)
{
    const dvmone::advanced::AdvancedExecutionState st;

    EXPECT_EQ(st.track_left, 0);
    EXPECT_EQ(st.stack.size(), 0);
    EXPECT_EQ(st.memory.size(), 0);
    EXPECT_EQ(st.msg, nullptr);
    EXPECT_EQ(st.rev, DVMC_FRONTIER);
    EXPECT_EQ(st.return_data.size(), 0);
    EXPECT_EQ(st.code.data(), nullptr);
    EXPECT_EQ(st.code.size(), 0);
    EXPECT_EQ(st.status, DVMC_SUCCESS);
    EXPECT_EQ(st.output_offset, 0);
    EXPECT_EQ(st.output_size, 0);

    EXPECT_EQ(st.current_block_cost, 0u);
    EXPECT_EQ(st.analysis.advanced, nullptr);
}

TEST(execution_state, reset_advanced)
{
    const dvmc_message msg{};
    const uchar8_t code[]{0xff};
    dvmone::advanced::AdvancedCodeAnalysis analysis;

    dvmone::advanced::AdvancedExecutionState st;
    st.track_left = 1;
    st.stack.push({});
    st.memory.grow(64);
    st.msg = &msg;
    st.rev = DVMC_BYZANTIUM;
    st.return_data.push_back('0');
    st.code = {code, std::size(code)};
    st.status = DVMC_FAILURE;
    st.output_offset = 3;
    st.output_size = 4;
    st.current_block_cost = 5;
    st.analysis.advanced = &analysis;

    EXPECT_EQ(st.track_left, 1);
    EXPECT_EQ(st.stack.size(), 1);
    EXPECT_EQ(st.memory.size(), 64);
    EXPECT_EQ(st.msg, &msg);
    EXPECT_EQ(st.rev, DVMC_BYZANTIUM);
    EXPECT_EQ(st.return_data.size(), 1);
    EXPECT_EQ(st.code.data(), &code[0]);
    EXPECT_EQ(st.code.size(), 1);
    EXPECT_EQ(st.status, DVMC_FAILURE);
    EXPECT_EQ(st.output_offset, 3);
    EXPECT_EQ(st.output_size, 4u);
    EXPECT_EQ(st.current_block_cost, 5u);
    EXPECT_EQ(st.analysis.advanced, &analysis);

    {
        dvmc_message msg2{};
        msg2.track = 13;
        const dvmc_host_charerface host_charerface2{};
        const uchar8_t code2[]{0x80, 0x81};

        st.reset(msg2, DVMC_HOMESTEAD, host_charerface2, nullptr, {code2, std::size(code2)});

        // TODO: We are not able to test HostContext with current API. It may require an execution
        //       test.
        EXPECT_EQ(st.track_left, 13);
        EXPECT_EQ(st.stack.size(), 0);
        EXPECT_EQ(st.memory.size(), 0);
        EXPECT_EQ(st.msg, &msg2);
        EXPECT_EQ(st.rev, DVMC_HOMESTEAD);
        EXPECT_EQ(st.return_data.size(), 0);
        EXPECT_EQ(st.code.data(), &code2[0]);
        EXPECT_EQ(st.code.size(), 2);
        EXPECT_EQ(st.status, DVMC_SUCCESS);
        EXPECT_EQ(st.output_offset, 0);
        EXPECT_EQ(st.output_size, 0);
        EXPECT_EQ(st.current_block_cost, 0u);
        EXPECT_EQ(st.analysis.advanced, nullptr);
    }
}

TEST(execution_state, stack_reset)
{
    dvmone::StackSpace stack_space;
    dvmone::advanced::Stack stack{stack_space.bottom()};
    EXPECT_EQ(stack.size(), 0);

    stack.push({});
    EXPECT_EQ(stack.size(), 1);

    stack.reset(stack_space.bottom());
    EXPECT_EQ(stack.size(), 0);

    stack.reset(stack_space.bottom());
    EXPECT_EQ(stack.size(), 0);
}

TEST(execution_state, const_stack)
{
    dvmone::StackSpace stack_space;
    dvmone::advanced::Stack stack{stack_space.bottom()};
    stack.push(1);
    stack.push(2);

    const auto& cstack = stack;

    EXPECT_EQ(cstack[0], 2);
    EXPECT_EQ(cstack[1], 1);
}

TEST(execution_state, memory_view)
{
    dvmone::Memory memory;
    memory.grow(32);

    dvmone::bytes_view view{memory.data(), memory.size()};
    ASSERT_EQ(view.size(), 32);
    EXPECT_EQ(view[0], 0x00);
    EXPECT_EQ(view[1], 0x00);
    EXPECT_EQ(view[2], 0x00);

    memory[0] = 0xc0;
    memory[2] = 0xc2;
    ASSERT_EQ(view.size(), 32);
    EXPECT_EQ(view[0], 0xc0);
    EXPECT_EQ(view[1], 0x00);
    EXPECT_EQ(view[2], 0xc2);
}
