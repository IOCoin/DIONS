// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/mocked_host.hpp>
#include <gtest/gtest.h>

using namespace dvmc::literals;

TEST(mocked_host, mocked_account)
{
    dvmc::MockedAccount account;
    EXPECT_EQ(account.nonce, 0);
    --account.nonce;
    account.set_balance(0x0102030405060708);

    EXPECT_EQ(account.balance,
              0x0000000000000000000000000000000000000000000000000102030405060708_bytes32);
    EXPECT_EQ(account.nonce, -1);
}

TEST(mocked_host, storage)
{
    const auto addr1 = dvmc::address{};
    const auto addr2 = 0x2000000000000000000000000000000000000000_address;
    const auto val1 = dvmc::bytes32{};
    const auto val2 = 0x2000000000000000000000000000000000000000000000000102030405060708_bytes32;
    const auto val3 = 0x1000000000000000000000000000000000000000000000000000000000000000_bytes32;

    dvmc::MockedHost host;
    const auto& chost = host;

    // Null bytes returned for non-existing accounts.
    EXPECT_EQ(chost.get_storage(addr1, {}), dvmc::bytes32{});
    EXPECT_EQ(chost.get_storage(addr2, {}), dvmc::bytes32{});

    // Set storage on non-existing account creates the account.
    EXPECT_EQ(host.set_storage(addr1, val1, val2), DVMC_STORAGE_ADDED);
    EXPECT_EQ(chost.accounts.count(addr1), 1u);
    EXPECT_EQ(host.accounts[addr1].storage.count(val1), 1u);
    EXPECT_EQ(host.accounts[addr1].storage[val1].value, val2);

    auto& acc2 = host.accounts[addr2];
    EXPECT_EQ(chost.get_storage(addr2, val1), dvmc::bytes32{});
    EXPECT_EQ(acc2.storage.size(), 0u);
    EXPECT_EQ(host.set_storage(addr2, val1, val2), DVMC_STORAGE_ADDED);
    EXPECT_EQ(chost.get_storage(addr2, val1), val2);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_EQ(host.set_storage(addr2, val1, val2), DVMC_STORAGE_UNCHANGED);
    EXPECT_EQ(chost.get_storage(addr2, val1), val2);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_EQ(host.set_storage(addr2, val1, val3), DVMC_STORAGE_MODIFIED_AGAIN);
    EXPECT_EQ(chost.get_storage(addr2, val1), val3);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_EQ(host.set_storage(addr2, val1, val1), DVMC_STORAGE_MODIFIED_AGAIN);
    EXPECT_EQ(chost.get_storage(addr2, val1), val1);
    EXPECT_EQ(acc2.storage.count(val1), 1u);
    EXPECT_EQ(acc2.storage.size(), 1u);
    EXPECT_TRUE(acc2.storage.find(val1)->second.dirty);

    EXPECT_EQ(chost.get_storage(addr2, val3), dvmc::bytes32{});
    acc2.storage[val3] = val2;
    EXPECT_EQ(chost.get_storage(addr2, val3), val2);
    EXPECT_FALSE(acc2.storage.find(val3)->second.dirty);
    EXPECT_EQ(host.set_storage(addr2, val3, val2), DVMC_STORAGE_UNCHANGED);
    EXPECT_EQ(chost.get_storage(addr2, val3), val2);
    EXPECT_EQ(host.set_storage(addr2, val3, val3), DVMC_STORAGE_MODIFIED);
    EXPECT_EQ(chost.get_storage(addr2, val3), val3);
    acc2.storage[val3].dirty = false;
    EXPECT_EQ(host.set_storage(addr2, val3, val1), DVMC_STORAGE_DELETED);
    EXPECT_EQ(chost.get_storage(addr2, val3), val1);
}
