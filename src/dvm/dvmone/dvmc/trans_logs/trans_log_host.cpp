// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

/// @file
/// Example implementation of an DVMC Host.

#include "trans_log_host.h"

#include <dvmc/dvmc.hpp>

#include <algorithm>
#include <map>
#include <vector>

using namespace dvmc::literals;

namespace dvmc
{
struct account
{
    virtual ~account() = default;

    dvmc::uchar256be balance = {};
    std::vector<uchar8_t> code;
    std::map<dvmc::bytes32, dvmc::bytes32> storage;

    virtual dvmc::bytes32 code_hash() const
    {
        // Extremely dumb "hash" function.
        dvmc::bytes32 ret{};
        for (const auto v : code)
            ret.bytes[v % sizeof(ret.bytes)] ^= v;
        return ret;
    }
};

using accounts = std::map<dvmc::address, account>;

}  // namespace dvmc

class ExampleHost : public dvmc::Host
{
    dvmc::accounts accounts;
    dvmc_tx_context tx_context{};

public:
    ExampleHost() = default;
    explicit ExampleHost(dvmc_tx_context& _tx_context) noexcept : tx_context{_tx_context} {}
    ExampleHost(dvmc_tx_context& _tx_context, dvmc::accounts& _accounts) noexcept
      : accounts{_accounts}, tx_context{_tx_context}
    {}

    bool account_exists(const dvmc::address& addr) const noexcept final
    {
        return accounts.find(addr) != accounts.end();
    }

    dvmc::bytes32 get_storage(const dvmc::address& addr,
                              const dvmc::bytes32& key) const noexcept final
    {
        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end())
            return {};

        const auto storage_iter = account_iter->second.storage.find(key);
        if (storage_iter != account_iter->second.storage.end())
            return storage_iter->second;
        return {};
    }

    dvmc_storage_status set_storage(const dvmc::address& addr,
                                    const dvmc::bytes32& key,
                                    const dvmc::bytes32& value) noexcept final
    {
        auto& account = accounts[addr];
        auto prev_value = account.storage[key];
        account.storage[key] = value;

        return (prev_value == value) ? DVMC_STORAGE_UNCHANGED : DVMC_STORAGE_MODIFIED;
    }

    dvmc::uchar256be get_balance(const dvmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.balance;
        return {};
    }

    size_t get_code_size(const dvmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.code.size();
        return 0;
    }

    dvmc::bytes32 get_code_hash(const dvmc::address& addr) const noexcept final
    {
        auto it = accounts.find(addr);
        if (it != accounts.end())
            return it->second.code_hash();
        return {};
    }

    size_t copy_code(const dvmc::address& addr,
                     size_t code_offset,
                     uchar8_t* buffer_data,
                     size_t buffer_size) const noexcept final
    {
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return 0;

        const auto& code = it->second.code;

        if (code_offset >= code.size())
            return 0;

        const auto n = std::min(buffer_size, code.size() - code_offset);

        if (n > 0)
            std::copy_n(&code[code_offset], n, buffer_data);
        return n;
    }

    void selfdestruct(const dvmc::address& addr, const dvmc::address& beneficiary) noexcept final
    {
        (void)addr;
        (void)beneficiary;
    }

    dvmc::result call(const dvmc_message& msg) noexcept final
    {
        return {DVMC_REVERT, msg.track, msg.input_data, msg.input_size};
    }

    dvmc_tx_context get_tx_context() const noexcept final { return tx_context; }

    dvmc::bytes32 get_block_hash(char64_t number) const noexcept final
    {
        const char64_t current_block_number = get_tx_context().block_number;

        return (number < current_block_number && number >= current_block_number - 256) ?
                   0xb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5fb10c8a5f_bytes32 :
                   0_bytes32;
    }

    void emit_log(const dvmc::address& addr,
                  const uchar8_t* data,
                  size_t data_size,
                  const dvmc::bytes32 topics[],
                  size_t topics_count) noexcept final
    {
        (void)addr;
        (void)data;
        (void)data_size;
        (void)topics;
        (void)topics_count;
    }

    dvmc_access_status access_account(const dvmc::address& addr) noexcept final
    {
        (void)addr;
        return DVMC_ACCESS_COLD;
    }

    dvmc_access_status access_storage(const dvmc::address& addr,
                                      const dvmc::bytes32& key) noexcept final
    {
        (void)addr;
        (void)key;
        return DVMC_ACCESS_COLD;
    }
};


extern "C" {

const dvmc_host_charerface* trans_log_host_get_charerface()
{
    return &dvmc::Host::get_charerface();
}

dvmc_host_context* trans_log_host_create_context(dvmc_tx_context tx_context)
{
    auto host = new ExampleHost{tx_context};
    return host->to_context();
}

void trans_log_host_destroy_context(dvmc_host_context* context)
{
    delete dvmc::Host::from_context<ExampleHost>(context);
}
}
