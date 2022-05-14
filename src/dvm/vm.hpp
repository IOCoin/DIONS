// dvm_base__: Fast Ethereum Virtual Machine implementation
// Copyright 2021 The dvm_base__ Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "tracing.hpp"
#include <evmc/evmc.h>

namespace dvm_base__
{
/// The dvm_base__ DVMC instance.
class VM : public evmc_vm
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
}  // namespace dvm_base__
