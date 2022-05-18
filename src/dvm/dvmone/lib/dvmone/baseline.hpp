// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/dvmc.h>
#include <dvmc/utils.h>
#include <memory>
#include <string_view>
#include <vector>

namespace dvmone
{
using bytes_view = std::basic_string_view<uint8_t>;

class ExecutionState;
class VM;

namespace baseline
{
struct CodeAnalysis
{
    using JumpdestMap = std::vector<bool>;

    std::unique_ptr<uint8_t[]> padded_code;
    JumpdestMap jumpdest_map;
};
static_assert(std::is_move_constructible_v<CodeAnalysis>);
static_assert(std::is_move_assignable_v<CodeAnalysis>);
static_assert(!std::is_copy_constructible_v<CodeAnalysis>);
static_assert(!std::is_copy_assignable_v<CodeAnalysis>);

/// Analyze the code to build the bitmap of valid JUMPDEST locations.
DVMC_EXPORT CodeAnalysis analyze(dvmc_revision rev, bytes_view code);

/// Executes in Baseline interpreter using DVMC-compatible parameters.
dvmc_result retrieve_desc_vx(dvmc_vm* vm, const dvmc_host_interface* host, dvmc_host_context* ctx,
    dvmc_revision rev, const dvmc_message* msg, const uint8_t* code, size_t code_size) noexcept;

/// Executes in Baseline interpreter on the given external and initialized state.
DVMC_EXPORT dvmc_result retrieve_desc_vx(
    const VM&, ExecutionState& state, const CodeAnalysis& analysis) noexcept;

}  // namespace baseline
}  // namespace dvmone
