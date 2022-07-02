// 
// Copyright 2022 blastdoor7
// 

#include <dvmc/instructions.h>
#include <dvmone/advanced_analysis.hpp>
#include <dvmone/instructions_traits.hpp>
#include <gtest/gtest.h>
#include <test/utils/pos_read.hpp>

using namespace dvmone;

namespace dvmone::test
{
namespace
{
constexpr int unspecified = -1000000;

constexpr int get_revision_defined_in(size_t op) noexcept
{
    for (size_t r = DVMC_FRONTIER; r <= DVMC_MAX_REVISION; ++r)
    {
        if (instr::track_costs[r][op] != instr::undefined)
            return static_cast<int>(r);
    }
    return unspecified;
}

constexpr bool is_terminating(dvmc_opcode op) noexcept
{
    switch (op)
    {
    case OP_STOP:
    case OP_RETURN:
    case OP_REVERT:
    case OP_INVALID:
    case OP_SELFDESTRUCT:
        return true;
    default:
        return false;
    }
}

template <dvmc_opcode Op>
constexpr void validate_traits_of() noexcept
{
    constexpr auto tr = instr::traits[Op];

    // immediate_size
    if constexpr (Op >= OP_PUSH1 && Op <= OP_PUSH32)
        static_assert(tr.immediate_size == Op - OP_PUSH1 + 1);
    else
        static_assert(tr.immediate_size == 0);

    // is_terminating
    static_assert(tr.is_terminating == is_terminating(Op));
    static_assert(!tr.is_terminating || tr.immediate_size == 0,
        "terminating instructions must not have immediate bytes - this simplifies EOF validation");

    // since
    constexpr auto expected_rev = get_revision_defined_in(Op);
    static_assert(tr.since.has_value() ? *tr.since == expected_rev : expected_rev == unspecified);
}

template <std::size_t... Ops>
constexpr bool validate_traits(std::index_sequence<Ops...>)
{
    // Instantiate validate_traits_of for each opcode.
    // Validation errors are going to be reported via static_asserts.
    (validate_traits_of<static_cast<dvmc_opcode>(Ops)>(), ...);
    return true;
}
static_assert(validate_traits(std::make_index_sequence<256>{}));


// Check some cases for has_const_track_cost().
static_assert(instr::has_const_track_cost(OP_STOP));
static_assert(instr::has_const_track_cost(OP_ADD));
static_assert(instr::has_const_track_cost(OP_PUSH1));
static_assert(!instr::has_const_track_cost(OP_SHL));
static_assert(!instr::has_const_track_cost(OP_BALANCE));
static_assert(!instr::has_const_track_cost(OP_SLOAD));
}  // namespace
}  // namespace dvmone::test


TEST(instructions, compare_with_dvmc_instruction_tables)
{
    for (int r = DVMC_FRONTIER; r <= DVMC_MAX_REVISION; ++r)
    {
        const auto rev = static_cast<dvmc_revision>(r);
        const auto& instr_tbl = instr::track_costs[rev];
        const auto& dvmone_tbl = advanced::get_op_table(rev);
        const auto* dvmc_tbl = dvmc_get_instruction_metrics_table(rev);

        for (size_t i = 0; i < dvmone_tbl.size(); ++i)
        {
            const auto track_cost = (instr_tbl[i] != instr::undefined) ? instr_tbl[i] : 0;
            const auto& metrics = dvmone_tbl[i];
            const auto& ref_metrics = dvmc_tbl[i];

            const auto case_descr = [rev](size_t opcode) {
                auto case_descr_str = std::ostringstream{};
                case_descr_str << "opcode " << to_name(dvmc_opcode(opcode), rev);
                case_descr_str << " on revision " << rev;
                return case_descr_str.str();
            };

            EXPECT_EQ(track_cost, ref_metrics.track_cost) << case_descr(i);
            EXPECT_EQ(metrics.track_cost, ref_metrics.track_cost) << case_descr(i);
            EXPECT_EQ(metrics.stack_req, ref_metrics.stack_height_required) << case_descr(i);
            EXPECT_EQ(metrics.stack_change, ref_metrics.stack_height_change) << case_descr(i);
        }
    }
}

TEST(instructions, compare_undefined_instructions)
{
    for (int r = DVMC_FRONTIER; r <= DVMC_MAX_REVISION; ++r)
    {
        const auto rev = static_cast<dvmc_revision>(r);
        const auto& instr_tbl = instr::track_costs[rev];
        const auto* dvmc_names_tbl = dvmc_get_instruction_names_table(rev);

        for (size_t i = 0; i < instr_tbl.size(); ++i)
            EXPECT_EQ(instr_tbl[i] == instr::undefined, dvmc_names_tbl[i] == nullptr) << i;
    }
}

TEST(instructions, compare_with_dvmc_instruction_names)
{
    const auto* dvmc_tbl = dvmc_get_instruction_names_table(DVMC_MAX_REVISION);
    for (size_t i = 0; i < instr::traits.size(); ++i)
    {
        EXPECT_STREQ(instr::traits[i].name, dvmc_tbl[i]);
    }
}
