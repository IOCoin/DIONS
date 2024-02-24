#include "SMCValidator.h"

namespace dev
{
namespace eth
{
void VM::copyDataToMemory(bytesConstRef _data, intx::uint256*_sp)
{
    auto offset = static_cast<size_t>(_sp[0]);
    intx::uint512 bigIndex = _sp[1];
    auto index = static_cast<size_t>(bigIndex);
    auto size = static_cast<size_t>(_sp[2]);

    size_t sizeToBeCopied = bigIndex + size > _data.size() ? _data.size() < bigIndex ? 0 : _data.size() - index : size;

    if (sizeToBeCopied > 0)
        std::memcpy(m_mem.data() + offset, _data.data() + index, sizeToBeCopied);
    if (size > sizeToBeCopied)
        std::memset(m_mem.data() + offset + sizeToBeCopied, 0, size - sizeToBeCopied);
}


void VM::throwOutOfGas()
{
    BOOST_THROW_EXCEPTION(OutOfGas());
}

void VM::throwInvalidInstruction()
{
    BOOST_THROW_EXCEPTION(InvalidInstruction());
}

void VM::throwBadInstruction()
{
    BOOST_THROW_EXCEPTION(BadInstruction());
}

void VM::throwBadJumpDestination()
{
    BOOST_THROW_EXCEPTION(BadJumpDestination());
}

void VM::throwDisallowedStateChange()
{
    BOOST_THROW_EXCEPTION(DisallowedStateChange());
}

void VM::throwBadStack(int _required, int _change)
{
    bigint size = m_stackEnd - m_SPP;
    if (size < _required)
        BOOST_THROW_EXCEPTION(StackUnderflow() << RequirementError((bigint)_required, size));
    else
        BOOST_THROW_EXCEPTION(OutOfStack() << RequirementError((bigint)_change, size));
}

void VM::throwRevertInstruction(owning_bytes_ref&& _output)
{
    throw RevertInstruction(std::move(_output));
}

void VM::throwBufferOverrun(intx::uint512 const& _endOfAccess)
{
    BOOST_THROW_EXCEPTION(
        BufferOverrun() << RequirementError(
            bigint(std::string("0x") + intx::hex(_endOfAccess)), bigint(m_returnData.size())));
}

int64_t VM::verifyJumpDest(intx::uint256 const& _dest, bool _throw)
{
    if (_dest <= 0x7FFFFFFFFFFFFFFF) {

        uint64_t pc = uint64_t(_dest);
        if (std::binary_search(m_jumpDests.begin(), m_jumpDests.end(), pc))
            return pc;
    }
    if (_throw)
        throwBadJumpDestination();
    return -1;
}


void VM::caseCreate()
{
    m_bounce = &VM::interpretCases;
    m_runGas = VMSchedule::createGas;

    intx::uint256 const endowment = m_SP[0];
    intx::uint256 const initOff = m_SP[1];
    intx::uint256 const initSize = m_SP[2];

    intx::uint256 salt;
    if (m_OP == Instruction::CREATE2)
    {
        salt = m_SP[3];
        m_runGas += toInt63((intx::uint512{initSize} + 31) / 32 * uint64_t{VMSchedule::sha3WordGas});
    }

    updateMem(memNeed(initOff, initSize));
    updateIOGas();

    m_returnData.clear();

    auto const balance = intx::be::load<intx::uint256>(m_host->get_balance(m_context, &m_message->destination));
    if (balance >= endowment && m_message->depth < 1024)
    {
        evmc_message msg = {};
        msg.gas = m_io_gas;
        if (m_rev >= EVMC_TANGERINE_WHISTLE)
            msg.gas -= msg.gas / 64;

        auto off = static_cast<size_t>(initOff);
        auto size = static_cast<size_t>(initSize);

        msg.input_data = &m_mem[off];
        msg.input_size = size;
        msg.create2_salt = intx::be::store<evmc_uint256be>(salt);
        msg.sender = m_message->destination;
        msg.depth = m_message->depth + 1;
        msg.value = intx::be::store<evmc_uint256be>(endowment);

        evmc_result result = m_host->call(m_context, &msg);

        if (result.status_code == EVMC_SUCCESS)
            m_SPP[0] = intx::be::load<intx::uint256>(result.create_address);
        else
            m_SPP[0] = 0;
        m_returnData.assign(result.output_data, result.output_data + result.output_size);

        m_io_gas -= (msg.gas - result.gas_left);

        if (result.release)
            result.release(&result);
    }
    else
        m_SPP[0] = 0;
    ++m_PC;
}

void VM::caseCall()
{
    m_bounce = &VM::interpretCases;

    evmc_message msg = {};

    m_returnData.clear();

    bytesRef output;
    if (caseCallSetup(msg, output))
    {
        evmc_result result = m_host->call(m_context, &msg);

        m_returnData.assign(result.output_data, result.output_data + result.output_size);
        bytesConstRef{&m_returnData}.copyTo(output);

        m_SPP[0] = result.status_code == EVMC_SUCCESS ? 1 : 0;
        m_io_gas += result.gas_left;

        if (result.release)
            result.release(&result);
    }
    else
    {
        m_SPP[0] = 0;
        m_io_gas += msg.gas;
    }
    ++m_PC;
}

bool VM::caseCallSetup(evmc_message& o_msg, bytesRef& o_output)
{
    auto const destination = intx::be::trunc<evmc::address>(m_SP[1]);

    if (m_rev >= EVMC_BERLIN && m_message->destination == destination)
        m_runGas = VMSchedule::callSelfGas;

    switch (m_OP)
    {
    case Instruction::CALL:
    case Instruction::STATICCALL:
    default:
        o_msg.kind = EVMC_CALL;
        break;
    case Instruction::CALLCODE:
        o_msg.kind = EVMC_CALLCODE;
        break;
    case Instruction::DELEGATECALL:
        o_msg.kind = EVMC_DELEGATECALL;
        break;
    }

    if (m_OP == Instruction::STATICCALL || m_message->flags & EVMC_STATIC)
        o_msg.flags = EVMC_STATIC;

    bool const haveValueArg = m_OP == Instruction::CALL || m_OP == Instruction::CALLCODE;

    if (m_OP == Instruction::CALL && (m_SP[2] > 0 || m_rev < EVMC_SPURIOUS_DRAGON) &&
        !m_host->account_exists(m_context, &destination))
    {
        m_runGas += VMSchedule::callNewAccount;
    }

    if (haveValueArg && m_SP[2] > 0)
        m_runGas += VMSchedule::valueTransferGas;

    size_t const sizesOffset = haveValueArg ? 3 : 2;
    intx::uint256 inputOffset  = m_SP[sizesOffset];
    intx::uint256 inputSize    = m_SP[sizesOffset + 1];
    intx::uint256 outputOffset = m_SP[sizesOffset + 2];
    intx::uint256 outputSize   = m_SP[sizesOffset + 3];
    uint64_t inputMemNeed = memNeed(inputOffset, inputSize);
    uint64_t outputMemNeed = memNeed(outputOffset, outputSize);

    m_newMemSize = std::max(inputMemNeed, outputMemNeed);
    updateMem(m_newMemSize);
    updateIOGas();

    intx::uint256 callGas = m_SP[0];
    if (m_rev >= EVMC_TANGERINE_WHISTLE)
    {
        intx::uint256 maxAllowedCallGas = m_io_gas - m_io_gas / 64;
        callGas = std::min(callGas, maxAllowedCallGas);
    }

    o_msg.gas = toInt63(callGas);
    m_runGas = o_msg.gas;
    updateIOGas();

    o_msg.depth = m_message->depth + 1;
    o_msg.destination = destination;
    o_msg.sender = m_message->destination;
    o_msg.input_data = m_mem.data() + size_t(inputOffset);
    o_msg.input_size = size_t(inputSize);

    bool balanceOk = true;
    if (haveValueArg)
    {
        intx::uint256 value = m_SP[2];
        if (value > 0)
        {
            o_msg.value = intx::be::store<evmc_uint256be>(m_SP[2]);
            o_msg.gas += VMSchedule::callStipend;
            {
                auto const balance =
                    intx::be::load<intx::uint256>(m_host->get_balance(m_context, &m_message->destination));
                balanceOk = balance >= value;
            }
        }
    }
    else if (m_OP == Instruction::DELEGATECALL)
    {
        o_msg.sender = m_message->sender;
        o_msg.value = m_message->value;
    }

    if (balanceOk && m_message->depth < 1024)
    {
        size_t outOff = size_t(outputOffset);
        size_t outSize = size_t(outputSize);
        o_output = bytesRef(m_mem.data() + outOff, outSize);
        return true;
    }
    return false;
}
}
}
