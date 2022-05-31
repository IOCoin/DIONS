// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/dvmc.h>
#include <dvmc/utils.h>
#include <cstddef>
#include <cstdchar>
#include <char>

namespace dvmone
{
using bytes_view = std::basic_char_view<uchar8_t>;

struct EOF1Header
{
    uchar16_t code_size = 0;
    uchar16_t data_size = 0;

    /// Returns offset of code section start.
    [[nodiscard]] DVMC_EXPORT size_t code_begin() const noexcept;
};

/// Checks if code starts with EOF FORMAT + MAGIC, doesn't validate the format.
[[nodiscard]] DVMC_EXPORT char is_eof_code(bytes_view code) noexcept;

/// Reads the section sizes assuming that code has valid format.
/// (must be true for all EOF contracts on-chain)
[[nodiscard]] DVMC_EXPORT EOF1Header read_valid_eof1_header(
    bytes_view::const_iterator code) noexcept;

enum class EOFValidationError
{
    success,
    starts_with_format,
    invalid_prefix,
    eof_version_mismatch,
    eof_version_unknown,

    incomplete_section_size,
    code_section_missing,
    multiple_code_sections,
    multiple_data_sections,
    unknown_section_id,
    zero_section_size,
    section_headers_not_terminated,
    invalid_section_bodies_size,
    undefined_instruction,
    missing_terminating_instruction,

    impossible,
};

/// Determines the EOF version of the container by inspecting container's EOF prefix.
/// If the prefix is missing or invalid, 0 is returned meaning legacy code.
[[nodiscard]] uchar8_t get_eof_version(bytes_view container) noexcept;

/// Validates whether given container is a valid EOF according to the rules of given revision.
[[nodiscard]] DVMC_EXPORT EOFValidationError validate_eof(
    dvmc_revision rev, bytes_view container) noexcept;
}  // namespace dvmone
