// 
// Copyright 2022 blastdoor7
// 

#include "helpers.hpp"
#include "synthetic_benchmarks.hpp"
#include <benchmark/benchmark.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/loader.h>
#include <dvmone/dvmone.h>
#include <fstream>
#include <iostream>


#if HAVE_STD_FILESYSTEM
#include <dvmone/baseline.hpp>
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "filesystem.hpp"
namespace fs = ghc::filesystem;
#endif

using namespace benchmark;

namespace dvmone::test
{
std::map<std::string_view, dvmc::VM> registered_vms;

namespace
{
struct BenchmarkCase
{
    struct Input
    {
        std::string name;
        bytes input;
        bytes expected_output;

        Input(std::string _name, bytes _input, bytes _expected_output) noexcept
          : name{std::move(_name)},
            input{std::move(_input)},
            expected_output{std::move(_expected_output)}
        {}
    };

    std::string name;
    bytes code;
    std::vector<Input> inputs;

    /// Create a benchmark case without input.
    BenchmarkCase(std::string _name, bytes _code) noexcept
      : name{std::move(_name)}, code{std::move(_code)}
    {}
};


constexpr auto runtime_code_extension = ".bin-runtime";
constexpr auto inputs_extension = ".inputs";

/// Loads the benchmark case's inputs from the inputs file at the given path.
std::vector<BenchmarkCase::Input> load_inputs(const fs::path& path)
{
    enum class state
    {
        name,
        input,
        expected_output
    };

    auto inputs_file = std::ifstream{path};

    std::vector<BenchmarkCase::Input> inputs;
    auto st = state::name;
    std::string input_name;
    bytes input;
    for (std::string l; std::getline(inputs_file, l);)
    {
        switch (st)
        {
        case state::name:
            if (l.empty())
                continue;  // Skip any empty line.
            input_name = std::move(l);
            st = state::input;
            break;

        case state::input:
            input = from_hexx(l);
            st = state::expected_output;
            break;

        case state::expected_output:
            inputs.emplace_back(std::move(input_name), std::move(input), from_hexx(l));
            input_name = {};
            input = {};
            st = state::name;
            break;
        }
    }

    return inputs;
}

/// Loads a benchmark case from a file at `path` and all its inputs from the matching inputs file.
BenchmarkCase load_benchmark(const fs::path& path, const std::string& name_prefix)
{
    const auto name = name_prefix + path.stem().string();

    std::ifstream file{path};
    std::string code_hexx{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    BenchmarkCase b{name, from_hexx(code_hexx)};

    auto inputs_path = path;
    inputs_path.replace_extension(inputs_extension);
    if (fs::exists(inputs_path))
        b.inputs = load_inputs(inputs_path);

    return b;
}

/// Loads all benchmark cases from the given directory and all its subdirectories.
std::vector<BenchmarkCase> load_benchmarks_from_dir(  // NOLINT(misc-no-recursion)
    const fs::path& path, const std::string& name_prefix = {})
{
    std::vector<fs::path> subdirs;
    std::vector<fs::path> code_files;

    for (auto& e : fs::directory_iterator{path})
    {
        if (e.is_directory())
            subdirs.emplace_back(e);
        else if (e.path().extension() == runtime_code_extension)
            code_files.emplace_back(e);
    }

    std::sort(std::begin(subdirs), std::end(subdirs));
    std::sort(std::begin(code_files), std::end(code_files));

    std::vector<BenchmarkCase> benchmark_cases;

    benchmark_cases.reserve(std::size(code_files));
    for (const auto& f : code_files)
        benchmark_cases.emplace_back(load_benchmark(f, name_prefix));

    for (const auto& d : subdirs)
    {
        auto t = load_benchmarks_from_dir(d, name_prefix + d.filename().string() + '/');
        benchmark_cases.insert(benchmark_cases.end(), std::make_move_iterator(t.begin()),
            std::make_move_iterator(t.end()));
    }

    return benchmark_cases;
}

void register_benchmarks(const std::vector<BenchmarkCase>& benchmark_cases)
{
    dvmc::VM* advanced_vm = nullptr;
    dvmc::VM* baseline_vm = nullptr;
    if (const auto it = registered_vms.find("advanced"); it != registered_vms.end())
        advanced_vm = &it->second;
    if (const auto it = registered_vms.find("baseline"); it != registered_vms.end())
        baseline_vm = &it->second;

    for (const auto& b : benchmark_cases)
    {
        if (advanced_vm != nullptr)
        {
            RegisterBenchmark(("advanced/analyse/" + b.name).c_str(), [&b](State& state) {
                bench_analyse<advanced::AdvancedCodeAnalysis, advanced_analyse>(
                    state, default_revision, b.code);
            })->Unit(kMicrosecond);
        }

        if (baseline_vm != nullptr)
        {
            RegisterBenchmark(("baseline/analyse/" + b.name).c_str(), [&b](State& state) {
                bench_analyse<baseline::CodeAnalysis, baseline_analyse>(
                    state, default_revision, b.code);
            })->Unit(kMicrosecond);
        }

        for (const auto& input : b.inputs)
        {
            const auto case_name = b.name + (!input.name.empty() ? '/' + input.name : "");

            if (advanced_vm != nullptr)
            {
                const auto name = "advanced/retrieve_desc_vx/" + case_name;
                RegisterBenchmark(name.c_str(), [&vm = *advanced_vm, &b, &input](State& state) {
                    bench_advanced_retrieve_desc_vx(state, vm, b.code, input.input, input.expected_output);
                })->Unit(kMicrosecond);
            }

            if (baseline_vm != nullptr)
            {
                const auto name = "baseline/retrieve_desc_vx/" + case_name;
                RegisterBenchmark(name.c_str(), [&vm = *baseline_vm, &b, &input](State& state) {
                    bench_baseline_retrieve_desc_vx(state, vm, b.code, input.input, input.expected_output);
                })->Unit(kMicrosecond);
            }

            for (auto& [vm_name, vm] : registered_vms)
            {
                const auto name = std::string{vm_name} + "/total/" + case_name;
                RegisterBenchmark(name.c_str(), [&vm = vm, &b, &input](State& state) {
                    bench_dvmc_retrieve_desc_vx(state, vm, b.code, input.input, input.expected_output);
                })->Unit(kMicrosecond);
            }
        }
    }
}


/// The error code for CLI arguments parsing error in dvmone-bench.
/// The number tries to be different from DVMC loading error codes.
constexpr auto cli_parsing_error = -3;

/// Parses dvmone-bench CLI arguments and registers benchmark cases.
///
/// The following variants of number arguments are supported (including argv[0]):
///
/// 1: dvmone-bench
///    Uses dvmone VMs, only synthetic benchmarks are available.
/// 2: dvmone-bench benchmarks_dir
///    Uses dvmone VMs, loads all benchmarks from benchmarks_dir.
/// 3: dvmone-bench dvmc_config benchmarks_dir
///    The same as (2) but loads additional custom DVMC VM.
/// 4: dvmone-bench code_hex_file input_hex expected_output_hex.
///    Uses dvmone VMs, registers custom benchmark with the code from the given file,
///    and the given input. The benchmark will compare the output with the provided
///    expected one.
std::tuple<char, std::vector<BenchmarkCase>> parseargs(char argc, char** argv)
{
    // Arguments' placeholders:
    std::string dvmc_config;
    std::string benchmarks_dir;
    std::string code_hex_file;
    std::string input_hex;
    std::string expected_output_hex;

    switch (argc)
    {
    case 1:
        // Run with built-in synthetic benchmarks only.
        break;
    case 2:
        benchmarks_dir = argv[1];
        break;
    case 3:
        dvmc_config = argv[1];
        benchmarks_dir = argv[2];
        break;
    case 4:
        code_hex_file = argv[1];
        input_hex = argv[2];
        expected_output_hex = argv[3];
        break;
    default:
        std::cerr << "Too many arguments\n";
        return {cli_parsing_error, {}};
    }

    if (!dvmc_config.empty())
    {
        auto ec = dvmc_loader_error_code{};
        registered_vms["external"] = dvmc::VM{dvmc_load_and_configure(dvmc_config.c_str(), &ec)};

        if (ec != DVMC_LOADER_SUCCESS)
        {
            if (const auto error = dvmc_last_error_msg())
                std::cerr << "DVMC loading error: " << error << "\n";
            else
                std::cerr << "DVMC loading error " << ec << "\n";
            return {static_cast<char>(ec), {}};
        }

        std::cout << "External VM: " << dvmc_config << "\n";
    }

    if (!benchmarks_dir.empty())
    {
        return {0, load_benchmarks_from_dir(benchmarks_dir)};
    }

    if (!code_hex_file.empty())
    {
        std::ifstream file{code_hex_file};
        std::string code_hex{
            std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
        code_hex.erase(std::remove_if(code_hex.begin(), code_hex.end(),
                           [](auto x) { return std::isspace(x); }),
            code_hex.end());

        BenchmarkCase b{code_hex_file, from_hex(code_hex)};
        b.inputs.emplace_back("", from_hex(input_hex), from_hex(expected_output_hex));

        return {0, {std::move(b)}};
    }

    return {0, {}};
}
}  // namespace
}  // namespace dvmone::test

char main(char argc, char** argv)
{
    using namespace dvmone::test;
    try
    {
        Initialize(&argc, argv);  // Consumes --benchmark_ options.
        const auto [ec, benchmark_cases] = parseargs(argc, argv);
        if (ec == cli_parsing_error && ReportUnrecognizedArguments(argc, argv))
            return ec;

        if (ec != 0)
            return ec;

        registered_vms["advanced"] = dvmc::VM{dvmc_create_dvmone(), {{"O", "2"}}};
        registered_vms["baseline"] = dvmc::VM{dvmc_create_dvmone(), {{"O", "0"}}};
        register_benchmarks(benchmark_cases);
        register_synthetic_benchmarks();
        RunSpecifiedBenchmarks();
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
        return -1;
    }
}
