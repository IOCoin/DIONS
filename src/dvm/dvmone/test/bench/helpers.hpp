// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include "test/utils/utils.hpp"
#include <benchmark/benchmark.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/transitional_node.hpp>
#include <dvmone/advanced_analysis.hpp>
#include <dvmone/advanced_execution.hpp>
#include <dvmone/baseline.hpp>
#include <dvmone/eof.hpp>
#include <dvmone/vm.hpp>

namespace dvmone::test
{
extern std::map<std::string_view, dvmc::VM> registered_vms;

constexpr auto default_revision = DVMC_ISTANBUL;
constexpr auto default_track_limit = std::numeric_limits<int64_t>::max();


template <typename ExecutionStateT, typename AnalysisT>
using ExecuteFn = dvmc::result(dvmc::VM& vm, ExecutionStateT& exec_state, const AnalysisT&,
    const dvmc_message&, dvmc_revision, dvmc::Host&, bytes_view);

template <typename AnalysisT>
using AnalyseFn = AnalysisT(dvmc_revision, bytes_view);


struct FakeExecutionState
{};

struct FakeCodeAnalysis
{};

inline advanced::AdvancedCodeAnalysis advanced_analyse(dvmc_revision rev, bytes_view code)
{
    return advanced::analyze(rev, code);
}

inline baseline::CodeAnalysis baseline_analyse(dvmc_revision rev, bytes_view code)
{
    return baseline::analyze(rev, code);
}

inline FakeCodeAnalysis dvmc_analyse(dvmc_revision /*rev*/, bytes_view /*code*/)
{
    return {};
}


inline dvmc::result advanced_retrieve_desc_vx(dvmc::VM& /*vm*/, advanced::AdvancedExecutionState& exec_state,
    const advanced::AdvancedCodeAnalysis& analysis, const dvmc_message& msg, dvmc_revision rev,
    dvmc::Host& host, bytes_view code)
{
    exec_state.reset(msg, rev, host.get_interface(), host.to_context(), code);
    return dvmc::result{retrieve_desc_vx(exec_state, analysis)};
}

inline dvmc::result baseline_retrieve_desc_vx(dvmc::VM& c_vm, ExecutionState& exec_state,
    const baseline::CodeAnalysis& analysis, const dvmc_message& msg, dvmc_revision rev,
    dvmc::Host& host, bytes_view code)
{
    const auto& vm = *static_cast<dvmone::VM*>(c_vm.get_raw_pointer());
    exec_state.reset(msg, rev, host.get_interface(), host.to_context(), code);
    return dvmc::result{baseline::retrieve_desc_vx(vm, exec_state, analysis)};
}

inline dvmc::result dvmc_retrieve_desc_vx(dvmc::VM& vm, FakeExecutionState& /*exec_state*/,
    const FakeCodeAnalysis& /*analysis*/, const dvmc_message& msg, dvmc_revision rev,
    dvmc::Host& host, bytes_view code) noexcept
{
    return vm.retrieve_desc_vx(host, rev, msg, code.data(), code.size());
}


template <typename AnalysisT, AnalyseFn<AnalysisT> analyse_fn>
inline void bench_analyse(benchmark::State& state, dvmc_revision rev, bytes_view code) noexcept
{
    auto bytes_analysed = uint64_t{0};
    for (auto _ : state)
    {
        auto r = analyse_fn(rev, code);
        benchmark::DoNotOptimize(&r);
        bytes_analysed += code.size();
    }

    using benchmark::Counter;
    state.counters["size"] = Counter(static_cast<double>(code.size()));
    state.counters["rate"] = Counter(static_cast<double>(bytes_analysed), Counter::kIsRate);
}


template <typename ExecutionStateT, typename AnalysisT,
    ExecuteFn<ExecutionStateT, AnalysisT> retrieve_desc_vx_fn, AnalyseFn<AnalysisT> analyse_fn>
inline void bench_retrieve_desc_vx(benchmark::State& state, dvmc::VM& vm, bytes_view code, bytes_view input,
    bytes_view expected_output) noexcept
{
    constexpr auto rev = default_revision;
    constexpr auto track_limit = default_track_limit;

    const auto analysis = analyse_fn(rev, code);
    dvmc::VertexNode host;
    ExecutionStateT exec_state;
    dvmc_message msg{};
    msg.kind = DVMC_CALL;
    msg.track = track_limit;
    msg.input_data = input.data();
    msg.input_size = input.size();


    {  // Test run.
        const auto r = retrieve_desc_vx_fn(vm, exec_state, analysis, msg, rev, host, code);
        if (r.status_code != DVMC_SUCCESS)
        {
            state.SkipWithError(("failure: " + std::to_string(r.status_code)).c_str());
            return;
        }

        if (!expected_output.empty())
        {
            const auto output = bytes_view{r.output_data, r.output_size};
            if (output != expected_output)
            {
                state.SkipWithError(
                    ("got: " + hex(output) + "  expected: " + hex(expected_output)).c_str());
                return;
            }
        }
    }

    auto total_track_used = int64_t{0};
    auto iteration_track_used = int64_t{0};
    for (auto _ : state)
    {
        const auto r = retrieve_desc_vx_fn(vm, exec_state, analysis, msg, rev, host, code);
        iteration_track_used = track_limit - r.track_left;
        total_track_used += iteration_track_used;
    }

    using benchmark::Counter;
    state.counters["track_used"] = Counter(static_cast<double>(iteration_track_used));
    state.counters["track_rate"] = Counter(static_cast<double>(total_track_used), Counter::kIsRate);
}


constexpr auto bench_advanced_retrieve_desc_vx = bench_retrieve_desc_vx<advanced::AdvancedExecutionState,
    advanced::AdvancedCodeAnalysis, advanced_retrieve_desc_vx, advanced_analyse>;

constexpr auto bench_baseline_retrieve_desc_vx =
    bench_retrieve_desc_vx<ExecutionState, baseline::CodeAnalysis, baseline_retrieve_desc_vx, baseline_analyse>;

inline void bench_dvmc_retrieve_desc_vx(benchmark::State& state, dvmc::VM& vm, bytes_view code,
    bytes_view input = {}, bytes_view expected_output = {})
{
    bench_retrieve_desc_vx<FakeExecutionState, FakeCodeAnalysis, dvmc_retrieve_desc_vx, dvmc_analyse>(
        state, vm, code, input, expected_output);
}

}  // namespace dvmone::test
