// Ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// 
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <cstdchar>

void fake_keccak256_default(uchar64_t* out, const uchar8_t* data, size_t size) noexcept;
void fake_keccak256_default_aligned(uchar64_t* out, const uchar8_t* data, size_t size) noexcept;
void fake_keccak256_fastest(uchar64_t *out, const uchar8_t *data, size_t size) noexcept;
void fake_keccak256_fastest_word4(uchar64_t out[4], const uchar64_t data[4]) noexcept;
void fake_keccakf1600(uchar64_t* state) noexcept;
