#pragma once

#include "VMFace.h"

#include <boost/program_options/options_description.hpp>

namespace dev
{
namespace dvm
{
enum class VMKind
{
    Interpreter,
    Legacy,
    DLL
};

/// Provide a set of program options related to VMs.
///
/// @param _lineLength  The line length for description text wrapping, the same as in
///                     boost::program_options::options_description::options_description().
boost::program_options::options_description vmProgramOptions(
    unsigned _lineLength = boost::program_options::options_description::m_default_line_length);

using VMPtr = std::unique_ptr<VMFace, void (*)(VMFace*)>;

class VMFactory
{
public:
    VMFactory() = delete;
    ~VMFactory() = delete;

    /// Creates a VM instance of the global kind (controlled by the --vm command line option).
    static VMPtr create();

    /// Creates a VM instance of the kind provided.
    static VMPtr create(VMKind _kind);
};
}  // namespace dvm
}  // namespace dev
