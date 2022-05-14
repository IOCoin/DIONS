// dvm_base__: Fast Ethereum Virtual Machine implementation
// Copyright 2021 The dvm_base__ Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <evmc/instructions.h>
#include <memory>
#include <ostream>
#include <string_view>

namespace dvm_base__
{
using bytes_view = std::basic_string_view<uint8_t>;

struct ExecutionState;

class Tracer
{
    friend class VM;  // Has access the the m_next_tracer to traverse the list forward.
    std::unique_ptr<Tracer> m_next_tracer;

public:
    virtual ~Tracer() = default;

    void notify_execution_start(  // NOLINT(misc-no-recursion)
        evmc_revision rev, const evmc_message& msg, bytes_view code) noexcept
    {
        on_execution_start(rev, msg, code);
        if (m_next_tracer)
            m_next_tracer->notify_execution_start(rev, msg, code);
    }

    void notify_execution_end(const evmc_result& result) noexcept  // NOLINT(misc-no-recursion)
    {
        on_execution_end(result);
        if (m_next_tracer)
            m_next_tracer->notify_execution_end(result);
    }

    void notify_instruction_start(  // NOLINT(misc-no-recursion)
        uint32_t pc, const ExecutionState& state) noexcept
    {
        on_instruction_start(pc, state);
        if (m_next_tracer)
            m_next_tracer->notify_instruction_start(pc, state);
    }

private:
    virtual void on_execution_start(
        evmc_revision rev, const evmc_message& msg, bytes_view code) noexcept = 0;
    virtual void on_instruction_start(uint32_t pc, const ExecutionState& state) noexcept = 0;
    virtual void on_execution_end(const evmc_result& result) noexcept = 0;
};

/// Creates the "histogram" tracer which counts occurrences of individual opcodes during execution
/// and reports this data in CSV format.
///
/// @param out  Report output stream.
/// @return     Histogram tracer object.
DVMC_EXPORT std::unique_ptr<Tracer> create_histogram_tracer(std::ostream& out);

DVMC_EXPORT std::unique_ptr<Tracer> create_instruction_tracer(std::ostream& out);

}  // namespace dvm_base__
