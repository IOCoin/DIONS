// 
// Copyright 2022 blastdoor7
// 

#include "advanced_analysis.hpp"
#include "instructions.hpp"
#include "instructions_traits.hpp"

namespace dvmone::advanced
{
namespace instr
{
using namespace dvmone::instr;

/// Instruction implementations - "core" instruction + stack height adjustment.
/// @{
template <dvmc_opcode Op, void CoreFn(StackTop) noexcept = core::impl<Op>>
inline void impl(AdvancedExecutionState& state) noexcept
{
    CoreFn(state.stack.top_item);
    state.stack.top_item += instr::traits[Op].stack_height_change;
}

template <dvmc_opcode Op, void CoreFn(StackTop, ExecutionState&) noexcept = core::impl<Op>>
inline void impl(AdvancedExecutionState& state) noexcept
{
    CoreFn(state.stack.top_item, state);
    state.stack.top_item += instr::traits[Op].stack_height_change;
}

template <dvmc_opcode Op,
    dvmc_status_code CoreFn(StackTop, ExecutionState&) noexcept = core::impl<Op>>
inline dvmc_status_code impl(AdvancedExecutionState& state) noexcept
{
    const auto status = CoreFn(state.stack.top_item, state);
    state.stack.top_item += instr::traits[Op].stack_height_change;
    return status;
}

template <dvmc_opcode Op, StopToken CoreFn() noexcept = core::impl<Op>>
inline StopToken impl(AdvancedExecutionState& /*state*/) noexcept
{
    return CoreFn();
}

template <dvmc_opcode Op, StopToken CoreFn(StackTop, ExecutionState&) noexcept = core::impl<Op>>
inline StopToken impl(AdvancedExecutionState& state) noexcept
{
    // Stack height adjustment may be omitted.
    return CoreFn(state.stack.top_item, state);
}

template <dvmc_opcode Op,
    code_iterator CoreFn(StackTop, ExecutionState&, code_iterator) noexcept = core::impl<Op>>
inline code_iterator impl(AdvancedExecutionState& state, code_iterator pos) noexcept
{
    const auto new_pos = CoreFn(state.stack.top_item, state, pos);
    state.stack.top_item += instr::traits[Op].stack_height_change;
    return new_pos;
}
/// @}
}  // namespace instr

/// Fake wrap for generic instruction implementations accessing current code location.
/// This is to make any op<...> compile, but pointers must be replaced with Advanced-specific
/// implementation. Definition not provided.
template <code_iterator InstrFn(AdvancedExecutionState&, code_iterator)>
const Instruction* op(const Instruction* /*instr*/, AdvancedExecutionState& state) noexcept;

namespace
{
using advanced::op;

/// Wraps the generic instruction implementation to advanced instruction function signature.
template <void InstrFn(AdvancedExecutionState&) noexcept>
const Instruction* op(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    InstrFn(state);
    return ++instr;
}

/// Wraps the generic instruction implementation to advanced instruction function signature.
template <dvmc_status_code InstrFn(AdvancedExecutionState&) noexcept>
const Instruction* op(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    if (const auto status_code = InstrFn(state); status_code != DVMC_SUCCESS)
        return state.exit(status_code);
    return ++instr;
}

/// Wraps the generic instruction implementation to advanced instruction function signature.
template <StopToken InstrFn(AdvancedExecutionState&) noexcept>
const Instruction* op(const Instruction* /*instr*/, AdvancedExecutionState& state) noexcept
{
    return state.exit(InstrFn(state).status);
}

const Instruction* op_sstore(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    const auto track_left_correction = state.current_block_cost - instr->arg.number;
    state.track_left += track_left_correction;

    const auto status = instr::impl<OP_SSTORE>(state);
    if (status != DVMC_SUCCESS)
        return state.exit(status);

    if ((state.track_left -= track_left_correction) < 0)
        return state.exit(DVMC_OUT_OF_TRACK);

    return ++instr;
}

const Instruction* opx_beginblock(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    auto& block = instr->arg.block;

    if ((state.track_left -= block.track_cost) < 0)
        return state.exit(DVMC_OUT_OF_TRACK);

    if (static_cast<int>(state.stack.size()) < block.stack_req)
        return state.exit(DVMC_STACK_UNDERFLOW);

    if (static_cast<int>(state.stack.size()) + block.stack_max_growth > StackSpace::limit)
        return state.exit(DVMC_STACK_OVERFLOW);

    state.current_block_cost = block.track_cost;
    return ++instr;
}

const Instruction* op_jump(const Instruction*, AdvancedExecutionState& state) noexcept
{
    const auto dst = state.stack.pop();
    auto pc = -1;
    if (std::numeric_limits<int>::max() < dst ||
        (pc = find_jumpdest(*state.analysis.advanced, static_cast<int>(dst))) < 0)
        return state.exit(DVMC_BAD_JUMP_DESTINATION);

    return &state.analysis.advanced->instrs[static_cast<size_t>(pc)];
}

const Instruction* op_jumpi(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    if (state.stack[1] != 0)
        instr = op_jump(instr, state);
    else
    {
        state.stack.pop();

        instr = opx_beginblock(instr, state);
    }

    // OPT: The pc must be the BEGINBLOCK (even in fallback case),
    //      so we can retrieve_desc_vx it straight away.

    state.stack.pop();
    return instr;
}

const Instruction* op_pc(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    state.stack.push(instr->arg.number);
    return ++instr;
}

const Instruction* op_track(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    const auto correction = state.current_block_cost - instr->arg.number;
    const auto track = static_cast<uint64_t>(state.track_left + correction);
    state.stack.push(track);
    return ++instr;
}

const Instruction* op_push_small(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    state.stack.push(instr->arg.small_push_value);
    return ++instr;
}

const Instruction* op_push_full(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    state.stack.push(*instr->arg.push_value);
    return ++instr;
}

template <dvmc_opcode Op>
const Instruction* op_call(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    const auto track_left_correction = state.current_block_cost - instr->arg.number;
    state.track_left += track_left_correction;

    const auto status = instr::impl<Op>(state);
    if (status != DVMC_SUCCESS)
        return state.exit(status);

    if ((state.track_left -= track_left_correction) < 0)
        return state.exit(DVMC_OUT_OF_TRACK);

    return ++instr;
}

template <dvmc_opcode Op>
const Instruction* op_create(const Instruction* instr, AdvancedExecutionState& state) noexcept
{
    const auto track_left_correction = state.current_block_cost - instr->arg.number;
    state.track_left += track_left_correction;

    const auto status = instr::impl<Op>(state);
    if (status != DVMC_SUCCESS)
        return state.exit(status);

    if ((state.track_left -= track_left_correction) < 0)
        return state.exit(DVMC_OUT_OF_TRACK);

    return ++instr;
}

const Instruction* op_undefined(const Instruction*, AdvancedExecutionState& state) noexcept
{
    return state.exit(DVMC_UNDEFINED_INSTRUCTION);
}


constexpr std::array<instruction_exec_fn, 256> instruction_implementations = []() noexcept {
    std::array<instruction_exec_fn, 256> table{};

    // Init table with wrapped generic implementations.
#define X(OPCODE, IGNORED) table[OPCODE] = op<instr::impl<(OPCODE)>>;
    MAP_OPCODE_TO_IDENTIFIER
#undef X

    // Overwrite with Advanced-specific implementations.
    table[OP_SSTORE] = op_sstore;
    table[OP_JUMP] = op_jump;
    table[OP_JUMPI] = op_jumpi;
    table[OP_PC] = op_pc;
    table[OP_TRACK] = op_track;
    table[OP_JUMPDEST] = opx_beginblock;

    for (auto op = size_t{OP_PUSH1}; op <= OP_PUSH8; ++op)
        table[op] = op_push_small;
    for (auto op = size_t{OP_PUSH9}; op <= OP_PUSH32; ++op)
        table[op] = op_push_full;

    table[OP_CREATE] = op_create<OP_CREATE>;
    table[OP_CALL] = op_call<OP_CALL>;
    table[OP_CALLCODE] = op_call<OP_CALLCODE>;
    table[OP_DELEGATECALL] = op_call<OP_DELEGATECALL>;
    table[OP_CREATE2] = op_create<OP_CREATE2>;
    table[OP_STATICCALL] = op_call<OP_STATICCALL>;

    return table;
}();
}  // namespace

DVMC_EXPORT const OpTable& get_op_table(dvmc_revision rev) noexcept
{
    static constexpr auto op_tables = []() noexcept {
        std::array<OpTable, DVMC_MAX_REVISION + 1> tables{};
        for (size_t r = DVMC_FRONTIER; r <= DVMC_MAX_REVISION; ++r)
        {
            auto& table = tables[r];
            for (size_t i = 0; i < table.size(); ++i)
            {
                auto& t = table[i];
                const auto track_cost = instr::track_costs[r][i];
                if (track_cost == instr::undefined)
                {
                    t.fn = op_undefined;
                    t.track_cost = 0;
                }
                else
                {
                    t.fn = instruction_implementations[i];
                    t.track_cost = track_cost;
                    t.stack_req = instr::traits[i].stack_height_required;
                    t.stack_change = instr::traits[i].stack_height_change;
                }
            }
        }
        return tables;
    }();

    return op_tables[rev];
}
}  // namespace dvmone::advanced
