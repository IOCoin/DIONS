#include "DVM.h"

using namespace std;
using namespace dev;
using namespace dev::dvm;

std::array<InstructionMetric, 256> DVM::c_metrics;
void DVM::initMetrics()
{
	static bool done =
	[]()
	{
		for (unsigned i = 0; i < 256; ++i)
		{
			InstructionInfo op = instructionInfo((Instruction)i);
			c_metrics[i].gasPriceTier = op.gasPriceTier;
			c_metrics[i].args = op.args;
			c_metrics[i].ret = op.ret;
		}
		return true;
	} ();
	(void)done;
}

void DVM::copyCode(int _extraBytes)
{
	// Copy code so that it can be safely modified and extend code by
	// _extraBytes zero bytes to allow reading virtual data at the end
	// of the code without bounds checks.
	auto extendedSize = m_ext->code.size() + _extraBytes;
	m_code.reserve(extendedSize);
	m_code = m_ext->code;
	m_code.resize(extendedSize);
}

void DVM::optimize()
{
	copyCode(33);

	size_t const nBytes = m_ext->code.size();

	// build a table of jump destinations for use in verifyJumpDest
	
	TRACE_STR(1, "Build JUMPDEST table")
	for (size_t pc = 0; pc < nBytes; ++pc)
	{
		Instruction op = Instruction(m_code[pc]);
		TRACE_OP(2, pc, op);
				
		// make synthetic ops in user code trigger invalid instruction if run
		if (
			op == Instruction::PUSHC ||
			op == Instruction::JUMPC ||
			op == Instruction::JUMPCI
		)
		{
			TRACE_OP(1, pc, op);
			m_code[pc] = (byte)Instruction::INVALID;
		}

		if (op == Instruction::JUMPDEST)
		{
			m_jumpDests.push_back(pc);
		}
		else if (
			(byte)Instruction::PUSH1 <= (byte)op &&
			(byte)op <= (byte)Instruction::PUSH32
		)
		{
			pc += (byte)op - (byte)Instruction::PUSH1 + 1;
		}
#if EIP_615
		else if (
			op == Instruction::JUMPTO ||
			op == Instruction::JUMPIF ||
			op == Instruction::JUMPSUB)
		{
			++pc;
			pc += 4;
		}
		else if (op == Instruction::JUMPV || op == Instruction::JUMPSUBV)
		{
			++pc;
			pc += 4 * m_code[pc];  // number of 4-byte dests followed by table
		}
		else if (op == Instruction::BEGINSUB)
		{
			m_beginSubs.push_back(pc);
		}
		else if (op == Instruction::BEGINDATA)
		{
			break;
		}
#endif
	}
	
#ifdef DVM_DO_FIRST_PASS_OPTIMIZATION
	
	TRACE_STR(1, "Do first pass optimizations")
	for (size_t pc = 0; pc < nBytes; ++pc)
	{
		u256 val = 0;
		Instruction op = Instruction(m_code[pc]);

		if ((byte)Instruction::PUSH1 <= (byte)op && (byte)op <= (byte)Instruction::PUSH32)
		{
			byte nPush = (byte)op - (byte)Instruction::PUSH1 + 1;

			// decode pushed bytes to integral value
			val = m_code[pc+1];
			for (uint64_t i = pc+2, n = nPush; --n; ++i) {
				val = (val << 8) | m_code[i];
			}

		#if DVM_USE_CONSTANT_POOL

			// add value to constant pool and replace PUSHn with PUSHC
			// place offset in code as 2 bytes MSB-first
			// followed by one byte count of remaining pushed bytes
			if (5 < nPush)
			{
				uint16_t pool_off = m_pool.size();
				TRACE_VAL(1, "stash", val);
				TRACE_VAL(1, "... in pool at offset" , pool_off);
				m_pool.push_back(val);

				TRACE_PRE_OPT(1, pc, op);
				m_code[pc] = byte(op = Instruction::PUSHC);
				m_code[pc+3] = nPush - 2;
				m_code[pc+2] = pool_off & 0xff;
				m_code[pc+1] = pool_off >> 8;
				TRACE_POST_OPT(1, pc, op);
			}

		#endif

		#if DVM_REPLACE_CONST_JUMP	
			// replace JUMP or JUMPI to constant location with JUMPC or JUMPCI
			// verifyJumpDest is M = log(number of jump destinations)
			// outer loop is N = number of bytes in code array
			// so complexity is N log M, worst case is N log N
			size_t i = pc + nPush + 1;
			op = Instruction(m_code[i]);
			if (op == Instruction::JUMP)
			{
				TRACE_VAL(1, "Replace const JUMP with JUMPC to", val)
				TRACE_PRE_OPT(1, i, op);
				
				if (0 <= verifyJumpDest(val, false))
					m_code[i] = byte(op = Instruction::JUMPC);
				
				TRACE_POST_OPT(1, i, op);
			}
			else if (op == Instruction::JUMPI)
			{
				TRACE_VAL(1, "Replace const JUMPI with JUMPCI to", val)
				TRACE_PRE_OPT(1, i, op);
				
				if (0 <= verifyJumpDest(val, false))
					m_code[i] = byte(op = Instruction::JUMPCI);
				
				TRACE_POST_OPT(1, i, op);
			}
		#endif

			pc += nPush;
		}
	}
	TRACE_STR(1, "Finished optimizations")
#endif	
}


//
// Init interpreter on entry.
//
void DVM::initEntry()
{
	m_bounce = &DVM::interpretCases;
	initMetrics();
	optimize();
}


// Implementation of EXP.
//
// This implements exponentiation by squaring algorithm.
// Is faster than boost::multiprecision::powm() because it avoids explicit
// mod operation.
// Do not inline it.
u256 DVM::exp256(u256 _base, u256 _exponent)
{
	using boost::multiprecision::limb_type;
	u256 result = 1;
	while (_exponent)
	{
		if (static_cast<limb_type>(_exponent) & 1) // If exponent is odd.
			result *= _base;
		_base *= _base;
		_exponent >>= 1;
	}
	return result;
}
