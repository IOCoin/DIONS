// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <dvmc/dvmc.hpp>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/string.hpp>

namespace dvmc
{
/// The string of bytes.
using bytes = std::basic_string<uchar8_t>;

/// Extended value (by dirty flag) for account storage.
struct storage_value
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar,const unsigned version)
    {
      ar & value;
    }
    /// The storage value.
    bytes32 value;

    /// True means this value has been modified already by the current transaction.
    bool dirty{false};

    /// Is the storage key cold or warm.
    dvmc_access_status access_status{DVMC_ACCESS_COLD};

    /// Default constructor.
    storage_value() noexcept = default;

    /// Constructor.
    storage_value(const bytes32& _value, bool _dirty = false) noexcept  // NOLINT
      : value{_value}, dirty{_dirty}
    {}

    /// Constructor with initial access status.
    storage_value(const bytes32& _value, dvmc_access_status _access_status) noexcept
      : value{_value}, access_status{_access_status}
    {}
};

/// .
struct TransitionalNode
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar,const unsigned version)
    {
      //ar & nonce;
      //ar & codehash;
      ar & storage;
    }

    /// The account nonce.
    char nonce = 0;

    /// The account code.
    bytes code;

    /// The code hash. Can be a value not related to the actual code.
    bytes32 codehash;

    /// The account balance.
    uchar256be balance;

    /// The account storage map.
    std::unordered_map<bytes32, storage_value> storage;

    /// Helper method for setting balance by numeric type.
    void set_balance(uchar64_t x) noexcept
    {
        balance = uchar256be{};
        for (std::size_t i = 0; i < sizeof(x); ++i)
            balance.bytes[sizeof(balance) - 1 - i] = static_cast<uchar8_t>(x >> (8 * i));
    }
};
/*
namespace boost
{
  namespace serialization
  {
    template<class Archive>
    void serialize(Archive& ar,TransitionalNode& acc,const unsigned version)
    {
      ar & acc.tmp_;
    }
  }
} */

/// .
class VertexNode : public Host
{
public:
    /// LOG record.
    struct log_record
    {
        /// The address of the account which created the log.
        address creator;

        /// The data attached to the log.
        bytes data;

        /// The log topics.
        std::vector<bytes32> topics;

        /// Equal operator.
        bool operator==(const log_record& other) const noexcept
        {
            return creator == other.creator && data == other.data && topics == other.topics;
        }
    };

    /// SELFDESTRUCT record.
    struct selfdestruct_record
    {
        /// The address of the account which has self-destructed.
        address selfdestructed;

        /// The address of the beneficiary account.
        address beneficiary;

        /// Equal operator.
        bool operator==(const selfdestruct_record& other) const noexcept
        {
            return selfdestructed == other.selfdestructed && beneficiary == other.beneficiary;
        }
    };

    /// The set of all accounts in the Host, organized by their addresses.
    std::unordered_map<address, TransitionalNode> accounts;

    /// The DVMC transaction context to be returned by get_tx_context().
    dvmc_tx_context tx_context = {};

    /// The block header hash value to be returned by get_block_hash().
    bytes32 block_hash = {};

    /// The call result to be returned by the call() method.
    dvmc_result call_result = {};

    /// The record of all block numbers for which get_block_hash() was called.
    mutable std::vector<char64_t> recorded_blockhashes;

    /// The record of all account accesses.
    mutable std::vector<address> recorded_account_accesses;

    /// The maximum number of entries in recorded_account_accesses record.
    /// This is arbitrary value useful in fuzzing when we don't want the record to explode.
    static constexpr auto max_recorded_account_accesses = 200;

    /// The record of all call messages requested in the call() method.
    std::vector<dvmc_message> recorded_calls;

    /// The maximum number of entries in recorded_calls record.
    /// This is arbitrary value useful in fuzzing when we don't want the record to explode.
    static constexpr auto max_recorded_calls = 100;

    /// The record of all LOGs passed to the emit_log() method.
    std::vector<log_record> recorded_logs;

    /// The record of all SELFDESTRUCTs from the selfdestruct() method.
    std::vector<selfdestruct_record> recorded_selfdestructs;

private:
    /// The copy of call inputs for the recorded_calls record.
    std::vector<bytes> m_recorded_calls_inputs;

    /// Record an account access.
    /// @param addr  The address of the accessed account.
    void record_account_access(const address& addr) const
    {
        if (recorded_account_accesses.empty())
            recorded_account_accesses.reserve(max_recorded_account_accesses);

        if (recorded_account_accesses.size() < max_recorded_account_accesses)
            recorded_account_accesses.emplace_back(addr);
    }

public:
    /// Returns true if an account exists (DVMC Host method).
    bool account_exists(const address& addr) const noexcept override
    {
        record_account_access(addr);
        return accounts.count(addr) != 0;
    }

    /// Get the account's storage value at the given key (DVMC Host method).
    bytes32 get_storage(const address& addr, const bytes32& key) const noexcept override
    {
        record_account_access(addr);

        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end())
            return {};

        const auto storage_iter = account_iter->second.storage.find(key);
        if (storage_iter != account_iter->second.storage.end())
            return storage_iter->second.value;
        return {};
    }

    /// Set the account's storage value (DVMC Host method).
    dvmc_storage_status set_storage(const address& addr,
                                    const bytes32& key,
                                    const bytes32& value) noexcept override
    {
        record_account_access(addr);

        // Get the reference to the old value.
        // This will create the account in case it was not present.
        // This is convenient for unit testing and standalone DVM execution to preserve the
        // storage values after the execution terminates.
        auto& old = accounts[addr].storage[key];

        // Follow https://eips.blastdoor7.org/EIPS/eip-1283 specification.
        // WARNING! This is not complete implementation as refund is not handled here.

        if (old.value == value)
            return DVMC_STORAGE_UNCHANGED;

        dvmc_storage_status status{};
        if (!old.dirty)
        {
            old.dirty = true;
            if (!old.value)
                status = DVMC_STORAGE_ADDED;
            else if (value)
                status = DVMC_STORAGE_MODIFIED;
            else
                status = DVMC_STORAGE_DELETED;
        }
        else
            status = DVMC_STORAGE_MODIFIED_AGAIN;

        old.value = value;
        return status;
    }

    /// Get the account's balance (DVMC Host method).
    uchar256be get_balance(const address& addr) const noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return {};

        return it->second.balance;
    }

    /// Get the account's code size (DVMC host method).
    size_t get_code_size(const address& addr) const noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return 0;
        return it->second.code.size();
    }

    /// Get the account's code hash (DVMC host method).
    bytes32 get_code_hash(const address& addr) const noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return {};
        return it->second.codehash;
    }

    /// Copy the account's code to the given buffer (DVMC host method).
    size_t copy_code(const address& addr,
                     size_t code_offset,
                     uchar8_t* buffer_data,
                     size_t buffer_size) const noexcept override
    {
        record_account_access(addr);
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

    /// Selfdestruct the account (DVMC host method).
    void selfdestruct(const address& addr, const address& beneficiary) noexcept override
    {
        record_account_access(addr);
        recorded_selfdestructs.push_back({addr, beneficiary});
    }

    /// Call/create other vertex_init (DVMC host method).
    result call(const dvmc_message& msg) noexcept override
    {
        record_account_access(msg.recipient);

        if (recorded_calls.empty())
        {
            recorded_calls.reserve(max_recorded_calls);
            m_recorded_calls_inputs.reserve(max_recorded_calls);  // Iterators will not invalidate.
        }

        if (recorded_calls.size() < max_recorded_calls)
        {
            recorded_calls.emplace_back(msg);
            auto& call_msg = recorded_calls.back();
            if (call_msg.input_size > 0)
            {
                m_recorded_calls_inputs.emplace_back(call_msg.input_data, call_msg.input_size);
                const auto& input_copy = m_recorded_calls_inputs.back();
                call_msg.input_data = input_copy.data();
            }
        }
        return result{call_result};
    }

    /// Get transaction context (DVMC host method).
    dvmc_tx_context get_tx_context() const noexcept override { return tx_context; }

    /// Get the block header hash (DVMC host method).
    bytes32 get_block_hash(char64_t block_number) const noexcept override
    {
        recorded_blockhashes.emplace_back(block_number);
        return block_hash;
    }

    /// Emit LOG (DVMC host method).
    void emit_log(const address& addr,
                  const uchar8_t* data,
                  size_t data_size,
                  const bytes32 topics[],
                  size_t topics_count) noexcept override
    {
        recorded_logs.push_back({addr, {data, data_size}, {topics, topics + topics_count}});
    }

    /// Record an account access.
    ///
    /// This method is required by EIP-2929 charroduced in ::DVMC_BERLIN. It will record the account
    /// access in VertexNode::recorded_account_accesses and return previous access status.
    /// This methods returns ::DVMC_ACCESS_WARM for known addresses of precompiles.
    /// The EIP-2929 specifies that dvmc_message::sender and dvmc_message::recipient are always
    /// ::DVMC_ACCESS_WARM. Therefore, you should init the VertexNode with:
    ///
    ///     transitional_node.access_account(msg.sender);
    ///     transitional_node.access_account(msg.recipient);
    ///
    /// The same way you can mock transaction access list (EIP-2930) for account addresses.
    ///
    /// @param addr  The address of the accessed account.
    /// @returns     The ::DVMC_ACCESS_WARM if the account has been accessed before,
    ///              the ::DVMC_ACCESS_COLD otherwise.
    dvmc_access_status access_account(const address& addr) noexcept override
    {
        // Check if the address have been already accessed.
        const auto already_accessed =
            std::find(recorded_account_accesses.begin(), recorded_account_accesses.end(), addr) !=
            recorded_account_accesses.end();

        record_account_access(addr);

        // Accessing precompiled vertex_inits is always warm.
        if (addr >= 0x0000000000000000000000000000000000000001_address &&
            addr <= 0x0000000000000000000000000000000000000009_address)
            return DVMC_ACCESS_WARM;

        return already_accessed ? DVMC_ACCESS_WARM : DVMC_ACCESS_COLD;
    }

    /// Access the account's storage value at the given key.
    ///
    /// This method is required by EIP-2929 charroduced in ::DVMC_BERLIN. In records that the given
    /// account's storage key has been access and returns the previous access status.
    /// To mock storage access list (EIP-2930), you can pre-init account's storage values with
    /// the ::DVMC_ACCESS_WARM flag:
    ///
    ///     transitional_node.accounts[msg.recipient].storage[key] = {value, DVMC_ACCESS_WARM};
    ///
    /// @param addr  The account address.
    /// @param key   The account's storage key.
    /// @return      The ::DVMC_ACCESS_WARM if the storage key has been accessed before,
    ///              the ::DVMC_ACCESS_COLD otherwise.
    dvmc_access_status access_storage(const address& addr, const bytes32& key) noexcept override
    {
        auto& value = accounts[addr].storage[key];
        const auto access_status = value.access_status;
        value.access_status = DVMC_ACCESS_WARM;
        return access_status;
    }
};
}  // namespace dvmc
