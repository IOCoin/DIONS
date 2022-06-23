// 
// Copyright 2022 blastdoor7
// 
#pragma once

#include <dvmc/hex.hpp>

using dvmc::bytes;
using dvmc::bytes_view;
using dvmc::from_hex;
using dvmc::hex;

/// Decodes the hexx encoded string.
///
/// The hexx encoding format is the hex format (base 16) with the extension
/// for run-length encoding. The parser replaces expressions like
///     `(` <num_repetitions> `x` <element> `)`
/// with `<element>` repeated `<num_repetitions>` times.
/// E.g. `(2x1d3)` is `1d31d3` in hex.
///
/// @param hexx  The hexx encoded string.
/// @return      The decoded bytes.
bytes from_hexx(const std::string& hexx);
