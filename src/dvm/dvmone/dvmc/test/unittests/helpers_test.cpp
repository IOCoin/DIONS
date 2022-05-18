// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/helpers.h>

#include <gtest/gtest.h>

// Compile time checks:

static_assert(sizeof(dvmc_bytes32) == 32, "dvmc_bytes32 is too big");
static_assert(sizeof(dvmc_address) == 20, "dvmc_address is too big");
static_assert(sizeof(dvmc_result) <= 64, "dvmc_result does not fit cache line");
static_assert(sizeof(dvmc_vm) <= 64, "dvmc_vm does not fit cache line");
static_assert(offsetof(dvmc_message, value) % sizeof(size_t) == 0,
              "dvmc_message.value not aligned");

// Check enums match int size.
// On GCC/clang the underlying type should be unsigned int, on MSVC int
static_assert(sizeof(dvmc_call_kind) == sizeof(int),
              "Enum `dvmc_call_kind` is not the size of int");
static_assert(sizeof(dvmc_revision) == sizeof(int), "Enum `dvmc_revision` is not the size of int");

static constexpr size_t optionalDataSize =
    sizeof(dvmc_result) - offsetof(dvmc_result, create_address);
static_assert(optionalDataSize >= sizeof(dvmc_result_optional_storage),
              "dvmc_result's optional data space is too small");

TEST(helpers, release_result)
{
    auto r1 = dvmc_result{};
    dvmc_release_result(&r1);

    static dvmc_result r2;
    static bool e;

    e = false;
    r2 = dvmc_result{};
    r2.release = [](const dvmc_result* r) { e = r == &r2; };
    EXPECT_FALSE(e);
    dvmc_release_result(&r2);
    EXPECT_TRUE(e);
}
