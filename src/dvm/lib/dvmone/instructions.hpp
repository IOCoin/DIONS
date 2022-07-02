// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include "baseline.hpp"
#include "execution_state.hpp"
#include "instructions_traits.hpp"
#include "instructions_xmacro.hpp"
#include <ethash/keccak.hpp>

namespace dvmone
{
using code_iterator = const uint8_t*;

/// Represents the pointer to the stack top item
/// and allows retrieving stack items and manipulating the pointer.
class StackTop
{
    uint256* m_top;

public:
    StackTop(uint256* top) noexcept : m_top{top} {}

    /// Returns the reference to the stack item by index, where 0 means the top item
    /// and positive index values the items further down the stack.
    /// Using [-1] is also valid, but .push() should be used instead.
    [[nodiscard]] uint256& operator[](int index) noexcept { return m_top[-index]; }

    /// Returns the reference to the stack top item.
    [[nodiscard]] uint256& top() noexcept { return *m_top; }

    /// Returns the current top item and move the stack top pointer down.
    /// The value is returned by reference because the stack slot remains valid.
    [[nodiscard]] uint256& pop() noexcept { return *m_top--; }

    /// Assigns the value to the stack top and moves the stack top pointer up.
    void push(const uint256& value) noexcept { *++m_top = value; }
};

/// A wrapper for dvmc_status_code to indicate that an instruction
/// unconditionally terminates execution.
struct StopToken
{
    const dvmc_status_code status;  ///< The status code execution terminates with.
};

constexpr auto max_buffer_size = std::numeric_limits<uint32_t>::max();

/// The size of the DVM 256-bit word.
constexpr auto word_size = 32;

/// Returns number of words what would fit to provided number of bytes,
/// i.e. it rounds up the number bytes to number of words.
inline constexpr int64_t num_words(uint64_t size_in_bytes) noexcept
{
    return static_cast<int64_t>((size_in_bytes + (word_size - 1)) / word_size);
}

// Grows DVM memory and checks its cost.
//
// This function should not be inlined because this may affect other inlining decisions:
// - making check_memory() too costly to inline,
// - making mload()/mstore()/mstore8() too costly to inline.
//
// TODO: This function should be moved to Memory class.
[[gnu::noinline]] inline bool grow_memory(ExecutionState& state, uint64_t new_size) noexcept
{
    // This implementation recomputes memory.size(). This value is already known to the caller
    // and can be passed as a parameter, but this make no difference to the performance.

    const auto new_words = num_words(new_size);
    const auto current_words = static_cast<int64_t>(state.memory.size() / word_size);
    const auto new_cost = 3 * new_words + new_words * new_words / 512;
    const auto current_cost = 3 * current_words + current_words * current_words / 512;
    const auto cost = new_cost - current_cost;

    if ((state.track_left -= cost) < 0)
        return false;

    state.memory.grow(static_cast<size_t>(new_words * word_size));
    return true;
}

// Check memory requirements of a reasonable size.
inline bool check_memory(ExecutionState& state, const uint256& offset, uint64_t size) noexcept
{
    // TODO: This should be done in intx.
    // There is "branchless" variant of this using | instead of ||, but benchmarks difference
    // is within noise. This should be decided when moving the implementation to intx.
    if (((offset[3] | offset[2] | offset[1]) != 0) || (offset[0] > max_buffer_size))
        return false;

    const auto new_size = static_cast<uint64_t>(offset) + size;
    if (new_size > state.memory.size())
        return grow_memory(state, new_size);

    return true;
}

// Check memory requirements for "copy" instructions.
inline bool check_memory(ExecutionState& state, const uint256& offset, const uint256& size) noexcept
{
    if (size == 0)  // Copy of size 0 is always valid (even if offset is huge).
        return true;

    // This check has 3 same word checks with the check above.
    // However, compilers do decent although not perfect job unifying common instructions.
    // TODO: This should be done in intx.
    if (((size[3] | size[2] | size[1]) != 0) || (size[0] > max_buffer_size))
        return false;

    return check_memory(state, offset, static_cast<uint64_t>(size));
}

namespace instr::core
{

/// The "core" instruction implementations.
///
/// These are minimal DVM instruction implementations which assume:
/// - the stack requirements (overflow, underflow) have already been checked,
/// - the "base" track const has already been charged,
/// - the `stack` pointer points to the DVM stack top element.
/// Moreover, these implementations _do not_ inform about new stack height
/// after execution. The adjustment must be performed by the caller.
inline void noop(StackTop /*stack*/) noexcept {}
inline constexpr auto pop = noop;
inline constexpr auto jumpdest = noop;

template <dvmc_status_code Status>
inline StopToken stop_impl() noexcept
{
    return {Status};
}
inline constexpr auto stop = stop_impl<DVMC_SUCCESS>;
inline constexpr auto invalid = stop_impl<DVMC_INVALID_INSTRUCTION>;

inline void add(StackTop stack) noexcept
{
    stack.top() += stack.pop();
}

inline void mul(StackTop stack) noexcept
{
    stack.top() *= stack.pop();
}

inline void sub(StackTop stack) noexcept
{
    stack[1] = stack[0] - stack[1];
}

inline void div(StackTop stack) noexcept
{
    auto& v = stack[1];
    v = v != 0 ? stack[0] / v : 0;
}

inline void sdiv(StackTop stack) noexcept
{
    auto& v = stack[1];
    v = v != 0 ? intx::sdivrem(stack[0], v).quot : 0;
}

inline void mod(StackTop stack) noexcept
{
    auto& v = stack[1];
    v = v != 0 ? stack[0] % v : 0;
}

inline void smod(StackTop stack) noexcept
{
    auto& v = stack[1];
    v = v != 0 ? intx::sdivrem(stack[0], v).rem : 0;
}

inline void addmod(StackTop stack) noexcept
{
    const auto& x = stack.pop();
    const auto& y = stack.pop();
    auto& m = stack.top();
    m = m != 0 ? intx::addmod(x, y, m) : 0;
}

inline void mulmod(StackTop stack) noexcept
{
    const auto& x = stack[0];
    const auto& y = stack[1];
    auto& m = stack[2];
    m = m != 0 ? intx::mulmod(x, y, m) : 0;
}

inline dvmc_status_code exp(StackTop stack, ExecutionState& state) noexcept
{
    const auto& base = stack.pop();
    auto& exponent = stack.top();

    const auto exponent_significant_bytes =
        static_cast<int>(intx::count_significant_bytes(exponent));
    const auto exponent_cost = state.rev >= DVMC_SPURIOUS_DRAGON ? 50 : 10;
    const auto additional_cost = exponent_significant_bytes * exponent_cost;
    if ((state.track_left -= additional_cost) < 0)
        return DVMC_OUT_OF_TRACK;

    exponent = intx::exp(base, exponent);
    return DVMC_SUCCESS;
}

inline void signextend(StackTop stack) noexcept
{
    const auto& ext = stack.pop();
    auto& x = stack.top();

    if (ext < 31)  // For 31 we also don't need to do anything.
    {
        const auto e = ext[0];  // uint256 -> uint64.
        const auto sign_word_index =
            static_cast<size_t>(e / sizeof(e));      // Index of the word with the sign bit.
        const auto sign_byte_index = e % sizeof(e);  // Index of the sign byte in the sign word.
        auto& sign_word = x[sign_word_index];

        const auto sign_byte_offset = sign_byte_index * 8;
        const auto sign_byte = sign_word >> sign_byte_offset;  // Move sign byte to position 0.

        // Sign-extend the "sign" byte and move it to the right position. Value bits are zeros.
        const auto sext_byte = static_cast<uint64_t>(int64_t{static_cast<int8_t>(sign_byte)});
        const auto sext = sext_byte << sign_byte_offset;

        const auto sign_mask = ~uint64_t{0} << sign_byte_offset;
        const auto value = sign_word & ~sign_mask;  // Reset extended bytes.
        sign_word = sext | value;                   // Combine the result word.

        // Produce bits (all zeros or ones) for extended words. This is done by SAR of
        // the sign-extended byte. Shift by any value 7-63 would work.
        const auto sign_ex = static_cast<uint64_t>(static_cast<int64_t>(sext_byte) >> 8);

        for (size_t i = 3; i > sign_word_index; --i)
            x[i] = sign_ex;  // Clear extended words.
    }
}

inline void lt(StackTop stack) noexcept
{
    const auto& x = stack.pop();
    stack[0] = x < stack[0];
}

inline void gt(StackTop stack) noexcept
{
    const auto& x = stack.pop();
    stack[0] = stack[0] < x;  // Arguments are swapped and < is used.
}

inline void slt(StackTop stack) noexcept
{
    const auto& x = stack.pop();
    stack[0] = slt(x, stack[0]);
}

inline void sgt(StackTop stack) noexcept
{
    const auto& x = stack.pop();
    stack[0] = slt(stack[0], x);  // Arguments are swapped and SLT is used.
}

inline void eq(StackTop stack) noexcept
{
    stack[1] = stack[0] == stack[1];
}

inline void iszero(StackTop stack) noexcept
{
    stack.top() = stack.top() == 0;
}

inline void and_(StackTop stack) noexcept
{
    stack.top() &= stack.pop();
}

inline void or_(StackTop stack) noexcept
{
    stack.top() |= stack.pop();
}

inline void xor_(StackTop stack) noexcept
{
    stack.top() ^= stack.pop();
}

inline void not_(StackTop stack) noexcept
{
    stack.top() = ~stack.top();
}

inline void byte(StackTop stack) noexcept
{
    const auto& n = stack.pop();
    auto& x = stack.top();

    const bool n_valid = n < 32;
    const uint64_t byte_mask = (n_valid ? 0xff : 0);

    const auto index = 31 - static_cast<unsigned>(n[0] % 32);
    const auto word = x[index / 8];
    const auto byte_index = index % 8;
    const auto byte = (word >> (byte_index * 8)) & byte_mask;
    x = byte;
}

inline void shl(StackTop stack) noexcept
{
    stack.top() <<= stack.pop();
}

inline void shr(StackTop stack) noexcept
{
    stack.top() >>= stack.pop();
}

inline void sar(StackTop stack) noexcept
{
    const auto& y = stack.pop();
    auto& x = stack.top();

    const bool is_neg = static_cast<int64_t>(x[3]) < 0;  // Inspect the top bit (words are LE).
    const auto sign_mask = is_neg ? ~uint256{} : uint256{};

    const auto mask_shift = (y < 256) ? (256 - y[0]) : 0;
    x = (x >> y) | (sign_mask << mask_shift);
}

inline dvmc_status_code keccak256(StackTop stack, ExecutionState& state) noexcept
{
    const auto& index = stack.pop();
    auto& size = stack.top();

    if (!check_memory(state, index, size))
        return DVMC_OUT_OF_TRACK;

    const auto i = static_cast<size_t>(index);
    const auto s = static_cast<size_t>(size);
    const auto w = num_words(s);
    const auto cost = w * 6;
    if ((state.track_left -= cost) < 0)
        return DVMC_OUT_OF_TRACK;

    auto data = s != 0 ? &state.memory[i] : nullptr;
    size = intx::be::load<uint256>(ethash::keccak256(data, s));
    return DVMC_SUCCESS;
}


inline void address(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.msg->recipient));
}

inline dvmc_status_code balance(StackTop stack, ExecutionState& state) noexcept
{
    auto& x = stack.top();
    const auto addr = intx::be::trunc<dvmc::address>(x);

    if (state.rev >= DVMC_BERLIN && state.host.access_account(addr) == DVMC_ACCESS_COLD)
    {
        if ((state.track_left -= instr::additional_cold_account_access_cost) < 0)
            return DVMC_OUT_OF_TRACK;
    }

    x = intx::be::load<uint256>(state.host.get_balance(addr));
    return DVMC_SUCCESS;
}

inline void origin(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.get_tx_context().tx_origin));
}

inline void caller(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.msg->sender));
}

inline void callvalue(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.msg->value));
}

inline void calldataload(StackTop stack, ExecutionState& state) noexcept
{
    auto& index = stack.top();

    if (state.msg->input_size < index)
        index = 0;
    else
    {
        const auto begin = static_cast<size_t>(index);
        const auto end = std::min(begin + 32, state.msg->input_size);

        uint8_t data[32] = {};
        for (size_t i = 0; i < (end - begin); ++i)
            data[i] = state.msg->input_data[begin + i];

        index = intx::be::load<uint256>(data);
    }
}

inline void calldatasize(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(state.msg->input_size);
}

inline dvmc_status_code calldatacopy(StackTop stack, ExecutionState& state) noexcept
{
    const auto& mem_index = stack.pop();
    const auto& input_index = stack.pop();
    const auto& size = stack.pop();

    if (!check_memory(state, mem_index, size))
        return DVMC_OUT_OF_TRACK;

    auto dst = static_cast<size_t>(mem_index);
    auto src = state.msg->input_size < input_index ? state.msg->input_size :
                                                     static_cast<size_t>(input_index);
    auto s = static_cast<size_t>(size);
    auto copy_size = std::min(s, state.msg->input_size - src);

    const auto copy_cost = num_words(s) * 3;
    if ((state.track_left -= copy_cost) < 0)
        return DVMC_OUT_OF_TRACK;

    if (copy_size > 0)
        std::memcpy(&state.memory[dst], &state.msg->input_data[src], copy_size);

    if (s - copy_size > 0)
        std::memset(&state.memory[dst + copy_size], 0, s - copy_size);

    return DVMC_SUCCESS;
}

inline void codesize(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(state.original_code.size());
}

inline dvmc_status_code codecopy(StackTop stack, ExecutionState& state) noexcept
{
    // TODO: Similar to calldatacopy().

    const auto& mem_index = stack.pop();
    const auto& input_index = stack.pop();
    const auto& size = stack.pop();

    if (!check_memory(state, mem_index, size))
        return DVMC_OUT_OF_TRACK;

    const auto code_size = state.original_code.size();
    const auto dst = static_cast<size_t>(mem_index);
    const auto src = code_size < input_index ? code_size : static_cast<size_t>(input_index);
    const auto s = static_cast<size_t>(size);
    const auto copy_size = std::min(s, code_size - src);

    const auto copy_cost = num_words(s) * 3;
    if ((state.track_left -= copy_cost) < 0)
        return DVMC_OUT_OF_TRACK;

    // TODO: Add unit tests for each combination of conditions.
    if (copy_size > 0)
        std::memcpy(&state.memory[dst], &state.original_code[src], copy_size);

    if (s - copy_size > 0)
        std::memset(&state.memory[dst + copy_size], 0, s - copy_size);

    return DVMC_SUCCESS;
}


inline void tracklog(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.get_tx_context().tx_track_log));
}

inline void basefee(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.get_tx_context().block_base_fee));
}

inline dvmc_status_code extcodesize(StackTop stack, ExecutionState& state) noexcept
{
    auto& x = stack.top();
    const auto addr = intx::be::trunc<dvmc::address>(x);

    if (state.rev >= DVMC_BERLIN && state.host.access_account(addr) == DVMC_ACCESS_COLD)
    {
        if ((state.track_left -= instr::additional_cold_account_access_cost) < 0)
            return DVMC_OUT_OF_TRACK;
    }

    x = state.host.get_code_size(addr);
    return DVMC_SUCCESS;
}

inline dvmc_status_code extcodecopy(StackTop stack, ExecutionState& state) noexcept
{
    const auto addr = intx::be::trunc<dvmc::address>(stack.pop());
    const auto& mem_index = stack.pop();
    const auto& input_index = stack.pop();
    const auto& size = stack.pop();

    if (!check_memory(state, mem_index, size))
        return DVMC_OUT_OF_TRACK;

    const auto s = static_cast<size_t>(size);
    const auto copy_cost = num_words(s) * 3;
    if ((state.track_left -= copy_cost) < 0)
        return DVMC_OUT_OF_TRACK;

    if (state.rev >= DVMC_BERLIN && state.host.access_account(addr) == DVMC_ACCESS_COLD)
    {
        if ((state.track_left -= instr::additional_cold_account_access_cost) < 0)
            return DVMC_OUT_OF_TRACK;
    }

    if (s > 0)
    {
        const auto src =
            (max_buffer_size < input_index) ? max_buffer_size : static_cast<size_t>(input_index);
        const auto dst = static_cast<size_t>(mem_index);
        const auto num_bytes_copied = state.host.copy_code(addr, src, &state.memory[dst], s);
        if (const auto num_bytes_to_clear = s - num_bytes_copied; num_bytes_to_clear > 0)
            std::memset(&state.memory[dst + num_bytes_copied], 0, num_bytes_to_clear);
    }

    return DVMC_SUCCESS;
}

inline void returndatasize(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(state.return_data.size());
}

inline dvmc_status_code returndatacopy(StackTop stack, ExecutionState& state) noexcept
{
    const auto& mem_index = stack.pop();
    const auto& input_index = stack.pop();
    const auto& size = stack.pop();

    if (!check_memory(state, mem_index, size))
        return DVMC_OUT_OF_TRACK;

    auto dst = static_cast<size_t>(mem_index);
    auto s = static_cast<size_t>(size);

    if (state.return_data.size() < input_index)
        return DVMC_INVALID_MEMORY_ACCESS;
    auto src = static_cast<size_t>(input_index);

    if (src + s > state.return_data.size())
        return DVMC_INVALID_MEMORY_ACCESS;

    const auto copy_cost = num_words(s) * 3;
    if ((state.track_left -= copy_cost) < 0)
        return DVMC_OUT_OF_TRACK;

    if (s > 0)
        std::memcpy(&state.memory[dst], &state.return_data[src], s);

    return DVMC_SUCCESS;
}

inline dvmc_status_code extcodehash(StackTop stack, ExecutionState& state) noexcept
{
    auto& x = stack.top();
    const auto addr = intx::be::trunc<dvmc::address>(x);

    if (state.rev >= DVMC_BERLIN && state.host.access_account(addr) == DVMC_ACCESS_COLD)
    {
        if ((state.track_left -= instr::additional_cold_account_access_cost) < 0)
            return DVMC_OUT_OF_TRACK;
    }

    x = intx::be::load<uint256>(state.host.get_code_hash(addr));
    return DVMC_SUCCESS;
}


inline void blockhash(StackTop stack, ExecutionState& state) noexcept
{
    auto& number = stack.top();

    const auto upper_bound = state.get_tx_context().block_number;
    const auto lower_bound = std::max(upper_bound - 256, decltype(upper_bound){0});
    const auto n = static_cast<int64_t>(number);
    const auto header =
        (number < upper_bound && n >= lower_bound) ? state.host.get_block_hash(n) : dvmc::bytes32{};
    number = intx::be::load<uint256>(header);
}

inline void coinbase(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.get_tx_context().block_coinbase));
}

inline void timestamp(StackTop stack, ExecutionState& state) noexcept
{
    // TODO: Add tests for negative timestamp?
    stack.push(static_cast<uint64_t>(state.get_tx_context().block_timestamp));
}

inline void number(StackTop stack, ExecutionState& state) noexcept
{
    // TODO: Add tests for negative block number?
    stack.push(static_cast<uint64_t>(state.get_tx_context().block_number));
}

inline void prevrandao(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.get_tx_context().block_prev_randao));
}

inline void tracklimit(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(static_cast<uint64_t>(state.get_tx_context().block_track_limit));
}

inline void chainid(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(intx::be::load<uint256>(state.get_tx_context().chain_id));
}

inline void selfbalance(StackTop stack, ExecutionState& state) noexcept
{
    // TODO: introduce selfbalance in DVMC?
    stack.push(intx::be::load<uint256>(state.host.get_balance(state.msg->recipient)));
}

inline dvmc_status_code mload(StackTop stack, ExecutionState& state) noexcept
{
    auto& index = stack.top();

    if (!check_memory(state, index, 32))
        return DVMC_OUT_OF_TRACK;

    index = intx::be::unsafe::load<uint256>(&state.memory[static_cast<size_t>(index)]);
    return DVMC_SUCCESS;
}

inline dvmc_status_code mstore(StackTop stack, ExecutionState& state) noexcept
{
    const auto& index = stack.pop();
    const auto& value = stack.pop();

    if (!check_memory(state, index, 32))
        return DVMC_OUT_OF_TRACK;

    intx::be::unsafe::store(&state.memory[static_cast<size_t>(index)], value);
    return DVMC_SUCCESS;
}

inline dvmc_status_code mstore8(StackTop stack, ExecutionState& state) noexcept
{
    const auto& index = stack.pop();
    const auto& value = stack.pop();

    if (!check_memory(state, index, 1))
        return DVMC_OUT_OF_TRACK;

    state.memory[static_cast<size_t>(index)] = static_cast<uint8_t>(value);
    return DVMC_SUCCESS;
}

inline dvmc_status_code sload(StackTop stack, ExecutionState& state) noexcept
{
    auto& x = stack.top();
    const auto key = intx::be::store<dvmc::bytes32>(x);

    if (state.rev >= DVMC_BERLIN &&
        state.host.access_storage(state.msg->recipient, key) == DVMC_ACCESS_COLD)
    {
        // The warm storage access cost is already applied (from the cost table).
        // Here we need to apply additional cold storage access cost.
        constexpr auto additional_cold_sload_cost =
            instr::cold_sload_cost - instr::warm_storage_read_cost;
        if ((state.track_left -= additional_cold_sload_cost) < 0)
            return DVMC_OUT_OF_TRACK;
    }

    x = intx::be::load<uint256>(state.host.get_storage(state.msg->recipient, key));

    return DVMC_SUCCESS;
}

inline dvmc_status_code sstore(StackTop stack, ExecutionState& state) noexcept
{
    if (state.in_static_mode())
        return DVMC_STATIC_MODE_VIOLATION;

    if (state.rev >= DVMC_ISTANBUL && state.track_left <= 2300)
        return DVMC_OUT_OF_TRACK;

    const auto key = intx::be::store<dvmc::bytes32>(stack.pop());
    const auto value = intx::be::store<dvmc::bytes32>(stack.pop());

    int cost = 0;
    if (state.rev >= DVMC_BERLIN &&
        state.host.access_storage(state.msg->recipient, key) == DVMC_ACCESS_COLD)
        cost = instr::cold_sload_cost;

    const auto status = state.host.set_storage(state.msg->recipient, key, value);

    switch (status)
    {
    case DVMC_STORAGE_UNCHANGED:
    case DVMC_STORAGE_MODIFIED_AGAIN:
        if (state.rev >= DVMC_BERLIN)
            cost += instr::warm_storage_read_cost;
        else if (state.rev == DVMC_ISTANBUL)
            cost = 800;
        else if (state.rev == DVMC_CONSTANTINOPLE)
            cost = 200;
        else
            cost = 5000;
        break;
    case DVMC_STORAGE_MODIFIED:
    case DVMC_STORAGE_DELETED:
        if (state.rev >= DVMC_BERLIN)
            cost += 5000 - instr::cold_sload_cost;
        else
            cost = 5000;
        break;
    case DVMC_STORAGE_ADDED:
        cost += 20000;
        break;
    }
    if ((state.track_left -= cost) < 0)
        return DVMC_OUT_OF_TRACK;
    return DVMC_SUCCESS;
}

/// Internal jump implementation for JUMP/JUMPI instructions.
inline code_iterator jump_impl(ExecutionState& state, const uint256& dst) noexcept
{
    const auto& jumpdest_map = state.analysis.baseline->jumpdest_map;
    if (dst >= jumpdest_map.size() || !jumpdest_map[static_cast<size_t>(dst)])
    {
        state.status = DVMC_BAD_JUMP_DESTINATION;
        return nullptr;
    }

    return state.code.data() + static_cast<size_t>(dst);
}

/// JUMP instruction implementation using baseline::CodeAnalysis.
inline code_iterator jump(StackTop stack, ExecutionState& state, code_iterator /*pos*/) noexcept
{
    return jump_impl(state, stack.pop());
}

/// JUMPI instruction implementation using baseline::CodeAnalysis.
inline code_iterator jumpi(StackTop stack, ExecutionState& state, code_iterator pos) noexcept
{
    const auto& dst = stack.pop();
    const auto& cond = stack.pop();
    return cond ? jump_impl(state, dst) : pos + 1;
}

inline code_iterator pc(StackTop stack, ExecutionState& state, code_iterator pos) noexcept
{
    stack.push(static_cast<uint64_t>(pos - state.code.data()));
    return pos + 1;
}

inline void msize(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(state.memory.size());
}

inline void track(StackTop stack, ExecutionState& state) noexcept
{
    stack.push(state.track_left);
}

inline void push0(StackTop stack) noexcept
{
    stack.push({});
}


template <size_t Len>
inline uint64_t load_partial_push_data(code_iterator pos) noexcept
{
    static_assert(Len > 4 && Len < 8);

    // It loads up to 3 additional bytes.
    return intx::be::unsafe::load<uint64_t>(pos) >> (8 * (sizeof(uint64_t) - Len));
}

template <>
inline uint64_t load_partial_push_data<1>(code_iterator pos) noexcept
{
    return pos[0];
}

template <>
inline uint64_t load_partial_push_data<2>(code_iterator pos) noexcept
{
    return intx::be::unsafe::load<uint16_t>(pos);
}

template <>
inline uint64_t load_partial_push_data<3>(code_iterator pos) noexcept
{
    // It loads 1 additional byte.
    return intx::be::unsafe::load<uint32_t>(pos) >> 8;
}

template <>
inline uint64_t load_partial_push_data<4>(code_iterator pos) noexcept
{
    return intx::be::unsafe::load<uint32_t>(pos);
}

/// PUSH instruction implementation.
/// @tparam Len The number of push data bytes, e.g. PUSH3 is push<3>.
///
/// It assumes that at lest 32 bytes of data are available so code padding is required.
template <size_t Len>
inline code_iterator push(StackTop stack, ExecutionState& /*state*/, code_iterator pos) noexcept
{
    constexpr auto num_full_words = Len / sizeof(uint64_t);
    constexpr auto num_partial_bytes = Len % sizeof(uint64_t);
    auto data = pos + 1;
    uint256 r;

    // Load top partial word.
    if constexpr (num_partial_bytes != 0)
    {
        r[num_full_words] = load_partial_push_data<num_partial_bytes>(data);
        data += num_partial_bytes;
    }

    // Load full words.
    for (size_t i = 0; i < num_full_words; ++i)
    {
        r[num_full_words - 1 - i] = intx::be::unsafe::load<uint64_t>(data);
        data += sizeof(uint64_t);
    }

    stack.push(r);
    return pos + (Len + 1);
}

/// DUP instruction implementation.
/// @tparam N  The number as in the instruction definition, e.g. DUP3 is dup<3>.
template <int N>
inline void dup(StackTop stack) noexcept
{
    static_assert(N >= 1 && N <= 16);
    stack.push(stack[N - 1]);
}

/// SWAP instruction implementation.
/// @tparam N  The number as in the instruction definition, e.g. SWAP3 is swap<3>.
template <int N>
inline void swap(StackTop stack) noexcept
{
    static_assert(N >= 1 && N <= 16);
    std::swap(stack.top(), stack[N]);
}

template <size_t NumTopics>
inline dvmc_status_code log(StackTop stack, ExecutionState& state) noexcept
{
    static_assert(NumTopics <= 4);

    if (state.in_static_mode())
        return DVMC_STATIC_MODE_VIOLATION;

    const auto& offset = stack.pop();
    const auto& size = stack.pop();

    if (!check_memory(state, offset, size))
        return DVMC_OUT_OF_TRACK;

    const auto o = static_cast<size_t>(offset);
    const auto s = static_cast<size_t>(size);

    const auto cost = int64_t(s) * 8;
    if ((state.track_left -= cost) < 0)
        return DVMC_OUT_OF_TRACK;

    std::array<dvmc::bytes32, NumTopics> topics;  // NOLINT(cppcoreguidelines-pro-type-member-init)
    for (auto& topic : topics)
        topic = intx::be::store<dvmc::bytes32>(stack.pop());

    const auto data = s != 0 ? &state.memory[o] : nullptr;
    state.host.emit_log(state.msg->recipient, data, s, topics.data(), NumTopics);
    return DVMC_SUCCESS;
}


template <dvmc_opcode Op>
dvmc_status_code call_impl(StackTop stack, ExecutionState& state) noexcept;
inline constexpr auto call = call_impl<OP_CALL>;
inline constexpr auto callcode = call_impl<OP_CALLCODE>;
inline constexpr auto delegatecall = call_impl<OP_DELEGATECALL>;
inline constexpr auto staticcall = call_impl<OP_STATICCALL>;

template <dvmc_opcode Op>
dvmc_status_code create_impl(StackTop stack, ExecutionState& state) noexcept;
inline constexpr auto create = create_impl<OP_CREATE>;
inline constexpr auto create2 = create_impl<OP_CREATE2>;

template <dvmc_status_code StatusCode>
inline StopToken return_impl(StackTop stack, ExecutionState& state) noexcept
{
    const auto& offset = stack[0];
    const auto& size = stack[1];

    if (!check_memory(state, offset, size))
        return {DVMC_OUT_OF_TRACK};

    state.output_size = static_cast<size_t>(size);
    if (state.output_size != 0)
        state.output_offset = static_cast<size_t>(offset);
    return {StatusCode};
}
inline constexpr auto return_ = return_impl<DVMC_SUCCESS>;
inline constexpr auto revert = return_impl<DVMC_REVERT>;

inline StopToken selfdestruct(StackTop stack, ExecutionState& state) noexcept
{
    if (state.in_static_mode())
        return {DVMC_STATIC_MODE_VIOLATION};

    const auto beneficiary = intx::be::trunc<dvmc::address>(stack[0]);

    if (state.rev >= DVMC_BERLIN && state.host.access_account(beneficiary) == DVMC_ACCESS_COLD)
    {
        if ((state.track_left -= instr::cold_account_access_cost) < 0)
            return {DVMC_OUT_OF_TRACK};
    }

    if (state.rev >= DVMC_TANGERINE_WHISTLE)
    {
        if (state.rev == DVMC_TANGERINE_WHISTLE || state.host.get_balance(state.msg->recipient))
        {
            // After TANGERINE_WHISTLE apply additional cost of
            // sending value to a non-existing account.
            if (!state.host.account_exists(beneficiary))
            {
                if ((state.track_left -= 25000) < 0)
                    return {DVMC_OUT_OF_TRACK};
            }
        }
    }

    state.host.selfdestruct(state.msg->recipient, beneficiary);
    return {DVMC_SUCCESS};
}


/// Maps an opcode to the instruction implementation.
///
/// The set of template specializations which map opcodes `Op` to the function
/// implementing the instruction identified by the opcode.
///     instr::impl<OP_DUP1>(/*...*/);
/// The unspecialized template is invalid and should never to used.
template <dvmc_opcode Op>
inline constexpr auto impl = nullptr;

#define X(OPCODE, IDENTIFIER) \
    template <>               \
    inline constexpr auto impl<OPCODE> = IDENTIFIER;  // opcode -> implementation
MAP_OPCODE_TO_IDENTIFIER
#undef X
}  // namespace instr::core
}  // namespace dvmone
