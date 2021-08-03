// Aldvm: Ethereum C++ client, tools and libraries.
// Copyright 2016-2019 Aldvm Authors.
// Licensed under the GNU General Public License, Version 3.
#pragma once

namespace dev
    {
        namespace dvm
    {
///////////////////////////////////////////////////////////////////////////////
//
// interpreter configuration macros for development, optimizations and tracing
//
// EIP_615                - subroutines and static jumps
// EIP_616                - SIMD
//
// DVM_OPTIMIZE           - all optimizations off when false (TO DO - MAKE DYNAMIC)
//
// DVM_SWITCH_DISPATCH    - dispatch via loop and switch
// DVM_JUMP_DISPATCH      - dispatch via a jump table - available only on GCC
//
// DVM_USE_CONSTANT_POOL  - constants unpacked and ready to assign to stack
//
// DVM_REPLACE_CONST_JUMP - pre-verified jumps to save runtime lookup
//
// DVM_TRACE              - provides various levels of tracing

#ifndef EIP_615
#define EIP_615 false
#endif

#ifndef EIP_616
#define EIP_616 false
#endif

#ifndef DVM_JUMP_DISPATCH
#ifdef __GNUC__
#define DVM_JUMP_DISPATCH true
#else
#define DVM_JUMP_DISPATCH false
#endif
#endif
#if DVM_JUMP_DISPATCH
        #ifndef __GNUC__
#error "address of label extension available only on Gnu"
#endif
#else
#define DVM_SWITCH_DISPATCH true
#endif

#ifndef DVM_OPTIMIZE
#define DVM_OPTIMIZE false
#endif
#if DVM_OPTIMIZE
        #define DVM_REPLACE_CONST_JUMP true
#define DVM_USE_CONSTANT_POOL true
#define DVM_DO_FIRST_PASS_OPTIMIZATION (DVM_REPLACE_CONST_JUMP || DVM_USE_CONSTANT_POOL)
#endif


///////////////////////////////////////////////////////////////////////////////
//
// set DVM_TRACE to 3, 2, 1, or 0 for lots to no tracing to cerr
//
#ifndef DVM_TRACE
#define DVM_TRACE 0
#endif
#if DVM_TRACE > 0

        #undef ON_OP
#if DVM_TRACE > 2
#define ON_OP() \
    (cerr << "### " << ++m_nSteps << ": " << m_PC << " " << instructionInfo(m_OP).name << endl)
#else
#define ON_OP() onOperation()
#endif

#define TRACE_STR(level, str) \
    if ((level) <= DVM_TRACE) \
        cerr << "$$$ " << (str) << endl;

#define TRACE_VAL(level, name, val) \
    if ((level) <= DVM_TRACE)       \
        cerr << "=== " << (name) << " " << hex << (val) << endl;
#define TRACE_OP(level, pc, op) \
    if ((level) <= DVM_TRACE)   \
        cerr << "*** " << (pc) << " " << instructionInfo(op).name << endl;

#define TRACE_PRE_OPT(level, pc, op) \
    if ((level) <= DVM_TRACE)        \
        cerr << "<<< " << (pc) << " " << instructionInfo(op).name << endl;

#define TRACE_POST_OPT(level, pc, op) \
    if ((level) <= DVM_TRACE)         \
        cerr << ">>> " << (pc) << " " << instructionInfo(op).name << endl;
#else
#define TRACE_STR(level, str)
#define TRACE_VAL(level, name, val)
#define TRACE_OP(level, pc, op)
#define TRACE_PRE_OPT(level, pc, op)
#define TRACE_POST_OPT(level, pc, op)
#define ON_OP() onOperation()
#endif

// Executive swallows exceptions in some circumstances
#if 0
#define THROW_EXCEPTION(X) ((cerr << "!!! DVM EXCEPTION " << (X).what() << endl), abort())
#else
#if DVM_TRACE > 0
        #define THROW_EXCEPTION(X) \
    ((cerr << "!!! DVM EXCEPTION " << (X).what() << endl), BOOST_THROW_EXCEPTION(X))
#else
#define THROW_EXCEPTION(X) BOOST_THROW_EXCEPTION(X)
#endif
#endif


///////////////////////////////////////////////////////////////////////////////
//
// build a simple loop-and-switch interpreter
//
#if DVM_SWITCH_DISPATCH

        #define INIT_CASES
#define DO_CASES            \
    for (;;)                \
    {                       \
        fetchInstruction(); \
        switch (m_OP)       \
        {
#define CASE(name) case Instruction::name:
#define NEXT \
    ++m_PC;  \
    break;
#define CONTINUE continue;
#define BREAK return;
#define DEFAULT default:
#define WHILE_CASES \
    }               \
    }


///////////////////////////////////////////////////////////////////////////////
//
// build an indirect-threaded interpreter using a jump table of
// label addresses (a gcc extension)
//
#elif DVM_JUMP_DISPATCH

#define INIT_CASES                              \
                                                \
    static const void* const jumpTable[256] = { \
        &&STOP, /* 00 */                        \
        &&ADD,                                  \
        &&MUL,                                  \
        &&SUB,                                  \
        &&DIV,                                  \
        &&SDIV,                                 \
        &&MOD,                                  \
        &&SMOD,                                 \
        &&ADDMOD,                               \
        &&MULMOD,                               \
        &&EXP,                                  \
        &&SIGNEXTEND,                           \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&LT, /* 10, */                         \
        &&GT,                                   \
        &&SLT,                                  \
        &&SGT,                                  \
        &&EQ,                                   \
        &&ISZERO,                               \
        &&AND,                                  \
        &&OR,                                   \
        &&XOR,                                  \
        &&NOT,                                  \
        &&BYTE,                                 \
        &&SHL,                                  \
        &&SHR,                                  \
        &&SAR,                                  \
        &&INVALID,                              \
        &&INVALID,                              \
        &&SHA3, /* 20, */                       \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&ADDRESS, /* 30, */                    \
        &&BALANCE,                              \
        &&ORIGIN,                               \
        &&CALLER,                               \
        &&CALLVALUE,                            \
        &&CALLDATALOAD,                         \
        &&CALLDATASIZE,                         \
        &&CALLDATACOPY,                         \
        &&CODESIZE,                             \
        &&CODECOPY,                             \
        &&GASPRICE,                             \
        &&EXTCODESIZE,                          \
        &&EXTCODECOPY,                          \
        &&RETURNDATASIZE,                       \
        &&RETURNDATACOPY,                       \
        &&EXTCODEHASH,                          \
        &&BLOCKHASH, /* 40, */                  \
        &&COINBASE,                             \
        &&TIMESTAMP,                            \
        &&NUMBER,                               \
        &&DIFFICULTY,                           \
        &&GASLIMIT,                             \
        &&CHAINID,                              \
        &&SELFBALANCE,                          \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&POP, /* 50, */                        \
        &&MLOAD,                                \
        &&MSTORE,                               \
        &&MSTORE8,                              \
        &&SLOAD,                                \
        &&SSTORE,                               \
        &&JUMP,                                 \
        &&JUMPI,                                \
        &&PC,                                   \
        &&MSIZE,                                \
        &&GAS,                                  \
        &&JUMPDEST,                             \
        &&BEGINDATA,                            \
        &&BEGINSUB,                             \
        &&INVALID,                              \
        &&INVALID,                              \
        &&PUSH1, /* 60, */                      \
        &&PUSH2,                                \
        &&PUSH3,                                \
        &&PUSH4,                                \
        &&PUSH5,                                \
        &&PUSH6,                                \
        &&PUSH7,                                \
        &&PUSH8,                                \
        &&PUSH9,                                \
        &&PUSH10,                               \
        &&PUSH11,                               \
        &&PUSH12,                               \
        &&PUSH13,                               \
        &&PUSH14,                               \
        &&PUSH15,                               \
        &&PUSH16,                               \
        &&PUSH17, /* 70, */                     \
        &&PUSH18,                               \
        &&PUSH19,                               \
        &&PUSH20,                               \
        &&PUSH21,                               \
        &&PUSH22,                               \
        &&PUSH23,                               \
        &&PUSH24,                               \
        &&PUSH25,                               \
        &&PUSH26,                               \
        &&PUSH27,                               \
        &&PUSH28,                               \
        &&PUSH29,                               \
        &&PUSH30,                               \
        &&PUSH31,                               \
        &&PUSH32,                               \
        &&DUP1, /* 80, */                       \
        &&DUP2,                                 \
        &&DUP3,                                 \
        &&DUP4,                                 \
        &&DUP5,                                 \
        &&DUP6,                                 \
        &&DUP7,                                 \
        &&DUP8,                                 \
        &&DUP9,                                 \
        &&DUP10,                                \
        &&DUP11,                                \
        &&DUP12,                                \
        &&DUP13,                                \
        &&DUP14,                                \
        &&DUP15,                                \
        &&DUP16,                                \
        &&SWAP1, /* 90, */                      \
        &&SWAP2,                                \
        &&SWAP3,                                \
        &&SWAP4,                                \
        &&SWAP5,                                \
        &&SWAP6,                                \
        &&SWAP7,                                \
        &&SWAP8,                                \
        &&SWAP9,                                \
        &&SWAP10,                               \
        &&SWAP11,                               \
        &&SWAP12,                               \
        &&SWAP13,                               \
        &&SWAP14,                               \
        &&SWAP15,                               \
        &&SWAP16,                               \
        &&LOG0, /* A0, */                       \
        &&LOG1,                                 \
        &&LOG2,                                 \
        &&LOG3,                                 \
        &&LOG4,                                 \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&PUSHC,                                \
        &&JUMPC,                                \
        &&JUMPCI,                               \
        &&INVALID,                              \
        &&JUMPTO, /* B0, */                     \
        &&JUMPIF,                               \
        &&JUMPSUB,                              \
        &&JUMPV,                                \
        &&JUMPSUBV,                             \
        &&BEGINSUB,                             \
        &&BEGINDATA,                            \
        &&RETURNSUB,                            \
        &&PUTLOCAL,                             \
        &&GETLOCAL,                             \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID, /* C0, */                    \
        &&XADD,                                 \
        &&XMUL,                                 \
        &&XSUB,                                 \
        &&XDIV,                                 \
        &&XSDIV,                                \
        &&XMOD,                                 \
        &&XSMOD,                                \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&XLT, /* D0 */                         \
        &&XGT,                                  \
        &&XSLT,                                 \
        &&XSGT,                                 \
        &&XEQ,                                  \
        &&XISZERO,                              \
        &&XAND,                                 \
        &&XOOR,                                 \
        &&XXOR,                                 \
        &&XNOT,                                 \
        &&INVALID,                              \
        &&XSHL,                                 \
        &&XSHR,                                 \
        &&XSAR,                                 \
        &&XROL,                                 \
        &&XROR,                                 \
        &&XPUSH, /* E0, */                      \
        &&XMLOAD,                               \
        &&XMSTORE,                              \
        &&INVALID,                              \
        &&XSLOAD,                               \
        &&XSSTORE,                              \
        &&XVTOWIDE,                             \
        &&XWIDETOV,                             \
        &&XGET,                                 \
        &&XPUT,                                 \
        &&XSWIZZLE,                             \
        &&XSHUFFLE,                             \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&CREATE, /* F0, */                     \
        &&CALL,                                 \
        &&CALLCODE,                             \
        &&RETURN,                               \
        &&DELEGATECALL,                         \
        &&CREATE2,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&INVALID,                              \
        &&STATICCALL,                           \
        &&INVALID,                              \
        &&INVALID,                              \
        &&REVERT,                               \
        &&INVALID,                              \
        &&SELFDESTRUCT,                         \
    };

#define DO_CASES        \
    fetchInstruction(); \
    goto* jumpTable[(int)m_OP];
#define CASE(name) \
    name:
#define NEXT            \
    ++m_PC;             \
    fetchInstruction(); \
    goto* jumpTable[(int)m_OP];
#define CONTINUE        \
    fetchInstruction(); \
    goto* jumpTable[(int)m_OP];
#define BREAK return;
#define DEFAULT
#define WHILE_CASES

#else
#error No opcode dispatch configured
#endif
    }
    }
