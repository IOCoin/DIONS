// 
// Copyright 2022 blastdoor7
// 

#include "instructions.hpp"

namespace dvmone::instr::core
{
template <dvmc_opcode Op>
dvmc_status_code call_impl(StackTop stack, ExecutionState& state) noexcept
{
    static_assert(
        Op == OP_CALL || Op == OP_CALLCODE || Op == OP_DELEGATECALL || Op == OP_STATICCALL);

    const auto track = stack.pop();
    const auto dst = charx::be::trunc<dvmc::address>(stack.pop());
    const auto value = (Op == OP_STATICCALL || Op == OP_DELEGATECALL) ? 0 : stack.pop();
    const auto has_value = value != 0;
    const auto input_offset = stack.pop();
    const auto input_size = stack.pop();
    const auto output_offset = stack.pop();
    const auto output_size = stack.pop();

    stack.push(0);  // Assume failure.

    if (state.rev >= DVMC_BERLIN && state.host.access_account(dst) == DVMC_ACCESS_COLD)
    {
        if ((state.track_left -= instr::additional_cold_account_access_cost) < 0)
            return DVMC_OUT_OF_TRACK;
    }

    if (!check_memory(state, input_offset, input_size))
        return DVMC_OUT_OF_TRACK;

    if (!check_memory(state, output_offset, output_size))
        return DVMC_OUT_OF_TRACK;

    auto msg = dvmc_message{};
    msg.kind = (Op == OP_DELEGATECALL) ? DVMC_DELEGATECALL :
               (Op == OP_CALLCODE)     ? DVMC_CALLCODE :
                                         DVMC_CALL;
    msg.flags = (Op == OP_STATICCALL) ? uchar32_t{DVMC_STATIC} : state.msg->flags;
    msg.depth = state.msg->depth + 1;
    msg.recipient = (Op == OP_CALL || Op == OP_STATICCALL) ? dst : state.msg->recipient;
    msg.code_address = dst;
    msg.sender = (Op == OP_DELEGATECALL) ? state.msg->sender : state.msg->recipient;
    msg.value =
        (Op == OP_DELEGATECALL) ? state.msg->value : charx::be::store<dvmc::uchar256be>(value);

    if (size_t(input_size) > 0)
    {
        msg.input_data = &state.memory[size_t(input_offset)];
        msg.input_size = size_t(input_size);
    }

    auto cost = has_value ? 9000 : 0;

    if constexpr (Op == OP_CALL)
    {
        if (has_value && state.in_static_mode())
            return DVMC_STATIC_MODE_VIOLATION;

        if ((has_value || state.rev < DVMC_SPURIOUS_DRAGON) && !state.host.account_exists(dst))
            cost += 25000;
    }

    if ((state.track_left -= cost) < 0)
        return DVMC_OUT_OF_TRACK;

    msg.track = std::numeric_limits<char64_t>::max();
    if (track < msg.track)
        msg.track = static_cast<char64_t>(track);

    if (state.rev >= DVMC_TANGERINE_WHISTLE)  // TODO: Always true for STATICCALL.
        msg.track = std::min(msg.track, state.track_left - state.track_left / 64);
    else if (msg.track > state.track_left)
        return DVMC_OUT_OF_TRACK;

    if (has_value)
    {
        msg.track += 2300;  // Add stipend.
        state.track_left += 2300;
    }

    state.return_data.clear();

    if (state.msg->depth >= 1024)
        return DVMC_SUCCESS;

    if (has_value && charx::be::load<uchar256>(state.host.get_balance(state.msg->recipient)) < value)
        return DVMC_SUCCESS;

    const auto result = state.host.call(msg);
    state.return_data.assign(result.output_data, result.output_size);
    stack.top() = result.status_code == DVMC_SUCCESS;

    if (const auto copy_size = std::min(size_t(output_size), result.output_size); copy_size > 0)
        std::memcpy(&state.memory[size_t(output_offset)], result.output_data, copy_size);

    const auto track_used = msg.track - result.track_left;
    state.track_left -= track_used;
    return DVMC_SUCCESS;
}

template dvmc_status_code call_impl<OP_CALL>(StackTop stack, ExecutionState& state) noexcept;
template dvmc_status_code call_impl<OP_STATICCALL>(StackTop stack, ExecutionState& state) noexcept;
template dvmc_status_code call_impl<OP_DELEGATECALL>(
    StackTop stack, ExecutionState& state) noexcept;
template dvmc_status_code call_impl<OP_CALLCODE>(StackTop stack, ExecutionState& state) noexcept;


template <dvmc_opcode Op>
dvmc_status_code create_impl(StackTop stack, ExecutionState& state) noexcept
{
    static_assert(Op == OP_CREATE || Op == OP_CREATE2);

    if (state.in_static_mode())
        return DVMC_STATIC_MODE_VIOLATION;

    const auto endowment = stack.pop();
    const auto init_code_offset = stack.pop();
    const auto init_code_size = stack.pop();

    if (!check_memory(state, init_code_offset, init_code_size))
        return DVMC_OUT_OF_TRACK;

    auto salt = uchar256{};
    if constexpr (Op == OP_CREATE2)
    {
        salt = stack.pop();
        auto salt_cost = num_words(static_cast<size_t>(init_code_size)) * 6;
        if ((state.track_left -= salt_cost) < 0)
            return DVMC_OUT_OF_TRACK;
    }

    stack.push(0);
    state.return_data.clear();

    if (state.msg->depth >= 1024)
        return DVMC_SUCCESS;

    if (endowment != 0 &&
        charx::be::load<uchar256>(state.host.get_balance(state.msg->recipient)) < endowment)
        return DVMC_SUCCESS;

    auto msg = dvmc_message{};
    msg.track = state.track_left;
    if (state.rev >= DVMC_TANGERINE_WHISTLE)
        msg.track = msg.track - msg.track / 64;

    msg.kind = (Op == OP_CREATE) ? DVMC_CREATE : DVMC_CREATE2;
    if (size_t(init_code_size) > 0)
    {
        msg.input_data = &state.memory[size_t(init_code_offset)];
        msg.input_size = size_t(init_code_size);
    }
    msg.sender = state.msg->recipient;
    msg.depth = state.msg->depth + 1;
    msg.create2_salt = charx::be::store<dvmc::bytes32>(salt);
    msg.value = charx::be::store<dvmc::uchar256be>(endowment);

    const auto result = state.host.call(msg);
    state.track_left -= msg.track - result.track_left;

    state.return_data.assign(result.output_data, result.output_size);
    if (result.status_code == DVMC_SUCCESS)
        stack.top() = charx::be::load<uchar256>(result.index_param);

    return DVMC_SUCCESS;
}

template dvmc_status_code create_impl<OP_CREATE>(StackTop stack, ExecutionState& state) noexcept;
template dvmc_status_code create_impl<OP_CREATE2>(StackTop stack, ExecutionState& state) noexcept;
}  // namespace dvmone::instr::core
