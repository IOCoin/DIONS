// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <dvmc/dvmc.h>
#include <dvmc/helpers.h>

#include <functional>
#include <initializer_list>
#include <ostream>
#include <utility>

//#include <boost/archive/text_iarchive.hpp>
//#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>

static_assert(DVMC_LATEST_STABLE_REVISION <= DVMC_MAX_REVISION,
              "latest stable revision ill-defined");

/// DVMC C++ API - wrappers and bindings for C++
/// @ingroup cpp
namespace dvmc
{
/// The big-endian 160-bit hash suitable for keeping an DVM address.
///
/// This type wraps C ::dvmc_address to make sure objects of this type are always initialized.
struct address : dvmc_address
{
    /// Default and converting constructor.
    ///
    /// Initializes bytes to zeros if not other @p init value provided.
    constexpr address(dvmc_address init = {}) noexcept : dvmc_address {
        init
    } {}

    /// Converting constructor from unsigned integer value.
    ///
    /// This constructor assigns the @p v value to the last 8 bytes [12:19]
    /// in big-endian order.
    constexpr explicit address(uint64_t v) noexcept
        : dvmc_address {
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            static_cast<uint8_t>(v >> 56),
            static_cast<uint8_t>(v >> 48),
            static_cast<uint8_t>(v >> 40),
            static_cast<uint8_t>(v >> 32),
            static_cast<uint8_t>(v >> 24),
            static_cast<uint8_t>(v >> 16),
            static_cast<uint8_t>(v >> 8),
            static_cast<uint8_t>(v >> 0)
        }
    }
    {}

    /// Explicit operator converting to bool.
    inline constexpr explicit operator bool() const noexcept;
};

/// The fixed size array of 32 bytes for storing 256-bit DVM values.
///
/// This type wraps C ::dvmc_bytes32 to make sure objects of this type are always initialized.
struct bytes32 : dvmc_bytes32
{
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar,const unsigned version)
    {
      ar & this->bytes;
    }

    /// Default and converting constructor.
    ///
    /// Initializes bytes to zeros if not other @p init value provided.
    constexpr bytes32(dvmc_bytes32 init = {}) noexcept : dvmc_bytes32 {
        init
    } {}

    /// Converting constructor from unsigned integer value.
    ///
    /// This constructor assigns the @p v value to the last 8 bytes [24:31]
    /// in big-endian order.
    constexpr explicit bytes32(uint64_t v) noexcept
        : dvmc_bytes32 {
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            static_cast<uint8_t>(v >> 56),
            static_cast<uint8_t>(v >> 48),
            static_cast<uint8_t>(v >> 40),
            static_cast<uint8_t>(v >> 32),
            static_cast<uint8_t>(v >> 24),
            static_cast<uint8_t>(v >> 16),
            static_cast<uint8_t>(v >> 8),
            static_cast<uint8_t>(v >> 0)
        }
    }
    {}

    /// Explicit operator converting to bool.
    constexpr inline explicit operator bool() const noexcept;
};
/*
namespace boost
{
  namespace serialization
  {
    template<class Archive>
    void serialize(Archive& ar,bytes32& b,const unsigned version)
    {
      ar & b.tmp_;
    }
  }
} */

/// The alias for dvmc::bytes32 to represent a big-endian 256-bit integer.
using uint256be = bytes32;


/// Loads 64 bits / 8 bytes of data from the given @p data array in big-endian order.
inline constexpr uint64_t load64be(const uint8_t* data) noexcept
{
    return (uint64_t{data[0]} << 56) | (uint64_t{data[1]} << 48) | (uint64_t{data[2]} << 40) |
           (uint64_t{data[3]} << 32) | (uint64_t{data[4]} << 24) | (uint64_t{data[5]} << 16) |
           (uint64_t{data[6]} << 8) | uint64_t{data[7]};
}

/// Loads 64 bits / 8 bytes of data from the given @p data array in little-endian order.
inline constexpr uint64_t load64le(const uint8_t* data) noexcept
{
    return uint64_t{data[0]} | (uint64_t{data[1]} << 8) | (uint64_t{data[2]} << 16) |
           (uint64_t{data[3]} << 24) | (uint64_t{data[4]} << 32) | (uint64_t{data[5]} << 40) |
           (uint64_t{data[6]} << 48) | (uint64_t{data[7]} << 56);
}

/// Loads 32 bits / 4 bytes of data from the given @p data array in big-endian order.
inline constexpr uint32_t load32be(const uint8_t* data) noexcept
{
    return (uint32_t{data[0]} << 24) | (uint32_t{data[1]} << 16) | (uint32_t{data[2]} << 8) |
           uint32_t{data[3]};
}

/// Loads 32 bits / 4 bytes of data from the given @p data array in little-endian order.
inline constexpr uint32_t load32le(const uint8_t* data) noexcept
{
    return uint32_t{data[0]} | (uint32_t{data[1]} << 8) | (uint32_t{data[2]} << 16) |
           (uint32_t{data[3]} << 24);
}

namespace fnv
{
constexpr auto prime = 0x100000001b3;              ///< The 64-bit FNV prime number.
constexpr auto offset_basis = 0xcbf29ce484222325;  ///< The 64-bit FNV offset basis.

/// The hashing transformation for 64-bit inputs based on the FNV-1a formula.
inline constexpr uint64_t fnv1a_by64(uint64_t h, uint64_t x) noexcept
{
    return (h ^ x) * prime;
}
}  // namespace fnv


/// The "equal to" comparison operator for the dvmc::address type.
inline constexpr bool operator==(const address& a, const address& b) noexcept
{
    return load64le(&a.bytes[0]) == load64le(&b.bytes[0]) &&
           load64le(&a.bytes[8]) == load64le(&b.bytes[8]) &&
           load32le(&a.bytes[16]) == load32le(&b.bytes[16]);
}

/// The "not equal to" comparison operator for the dvmc::address type.
inline constexpr bool operator!=(const address& a, const address& b) noexcept
{
    return !(a == b);
}

/// The "less than" comparison operator for the dvmc::address type.
inline constexpr bool operator<(const address& a, const address& b) noexcept
{
    return load64be(&a.bytes[0]) < load64be(&b.bytes[0]) ||
           (load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
            (load64be(&a.bytes[8]) < load64be(&b.bytes[8]) ||
             (load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
              load32be(&a.bytes[16]) < load32be(&b.bytes[16]))));
}

/// The "greater than" comparison operator for the dvmc::address type.
inline constexpr bool operator>(const address& a, const address& b) noexcept
{
    return b < a;
}

/// The "less than or equal to" comparison operator for the dvmc::address type.
inline constexpr bool operator<=(const address& a, const address& b) noexcept
{
    return !(b < a);
}

/// The "greater than or equal to" comparison operator for the dvmc::address type.
inline constexpr bool operator>=(const address& a, const address& b) noexcept
{
    return !(a < b);
}

/// The "equal to" comparison operator for the dvmc::bytes32 type.
inline constexpr bool operator==(const bytes32& a, const bytes32& b) noexcept
{
    return load64le(&a.bytes[0]) == load64le(&b.bytes[0]) &&
           load64le(&a.bytes[8]) == load64le(&b.bytes[8]) &&
           load64le(&a.bytes[16]) == load64le(&b.bytes[16]) &&
           load64le(&a.bytes[24]) == load64le(&b.bytes[24]);
}

/// The "not equal to" comparison operator for the dvmc::bytes32 type.
inline constexpr bool operator!=(const bytes32& a, const bytes32& b) noexcept
{
    return !(a == b);
}

/// The "less than" comparison operator for the dvmc::bytes32 type.
inline constexpr bool operator<(const bytes32& a, const bytes32& b) noexcept
{
    return load64be(&a.bytes[0]) < load64be(&b.bytes[0]) ||
           (load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
            (load64be(&a.bytes[8]) < load64be(&b.bytes[8]) ||
             (load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
              (load64be(&a.bytes[16]) < load64be(&b.bytes[16]) ||
               (load64be(&a.bytes[16]) == load64be(&b.bytes[16]) &&
                load64be(&a.bytes[24]) < load64be(&b.bytes[24]))))));
}

/// The "greater than" comparison operator for the dvmc::bytes32 type.
inline constexpr bool operator>(const bytes32& a, const bytes32& b) noexcept
{
    return b < a;
}

/// The "less than or equal to" comparison operator for the dvmc::bytes32 type.
inline constexpr bool operator<=(const bytes32& a, const bytes32& b) noexcept
{
    return !(b < a);
}

/// The "greater than or equal to" comparison operator for the dvmc::bytes32 type.
inline constexpr bool operator>=(const bytes32& a, const bytes32& b) noexcept
{
    return !(a < b);
}

/// Checks if the given address is the zero address.
inline constexpr bool is_zero(const address& a) noexcept
{
    return a == address{};
}

inline constexpr address::operator bool() const noexcept
{
    return !is_zero(*this);
}

/// Checks if the given bytes32 object has all zero bytes.
inline constexpr bool is_zero(const bytes32& a) noexcept
{
    return a == bytes32{};
}

inline constexpr bytes32::operator bool() const noexcept
{
    return !is_zero(*this);
}

namespace literals
{
namespace internal
{
constexpr int from_hex(char c) noexcept
{
    return (c >= 'a' && c <= 'f') ? c - ('a' - 10) :
           (c >= 'A' && c <= 'F') ? c - ('A' - 10) :
           c - '0';
}

constexpr uint8_t byte(const char* s, size_t i) noexcept
{
    return static_cast<uint8_t>((from_hex(s[2 * i]) << 4) | from_hex(s[2 * i + 1]));
}

template <typename T>
T from_hex(const char*) noexcept;

template <>
constexpr bytes32 from_hex<bytes32>(const char* s) noexcept
{
    return {
        {   {   byte(s, 0),  byte(s, 1),  byte(s, 2),  byte(s, 3),  byte(s, 4),  byte(s, 5),  byte(s, 6),
                byte(s, 7),  byte(s, 8),  byte(s, 9),  byte(s, 10), byte(s, 11), byte(s, 12), byte(s, 13),
                byte(s, 14), byte(s, 15), byte(s, 16), byte(s, 17), byte(s, 18), byte(s, 19), byte(s, 20),
                byte(s, 21), byte(s, 22), byte(s, 23), byte(s, 24), byte(s, 25), byte(s, 26), byte(s, 27),
                byte(s, 28), byte(s, 29), byte(s, 30), byte(s, 31)
            }
        }};
}

template <>
constexpr address from_hex<address>(const char* s) noexcept
{
    return {
        {   {   byte(s, 0),  byte(s, 1),  byte(s, 2),  byte(s, 3),  byte(s, 4),  byte(s, 5),  byte(s, 6),
                byte(s, 7),  byte(s, 8),  byte(s, 9),  byte(s, 10), byte(s, 11), byte(s, 12), byte(s, 13),
                byte(s, 14), byte(s, 15), byte(s, 16), byte(s, 17), byte(s, 18), byte(s, 19)
            }
        }};
}

template <typename T, char... c>
constexpr T from_literal() noexcept
{
    constexpr auto size = sizeof...(c);
    constexpr char literal[] = {c...};
    constexpr bool is_simple_zero = size == 1 && literal[0] == '0';

    static_assert(is_simple_zero || (literal[0] == '0' && literal[1] == 'x'),
                  "literal must be in hexadecimal notation");
    static_assert(is_simple_zero || size == 2 * sizeof(T) + 2,
                  "literal must match the result type size");

    return is_simple_zero ? T{} :
           from_hex<T>(&literal[2]);
}
}  // namespace internal

/// Literal for dvmc::address.
template <char... c>
constexpr address operator""_address() noexcept
{
    return internal::from_literal<address, c...>();
}

/// Literal for dvmc::bytes32.
template <char... c>
constexpr bytes32 operator""_bytes32() noexcept
{
    return internal::from_literal<bytes32, c...>();
}
}  // namespace literals

using namespace literals;


/// @copydoc dvmc_status_code_to_string
inline const char* to_string(dvmc_status_code status_code) noexcept
{
    return dvmc_status_code_to_string(status_code);
}

/// @copydoc dvmc_revision_to_string
inline const char* to_string(dvmc_revision rev) noexcept
{
    return dvmc_revision_to_string(rev);
}


/// Alias for dvmc_make_result().
constexpr auto make_result = dvmc_make_result;

/// @copydoc dvmc_result
///
/// This is a RAII wrapper for dvmc_result and objects of this type
/// automatically release attached resources.
class result : private dvmc_result
{
public:
    using dvmc_result::create_address;
    using dvmc_result::track_left;
    using dvmc_result::output_data;
    using dvmc_result::output_size;
    using dvmc_result::status_code;

    /// Creates the result from the provided arguments.
    ///
    /// The provided output is copied to memory allocated with malloc()
    /// and the dvmc_result::release function is set to one invoking free().
    ///
    /// @param _status_code  The status code.
    /// @param _track_left     The amount of track left.
    /// @param _output_data  The pointer to the output.
    /// @param _output_size  The output size.
    result(dvmc_status_code _status_code,
           int64_t _track_left,
           const uint8_t* _output_data,
           size_t _output_size) noexcept
        : dvmc_result {
        make_result(_status_code, _track_left, _output_data, _output_size)
    }
    {}

    /// Converting constructor from raw dvmc_result.
    explicit result(dvmc_result const& res) noexcept : dvmc_result {
        res
    } {}

    /// Destructor responsible for automatically releasing attached resources.
    ~result() noexcept
    {
        if (release != nullptr)
            release(this);
    }

    /// Move constructor.
    result(result&& other) noexcept : dvmc_result {
        other
    }
    {
        other.release = nullptr;  // Disable releasing of the rvalue object.
    }

    /// Move assignment operator.
    ///
    /// The self-assigment MUST never happen.
    ///
    /// @param other The other result object.
    /// @return      The reference to the left-hand side object.
    result& operator=(result&& other) noexcept
    {
        this->~result();                           // Release this object.
        static_cast<dvmc_result&>(*this) = other;  // Copy data.
        other.release = nullptr;                   // Disable releasing of the rvalue object.
        return *this;
    }

    /// Releases the ownership and returns the raw copy of dvmc_result.
    ///
    /// This method drops the ownership of the result
    /// (result's resources are not going to be released when this object is destructed).
    /// It is the caller's responsibility having the returned copy of the result to release it.
    /// This object MUST NOT be used after this method is invoked.
    ///
    /// @return  The copy of this object converted to raw dvmc_result.
    dvmc_result release_raw() noexcept
    {
        const auto out = dvmc_result{*this};  // Copy data.
        this->release = nullptr;              // Disable releasing of this object.
        return out;
    }
};


/// The DVMC Host interface
class HostInterface
{
public:
    virtual ~HostInterface() noexcept = default;

    /// @copydoc dvmc_host_interface::account_exists
    virtual bool account_exists(const address& addr) const noexcept = 0;

    /// @copydoc dvmc_host_interface::get_storage
    virtual bytes32 get_storage(const address& addr, const bytes32& key) const noexcept = 0;

    /// @copydoc dvmc_host_interface::set_storage
    virtual dvmc_storage_status set_storage(const address& addr,
                                            const bytes32& key,
                                            const bytes32& value) noexcept = 0;

    /// @copydoc dvmc_host_interface::get_balance
    virtual uint256be get_balance(const address& addr) const noexcept = 0;

    /// @copydoc dvmc_host_interface::get_code_size
    virtual size_t get_code_size(const address& addr) const noexcept = 0;

    /// @copydoc dvmc_host_interface::get_code_hash
    virtual bytes32 get_code_hash(const address& addr) const noexcept = 0;

    /// @copydoc dvmc_host_interface::copy_code
    virtual size_t copy_code(const address& addr,
                             size_t code_offset,
                             uint8_t* buffer_data,
                             size_t buffer_size) const noexcept = 0;

    /// @copydoc dvmc_host_interface::selfdestruct
    virtual void selfdestruct(const address& addr, const address& beneficiary) noexcept = 0;

    /// @copydoc dvmc_host_interface::call
    virtual result call(const dvmc_message& msg) noexcept = 0;

    /// @copydoc dvmc_host_interface::get_tx_context
    virtual dvmc_tx_context get_tx_context() const noexcept = 0;

    /// @copydoc dvmc_host_interface::get_block_hash
    virtual bytes32 get_block_hash(int64_t block_number) const noexcept = 0;

    /// @copydoc dvmc_host_interface::emit_log
    virtual void emit_log(const address& addr,
                          const uint8_t* data,
                          size_t data_size,
                          const bytes32 topics[],
                          size_t num_topics) noexcept = 0;

    /// @copydoc dvmc_host_interface::access_account
    virtual dvmc_access_status access_account(const address& addr) noexcept = 0;

    /// @copydoc dvmc_host_interface::access_storage
    virtual dvmc_access_status access_storage(const address& addr, const bytes32& key) noexcept = 0;
};


/// Wrapper around DVMC host context / host interface.
///
/// To be used by VM implementations as better alternative to using ::dvmc_host_context directly.
class HostContext : public HostInterface
{
    const dvmc_host_interface* host = nullptr;
    dvmc_host_context* context = nullptr;

public:
    /// Default constructor for null Host context.
    HostContext() = default;

    /// Constructor from the DVMC Host primitives.
    /// @param interface  The reference to the Host interface.
    /// @param ctx        The pointer to the Host context object. This parameter MAY be null.
    HostContext(const dvmc_host_interface& interface, dvmc_host_context* ctx) noexcept
        : host{&interface}, context{ctx}
    {}

    bool account_exists(const address& address) const noexcept final
    {
        return host->account_exists(context, &address);
    }

    bytes32 get_storage(const address& address, const bytes32& key) const noexcept final
    {
        return host->get_storage(context, &address, &key);
    }

    dvmc_storage_status set_storage(const address& address,
                                    const bytes32& key,
                                    const bytes32& value) noexcept final
    {
        return host->set_storage(context, &address, &key, &value);
    }

    uint256be get_balance(const address& address) const noexcept final
    {
        return host->get_balance(context, &address);
    }

    size_t get_code_size(const address& address) const noexcept final
    {
        return host->get_code_size(context, &address);
    }

    bytes32 get_code_hash(const address& address) const noexcept final
    {
        return host->get_code_hash(context, &address);
    }

    size_t copy_code(const address& address,
                     size_t code_offset,
                     uint8_t* buffer_data,
                     size_t buffer_size) const noexcept final
    {
        return host->copy_code(context, &address, code_offset, buffer_data, buffer_size);
    }

    void selfdestruct(const address& addr, const address& beneficiary) noexcept final
    {
        host->selfdestruct(context, &addr, &beneficiary);
    }

    result call(const dvmc_message& message) noexcept final
    {
        return result{host->call(context, &message)};
    }

    /// @copydoc HostInterface::get_tx_context()
    dvmc_tx_context get_tx_context() const noexcept final {
        return host->get_tx_context(context);
    }

    bytes32 get_block_hash(int64_t number) const noexcept final
    {
        return host->get_block_hash(context, number);
    }

    void emit_log(const address& addr,
                  const uint8_t* data,
                  size_t data_size,
                  const bytes32 topics[],
                  size_t topics_count) noexcept final
    {
        host->emit_log(context, &addr, data, data_size, topics, topics_count);
    }

    dvmc_access_status access_account(const address& address) noexcept final
    {
        return host->access_account(context, &address);
    }

    dvmc_access_status access_storage(const address& address, const bytes32& key) noexcept final
    {
        return host->access_storage(context, &address, &key);
    }
};


/// Abstract class to be used by Host implementations.
///
/// When implementing DVMC Host, you can directly inherit from the dvmc::Host class.
/// This way your implementation will be simpler by avoiding manual handling
/// of the ::dvmc_host_context and the ::dvmc_host_interface.
class Host : public HostInterface
{
public:
    /// Provides access to the global host interface.
    /// @returns  Reference to the host interface object.
    static const dvmc_host_interface& get_interface() noexcept;

    /// Converts the Host object to the opaque host context pointer.
    /// @returns  Pointer to dvmc_host_context.
    dvmc_host_context* to_context() noexcept {
        return reinterpret_cast<dvmc_host_context*>(this);
    }

    /// Converts the opaque host context pointer back to the original Host object.
    /// @tparam DerivedClass  The class derived from the Host class.
    /// @param context        The opaque host context pointer.
    /// @returns              The pointer to DerivedClass.
    template <typename DerivedClass = Host>
    static DerivedClass* from_context(dvmc_host_context* context) noexcept
    {
        // Get pointer of the Host base class.
        auto* h = reinterpret_cast<Host*>(context);

        // Additional downcast, only possible if DerivedClass inherits from Host.
        return static_cast<DerivedClass*>(h);
    }
};


/// @copybrief dvmc_vm
///
/// This is a RAII wrapper for dvmc_vm, and object of this type
/// automatically destroys the VM instance.
class VM
{
public:
    VM() noexcept = default;

    /// Converting constructor from dvmc_vm.
    explicit VM(dvmc_vm* vm) noexcept : m_instance {
        vm
    } {}

    /// Destructor responsible for automatically destroying the VM instance.
    ~VM() noexcept
    {
        if (m_instance != nullptr)
            m_instance->destroy(m_instance);
    }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;

    /// Move constructor.
    VM(VM&& other) noexcept : m_instance {
        other.m_instance
    } {
        other.m_instance = nullptr;
    }

    /// Move assignment operator.
    VM& operator=(VM&& other) noexcept
    {
        this->~VM();
        m_instance = other.m_instance;
        other.m_instance = nullptr;
        return *this;
    }

    /// The constructor that captures a VM instance and configures the instance
    /// with the provided list of options.
    inline VM(dvmc_vm* vm,
              std::initializer_list<std::pair<const char*, const char*>> options) noexcept;

    /// Checks if contains a valid pointer to the VM instance.
    explicit operator bool() const noexcept {
        return m_instance != nullptr;
    }

    /// Checks whenever the VM instance is ABI compatible with the current DVMC API.
    bool is_abi_compatible() const noexcept {
        return m_instance->abi_version == DVMC_ABI_VERSION;
    }

    /// @copydoc dvmc_vm::name
    char const* name() const noexcept {
        return m_instance->name;
    }

    /// @copydoc dvmc_vm::version
    char const* version() const noexcept {
        return m_instance->version;
    }

    /// Checks if the VM has the given capability.
    bool has_capability(dvmc_capabilities capability) const noexcept
    {
        return (get_capabilities() & static_cast<dvmc_capabilities_flagset>(capability)) != 0;
    }

    /// @copydoc dvmc_vm::get_capabilities
    dvmc_capabilities_flagset get_capabilities() const noexcept
    {
        return m_instance->get_capabilities(m_instance);
    }

    /// @copydoc dvmc_set_option()
    dvmc_set_option_result set_option(const char name[], const char value[]) noexcept
    {
        return dvmc_set_option(m_instance, name, value);
    }

    /// @copydoc dvmc_retrieve_desc_vx()
    result retrieve_desc_vx(const dvmc_host_interface& host,
                            dvmc_host_context* ctx,
                            dvmc_revision rev,
                            const dvmc_message& msg,
                            const uint8_t* code,
                            size_t code_size) noexcept
    {
        return result{m_instance->retrieve_desc_vx(m_instance, &host, ctx, rev, &msg, code, code_size)};
    }

    /// Convenient variant of the VM::retrieve_desc_vx() that takes reference to dvmc::Host class.
    result retrieve_desc_vx(Host& host,
                            dvmc_revision rev,
                            const dvmc_message& msg,
                            const uint8_t* code,
                            size_t code_size) noexcept
    {
        return retrieve_desc_vx(Host::get_interface(), host.to_context(), rev, msg, code, code_size);
    }

    /// Executes code without the Host context.
    ///
    /// The same as
    /// retrieve_desc_vx(const dvmc_host_interface&, dvmc_host_context*, dvmc_revision,
    ///         const dvmc_message&, const uint8_t*, size_t),
    /// but without providing the Host context and interface.
    /// This method is for experimental precompiles support where execution is
    /// guaranteed not to require any Host access.
    result retrieve_desc_vx(dvmc_revision rev,
                            const dvmc_message& msg,
                            const uint8_t* code,
                            size_t code_size) noexcept
    {
        return result{
            m_instance->retrieve_desc_vx(m_instance, nullptr, nullptr, rev, &msg, code, code_size)};
    }

    /// Returns the pointer to C DVMC struct representing the VM.
    ///
    /// Gives access to the C DVMC VM struct to allow advanced interaction with the VM not supported
    /// by the C++ interface. Use as the last resort.
    /// This object still owns the VM after returning the pointer. The returned pointer MAY be null.
    dvmc_vm* get_raw_pointer() const noexcept {
        return m_instance;
    }

private:
    dvmc_vm* m_instance = nullptr;
};

inline VM::VM(dvmc_vm* vm,
              std::initializer_list<std::pair<const char*, const char*>> options) noexcept
    : m_instance {
    vm
}
{
    // This constructor is implemented outside of the class definition to workaround a doxygen bug.
    for (const auto& option : options)
        set_option(option.first, option.second);
}


namespace internal
{
inline bool account_exists(dvmc_host_context* h, const dvmc_address* addr) noexcept
{
    return Host::from_context(h)->account_exists(*addr);
}

inline dvmc_bytes32 get_storage(dvmc_host_context* h,
                                const dvmc_address* addr,
                                const dvmc_bytes32* key) noexcept
{
    return Host::from_context(h)->get_storage(*addr, *key);
}

inline dvmc_storage_status set_storage(dvmc_host_context* h,
                                       const dvmc_address* addr,
                                       const dvmc_bytes32* key,
                                       const dvmc_bytes32* value) noexcept
{
    return Host::from_context(h)->set_storage(*addr, *key, *value);
}

inline dvmc_uint256be get_balance(dvmc_host_context* h, const dvmc_address* addr) noexcept
{
    return Host::from_context(h)->get_balance(*addr);
}

inline size_t get_code_size(dvmc_host_context* h, const dvmc_address* addr) noexcept
{
    return Host::from_context(h)->get_code_size(*addr);
}

inline dvmc_bytes32 get_code_hash(dvmc_host_context* h, const dvmc_address* addr) noexcept
{
    return Host::from_context(h)->get_code_hash(*addr);
}

inline size_t copy_code(dvmc_host_context* h,
                        const dvmc_address* addr,
                        size_t code_offset,
                        uint8_t* buffer_data,
                        size_t buffer_size) noexcept
{
    return Host::from_context(h)->copy_code(*addr, code_offset, buffer_data, buffer_size);
}

inline void selfdestruct(dvmc_host_context* h,
                         const dvmc_address* addr,
                         const dvmc_address* beneficiary) noexcept
{
    Host::from_context(h)->selfdestruct(*addr, *beneficiary);
}

inline dvmc_result call(dvmc_host_context* h, const dvmc_message* msg) noexcept
{
    return Host::from_context(h)->call(*msg).release_raw();
}

inline dvmc_tx_context get_tx_context(dvmc_host_context* h) noexcept
{
    return Host::from_context(h)->get_tx_context();
}

inline dvmc_bytes32 get_block_hash(dvmc_host_context* h, int64_t block_number) noexcept
{
    return Host::from_context(h)->get_block_hash(block_number);
}

inline void emit_log(dvmc_host_context* h,
                     const dvmc_address* addr,
                     const uint8_t* data,
                     size_t data_size,
                     const dvmc_bytes32 topics[],
                     size_t num_topics) noexcept
{
    Host::from_context(h)->emit_log(*addr, data, data_size, static_cast<const bytes32*>(topics),
                                    num_topics);
}

inline dvmc_access_status access_account(dvmc_host_context* h, const dvmc_address* addr) noexcept
{
    return Host::from_context(h)->access_account(*addr);
}

inline dvmc_access_status access_storage(dvmc_host_context* h,
        const dvmc_address* addr,
        const dvmc_bytes32* key) noexcept
{
    return Host::from_context(h)->access_storage(*addr, *key);
}
}  // namespace internal

inline const dvmc_host_interface& Host::get_interface() noexcept
{
    static constexpr dvmc_host_interface interface = {
        ::dvmc::internal::account_exists, ::dvmc::internal::get_storage,
        ::dvmc::internal::set_storage,    ::dvmc::internal::get_balance,
        ::dvmc::internal::get_code_size,  ::dvmc::internal::get_code_hash,
        ::dvmc::internal::copy_code,      ::dvmc::internal::selfdestruct,
        ::dvmc::internal::call,           ::dvmc::internal::get_tx_context,
        ::dvmc::internal::get_block_hash, ::dvmc::internal::emit_log,
        ::dvmc::internal::access_account, ::dvmc::internal::access_storage,
    };
    return interface;
}
}  // namespace dvmc


/// "Stream out" operator implementation for ::dvmc_status_code.
///
/// @note This is defined in global namespace to match ::dvmc_status_code definition and allow
///       convenient operator overloading usage.
inline std::ostream& operator<<(std::ostream& os, dvmc_status_code status_code)
{
    return os << dvmc::to_string(status_code);
}

/// "Stream out" operator implementation for ::dvmc_revision.
///
/// @note This is defined in global namespace to match ::dvmc_revision definition and allow
///       convenient operator overloading usage.
inline std::ostream& operator<<(std::ostream& os, dvmc_revision rev)
{
    return os << dvmc::to_string(rev);
}

namespace std
{
/// Hash operator template specialization for dvmc::address. Needed for unordered containers.
template <>
struct hash<dvmc::address>
{
    /// Hash operator using FNV1a-based folding.
    constexpr size_t operator()(const dvmc::address& s) const noexcept
    {
        using namespace dvmc;
        using namespace fnv;
        return static_cast<size_t>(fnv1a_by64(
                                       fnv1a_by64(fnv1a_by64(fnv::offset_basis, load64le(&s.bytes[0])), load64le(&s.bytes[8])),
                                       load32le(&s.bytes[16])));
    }
};

/// Hash operator template specialization for dvmc::bytes32. Needed for unordered containers.
template <>
struct hash<dvmc::bytes32>
{
    /// Hash operator using FNV1a-based folding.
    constexpr size_t operator()(const dvmc::bytes32& s) const noexcept
    {
        using namespace dvmc;
        using namespace fnv;
        return static_cast<size_t>(
                   fnv1a_by64(fnv1a_by64(fnv1a_by64(fnv1a_by64(fnv::offset_basis, load64le(&s.bytes[0])),
                                         load64le(&s.bytes[8])),
                                         load64le(&s.bytes[16])),
                              load64le(&s.bytes[24])));
    }
};
}  // namespace std
