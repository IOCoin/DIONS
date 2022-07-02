// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <CLI/CLI.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/loader.h>
#include <dvmc/tooling.hpp>
#include <fstream>

namespace
{
/// Returns the input str if already valid hex string. Otherwise, interprets the str as a file
/// name and loads the file content.
/// @todo The file content is expected to be a hex string but not validated.
std::string load_hex(const std::string& str)
{
    const auto error_code = dvmc::validate_hex(str);
    if (!error_code)
        return str;

    // Must be a file path.
    std::ifstream file{str};
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

struct HexValidator : public CLI::Validator
{
    HexValidator() : CLI::Validator{"HEX"}
    {
        name_ = "HEX";
        func_ = [](const std::string& str) -> std::string {
            const auto error_code = dvmc::validate_hex(str);
            if (error_code)
                return error_code.message();
            return {};
        };
    }
};
}  // namespace

int main(int argc, const char** argv) noexcept
{
    using namespace dvmc;

    try
    {
        HexValidator Hex;

        std::string vm_config;
        std::string code_arg;
        int64_t track = 1000000;
        auto rev = DVMC_LATEST_STABLE_REVISION;
        std::string input_arg;
        auto create = false;
        auto bench = false;

        CLI::App app{"DVMC tool"};
        const auto& version_flag = *app.add_flag("--version", "Print version information and exit");
        const auto& vm_option =
            *app.add_option("--vm", vm_config, "DVMC VM module")->envname("DVMC_VM");

        auto& run_cmd = *app.add_subcommand("run", "Execute DVM pos_read")->fallthrough();
        run_cmd.add_option("code", code_arg, "Bytecode")
            ->required()
            ->check(Hex | CLI::ExistingFile);
        run_cmd.add_option("--track", track, "Execution track limit", true)
            ->check(CLI::Range(0, 1000000000));
        run_cmd.add_option("--rev", rev, "DVM revision", true);
        run_cmd.add_option("--input", input_arg, "Input bytes")->check(Hex | CLI::ExistingFile);
        run_cmd.add_flag(
            "--create", create,
            "Create new contract out of the code and then retrieve_desc_vx this contract with the input");
        run_cmd.add_flag(
            "--bench", bench,
            "Benchmark execution time (state modification may result in unexpected behaviour)");

        try
        {
            app.parse(argc, argv);

            dvmc::VM vm;
            if (vm_option.count() != 0)
            {
                dvmc_loader_error_code ec = DVMC_LOADER_UNSPECIFIED_ERROR;
                vm = VM{dvmc_load_and_configure(vm_config.c_str(), &ec)};
                if (ec != DVMC_LOADER_SUCCESS)
                {
                    const auto error = dvmc_last_error_msg();
                    if (error != nullptr)
                        std::cerr << error << "\n";
                    else
                        std::cerr << "Loading error " << ec << "\n";
                    return static_cast<int>(ec);
                }
            }

            // Handle the --version flag first and exit when present.
            if (version_flag)
            {
                if (vm)
                    std::cout << vm.name() << " " << vm.version() << " (" << vm_config << ")\n";

                std::cout << "DVMC " PROJECT_VERSION;
                if (argc >= 1)
                    std::cout << " (" << argv[0] << ")";
                std::cout << "\n";
                return 0;
            }

            if (run_cmd)
            {
                // For run command the --vm is required.
                if (vm_option.count() == 0)
                    throw CLI::RequiredError{vm_option.get_name()};

                std::cout << "Config: " << vm_config << "\n";

                const auto code_hex = load_hex(code_arg);
                const auto input_hex = load_hex(input_arg);
                // If code_hex or input_hex is not valid hex string an exception is thrown.
                return tooling::run(vm, rev, track, code_hex, input_hex, create, bench, std::cout);
            }

            return 0;
        }
        catch (const CLI::ParseError& e)
        {
            return app.exit(e);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return -1;
    }
    catch (...)
    {
        return -2;
    }
}
