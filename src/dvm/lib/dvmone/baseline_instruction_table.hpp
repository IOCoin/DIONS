// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/dvmc.h>
#include <array>

namespace dvmone::baseline
{
using CostTable = std::array<int16_t, 256>;

const CostTable& get_baseline_cost_table(dvmc_revision rev) noexcept;
}  // namespace dvmone::baseline
