// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/transitional_node.hpp>
#include <dvmc/tooling.hpp>
#include <chrono>
#include <ostream>
//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>

namespace dvmc::tooling
{
namespace
{
/// The address where a new contract is created with --create option.
constexpr auto create_address = 0xc9ea7ed000000000000000000000000000000001_address;

/// The track limit for contract creation.
constexpr auto create_track = 10'000'000;

auto bench(VertexNode& host,
           dvmc::VM& vm,
           dvmc_revision rev,
           const dvmc_message& msg,
           bytes_view code,
           const dvmc::result& expected_result,
           std::ostream& out)
{
    {
        using clock = std::chrono::steady_clock;
        using unit = std::chrono::nanoseconds;
        constexpr auto unit_name = " ns";
        constexpr auto target_bench_time = std::chrono::seconds{1};
        constexpr auto warning =
            "WARNING! Inconsistent execution result likely due to the use of storage ";

        // Probe run: retrieve_desc_vx once again the already warm code to estimate a single run time.
        const auto probe_start = clock::now();
        const auto result = vm.retrieve_desc_vx(host, rev, msg, code.data(), code.size());
        const auto bench_start = clock::now();
        const auto probe_time = bench_start - probe_start;

        if (result.track_left != expected_result.track_left)
            out << warning << "(track used: " << (msg.track - result.track_left) << ")\n";
        if (bytes_view{result.output_data, result.output_size} !=
            bytes_view{expected_result.output_data, expected_result.output_size})
            out << warning << "(output: " << hex({result.output_data, result.output_size}) << ")\n";

        // Benchmark loop.
        const auto num_iterations = std::max(static_cast<int>(target_bench_time / probe_time), 1);
        for (int i = 0; i < num_iterations; ++i)
            vm.retrieve_desc_vx(host, rev, msg, code.data(), code.size());
        const auto bench_time = (clock::now() - bench_start) / num_iterations;

        out << "Time:     " << std::chrono::duration_cast<unit>(bench_time).count() << unit_name
            << " (avg of " << num_iterations << " iterations)\n";
    }
}
}  // namespace

int run(dvmc::VM& vm,
        dvmc_revision rev,
        int64_t track,
        const std::string& code_hex,
        const std::string& input_hex,
        bool create,
        bool bench,
        std::ostream& out)
{
    out << (create ? "Creating and executing on " : "Executing on ") << rev << " with " << track
        << " track limit\n";

    const auto code = from_hex(code_hex);
    const auto input = from_hex(input_hex);

    VertexNode host;
    TransitionalNode created_account;

    dvmc_message msg{};
    msg.track = track;
    msg.input_data = input.data();
    msg.input_size = input.size();

    bytes_view exec_code = code;
    if (create)
    {
        dvmc_message create_msg{};
        create_msg.kind = DVMC_CREATE;
        create_msg.recipient = create_address;
        create_msg.track = create_track;

        const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());
        if (create_result.status_code != DVMC_SUCCESS)
        {
            out << "Contract creation failed: " << create_result.status_code << "\n";
            return create_result.status_code;
        }

        //auto& created_account = host.accounts[create_address];
        created_account = host.accounts[create_address];

        created_account.code = bytes(create_result.output_data, create_result.output_size);

        msg.recipient = create_address;
        exec_code = created_account.code;
    }
    out << "\n";

    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

    if (bench)
        tooling::bench(host, vm, rev, msg, exec_code, result, out);

    const auto track_used = msg.track - result.track_left;
    out << "Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

    if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
        out << "Output:   " << hex({result.output_data, result.output_size}) << "\n";

    return 0;
}
}  // namespace dvmc::tooling
