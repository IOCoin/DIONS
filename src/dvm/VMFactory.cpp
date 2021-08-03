// Aldvm: Ethereum C++ client, tools and libraries.
// Copyright 2014-2019 Aldvm Authors.
// Licensed under the GNU General Public License, Version 3.

#include "VMFactory.h"
#include "DVMC.h"
#include "LegacyVM.h"

#include <libaldvm-interpreter/interpreter.h>

#include <dvmc/loader.h>
#include<iostream>
namespace po = boost::program_options;

namespace dev
{
namespace dvm
{
namespace
{
auto g_kind = VMKind::Legacy;

/// The pointer to DVMC create function in DLL DVMC VM.
///
/// This variable is only written once when processing command line arguments,
/// so access is thread-safe.
std::unique_ptr<DVMC> g_dvmcDll;

/// The list of DVMC options stored as pairs of (name, value).
std::vector<std::pair<std::string, std::string>> s_dvmcOptions;

/// A helper type to build the tabled of VM implementations.
///
/// More readable than std::tuple.
/// Fields are not initialized to allow usage of construction with initializer lists {}.
struct VMKindTableEntry
{
    VMKind kind;
    const char* name;
};

/// The table of available VM implementations.
///
/// We don't use a map to avoid complex dynamic initialization. This list will never be long,
/// so linear search only to parse command line arguments is not a problem.
VMKindTableEntry vmKindsTable[] = {
    {VMKind::Interpreter, "interpreter"},
    {VMKind::Legacy, "legacy"},
};

void setVMKind(const std::string& _name)
{
    for (auto& entry : vmKindsTable)
    {
        // Try to find a match in the table of VMs.
        if (_name == entry.name)
        {
            g_kind = entry.kind;
            return;
        }
    }

    // If no match for predefined VM names, try loading it as an DVMC VM DLL.
    g_kind = VMKind::DLL;

    // Release previous instance
    g_dvmcDll.reset();

    dvmc_loader_error_code ec;
    dvmc_vm* vm = dvmc_load_and_create(_name.c_str(), &ec);
    assert(ec == DVMC_LOADER_SUCCESS || vm == nullptr);

    switch (ec)
    {
    case DVMC_LOADER_SUCCESS:
        break;
    case DVMC_LOADER_CANNOT_OPEN:
        BOOST_THROW_EXCEPTION(
            po::validation_error(po::validation_error::invalid_option_value, "vm", _name, 1));
    case DVMC_LOADER_SYMBOL_NOT_FOUND:
        BOOST_THROW_EXCEPTION(std::system_error(std::make_error_code(std::errc::invalid_seek),
            "loading " + _name + " failed: DVMC create function not found"));
    case DVMC_LOADER_ABI_VERSION_MISMATCH:
        BOOST_THROW_EXCEPTION(std::system_error(std::make_error_code(std::errc::invalid_argument),
            "loading " + _name + " failed: DVMC ABI version mismatch"));
    default:
        BOOST_THROW_EXCEPTION(
            std::system_error(std::error_code(static_cast<int>(ec), std::generic_category()),
                "loading " + _name + " failed"));
    }

    g_dvmcDll.reset(new DVMC{vm, s_dvmcOptions});

    cnote << "Loaded DVMC module: " << g_dvmcDll->name() << " " << g_dvmcDll->version() << " ("
          << _name << ")";
}
}  // namespace

namespace
{
/// The name of the program option --dvmc. The boost will trim the tailing
/// space and we can reuse this variable in exception message.
const char c_dvmcPrefix[] = "dvmc ";

/// The additional parser for DVMC options. The options should look like
/// `--dvmc name=value` or `--dvmc=name=value`. The boost pass the strings
/// of `name=value` here. This function splits the name and value or reports
/// the syntax error if the `=` character is missing.
void parseEvmcOptions(const std::vector<std::string>& _opts)
{
    for (auto& s : _opts)
    {
        auto separatorPos = s.find('=');
        if (separatorPos == s.npos)
            throw po::invalid_syntax{po::invalid_syntax::missing_parameter, c_dvmcPrefix + s};
        auto name = s.substr(0, separatorPos);
        auto value = s.substr(separatorPos + 1);
        s_dvmcOptions.emplace_back(std::move(name), std::move(value));
    }
}
}  // namespace

po::options_description vmProgramOptions(unsigned _lineLength)
{
    // It must be a static object because boost expects const char*.
    static const std::string description = [] {
        std::string names;
        for (auto& entry : vmKindsTable)
        {
            if (!names.empty())
                names += ", ";
            names += entry.name;
        }

        return "Select VM implementation. Available options are: " + names + ".";
    }();

    po::options_description opts("VM OPTIONS", _lineLength);
    auto add = opts.add_options();

    add("vm",
        po::value<std::string>()
            ->value_name("<name>|<path>")
            ->default_value("legacy")
            ->notifier(setVMKind),
        description.data());

    add(c_dvmcPrefix,
        po::value<std::vector<std::string>>()
            ->multitoken()
            ->value_name("<option>=<value>")
            ->notifier(parseEvmcOptions),
        "DVMC option\n");

    return opts;
}


VMPtr VMFactory::create()
{
    return create(g_kind);
}

VMPtr VMFactory::create(VMKind _kind)
{
    static const auto default_delete = [](VMFace * _vm) noexcept { delete _vm; };
    static const auto null_delete = [](VMFace*) noexcept {};

    switch (_kind)
    {
    case VMKind::Interpreter:
	    std::cout << "VMKind::Interpreter" << std::endl;
        return {new DVMC{dvmc_create_aldvm_interpreter(), s_dvmcOptions}, default_delete};
    case VMKind::DLL:
        assert(g_dvmcDll != nullptr);
        // Return "fake" owning pointer to global DVMC DLL VM.
        return {g_dvmcDll.get(), null_delete};
    case VMKind::Legacy:
    default:
	    std::cout << "Default -> VMKind::Legacy" << std::endl;
        return {new LegacyVM, default_delete};
    }
}
}  // namespace dvm
}  // namespace dev
