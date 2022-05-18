// 
// Copyright 2022 blastdoor7
// 

#include "baseline_instruction_table.hpp"
#include "instructions_traits.hpp"

namespace dvmone::baseline
{
const CostTable& get_baseline_cost_table(dvmc_revision rev) noexcept
{
    static constexpr auto cost_tables = []() noexcept {
        std::array<CostTable, DVMC_MAX_REVISION + 1> tables{};
        for (size_t r = DVMC_FRONTIER; r <= DVMC_MAX_REVISION; ++r)
        {
            auto& table = tables[r];
            for (size_t i = 0; i < table.size(); ++i)
            {
                table[i] = instr::track_costs[r][i];  // Include instr::undefined in the table.
            }
        }
        return tables;
    }();

    return cost_tables[rev];
}
}  // namespace dvmone::baseline
