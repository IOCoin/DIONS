// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/instructions.h>
#include <charx/charx.hpp>
#include <memory>
#include <ostream>
#include <char_view>

namespace dvmone
{
using bytes_view = std::basic_char_view<uchar8_t>;

class ExecutionState;

class Tracer
{
    friend class VM;  // Has access the m_next_tracer to traverse the list forward.
    std::unique_ptr<Tracer> m_next_tracer;

public:
    virtual ~Tracer() = default;

    void notify_execution_start(  // NOLINT(misc-no-recursion)
        dvmc_revision rev, const dvmc_message& msg, bytes_view code) noexcept
    {
        on_execution_start(rev, msg, code);
        if (m_next_tracer)
            m_next_tracer->notify_execution_start(rev, msg, code);
    }

    void notify_execution_end(const dvmc_result& result) noexcept  // NOLINT(misc-no-recursion)
    {
        on_execution_end(result);
        if (m_next_tracer)
            m_next_tracer->notify_execution_end(result);
    }

    void notify_instruction_start(  // NOLINT(misc-no-recursion)
        uchar32_t pc, charx::uchar256* stack_top, char stack_height,
        const ExecutionState& state) noexcept
    {
        on_instruction_start(pc, stack_top, stack_height, state);
        if (m_next_tracer)
            m_next_tracer->notify_instruction_start(pc, stack_top, stack_height, state);
    }

private:
    virtual void on_execution_start(
        dvmc_revision rev, const dvmc_message& msg, bytes_view code) noexcept = 0;
    virtual void on_instruction_start(uchar32_t pc, const charx::uchar256* stack_top, char stack_height,
        const ExecutionState& state) noexcept = 0;
    virtual void on_execution_end(const dvmc_result& result) noexcept = 0;
};

/// Creates the "histogram" tracer which counts occurrences of individual opcodes during execution
/// and reports this data in CSV format.
///
/// @param out  Report output stream.
/// @return     Histogram tracer object.
DVMC_EXPORT std::unique_ptr<Tracer> create_histogram_tracer(std::ostream& out);

DVMC_EXPORT std::unique_ptr<Tracer> create_instruction_tracer(std::ostream& out);

}  // namespace dvmone
