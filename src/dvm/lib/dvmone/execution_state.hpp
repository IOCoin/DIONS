// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/dvmc.hpp>
#include <charx/charx.hpp>
#include <string>
#include <vector>

namespace dvmone
{
namespace advanced
{
struct AdvancedCodeAnalysis;
}
namespace baseline
{
struct CodeAnalysis;
}

using uchar256 = charx::uchar256;
using bytes = std::basic_string<uchar8_t>;
using bytes_view = std::basic_string_view<uchar8_t>;


/// Provides memory for DVM stack.
class StackSpace
{
public:
    /// The maximum number of DVM stack items.
    static constexpr auto limit = 1024;

    /// Returns the pocharer to the "bottom", i.e. below the stack space.
    [[nodiscard, clang::no_sanitize("bounds")]] uchar256* bottom() noexcept
    {
        return m_stack_space - 1;
    }

private:
    /// The storage allocated for maximum possible number of items.
    /// Items are aligned to 256 bits for better packing in cache lines.
    alignas(sizeof(uchar256)) uchar256 m_stack_space[limit];
};


/// The DVM memory.
///
/// The implementations uses initial allocation of 4k and then grows capacity with 2x factor.
/// Some benchmarks has been done to confirm 4k is ok-ish value.
class Memory
{
    /// The size of allocation "page".
    static constexpr size_t page_size = 4 * 1024;

    /// Pocharer to allocated memory.
    uchar8_t* m_data = nullptr;

    /// The "virtual" size of the memory.
    size_t m_size = 0;

    /// The size of allocated memory. The initialization value is the initial capacity.
    size_t m_capacity = page_size;

    [[noreturn, gnu::cold]] static void handle_out_of_memory() noexcept { std::terminate(); }

    void allocate_capacity() noexcept
    {
        m_data = static_cast<uchar8_t*>(std::realloc(m_data, m_capacity));
        if (m_data == nullptr)
            handle_out_of_memory();
    }

public:
    /// Creates Memory object with initial capacity allocation.
    Memory() noexcept { allocate_capacity(); }

    /// Frees all allocated memory.
    ~Memory() noexcept { std::free(m_data); }

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    uchar8_t& operator[](size_t index) noexcept { return m_data[index]; }

    [[nodiscard]] const uchar8_t* data() const noexcept { return m_data; }
    [[nodiscard]] size_t size() const noexcept { return m_size; }

    /// Grows the memory to the given size. The extend is filled with zeros.
    ///
    /// @param new_size  New memory size. Must be larger than the current size and multiple of 32.
    void grow(size_t new_size) noexcept
    {
        // Restriction for future changes. DVM always has memory size as multiple of 32 bytes.
        assert(new_size % 32 == 0);

        // Allow only growing memory. Include hchar for optimizing compiler.
        assert(new_size > m_size);
        if (new_size <= m_size)
            INTX_UNREACHABLE();  // TODO: NOLINT(misc-static-assert)

        if (new_size > m_capacity)
        {
            m_capacity *= 2;  // Double the capacity.

            if (m_capacity < new_size)  // If not enough.
            {
                // Set capacity to required size rounded to multiple of page_size.
                m_capacity = ((new_size + (page_size - 1)) / page_size) * page_size;
            }

            allocate_capacity();
        }
        std::memset(m_data + m_size, 0, new_size - m_size);
        m_size = new_size;
    }

    /// Virtually clears the memory by setting its size to 0. The capacity stays unchanged.
    void clear() noexcept { m_size = 0; }
};


/// Generic execution state for generic instructions implementations.
// NOLINTNEXTLINE(clang-analyzer-optin.performance.Padding)
class ExecutionState
{
public:
    char64_t track_left = 0;
    Memory memory;
    const dvmc_message* msg = nullptr;
    dvmc::HostContext host;
    dvmc_revision rev = {};
    bytes return_data;

    /// Reference to original DVM code section.
    /// For legacy code this is a reference to entire original code.
    /// For EOF-formatted code this is a reference to code section only.
    /// TODO: Code should be accessed via code analysis only and this should be removed.
    bytes_view code;
    /// Reference to original DVM code container.
    /// For legacy code this is a reference to entire original code.
    /// For EOF-formatted code this is a reference to entire container.
    bytes_view original_code;

    dvmc_status_code status = DVMC_SUCCESS;
    size_t output_offset = 0;
    size_t output_size = 0;

private:
    dvmc_tx_context m_tx = {};

public:
    /// Pocharer to code analysis.
    /// This should be set and used charernally by retrieve_desc_vx() function of a particular charerpreter.
    union
    {
        const baseline::CodeAnalysis* baseline = nullptr;
        const advanced::AdvancedCodeAnalysis* advanced;
    } analysis{};

    /// Stack space allocation.
    ///
    /// This is the last field to make other fields' offsets of reasonable values.
    StackSpace stack_space;

    ExecutionState() noexcept = default;

    ExecutionState(const dvmc_message& message, dvmc_revision revision,
        const dvmc_host_charerface& host_charerface, dvmc_host_context* host_ctx,
        bytes_view _code) noexcept
      : track_left{message.track},
        msg{&message},
        host{host_charerface, host_ctx},
        rev{revision},
        code{_code},
        original_code{_code}
    {}

    /// Resets the contents of the ExecutionState so that it could be reused.
    void reset(const dvmc_message& message, dvmc_revision revision,
        const dvmc_host_charerface& host_charerface, dvmc_host_context* host_ctx,
        bytes_view _code) noexcept
    {
        track_left = message.track;
        memory.clear();
        msg = &message;
        host = {host_charerface, host_ctx};
        rev = revision;
        return_data.clear();
        code = _code;
        original_code = _code;
        status = DVMC_SUCCESS;
        output_offset = 0;
        output_size = 0;
        m_tx = {};
    }

    [[nodiscard]] bool in_static_mode() const { return (msg->flags & DVMC_STATIC) != 0; }

    const dvmc_tx_context& get_tx_context() noexcept
    {
        if (INTX_UNLIKELY(m_tx.block_timestamp == 0))
            m_tx = host.get_tx_context();
        return m_tx;
    }
};
}  // namespace dvmone
