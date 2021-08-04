// Aldvm: Ethereum C++ client, tools and libraries.
// Copyright 2014-2019 Aldvm Authors.
// Licensed under the GNU General Public License, Version 3.

#include "ExtVMFace.h"

#include <dvmc/helpers.h>

namespace dev
{
namespace dvm
{
static_assert(sizeof(Address) == sizeof(dvmc_address), "Address types size mismatch");
static_assert(alignof(Address) == alignof(dvmc_address), "Address types alignment mismatch");
static_assert(sizeof(h256) == sizeof(dvmc_uint256be), "Hash types size mismatch");
static_assert(alignof(h256) == alignof(dvmc_uint256be), "Hash types alignment mismatch");

bool DvmCHost::account_exists(dvmc::address const& _addr) const noexcept
{
    return m_extVM.exists(fromDvmC(_addr));
}

dvmc::bytes32 DvmCHost::get_storage(dvmc::address const& _addr, dvmc::bytes32 const& _key) const
    noexcept
{
    (void)_addr;
    assert(fromDvmC(_addr) == m_extVM.myAddress);
    return toDvmC(m_extVM.store(fromDvmC(_key)));
}

dvmc_storage_status DvmCHost::set_storage(
    dvmc::address const& _addr, dvmc::bytes32 const& _key, dvmc::bytes32 const& _value) noexcept
{
    (void)_addr;
    assert(fromDvmC(_addr) == m_extVM.myAddress);
    u256 const index = fromDvmC(_key);
    u256 const newValue = fromDvmC(_value);
    u256 const currentValue = m_extVM.store(index);

    if (newValue == currentValue)
        return DVMC_STORAGE_UNCHANGED;

    DVMSchedule const& schedule = m_extVM.dvmSchedule();
    auto status = DVMC_STORAGE_MODIFIED;
    u256 const originalValue = m_extVM.originalStorageValue(index);
    if (originalValue == currentValue || !schedule.sstoreNetGasMetering())
    {
        if (currentValue == 0)
            status = DVMC_STORAGE_ADDED;
        else if (newValue == 0)
        {
            status = DVMC_STORAGE_DELETED;
            m_extVM.sub.refunds += schedule.sstoreRefundGas;
        }
    }
    else
    {
        status = DVMC_STORAGE_MODIFIED_AGAIN;
        if (originalValue != 0)
        {
            if (currentValue == 0)
                m_extVM.sub.refunds -= schedule.sstoreRefundGas;  // Can go negative.
            if (newValue == 0)
                m_extVM.sub.refunds += schedule.sstoreRefundGas;
        }
        if (originalValue == newValue)
        {
            if (originalValue == 0)
                m_extVM.sub.refunds += schedule.sstoreSetGas - schedule.sstoreUnchangedGas;
            else
                m_extVM.sub.refunds += schedule.sstoreResetGas - schedule.sstoreUnchangedGas;
        }
    }

    m_extVM.setStore(index, newValue);  // Interface uses native endianness

    return status;
}

dvmc::uint256be DvmCHost::get_balance(dvmc::address const& _addr) const noexcept
{
    return toDvmC(m_extVM.balance(fromDvmC(_addr)));
}

size_t DvmCHost::get_code_size(dvmc::address const& _addr) const noexcept
{
    return m_extVM.codeSizeAt(fromDvmC(_addr));
}

dvmc::bytes32 DvmCHost::get_code_hash(dvmc::address const& _addr) const noexcept
{
    return toDvmC(m_extVM.codeHashAt(fromDvmC(_addr)));
}

size_t DvmCHost::copy_code(dvmc::address const& _addr, size_t _codeOffset, byte* _bufferData,
    size_t _bufferSize) const noexcept
{
    Address addr = fromDvmC(_addr);
    bytes const& c = m_extVM.codeAt(addr);

    // Handle "big offset" edge case.
    if (_codeOffset >= c.size())
        return 0;

    size_t maxToCopy = c.size() - _codeOffset;
    size_t numToCopy = std::min(maxToCopy, _bufferSize);
    std::copy_n(&c[_codeOffset], numToCopy, _bufferData);
    return numToCopy;
}

void DvmCHost::selfdestruct(dvmc::address const& _addr, dvmc::address const& _beneficiary) noexcept
{
    (void)_addr;
    assert(fromDvmC(_addr) == m_extVM.myAddress);
    m_extVM.selfdestruct(fromDvmC(_beneficiary));
}


void DvmCHost::emit_log(dvmc::address const& _addr, uint8_t const* _data, size_t _dataSize,
    dvmc::bytes32 const _topics[], size_t _numTopics) noexcept
{
    (void)_addr;
    assert(fromDvmC(_addr) == m_extVM.myAddress);
    h256 const* pTopics = reinterpret_cast<h256 const*>(_topics);
    m_extVM.log(h256s{pTopics, pTopics + _numTopics}, bytesConstRef{_data, _dataSize});
}

dvmc_tx_context DvmCHost::get_tx_context() const noexcept
{
    dvmc_tx_context result = {};
    result.tx_gas_price = toDvmC(m_extVM.gasPrice);
    result.tx_origin = toDvmC(m_extVM.origin);

    auto const& envInfo = m_extVM.envInfo();
    result.block_coinbase = toDvmC(envInfo.author());
    result.block_number = envInfo.number();
    result.block_timestamp = envInfo.timestamp();
    result.block_gas_limit = static_cast<int64_t>(envInfo.gasLimit());
    result.block_difficulty = toDvmC(envInfo.difficulty());
    result.chain_id = toDvmC(envInfo.chainID());
    return result;
}

dvmc::bytes32 DvmCHost::get_block_hash(int64_t _number) const noexcept
{
    return toDvmC(m_extVM.blockHash(_number));
}

dvmc::result DvmCHost::create(dvmc_message const& _msg) noexcept
{
    u256 gas = _msg.gas;
    u256 value = fromDvmC(_msg.value);
    bytesConstRef init = {_msg.input_data, _msg.input_size};
    u256 salt = fromDvmC(_msg.create2_salt);
    Instruction opcode = _msg.kind == DVMC_CREATE ? Instruction::CREATE : Instruction::CREATE2;

    // ExtVM::create takes the sender address from .myAddress.
    assert(fromDvmC(_msg.sender) == m_extVM.myAddress);

    CreateResult result = m_extVM.create(value, gas, init, opcode, salt, {});
    dvmc_result dvmcResult = {};
    dvmcResult.status_code = result.status;
    dvmcResult.gas_left = static_cast<int64_t>(gas);

    if (result.status == DVMC_SUCCESS)
        dvmcResult.create_address = toDvmC(result.address);
    else
    {
        // Pass the output to the DVM without a copy. The DVM will delete it
        // when finished with it.

        // First assign reference. References are not invalidated when vector
        // of bytes is moved. See `.takeBytes()` below.
        dvmcResult.output_data = result.output.data();
        dvmcResult.output_size = result.output.size();

        // Place a new vector of bytes containing output in result's reserved memory.
        auto* data = dvmc_get_optional_storage(&dvmcResult);
        static_assert(sizeof(bytes) <= sizeof(*data), "Vector is too big");
        new (data) bytes(result.output.takeBytes());
        // Set the destructor to delete the vector.
        dvmcResult.release = [](dvmc_result const* _result) {
            auto* data = dvmc_get_const_optional_storage(_result);
            auto& output = reinterpret_cast<bytes const&>(*data);
            // Explicitly call vector's destructor to release its data.
            // This is normal pattern when placement new operator is used.
            output.~bytes();
        };
    }
    return dvmc::result{dvmcResult};
}

dvmc::result DvmCHost::call(dvmc_message const& _msg) noexcept
{
    assert(_msg.gas >= 0 && "Invalid gas value");
    assert(_msg.depth == static_cast<int>(m_extVM.depth) + 1);

    // Handle CREATE separately.
    if (_msg.kind == DVMC_CREATE || _msg.kind == DVMC_CREATE2)
        return create(_msg);

    CallParameters params;
    params.gas = _msg.gas;
    params.apparentValue = fromDvmC(_msg.value);
    params.valueTransfer = _msg.kind == DVMC_DELEGATECALL ? 0 : params.apparentValue;
    params.senderAddress = fromDvmC(_msg.sender);
    params.codeAddress = fromDvmC(_msg.destination);
    params.receiveAddress = _msg.kind == DVMC_CALL ? params.codeAddress : m_extVM.myAddress;
    params.data = {_msg.input_data, _msg.input_size};
    params.staticCall = (_msg.flags & DVMC_STATIC) != 0;
    params.onOp = {};

    CallResult result = m_extVM.call(params);
    dvmc_result dvmcResult = {};
    dvmcResult.status_code = result.status;
    dvmcResult.gas_left = static_cast<int64_t>(params.gas);

    // Pass the output to the DVM without a copy. The DVM will delete it
    // when finished with it.

    // First assign reference. References are not invalidated when vector
    // of bytes is moved. See `.takeBytes()` below.
    dvmcResult.output_data = result.output.data();
    dvmcResult.output_size = result.output.size();

    // Place a new vector of bytes containing output in result's reserved memory.
    auto* data = dvmc_get_optional_storage(&dvmcResult);
    static_assert(sizeof(bytes) <= sizeof(*data), "Vector is too big");
    new (data) bytes(result.output.takeBytes());
    // Set the destructor to delete the vector.
    dvmcResult.release = [](dvmc_result const* _result) {
        auto* data = dvmc_get_const_optional_storage(_result);
        auto& output = reinterpret_cast<bytes const&>(*data);
        // Explicitly call vector's destructor to release its data.
        // This is normal pattern when placement new operator is used.
        output.~bytes();
    };
    return dvmc::result{dvmcResult};
}

ExtVMFace::ExtVMFace(EnvInfo const& _envInfo, Address _myAddress, Address _caller, Address _origin,
    u256 _value, u256 _gasPrice, bytesConstRef _data, bytes _code, h256 const& _codeHash,
    u256 const& _version, unsigned _depth, bool _isCreate, bool _staticCall)
  : m_envInfo(_envInfo),
    myAddress(_myAddress),
    caller(_caller),
    origin(_origin),
    value(_value),
    gasPrice(_gasPrice),
    data(_data),
    code(std::move(_code)),
    codeHash(_codeHash),
    version(_version),
    depth(_depth),
    isCreate(_isCreate),
    staticCall(_staticCall)
{}

}  // namespace dvm
}  // namespace dev
