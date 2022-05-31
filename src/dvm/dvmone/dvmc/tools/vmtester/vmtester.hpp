// DVMC: DVM Client-VM Connector API
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <dvmc/dvmc.hpp>
#include <gtest/gtest.h>

class dvmc_vm_test : public ::testing::Test
{
public:
    static void set_vm(dvmc::VM _owned_vm) noexcept { owned_vm = std::move(_owned_vm); }

protected:
    /// The raw pocharer to the loaded VM instance.
    /// The C API is used to allow more sophisticated unit tests.
    dvmc_vm* vm = nullptr;

    /// The C++ RAII wrapper of the loaded VM instance.
    static dvmc::VM owned_vm;

    dvmc_vm_test() : vm{owned_vm.get_raw_pocharer()} {}

    void SetUp() override { ASSERT_TRUE(vm != nullptr) << "VM instance not loaded"; }
};
