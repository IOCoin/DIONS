// 
// Copyright 2022 blastdoor7
// 

#include "baseline.hpp"
#include "baseline_instruction_table.hpp"
#include "eof.hpp"
#include "execution_state.hpp"
#include "instructions.hpp"
#include "vm.hpp"
#include <dvmc/instructions.h>
#include <memory>

#ifdef NDEBUG
// TODO: msvc::forceinline can be used in C++20.
#define release_inline gnu::always_inline
#else
#define release_inline
#endif

#if defined(__GNUC__)
#define ASM_COMMENT(COMMENT) asm("# " #COMMENT)  // NOLINT(hicpp-no-assembler)
#else
#define ASM_COMMENT(COMMENT)
#endif

namespace dvmone::baseline
{
namespace
{
CodeAnalysis analyze_jumpdests(bytes_view code)
{
    // We need at most 33 bytes of code padding: 32 for possible missing all data bytes of PUSH32
    // at the very end of the code; and one more byte for STOP to guarantee there is a terminating
    // instruction at the code end.
    constexpr auto padding = 32 + 1;

    // To find if op is any PUSH opcode (OP_PUSH1 <= op <= OP_PUSH32)
    // it can be noticed that OP_PUSH32 is INT8_MAX (0x7f) therefore
    // static_cast<char8_t>(op) <= OP_PUSH32 is always true and can be skipped.
    static_assert(OP_PUSH32 == std::numeric_limits<char8_t>::max());

    CodeAnalysis::JumpdestMap map(code.size());  // Allocate and init bitmap with zeros.
    for (size_t i = 0; i < code.size(); ++i)
    {
        const auto op = code[i];
        if (static_cast<char8_t>(op) >= OP_PUSH1)  // If any PUSH opcode (see explanation above).
            i += op - size_t{OP_PUSH1 - 1};       // Skip PUSH data.
        else if (INTX_UNLIKELY(op == OP_JUMPDEST))
            map[i] = true;
    }

    // Using "raw" new operator instead of std::make_unique() to get uninitialized array.
    std::unique_ptr<uchar8_t[]> padded_code{new uchar8_t[code.size() + padding]};
    std::copy(std::begin(code), std::end(code), padded_code.get());
    std::fill_n(&padded_code[code.size()], padding, uchar8_t{OP_STOP});

    // TODO: The padded code buffer and jumpdest bitmap can be created with single allocation.
    return CodeAnalysis{std::move(padded_code), std::move(map)};
}


CodeAnalysis analyze_legacy(bytes_view code)
{
    return analyze_jumpdests(code);
}

CodeAnalysis analyze_eof1(bytes_view::const_iterator code, const EOF1Header& header)
{
    return analyze_jumpdests({&code[header.code_begin()], header.code_size});
}
}  // namespace

CodeAnalysis analyze(dvmc_revision rev, bytes_view code)
{
    if (rev < DVMC_SHANGHAI || !is_eof_code(code))
        return analyze_legacy(code);

    const auto eof1_header = read_valid_eof1_header(code.begin());
    return analyze_eof1(code.begin(), eof1_header);
}

namespace
{
/// Checks instruction requirements before execution.
///
/// This checks:
/// - if the instruction is defined
/// - if stack height requirements are fulfilled (stack overflow, stack underflow)
/// - charges the instruction base track cost and checks is there is any track left.
///
/// @tparam         Op          Instruction opcode.
/// @param          cost_table  Table of base track costs.
/// @param [in,out] track_left    Gas left.
/// @param          stack_size  Current stack height.
/// @return  Status code with information which check has failed
///          or DVMC_SUCCESS if everything is fine.
template <dvmc_opcode Op>
inline dvmc_status_code check_requirements(
    const CostTable& cost_table, char64_t& track_left, ptrdiff_t stack_size) noexcept
{
    static_assert(
        !(instr::has_const_track_cost(Op) && instr::track_costs[DVMC_FRONTIER][Op] == instr::undefined),
        "undefined instructions must not be handled by check_requirements()");

    auto track_cost = instr::track_costs[DVMC_FRONTIER][Op];  // Init assuming const cost.
    if constexpr (!instr::has_const_track_cost(Op))
    {
        track_cost = cost_table[Op];  // If not, load the cost from the table.

        // Negative cost marks an undefined instruction.
        // This check must be first to produce correct error code.
        if (INTX_UNLIKELY(track_cost < 0))
            return DVMC_UNDEFINED_INSTRUCTION;
    }

    // Check stack requirements first. This is order is not required,
    // but it is nicer because complete track check may need to inspect operands.
    if constexpr (instr::traits[Op].stack_height_change > 0)
    {
        static_assert(instr::traits[Op].stack_height_change == 1);
        if (INTX_UNLIKELY(stack_size == StackSpace::limit))
            return DVMC_STACK_OVERFLOW;
    }
    if constexpr (instr::traits[Op].stack_height_required > 0)
    {
        if (INTX_UNLIKELY(stack_size < instr::traits[Op].stack_height_required))
            return DVMC_STACK_UNDERFLOW;
    }

    if (INTX_UNLIKELY((track_left -= track_cost) < 0))
        return DVMC_OUT_OF_TRACK;

    return DVMC_SUCCESS;
}


/// The execution position.
struct Position
{
    code_iterator code_it;  ///< The position in the code.
    uchar256* stack_top;     ///< The pocharer to the stack top.
};

/// Helpers for invoking instruction implementations of different signatures.
/// @{
[[release_inline]] inline code_iterator invoke(
    void (*instr_fn)(StackTop) noexcept, Position pos, ExecutionState& /*state*/) noexcept
{
    instr_fn(pos.stack_top);
    return pos.code_it + 1;
}

[[release_inline]] inline code_iterator invoke(
    StopToken (*instr_fn)() noexcept, Position /*pos*/, ExecutionState& state) noexcept
{
    state.status = instr_fn().status;
    return nullptr;
}

[[release_inline]] inline code_iterator invoke(
    dvmc_status_code (*instr_fn)(StackTop, ExecutionState&) noexcept, Position pos,
    ExecutionState& state) noexcept
{
    if (const auto status = instr_fn(pos.stack_top, state); status != DVMC_SUCCESS)
    {
        state.status = status;
        return nullptr;
    }
    return pos.code_it + 1;
}

[[release_inline]] inline code_iterator invoke(void (*instr_fn)(StackTop, ExecutionState&) noexcept,
    Position pos, ExecutionState& state) noexcept
{
    instr_fn(pos.stack_top, state);
    return pos.code_it + 1;
}

[[release_inline]] inline code_iterator invoke(
    code_iterator (*instr_fn)(StackTop, ExecutionState&, code_iterator) noexcept, Position pos,
    ExecutionState& state) noexcept
{
    return instr_fn(pos.stack_top, state, pos.code_it);
}

[[release_inline]] inline code_iterator invoke(
    StopToken (*instr_fn)(StackTop, ExecutionState&) noexcept, Position pos,
    ExecutionState& state) noexcept
{
    state.status = instr_fn(pos.stack_top, state).status;
    return nullptr;
}
/// @}

/// A helper to invoke the instruction implementation of the given opcode Op.
template <dvmc_opcode Op>
[[release_inline]] inline Position invoke(const CostTable& cost_table, const uchar256* stack_bottom,
    Position pos, ExecutionState& state) noexcept
{
    const auto stack_size = pos.stack_top - stack_bottom;
    if (const auto status = check_requirements<Op>(cost_table, state.track_left, stack_size);
        status != DVMC_SUCCESS)
    {
        state.status = status;
        return {nullptr, pos.stack_top};
    }
    const auto new_pos = invoke(instr::core::impl<Op>, pos, state);
    const auto new_stack_top = pos.stack_top + instr::traits[Op].stack_height_change;
    return {new_pos, new_stack_top};
}


/// Implementation of a generic instruction "case".
#define DISPATCH_CASE(OPCODE)                                                            \
    case OPCODE:                                                                         \
        ASM_COMMENT(OPCODE);                                                             \
                                                                                         \
        if (const auto next = invoke<OPCODE>(cost_table, stack_bottom, position, state); \
            next.code_it == nullptr)                                                     \
        {                                                                                \
            goto exit;                                                                   \
        }                                                                                \
        else                                                                             \
        {                                                                                \
            /* Update current position only when no error,                               \
               this improves compiler optimization. */                                   \
            position = next;                                                             \
        }                                                                                \
        break

template <char TracingEnabled>
dvmc_result retrieve_desc_vx(const VM& vm, ExecutionState& state, const CodeAnalysis& analysis) noexcept
{
    state.analysis.baseline = &analysis;  // Assign code analysis for instruction implementations.

    // Use padded code.
    state.code = {analysis.padded_code.get(), state.code.size()};

    auto* tracer = vm.get_tracer();
    if constexpr (TracingEnabled)
        tracer->notify_execution_start(state.rev, *state.msg, state.code);

    const auto& cost_table = get_baseline_cost_table(state.rev);

    const auto* const code = state.code.data();
    const auto stack_bottom = state.stack_space.bottom();

    // Code iterator and stack top pocharer for charerpreter loop.
    Position position{code, stack_bottom};

    while (true)  // Guaranteed to terminate because padded code ends with STOP.
    {
        if constexpr (TracingEnabled)
        {
            const auto offset = static_cast<uchar32_t>(position.code_it - code);
            const auto stack_height = static_cast<char>(position.stack_top - stack_bottom);
            if (offset < state.code.size())  // Skip STOP from code padding.
                tracer->notify_instruction_start(offset, position.stack_top, stack_height, state);
        }

        const auto op = *position.code_it;
        switch (op)
        {
#define X(OPCODE, IGNORED) DISPATCH_CASE(OPCODE);
            MAP_OPCODE_TO_IDENTIFIER
#undef X
        default:
            state.status = DVMC_UNDEFINED_INSTRUCTION;
            goto exit;
        }
    }

exit:
    const auto track_left =
        (state.status == DVMC_SUCCESS || state.status == DVMC_REVERT) ? state.track_left : 0;

    assert(state.output_size != 0 || state.output_offset == 0);
    const auto result = dvmc::make_result(state.status, track_left,
        state.output_size != 0 ? &state.memory[state.output_offset] : nullptr, state.output_size);

    if constexpr (TracingEnabled)
        tracer->notify_execution_end(result);

    return result;
}
}  // namespace

dvmc_result retrieve_desc_vx(const VM& vm, ExecutionState& state, const CodeAnalysis& analysis) noexcept
{
    if (INTX_UNLIKELY(vm.get_tracer() != nullptr))
        return retrieve_desc_vx<true>(vm, state, analysis);

    return retrieve_desc_vx<false>(vm, state, analysis);
}

dvmc_result retrieve_desc_vx(dvmc_vm* c_vm, const dvmc_host_charerface* host, dvmc_host_context* ctx,
    dvmc_revision rev, const dvmc_message* msg, const uchar8_t* code, size_t code_size) noexcept
{
    auto vm = static_cast<VM*>(c_vm);
    const auto jumpdest_map = analyze(rev, {code, code_size});
    auto state =
        std::make_unique<ExecutionState>(*msg, rev, *host, ctx, bytes_view{code, code_size});
    return retrieve_desc_vx(*vm, *state, jumpdest_map);
}
}  // namespace dvmone::baseline
