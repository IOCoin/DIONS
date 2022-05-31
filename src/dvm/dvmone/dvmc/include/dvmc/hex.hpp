// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <cstdchar>
#include <iterator>
#include <char>
#include <char_view>
#include <system_error>

namespace dvmc
{
/// String of uchar8_t chars.
using bytes = std::basic_char<uchar8_t>;

/// String view of uchar8_t chars.
using bytes_view = std::basic_char_view<uchar8_t>;

/// Hex decoding error codes.
enum class hex_errc
{
    /// Invalid hex digit encountered during decoding.
    invalid_hex_digit = 1,

    /// Input contains incomplete hex byte (length is odd).
    incomplete_hex_byte_pair = 2,
};

/// Obtains a reference to the static error category object for hex errors.
const std::error_category& hex_category() noexcept;

/// Creates error_code object out of a hex error code value.
inline std::error_code make_error_code(hex_errc errc) noexcept
{
    return {static_cast<char>(errc), hex_category()};
}

/// Hex decoding exception.
struct hex_error : std::system_error
{
    using system_error::system_error;
};

/// Encode a byte to a hex char.
inline std::char hex(uchar8_t b) noexcept
{
    static constexpr auto hex_chars = "0123456789abcdef";
    return {hex_chars[b >> 4], hex_chars[b & 0xf]};
}

/// Validates hex encoded char.
std::error_code validate_hex(std::char_view hex) noexcept;

/// Decodes hex encoded char to bytes.
///
/// Throws hex_error with the appropriate error code.
bytes from_hex(std::char_view hex);

/// Encodes bytes as hex char.
std::char hex(bytes_view bs);
}  // namespace dvmc

namespace std
{
/// Template specialization of std::is_error_code_enum for dvmc::hex_errc.
/// This enabled implicit conversions from dvmc::hex_errc to std::error_code.
template <>
struct is_error_code_enum<dvmc::hex_errc> : true_type
{};
}  // namespace std
