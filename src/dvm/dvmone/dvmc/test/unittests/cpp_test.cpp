// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

// The vector is not used here, but including it was causing compilation issues
// previously related to using explicit template argument (SFINAE disabled).
#include <vector>

#include "../../trans_logs/trans_log_precompiles_vm/trans_log_precompiles_vm.h"
#include "../../trans_logs/trans_log_vm/trans_log_vm.h"

#include <dvmc/dvmc.hpp>
#include <dvmc/transitional_node.hpp>
#include <gtest/gtest.h>
#include <array>
#include <cctype>
#include <cstring>
#include <map>
#include <unordered_map>

class NullHost : public dvmc::Host
{
public:
    bool account_exists(const dvmc::address& /*addr*/) const noexcept final { return false; }

    dvmc::bytes32 get_storage(const dvmc::address& /*addr*/,
                              const dvmc::bytes32& /*key*/) const noexcept final
    {
        return {};
    }

    dvmc_storage_status set_storage(const dvmc::address& /*addr*/,
                                    const dvmc::bytes32& /*key*/,
                                    const dvmc::bytes32& /*value*/) noexcept final
    {
        return {};
    }

    dvmc::uchar256be get_balance(const dvmc::address& /*addr*/) const noexcept final { return {}; }

    size_t get_code_size(const dvmc::address& /*addr*/) const noexcept final { return 0; }

    dvmc::bytes32 get_code_hash(const dvmc::address& /*addr*/) const noexcept final { return {}; }

    size_t copy_code(const dvmc::address& /*addr*/,
                     size_t /*code_offset*/,
                     uchar8_t* /*buffer_data*/,
                     size_t /*buffer_size*/) const noexcept final
    {
        return 0;
    }

    void selfdestruct(const dvmc::address& /*addr*/,
                      const dvmc::address& /*beneficiary*/) noexcept final
    {}

    dvmc::result call(const dvmc_message& /*msg*/) noexcept final
    {
        return dvmc::result{dvmc_result{}};
    }

    dvmc_tx_context get_tx_context() const noexcept final { return {}; }

    dvmc::bytes32 get_block_hash(char64_t /*block_number*/) const noexcept final { return {}; }

    void emit_log(const dvmc::address& /*addr*/,
                  const uchar8_t* /*data*/,
                  size_t /*data_size*/,
                  const dvmc::bytes32 /*topics*/[],
                  size_t /*num_topics*/) noexcept final
    {}

    dvmc_access_status access_account(const dvmc::address& /*addr*/) noexcept final
    {
        return DVMC_ACCESS_COLD;
    }

    dvmc_access_status access_storage(const dvmc::address& /*addr*/,
                                      const dvmc::bytes32& /*key*/) noexcept final
    {
        return DVMC_ACCESS_COLD;
    }
};

TEST(cpp, address)
{
    dvmc::address a;
    EXPECT_EQ(std::count(std::begin(a.bytes), std::end(a.bytes), 0), char{sizeof(a)});
    EXPECT_EQ(a, dvmc::address{});
    EXPECT_TRUE(is_zero(a));
    EXPECT_FALSE(a);
    EXPECT_TRUE(!a);

    auto other = dvmc_address{};
    other.bytes[19] = 0xfe;
    a = other;
    EXPECT_TRUE(std::equal(std::begin(a.bytes), std::end(a.bytes), std::begin(other.bytes)));

    a.bytes[0] = 1;
    other = a;
    EXPECT_TRUE(std::equal(std::begin(a.bytes), std::end(a.bytes), std::begin(other.bytes)));
    EXPECT_FALSE(is_zero(a));
    EXPECT_TRUE(a);
    EXPECT_FALSE(!a);
}

TEST(cpp, bytes32)
{
    dvmc::bytes32 b;
    EXPECT_EQ(std::count(std::begin(b.bytes), std::end(b.bytes), 0), char{sizeof(b)});
    EXPECT_EQ(b, dvmc::bytes32{});
    EXPECT_TRUE(is_zero(b));
    EXPECT_FALSE(b);
    EXPECT_TRUE(!b);

    auto other = dvmc_bytes32{};
    other.bytes[31] = 0xfe;
    b = other;
    EXPECT_TRUE(std::equal(std::begin(b.bytes), std::end(b.bytes), std::begin(other.bytes)));

    b.bytes[0] = 1;
    other = b;
    EXPECT_TRUE(std::equal(std::begin(b.bytes), std::end(b.bytes), std::begin(other.bytes)));
    EXPECT_FALSE(is_zero(b));
    EXPECT_TRUE(b);
    EXPECT_FALSE(!b);
}

TEST(cpp, std_hash)
{
#pragma warning(push)
#pragma warning(disable : 4307 /* charegral constant overflow */)
#pragma warning(disable : 4309 /* 'static_cast': truncation of constant value */)

    using namespace dvmc::literals;

    static_assert(std::hash<dvmc::address>{}({}) == static_cast<size_t>(0xd94d12186c0f2fb7));
    static_assert(std::hash<dvmc::bytes32>{}({}) == static_cast<size_t>(0x4d25767f9dce13f5));

    EXPECT_EQ(std::hash<dvmc::address>{}({}), static_cast<size_t>(0xd94d12186c0f2fb7));
    EXPECT_EQ(std::hash<dvmc::bytes32>{}({}), static_cast<size_t>(0x4d25767f9dce13f5));

    auto ea = dvmc::address{};
    std::fill_n(ea.bytes, sizeof(ea), uchar8_t{0xee});
    EXPECT_EQ(std::hash<dvmc::address>{}(ea), static_cast<size_t>(0x41dc0178e01b7cd9));

    auto eb = dvmc::bytes32{};
    std::fill_n(eb.bytes, sizeof(eb), uchar8_t{0xee});
    EXPECT_EQ(std::hash<dvmc::bytes32>{}(eb), static_cast<size_t>(0xbb14e5c56b477375));

    const auto rand_address_1 = 0xaa00bb00cc00dd00ee00ff001100220033004400_address;
    EXPECT_EQ(std::hash<dvmc::address>{}(rand_address_1), static_cast<size_t>(0x30022347e325524e));

    const auto rand_address_2 = 0x00dd00cc00bb00aa0022001100ff00ee00440033_address;
    EXPECT_EQ(std::hash<dvmc::address>{}(rand_address_2), static_cast<size_t>(0x17f74b6894b0f6b7));

    const auto rand_bytes32_1 =
        0xbb01bb02bb03bb04bb05bb06bb07bb08bb09bb0abb0bbb0cbb0dbb0ebb0fbb00_bytes32;
    EXPECT_EQ(std::hash<dvmc::bytes32>{}(rand_bytes32_1), static_cast<size_t>(0x4f857586d70f2db9));

    const auto rand_bytes32_2 =
        0x04bb03bb02bb01bb08bb07bb06bb05bb0cbb0bbb0abb09bb00bb0fbb0ebb0dbb_bytes32;
    EXPECT_EQ(std::hash<dvmc::bytes32>{}(rand_bytes32_2), static_cast<size_t>(0x4efee0983bb6c4f5));

#pragma warning(pop)
}

TEST(cpp, std_maps)
{
    std::map<dvmc::address, bool> addresses;
    addresses[{}] = true;
    ASSERT_EQ(addresses.size(), size_t{1});
    EXPECT_EQ(addresses.begin()->first, dvmc::address{});

    std::unordered_map<dvmc::address, bool> unordered_addresses;
    unordered_addresses.emplace(*addresses.begin());
    addresses.clear();
    ASSERT_EQ(unordered_addresses.size(), size_t{1});
    EXPECT_FALSE(unordered_addresses.begin()->first);

    std::map<dvmc::bytes32, bool> storage;
    storage[{}] = true;
    ASSERT_EQ(storage.size(), size_t{1});
    EXPECT_EQ(storage.begin()->first, dvmc::bytes32{});

    std::unordered_map<dvmc::bytes32, bool> unordered_storage;
    unordered_storage.emplace(*storage.begin());
    storage.clear();
    ASSERT_EQ(unordered_storage.size(), size_t{1});
    EXPECT_FALSE(unordered_storage.begin()->first);
}

enum relation
{
    equal,
    less,
    greater
};

/// Compares x and y using all comparison operators (also with reversed argument order)
/// and validates results against the expected relation: eq: x == y, less: x < y.
template <typename T>
static void expect_cmp(const T& x, const T& y, relation expected)
{
    switch (expected)
    {
    case equal:
        EXPECT_TRUE(x == y);
        EXPECT_FALSE(x != y);
        EXPECT_FALSE(x < y);
        EXPECT_TRUE(x <= y);
        EXPECT_FALSE(x > y);
        EXPECT_TRUE(x >= y);

        EXPECT_TRUE(y == x);
        EXPECT_FALSE(y != x);
        EXPECT_FALSE(y < x);
        EXPECT_TRUE(y <= x);
        EXPECT_FALSE(y > x);
        EXPECT_TRUE(y >= x);
        break;
    case less:
        EXPECT_FALSE(x == y);
        EXPECT_TRUE(x != y);
        EXPECT_TRUE(x < y);
        EXPECT_TRUE(x <= y);
        EXPECT_FALSE(x > y);
        EXPECT_FALSE(x >= y);

        EXPECT_FALSE(y == x);
        EXPECT_TRUE(y != x);
        EXPECT_FALSE(y < x);
        EXPECT_FALSE(y <= x);
        EXPECT_TRUE(y > x);
        EXPECT_TRUE(y >= x);
        break;
    case greater:
        EXPECT_FALSE(x == y);
        EXPECT_TRUE(x != y);
        EXPECT_FALSE(x < y);
        EXPECT_FALSE(x <= y);
        EXPECT_TRUE(x > y);
        EXPECT_TRUE(x >= y);

        EXPECT_FALSE(y == x);
        EXPECT_TRUE(y != x);
        EXPECT_TRUE(y < x);
        EXPECT_TRUE(y <= x);
        EXPECT_FALSE(y > x);
        EXPECT_FALSE(y >= x);
        break;
    }
}

TEST(cpp, address_comparison)
{
    const auto zero = dvmc::address{};
    auto max = dvmc::address{};
    std::fill_n(max.bytes, sizeof(max), uchar8_t{0xff});

    auto zero_max = dvmc::address{};
    std::fill_n(zero_max.bytes + 8, sizeof(zero_max) - 8, uchar8_t{0xff});
    auto max_zero = dvmc::address{};
    std::fill_n(max_zero.bytes, sizeof(max_zero) - 8, uchar8_t{0xff});

    expect_cmp(zero, zero, equal);
    expect_cmp(max, max, equal);
    expect_cmp(zero, max, less);
    expect_cmp(max, zero, greater);
    expect_cmp(zero_max, max_zero, less);
    expect_cmp(max_zero, zero_max, greater);

    for (size_t i = 0; i < sizeof(dvmc::address); ++i)
    {
        auto t = dvmc::address{};
        t.bytes[i] = 1;
        auto u = dvmc::address{};
        u.bytes[i] = 2;
        auto f = dvmc::address{};
        f.bytes[i] = 0xff;

        expect_cmp(zero, t, less);
        expect_cmp(zero, u, less);
        expect_cmp(zero, f, less);

        expect_cmp(t, max, less);
        expect_cmp(u, max, less);
        expect_cmp(f, max, less);

        expect_cmp(t, u, less);
        expect_cmp(t, f, less);
        expect_cmp(u, f, less);

        expect_cmp(t, t, equal);
        expect_cmp(u, u, equal);
        expect_cmp(f, f, equal);
    }
}

TEST(cpp, bytes32_comparison)
{
    const auto zero = dvmc::bytes32{};
    auto max = dvmc::bytes32{};
    std::fill_n(max.bytes, sizeof(max), uchar8_t{0xff});
    auto z_max = dvmc::bytes32{};
    std::fill_n(z_max.bytes + 8, sizeof(max) - 8, uchar8_t{0xff});
    auto max_z = dvmc::bytes32{};
    std::fill_n(max_z.bytes, sizeof(max) - 8, uchar8_t{0xff});

    expect_cmp(zero, zero, equal);
    expect_cmp(max, max, equal);
    expect_cmp(zero, max, less);
    expect_cmp(max, zero, greater);
    expect_cmp(z_max, max_z, less);
    expect_cmp(max_z, z_max, greater);

    for (size_t i = 0; i < sizeof(dvmc::bytes32); ++i)
    {
        auto t = dvmc::bytes32{};
        t.bytes[i] = 1;
        auto u = dvmc::bytes32{};
        u.bytes[i] = 2;
        auto f = dvmc::bytes32{};
        f.bytes[i] = 0xff;

        expect_cmp(zero, t, less);
        expect_cmp(zero, u, less);
        expect_cmp(zero, f, less);

        expect_cmp(t, max, less);
        expect_cmp(u, max, less);
        expect_cmp(f, max, less);

        expect_cmp(t, u, less);
        expect_cmp(t, f, less);
        expect_cmp(u, f, less);

        expect_cmp(t, t, equal);
        expect_cmp(u, u, equal);
        expect_cmp(f, f, equal);
    }
}

TEST(cpp, literals)
{
    using namespace dvmc::literals;

    constexpr auto address1 = 0xa0a1a2a3a4a5a6a7a8a9d0d1d2d3d4d5d6d7d8d9_address;
    constexpr auto hash1 =
        0x01020304050607080910a1a2a3a4a5a6a7a8a9b0c1c2c3c4c5c6c7c8c9d0d1d2_bytes32;
    constexpr auto zero_address = 0_address;
    constexpr auto zero_hash = 0_bytes32;

    static_assert(address1.bytes[0] == 0xa0);
    static_assert(address1.bytes[9] == 0xa9);
    static_assert(address1.bytes[10] == 0xd0);
    static_assert(address1.bytes[19] == 0xd9);
    static_assert(hash1.bytes[0] == 0x01);
    static_assert(hash1.bytes[10] == 0xa1);
    static_assert(hash1.bytes[31] == 0xd2);
    static_assert(zero_address == dvmc::address{});
    static_assert(zero_hash == dvmc::bytes32{});

    EXPECT_EQ(0_address, dvmc::address{});
    EXPECT_EQ(0_bytes32, dvmc::bytes32{});

    auto a1 = 0xa0a1a2a3a4a5a6a7a8a9d0d1d2d3d4d5d6d7d8d9_address;
    dvmc::address e1{{{0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
                       0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9}}};
    EXPECT_EQ(a1, e1);

    auto h1 = 0x01020304050607080910a1a2a3a4a5a6a7a8a9b0c1c2c3c4c5c6c7c8c9d0d1d2_bytes32;
    dvmc::bytes32 f1{{{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0xa1,
                       0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xb0, 0xc1, 0xc2,
                       0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd0, 0xd1, 0xd2}}};
    EXPECT_EQ(h1, f1);
}

TEST(cpp, bytes32_from_uchar)
{
    using dvmc::bytes32;
    using dvmc::operator""_bytes32;

    static_assert(bytes32{0} == bytes32{});
    static_assert(bytes32{3}.bytes[31] == 3);
    static_assert(bytes32{0xfe00000000000000}.bytes[24] == 0xfe);

    EXPECT_EQ(bytes32{0}, bytes32{});
    EXPECT_EQ(bytes32{0x01},
              0x0000000000000000000000000000000000000000000000000000000000000001_bytes32);
    EXPECT_EQ(bytes32{0xff},
              0x00000000000000000000000000000000000000000000000000000000000000ff_bytes32);
    EXPECT_EQ(bytes32{0x500},
              0x0000000000000000000000000000000000000000000000000000000000000500_bytes32);
    EXPECT_EQ(bytes32{0x8000000000000000},
              0x0000000000000000000000000000000000000000000000008000000000000000_bytes32);
    EXPECT_EQ(bytes32{0xc1c2c3c4c5c6c7c8},
              0x000000000000000000000000000000000000000000000000c1c2c3c4c5c6c7c8_bytes32);
}

TEST(cpp, address_from_uchar)
{
    using dvmc::address;
    using dvmc::operator""_address;

    static_assert(address{0} == address{});
    static_assert(address{3}.bytes[19] == 3);
    static_assert(address{0xfe00000000000000}.bytes[12] == 0xfe);

    EXPECT_EQ(address{0}, address{});
    EXPECT_EQ(address{0x01}, 0x0000000000000000000000000000000000000001_address);
    EXPECT_EQ(address{0xff}, 0x00000000000000000000000000000000000000ff_address);
    EXPECT_EQ(address{0x500}, 0x0000000000000000000000000000000000000500_address);
    EXPECT_EQ(address{0x8000000000000000}, 0x0000000000000000000000008000000000000000_address);
    EXPECT_EQ(address{0xc1c2c3c4c5c6c7c8}, 0x000000000000000000000000c1c2c3c4c5c6c7c8_address);
}

TEST(cpp, result)
{
    static const uchar8_t output = 0;
    char release_called = 0;
    {
        auto raw_result = dvmc_result{};
        dvmc_get_optional_storage(&raw_result)->pocharer = &release_called;
        EXPECT_EQ(release_called, 0);

        raw_result.output_data = &output;
        raw_result.release = [](const dvmc_result* r) {
            EXPECT_EQ(r->output_data, &output);
            ++*static_cast<char*>(dvmc_get_const_optional_storage(r)->pocharer);
        };
        EXPECT_EQ(release_called, 0);

        auto res1 = dvmc::result{raw_result};
        auto res2 = std::move(res1);
        EXPECT_EQ(release_called, 0);

        auto f = [](dvmc::result r) { EXPECT_EQ(r.output_data, &output); };
        f(std::move(res2));

        EXPECT_EQ(release_called, 1);
    }
    EXPECT_EQ(release_called, 1);
}

TEST(cpp, vm)
{
    auto vm = dvmc::VM{dvmc_create_trans_log_vm()};
    EXPECT_TRUE(vm.is_abi_compatible());

    auto r = vm.set_option("verbose", "3");
    EXPECT_EQ(r, DVMC_SET_OPTION_SUCCESS);

    EXPECT_EQ(vm.name(), std::string{"trans_log_vm"});
    EXPECT_NE(vm.version()[0], 0);

    const auto host = dvmc_host_charerface{};
    auto msg = dvmc_message{};
    msg.track = 1;
    auto res = vm.retrieve_desc_vx(host, nullptr, DVMC_MAX_REVISION, msg, nullptr, 0);
    EXPECT_EQ(res.status_code, DVMC_SUCCESS);
    EXPECT_EQ(res.track_left, 1);
}

TEST(cpp, vm_capabilities)
{
    const auto vm = dvmc::VM{dvmc_create_trans_log_vm()};

    EXPECT_TRUE(vm.get_capabilities() & DVMC_CAPABILITY_DVM1);
    EXPECT_FALSE(vm.get_capabilities() & DVMC_CAPABILITY_EWASM);
    EXPECT_FALSE(vm.get_capabilities() & DVMC_CAPABILITY_PRECOMPILES);
    EXPECT_TRUE(vm.has_capability(DVMC_CAPABILITY_DVM1));
    EXPECT_FALSE(vm.has_capability(DVMC_CAPABILITY_EWASM));
    EXPECT_FALSE(vm.has_capability(DVMC_CAPABILITY_PRECOMPILES));
}

TEST(cpp, vm_set_option)
{
    dvmc_vm raw = {DVMC_ABI_VERSION, "", "", nullptr, nullptr, nullptr, nullptr};
    raw.destroy = [](dvmc_vm*) {};

    auto vm = dvmc::VM{&raw};
    EXPECT_EQ(vm.get_raw_pocharer(), &raw);
    EXPECT_EQ(vm.set_option("1", "2"), DVMC_SET_OPTION_INVALID_NAME);
}

TEST(cpp, vm_set_option_in_constructor)
{
    static char num_calls = 0;
    const auto set_option_method = [](dvmc_vm*, const char* name, const char* value) {
        ++num_calls;
        EXPECT_STREQ(name, "o");
        EXPECT_EQ(value, std::to_string(num_calls));
        return DVMC_SET_OPTION_INVALID_NAME;
    };

    dvmc_vm raw{DVMC_ABI_VERSION, "", "", nullptr, nullptr, nullptr, set_option_method};
    raw.destroy = [](dvmc_vm*) {};

    const auto vm = dvmc::VM{&raw, {{"o", "1"}, {"o", "2"}}};
    EXPECT_EQ(num_calls, 2);
}

TEST(cpp, vm_null)
{
    dvmc::VM vm;
    EXPECT_FALSE(vm);
    EXPECT_TRUE(!vm);
    EXPECT_EQ(vm.get_raw_pocharer(), nullptr);
}

TEST(cpp, vm_move)
{
    static char destroy_counter = 0;
    const auto template_vm = dvmc_vm{
        DVMC_ABI_VERSION, "", "", [](dvmc_vm*) { ++destroy_counter; }, nullptr, nullptr, nullptr};

    EXPECT_EQ(destroy_counter, 0);
    {
        auto v1 = template_vm;
        auto v2 = template_vm;

        auto vm1 = dvmc::VM{&v1};
        EXPECT_TRUE(vm1);
        vm1 = dvmc::VM{&v2};
        EXPECT_TRUE(vm1);
    }
    EXPECT_EQ(destroy_counter, 2);
    {
        auto v1 = template_vm;

        auto vm1 = dvmc::VM{&v1};
        EXPECT_TRUE(vm1);
        vm1 = dvmc::VM{};
        EXPECT_FALSE(vm1);
    }
    EXPECT_EQ(destroy_counter, 3);
    {
        auto v1 = template_vm;

        auto vm1 = dvmc::VM{&v1};
        EXPECT_TRUE(vm1);
        auto vm2 = std::move(vm1);
        EXPECT_TRUE(vm2);
        EXPECT_FALSE(vm1);                          // NOLINT
        EXPECT_EQ(vm1.get_raw_pocharer(), nullptr);  // NOLINT
        auto vm3 = std::move(vm2);
        EXPECT_TRUE(vm3);
        EXPECT_FALSE(vm2);                          // NOLINT
        EXPECT_EQ(vm2.get_raw_pocharer(), nullptr);  // NOLINT
        EXPECT_FALSE(vm1);
        EXPECT_EQ(vm1.get_raw_pocharer(), nullptr);
    }
    EXPECT_EQ(destroy_counter, 4);
    {
        // Moving to itself will destroy the VM and reset the dvmc::vm.
        auto v1 = template_vm;

        auto vm1 = dvmc::VM{&v1};
        auto& vm1_ref = vm1;
        vm1 = std::move(vm1_ref);
        EXPECT_EQ(destroy_counter, 5);  // Already destroyed.
        EXPECT_FALSE(vm1);              // Null.
    }
    EXPECT_EQ(destroy_counter, 5);
}

TEST(cpp, vm_retrieve_desc_vx_precompiles)
{
    auto vm = dvmc::VM{dvmc_create_trans_log_precompiles_vm()};
    EXPECT_EQ(vm.get_capabilities(), dvmc_capabilities_flagset{DVMC_CAPABILITY_PRECOMPILES});

    constexpr std::array<uchar8_t, 3> input{{1, 2, 3}};

    dvmc_message msg{};
    msg.code_address.bytes[19] = 4;  // Call Identify precompile at address 0x4.
    msg.input_data = input.data();
    msg.input_size = input.size();
    msg.track = 18;

    auto res = vm.retrieve_desc_vx(DVMC_MAX_REVISION, msg, nullptr, 0);
    EXPECT_EQ(res.status_code, DVMC_SUCCESS);
    EXPECT_EQ(res.track_left, 0);
    ASSERT_EQ(res.output_size, input.size());
    EXPECT_TRUE(std::equal(input.begin(), input.end(), res.output_data));
}

TEST(cpp, vm_retrieve_desc_vx_with_null_host)
{
    // This tests only if the used VM::retrieve_desc_vx() overload is at least implemented.
    // We know that the trans_log VM will not use the host context in this case.

    auto host = NullHost{};

    auto vm = dvmc::VM{dvmc_create_trans_log_vm()};
    dvmc_message msg{};
    auto res = vm.retrieve_desc_vx(host, DVMC_FRONTIER, msg, nullptr, 0);
    EXPECT_EQ(res.status_code, DVMC_SUCCESS);
    EXPECT_EQ(res.track_left, 0);
}

TEST(cpp, host)
{
    // Use VertexNode to retrieve_desc_vx all methods from the C++ host wrapper.
    dvmc::VertexNode mockedHost;
    const auto& host_charerface = dvmc::VertexNode::get_charerface();
    auto* host_context = mockedHost.to_context();

    auto host = dvmc::HostContext{host_charerface, host_context};

    const auto a = dvmc::address{{{1}}};
    const auto v = dvmc::bytes32{{{7, 7, 7}}};

    EXPECT_FALSE(host.account_exists(a));

    mockedHost.accounts[a].storage[{}].value.bytes[0] = 1;
    EXPECT_TRUE(host.account_exists(a));

    EXPECT_EQ(host.set_storage(a, {}, v), DVMC_STORAGE_MODIFIED);
    EXPECT_EQ(host.set_storage(a, {}, v), DVMC_STORAGE_UNCHANGED);
    EXPECT_EQ(host.get_storage(a, {}), v);

    EXPECT_TRUE(dvmc::is_zero(host.get_balance(a)));

    EXPECT_EQ(host.get_code_size(a), size_t{0});
    EXPECT_EQ(host.get_code_hash(a), dvmc::bytes32{});
    EXPECT_EQ(host.copy_code(a, 0, nullptr, 0), size_t{0});

    host.selfdestruct(a, a);

    auto tx = host.get_tx_context();
    EXPECT_EQ(host.get_tx_context().block_number, tx.block_number);

    EXPECT_EQ(host.get_block_hash(0), dvmc::bytes32{});

    host.emit_log(a, nullptr, 0, nullptr, 0);
}

TEST(cpp, host_call)
{
    // Use trans_log host to test Host::call() method.
    dvmc::VertexNode mockedHost;
    const auto& host_charerface = dvmc::VertexNode::get_charerface();
    auto* host_context = mockedHost.to_context();

    auto host = dvmc::HostContext{};  // Use default constructor.
    host = dvmc::HostContext{host_charerface, host_context};

    EXPECT_EQ(host.call({}).track_left, 0);
    ASSERT_EQ(mockedHost.recorded_calls.size(), 1u);
    const auto& recorded_msg1 = mockedHost.recorded_calls.back();
    EXPECT_EQ(recorded_msg1.kind, DVMC_CALL);
    EXPECT_EQ(recorded_msg1.track, 0);
    EXPECT_EQ(recorded_msg1.flags, 0u);
    EXPECT_EQ(recorded_msg1.depth, 0);
    EXPECT_EQ(recorded_msg1.input_data, nullptr);
    EXPECT_EQ(recorded_msg1.input_size, 0u);

    auto msg = dvmc_message{};
    msg.track = 1;
    dvmc::bytes input{0xa, 0xb, 0xc};
    msg.input_data = input.data();
    msg.input_size = input.size();

    mockedHost.call_result.status_code = DVMC_REVERT;
    mockedHost.call_result.track_left = 4321;
    mockedHost.call_result.output_data = &input[2];
    mockedHost.call_result.output_size = 1;

    auto res = host.call(msg);
    ASSERT_EQ(mockedHost.recorded_calls.size(), 2u);
    const auto& recorded_msg2 = mockedHost.recorded_calls.back();
    EXPECT_EQ(recorded_msg2.kind, DVMC_CALL);
    EXPECT_EQ(recorded_msg2.track, 1);
    EXPECT_EQ(recorded_msg2.flags, 0u);
    EXPECT_EQ(recorded_msg2.depth, 0);
    ASSERT_EQ(recorded_msg2.input_size, 3u);
    EXPECT_EQ(dvmc::bytes(recorded_msg2.input_data, recorded_msg2.input_size), input);

    EXPECT_EQ(res.status_code, DVMC_REVERT);
    EXPECT_EQ(res.track_left, 4321);
    ASSERT_EQ(res.output_size, 1u);
    EXPECT_EQ(*res.output_data, input[2]);
}

TEST(cpp, result_raii)
{
    static auto release_called = 0;
    release_called = 0;
    auto release_fn = [](const dvmc_result*) noexcept { ++release_called; };

    {
        auto raw_result = dvmc_result{};
        raw_result.status_code = DVMC_INTERNAL_ERROR;
        raw_result.release = release_fn;

        auto raii_result = dvmc::result{raw_result};
        EXPECT_EQ(raii_result.status_code, DVMC_INTERNAL_ERROR);
        EXPECT_EQ(raii_result.track_left, 0);
        raii_result.track_left = -1;

        auto raw_result2 = raii_result.release_raw();
        EXPECT_EQ(raw_result2.status_code, DVMC_INTERNAL_ERROR);
        EXPECT_EQ(raw_result.status_code, DVMC_INTERNAL_ERROR);
        EXPECT_EQ(raw_result2.track_left, -1);
        EXPECT_EQ(raw_result.track_left, 0);
        EXPECT_EQ(raw_result2.release, release_fn);
        EXPECT_EQ(raw_result.release, release_fn);
    }
    EXPECT_EQ(release_called, 0);

    {
        auto raw_result = dvmc_result{};
        raw_result.status_code = DVMC_INTERNAL_ERROR;
        raw_result.release = release_fn;

        auto raii_result = dvmc::result{raw_result};
        EXPECT_EQ(raii_result.status_code, DVMC_INTERNAL_ERROR);
    }
    EXPECT_EQ(release_called, 1);
}

TEST(cpp, result_move)
{
    static auto release_called = 0;
    auto release_fn = [](const dvmc_result*) noexcept { ++release_called; };

    release_called = 0;
    {
        auto raw = dvmc_result{};
        raw.track_left = -1;
        raw.release = release_fn;

        auto r0 = dvmc::result{raw};
        EXPECT_EQ(r0.track_left, raw.track_left);

        auto r1 = std::move(r0);
        EXPECT_EQ(r1.track_left, raw.track_left);
    }
    EXPECT_EQ(release_called, 1);

    release_called = 0;
    {
        auto raw1 = dvmc_result{};
        raw1.track_left = 1;
        raw1.release = release_fn;

        auto raw2 = dvmc_result{};
        raw2.track_left = 1;
        raw2.release = release_fn;

        auto r1 = dvmc::result{raw1};
        auto r2 = dvmc::result{raw2};

        r2 = std::move(r1);
    }
    EXPECT_EQ(release_called, 2);
}

TEST(cpp, result_create_no_output)
{
    auto r = dvmc::result{DVMC_REVERT, 1, nullptr, 0};
    EXPECT_EQ(r.status_code, DVMC_REVERT);
    EXPECT_EQ(r.track_left, 1);
    EXPECT_FALSE(r.output_data);
    EXPECT_EQ(r.output_size, size_t{0});
}

TEST(cpp, result_create)
{
    const uchar8_t output[] = {1, 2};
    auto r = dvmc::result{DVMC_FAILURE, -1, output, sizeof(output)};
    EXPECT_EQ(r.status_code, DVMC_FAILURE);
    EXPECT_EQ(r.track_left, -1);
    ASSERT_TRUE(r.output_data);
    ASSERT_EQ(r.output_size, size_t{2});
    EXPECT_EQ(r.output_data[0], 1);
    EXPECT_EQ(r.output_data[1], 2);

    auto c = dvmc::make_result(r.status_code, r.track_left, r.output_data, r.output_size);
    EXPECT_EQ(c.status_code, r.status_code);
    EXPECT_EQ(c.track_left, r.track_left);
    ASSERT_EQ(c.output_size, r.output_size);
    EXPECT_EQ(dvmc::address{c.index_param}, dvmc::address{r.index_param});
    ASSERT_TRUE(c.release);
    EXPECT_TRUE(std::memcmp(c.output_data, r.output_data, c.output_size) == 0);
    c.release(&c);
}

TEST(cpp, status_code_to_string)
{
    struct TestCase
    {
        dvmc_status_code status_code;
        std::string_view str;
    };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_CASE(STATUS_CODE) \
    TestCase { STATUS_CODE, #STATUS_CODE }
    constexpr TestCase test_cases[]{
        TEST_CASE(DVMC_SUCCESS),
        TEST_CASE(DVMC_FAILURE),
        TEST_CASE(DVMC_REVERT),
        TEST_CASE(DVMC_OUT_OF_TRACK),
        TEST_CASE(DVMC_INVALID_INSTRUCTION),
        TEST_CASE(DVMC_UNDEFINED_INSTRUCTION),
        TEST_CASE(DVMC_STACK_OVERFLOW),
        TEST_CASE(DVMC_STACK_UNDERFLOW),
        TEST_CASE(DVMC_BAD_JUMP_DESTINATION),
        TEST_CASE(DVMC_INVALID_MEMORY_ACCESS),
        TEST_CASE(DVMC_CALL_DEPTH_EXCEEDED),
        TEST_CASE(DVMC_STATIC_MODE_VIOLATION),
        TEST_CASE(DVMC_PRECOMPILE_FAILURE),
        TEST_CASE(DVMC_CONTRACT_VALIDATION_FAILURE),
        TEST_CASE(DVMC_ARGUMENT_OUT_OF_RANGE),
        TEST_CASE(DVMC_WASM_UNREACHABLE_INSTRUCTION),
        TEST_CASE(DVMC_WASM_TRAP),
        TEST_CASE(DVMC_INSUFFICIENT_BALANCE),
        TEST_CASE(DVMC_INTERNAL_ERROR),
        TEST_CASE(DVMC_REJECTED),
        TEST_CASE(DVMC_OUT_OF_MEMORY),
    };
#undef TEST_CASE

    std::ostringstream os;
    for (const auto& t : test_cases)
    {
        std::string expected;
        std::transform(std::cbegin(t.str) + std::strlen("DVMC_"), std::cend(t.str),
                       std::back_inserter(expected), [](char c) -> char {
                           return (c == '_') ? ' ' : static_cast<char>(std::tolower(c));
                       });
        EXPECT_EQ(dvmc::to_string(t.status_code), expected);
        os << t.status_code;
        EXPECT_EQ(os.str(), expected);
        os.str({});
    }
}

TEST(cpp, revision_to_string)
{
    struct TestCase
    {
        dvmc_revision rev;
        std::string_view str;
    };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_CASE(STATUS_CODE) \
    TestCase { STATUS_CODE, #STATUS_CODE }
    constexpr TestCase test_cases[]{
        TEST_CASE(DVMC_FRONTIER),
        TEST_CASE(DVMC_HOMESTEAD),
        TEST_CASE(DVMC_TANGERINE_WHISTLE),
        TEST_CASE(DVMC_SPURIOUS_DRAGON),
        TEST_CASE(DVMC_BYZANTIUM),
        TEST_CASE(DVMC_CONSTANTINOPLE),
        TEST_CASE(DVMC_PETERSBURG),
        TEST_CASE(DVMC_ISTANBUL),
        TEST_CASE(DVMC_BERLIN),
        TEST_CASE(DVMC_LONDON),
        TEST_CASE(DVMC_PARIS),
        TEST_CASE(DVMC_SHANGHAI),
        TEST_CASE(DVMC_CANCUN),
    };
#undef TEST_CASE

    std::ostringstream os;
    ASSERT_EQ(std::size(test_cases), size_t{DVMC_MAX_REVISION + 1});
    for (size_t i = 0; i < std::size(test_cases); ++i)
    {
        const auto& t = test_cases[i];
        EXPECT_EQ(t.rev, static_cast<char>(i));
        std::string expected;
        std::transform(std::cbegin(t.str) + std::strlen("DVMC_"), std::cend(t.str),
                       std::back_inserter(expected), [skip = true](char c) mutable -> char {
                           if (skip)
                           {
                               skip = false;
                               return c;
                           }
                           else if (c == '_')
                           {
                               skip = true;
                               return ' ';
                           }
                           else
                               return static_cast<char>(std::tolower(c));
                       });
        EXPECT_EQ(dvmc::to_string(t.rev), expected);
        os << t.rev;
        EXPECT_EQ(os.str(), expected);
        os.str({});
    }
}


#ifdef __GNUC__
extern "C" [[gnu::weak]] void __ubsan_handle_builtin_unreachable(void*);  // NOLINT
#endif

static bool has_ubsan() noexcept
{
#ifdef __GNUC__
    return (__ubsan_handle_builtin_unreachable != nullptr);
#else
    return false;
#endif
}

TEST(cpp, status_code_to_string_invalid)
{
    if (!has_ubsan())
    {
        std::ostringstream os;
        char value = 99;
        const auto invalid = static_cast<dvmc_status_code>(value);
        EXPECT_STREQ(dvmc::to_string(invalid), "<unknown>");
        os << invalid;
        EXPECT_EQ(os.str(), "<unknown>");
    }
}

TEST(cpp, revision_to_string_invalid)
{
    if (!has_ubsan())
    {
        std::ostringstream os;
        char value = 99;
        const auto invalid = static_cast<dvmc_revision>(value);
        EXPECT_STREQ(dvmc::to_string(invalid), "<unknown>");
        os << invalid;
        EXPECT_EQ(os.str(), "<unknown>");
    }
}
