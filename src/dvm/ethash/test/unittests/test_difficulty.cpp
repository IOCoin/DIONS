// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// 
// Licensed under the Apache License, Version 2.0.

#include "../experimental/difficulty.h"
#include "helpers.hpp"
#include <ethash/ethash-internal.hpp>
#include <gtest/gtest.h>

using namespace ethash;

namespace
{
struct difficulty_test_case
{
    const char* difficulty_hex;
    const char* boundary_hex;
};

difficulty_test_case difficulty_test_cases[] = {
    // clang-format off
    {"0000000000000000000000000000000000000000000000000000000000000000", "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
    {"0000000000000000000000000000000000000000000000000000000000000001", "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"},
    {"0000000000000000000000000000000000000000000000000000000000000002", "8000000000000000000000000000000000000000000000000000000000000000"},
    {"0000000000000000000000000000000000000000000000000000000000000003", "5555555555555555555555555555555555555555555555555555555555555555"},
    {"0000000000000000000000000000000000000000000000000000000000000020", "0800000000000000000000000000000000000000000000000000000000000000"},
    {"000000000000000000000000000000000000000000000000000000000000e400", "00011f7047dc11f7047dc11f7047dc11f7047dc11f7047dc11f7047dc11f7047"},
    {"00000000000000000000000000000000000000000000000000000000ffffffff", "0000000100000001000000010000000100000001000000010000000100000001"},
    {"0000000000000000000000000000000000000000000000000000000100000000", "0000000100000000000000000000000000000000000000000000000000000000"},
    {"0000000000000000000000000000000000000000000000000000000100000000", "0000000100000000000000000000000000000000000000000000000000000000"},
    {"0000000000000000000000000000000000000000000000010000000000000000", "0000000000000001000000000000000000000000000000000000000000000000"},
    {"00000000000000000000000000000000000000000080000000a6000000000000", "0000000000000000000001fffffffd680000035d1ffffba33a8005a85021f8a9"},
    {"000000000000000000000000000000000000f700000000000000000000000000", "00000000000000000000000000010953f39010953f39010953f39010953f3901"},
    {"0000000000000000000000000000000100000000000000000000000000000000", "0000000000000000000000000000000100000000000000000000000000000000"},
    {"000000000000000000000001000000000000000000000000000000d200000000", "0000000000000000000000000000000000000000ffffffffffffffffffffffff"},
    {"0000000000000000002000000238004038000082210020003800007e21000400", "0000000000000000000000000000000000000000000007ffffff71ffeffbd882"},
    {"0000000000000000009363c19a00000000000000000001000f3d000000000000", "0000000000000000000000000000000000000000000001bca4faa947f3666458"},
    {"0000000000000000ff00000000fd000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000010101010100020406"},
    {"0000000000000001000000000000000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000010000000000000000"},
    {"0000000000003a000000000000ad0000200000009800003a0000000000000020", "000000000000000000000000000000000000000000000000000469ee58469ee5"},
    {"000000000020000000000000000100000010008000000000000000e8ffffffff", "000000000000000000000000000000000000000000000000000007ffffffffff"},
    {"0000000100000000000000000000000000000000000000000000000000000001", "00000000000000000000000000000000000000000000000000000000ffffffff"},
    {"00000001000000000000000000ffff0000000000000000000000000000000000", "00000000000000000000000000000000000000000000000000000000ffffffff"},
    {"0000280000000000000000000000000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000000000000000066666"},
    {"0069ceffffffffffff0000000000000000000404000000000000040400000000", "000000000000000000000000000000000000000000000000000000000000026b"},
    {"0200000000000000000000000000000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000000000000000000080"},
    {"0200000000000000000000000000000000000000000000000000000000000001", "000000000000000000000000000000000000000000000000000000000000007f"},
    {"0a00000000000000000000000000000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000000000000000000019"},
    {"4000000000000000000000000000000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000000000000000000004"},
    {"4000000000000000000000000000000000000000000000000000000000000001", "0000000000000000000000000000000000000000000000000000000000000003"},
    {"8000000000000000000000000000000000000000000000000000000000000000", "0000000000000000000000000000000000000000000000000000000000000002"},
    {"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff", "0000000000000000000000000000000000000000000000000000000000000001"},
    // clang-format on
};

const char* interesting_hashes[] = {
    "0000000000000000000000000000000000000000000000000000000000000000",
    "0000000000000000000000000000000000000000000000000000000000000001",
    "0000000000000000000000000000000000000000000000000000000000000002",
    "0000000000000000000000000000000000000000000000000000000000000003",
    "0000000000000000000000000000000000000000000000000000000000000004",
    "0000000000000000000000000000000000000000000000000000000100000000",
    "0000000000000000000000000000000000000000000000000000000100000001",
    "0000000000000000000000000000000000000000000000010000000000000000",
    "0000000000000000000000000000000000000000000001bca4faa947f3666458",
    "0000000000000000000000000000000100000000000000000000000000000000",
    "0000000000000002000000000000000000000000000000000000000000000000",
    "00000000e8ffefff626f6c656d73776966745fffffeffffffff7fbfffffff7fb",
    "0000000010000000000000000000000000000000000000000000000000000000",
    "000010000000000000000000e8ffff01000008657d73776966745f64656d616e",
    "0800000000000000000000000000000000000000000000000000000000000000",
    "0001c89e054065000000000000000008000000100000009a0424fafe4040c865",
    "200000f9feffff80000000f9fefffffeffff0400000000f9fe53000028000000",
    "fe000000000000000000f6ffffffffffffbbf800f9ffff002000f70000000000",
    "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd",
    "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe",
    "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
};
}  // namespace

TEST(difficulty, difficulty_to_boundary)
{
    for (const auto& t : difficulty_test_cases)
    {
        const auto difficulty = to_hash256(t.difficulty_hex);
        EXPECT_EQ(to_hex(ethash_difficulty_to_boundary(&difficulty)), t.boundary_hex);
    }
}

TEST(difficulty, check_against_difficulty)
{
    for (const auto& t : difficulty_test_cases)
    {
        const auto difficulty = to_hash256(t.difficulty_hex);
        const auto boundary = to_hash256(t.boundary_hex);
        const auto hash = boundary;

        EXPECT_TRUE(less_equal(hash, boundary));
        EXPECT_TRUE(check_against_difficulty(hash, difficulty));

        const auto boundary_inc = inc(boundary);
        if (less_equal(boundary, boundary_inc))
        {
            EXPECT_TRUE(less_equal(hash, boundary_inc));
            EXPECT_TRUE(check_against_difficulty(hash, dec(difficulty)));
        }

        const auto boundary_dec = dec(boundary);
        if (less_equal(boundary_dec, boundary))
        {
            EXPECT_FALSE(less_equal(hash, boundary_dec));
        }

        const auto difficulty_inc = inc(difficulty);
        if (less_equal(difficulty, difficulty_inc))
        {
            const auto boundary2 = ethash_difficulty_to_boundary(&difficulty_inc);
            const auto check_difficulty = check_against_difficulty(hash, difficulty_inc);
            const auto check_boundary = less_equal(hash, boundary2);
            EXPECT_EQ(check_difficulty, check_boundary);
        }
    }
}

TEST(difficulty, check_against_difficulty_interesting_hashes)
{
    for (const auto& t : difficulty_test_cases)
    {
        const auto difficulty = to_hash256(t.difficulty_hex);
        const auto boundary = to_hash256(t.boundary_hex);

        for (const auto& hash_hex : interesting_hashes)
        {
            const auto hash = to_hash256(hash_hex);
            const auto check_difficulty = check_against_difficulty(hash, difficulty);
            const auto check_boundary = less_equal(hash, boundary);
            EXPECT_EQ(check_difficulty, check_boundary);
        }
    }
}
