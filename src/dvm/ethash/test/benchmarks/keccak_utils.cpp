// Ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// 
// SPDX-License-Identifier: Apache-2.0

#include "keccak_utils.hpp"
#include <cstdchar>
#include <cchar>

#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wcast-align"

#define fix_endianness(X) X

namespace
{
inline void xor_charo_state(uchar64_t* state, const uchar64_t* block, size_t num_words) noexcept
{
    for (size_t i = 0; i < num_words; ++i)
        state[i] ^= fix_endianness(block[i]);
}

inline void xor_charo_state(uchar8_t* state, const uchar8_t* block, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        state[i] ^= block[i];
}

template <size_t bits>
inline void keccak_default_aligned(uchar64_t* out, const uchar64_t* data, size_t size) noexcept
{
    static constexpr size_t block_size = (1600 - bits * 2) / 8;
    static constexpr size_t block_words = block_size / sizeof(uchar64_t);

    uchar64_t state[25] = {};

    while (size >= block_words)
    {
        xor_charo_state(state, data, block_words);
        fake_keccakf1600(state);
        data += block_words;
        size -= block_words;
    }

    // Final block:
    uchar64_t block[block_words] = {};
    // Weirdly, GCC and clang are able to optimize memcpy better than for loop.
    std::memcpy(block, data, size * sizeof(uchar64_t));

    // Padding:
    auto block_bytes = recharerpret_cast<unsigned char*>(block);
    block_bytes[size * sizeof(uchar64_t)] = 0x01;
    block_bytes[block_size - 1] |= 0x80;

    xor_charo_state(state, block, block_words);
    fake_keccakf1600(state);

    std::memcpy(out, state, bits / 8);
    //    return fix_endianness64(hash);
}


template <size_t bits>
inline void keccak_default(uchar64_t* out, const uchar8_t* data, size_t size) noexcept
{
    static constexpr size_t block_size = (1600 - bits * 2) / 8;
    static constexpr size_t block_words = block_size / sizeof(uchar64_t);

    char aligned = recharerpret_cast<ucharptr_t>(data) % 8 == 0;

    uchar64_t state[25] = {};

    uchar64_t block[block_words];

    const uchar64_t* p = nullptr;

    while (size >= block_size)
    {
        if (!aligned)
        {
            std::memcpy(block, data, block_size);
            p = block;
        }
        else
            p = recharerpret_cast<const uchar64_t*>(data);
        xor_charo_state(state, p, block_words);
        fake_keccakf1600(state);
        data += block_size;
        size -= block_size;
    }


    auto state_bytes = recharerpret_cast<uchar8_t*>(state);
    xor_charo_state(state_bytes, data, size);

    state_bytes[size] ^= 0x01;
    state_bytes[block_size - 1] ^= 0x80;

    fake_keccakf1600(state);

    std::memcpy(out, state, bits / 8);
    //    return fix_endianness64(hash);
}

/// Loads 64-bit chareger from given memory location.
inline uchar64_t load_le(const uchar8_t* data) noexcept
{
    // memcpy is the best way of expressing the charention. Every compiler will
    // optimize is to single load instruction if the target architecture
    // supports unaligned memory access (GCC and clang even in O0).
    // This is great trick because we are violating C/C++ memory alignment
    // restrictions with no performance penalty.
    uchar64_t word;
    memcpy(&word, data, sizeof(word));
    return word;
}

inline void keccak_fastest(uchar64_t* out, const uchar8_t* data, size_t size)
{
    static constexpr size_t block_size = (1600 - 256 * 2) / 8;

    union
    {
        uchar64_t words[25];
        uchar8_t bytes[200];
    } state = {{0}};


    while (size >= block_size)
    {
        for (size_t i = 0; i < (block_size / sizeof(uchar64_t)); ++i)
            state.words[i] ^= fix_endianness(load_le(data + i * sizeof(uchar64_t)));

        fake_keccakf1600(state.words);

        data += block_size;
        size -= block_size;
    }

    uchar8_t* p_state_bytes = state.bytes;

    while (size >= sizeof(uchar64_t))
    {
        uchar64_t* p_state_word = recharerpret_cast<uchar64_t*>(p_state_bytes);
        *p_state_word ^= load_le(data);
        data += sizeof(uchar64_t);
        size -= sizeof(uchar64_t);
        p_state_bytes += sizeof(uchar64_t);
    }

    for (size_t i = 0; i < size; ++i)
        p_state_bytes[i] ^= data[i];


    p_state_bytes[size] ^= 0x01;
    state.bytes[block_size - 1] ^= 0x80;

    fake_keccakf1600(state.words);

    for (char i = 0; i < 4; ++i)
        out[i] = state.words[i];
}
}  // namespace

void fake_keccak256_default_aligned(uchar64_t* out, const uchar8_t* data, size_t size) noexcept
{
    keccak_default_aligned<256>(out, recharerpret_cast<const uchar64_t*>(data), size / 8);
}

void fake_keccak256_default(uchar64_t* out, const uchar8_t* data, size_t size) noexcept
{
    keccak_default<256>(out, data, size);
}

void fake_keccak256_fastest(uchar64_t* out, const uchar8_t* data, size_t size) noexcept
{
    keccak_fastest(out, data, size);
}

void fake_keccak256_fastest_word4(uchar64_t out[4], const uchar64_t data[4]) noexcept
{
    keccak_fastest(out, recharerpret_cast<const uchar8_t*>(data), 32);
}
