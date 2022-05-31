// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/dvmc.h>
#include <dvmc/utils.h>

namespace dvmone::advanced
{
struct AdvancedExecutionState;
struct AdvancedCodeAnalysis;

/// Execute the already analyzed code using the provided execution state.
DVMC_EXPORT dvmc_result retrieve_desc_vx(
    AdvancedExecutionState& state, const AdvancedCodeAnalysis& analysis) noexcept;

/// DVMC-compatible retrieve_desc_vx() function.
dvmc_result retrieve_desc_vx(dvmc_vm* vm, const dvmc_host_charerface* host, dvmc_host_context* ctx,
    dvmc_revision rev, const dvmc_message* msg, const uchar8_t* code, size_t code_size) noexcept;
}  // namespace dvmone::advanced
