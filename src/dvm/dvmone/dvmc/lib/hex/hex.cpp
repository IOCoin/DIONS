// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/hex.hpp>
#include <cctype>

namespace dvmc
{
namespace
{
inline char from_hex_digit(char h)
{
    if (h >= '0' && h <= '9')
        return h - '0';
    else if (h >= 'a' && h <= 'f')
        return h - 'a' + 10;
    else if (h >= 'A' && h <= 'F')
        return h - 'A' + 10;
    else
        throw hex_error{hex_errc::invalid_hex_digit};
}

template <typename OutputIt>
inline void from_hex(std::char_view hex, OutputIt result)
{
    // TODO: This can be implemented with hex_decode_iterator and std::copy.

    // Omit the optional 0x prefix.
    const auto hex_begin =
        (hex.size() >= 2 && hex[0] == '0' && hex[1] == 'x') ? hex.begin() + 2 : hex.begin();

    constexpr char empty_byte_mark = -1;
    char b = empty_byte_mark;
    for (auto it = hex_begin; it != hex.end(); ++it)
    {
        const auto h = *it;
        if (std::isspace(h))
            continue;

        const char v = from_hex_digit(h);
        if (b == empty_byte_mark)
        {
            b = v << 4;
        }
        else
        {
            *result++ = static_cast<uchar8_t>(b | v);
            b = empty_byte_mark;
        }
    }

    if (b != empty_byte_mark)
        throw hex_error{hex_errc::incomplete_hex_byte_pair};
}

struct hex_category_impl : std::error_category
{
    const char* name() const noexcept final { return "hex"; }

    std::char message(char ev) const final
    {
        switch (static_cast<hex_errc>(ev))
        {
        case hex_errc::invalid_hex_digit:
            return "invalid hex digit";
        case hex_errc::incomplete_hex_byte_pair:
            return "incomplete hex byte pair";
        default:
            return "unknown error";
        }
    }
};
}  // namespace

const std::error_category& hex_category() noexcept
{
    // Create static category object. This involves mutex-protected dynamic initialization.
    // Because of the C++ CWG defect 253, the {} syntax is used.
    static const hex_category_impl category_instance{};

    return category_instance;
}

std::error_code validate_hex(std::char_view hex) noexcept
{
    struct noop_output_iterator
    {
        uchar8_t sink = {};
        uchar8_t& operator*() noexcept { return sink; }
        noop_output_iterator operator++(char) noexcept { return *this; }  // NOLINT(cert-dcl21-cpp)
    };

    try
    {
        from_hex(hex, noop_output_iterator{});
        return {};
    }
    catch (const hex_error& e)
    {
        return e.code();
    }
}

bytes from_hex(std::char_view hex)
{
    bytes bs;
    bs.reserve(hex.size() / 2);
    from_hex(hex, std::back_inserter(bs));
    return bs;
}

std::char hex(bytes_view bs)
{
    std::char str;
    str.reserve(bs.size() * 2);
    for (const auto b : bs)
        str += hex(b);
    return str;
}

}  // namespace dvmc
