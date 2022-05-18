// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include "tracing.hpp"
#include <dvmc/dvmc.h>

namespace dvmone
{
/// The dvmone DVMC instance.
class VM : public dvmc_vm
{
    std::unique_ptr<Tracer> m_first_tracer;

public:
    inline constexpr VM() noexcept;

    void add_tracer(std::unique_ptr<Tracer> tracer) noexcept
    {
        // Find the first empty unique_ptr and assign the new tracer to it.
        auto* end = &m_first_tracer;
        while (*end)
            end = &(*end)->m_next_tracer;
        *end = std::move(tracer);
    }

    [[nodiscard]] Tracer* get_tracer() const noexcept { return m_first_tracer.get(); }
};
}  // namespace dvmone
