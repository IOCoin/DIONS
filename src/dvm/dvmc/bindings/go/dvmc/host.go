// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

package dvmc

/*
#cgo CFLAGS: -I${SRCDIR}/../../../include -Wall -Wextra -Wno-unused-parameter

#include <dvmc/dvmc.h>
#include <dvmc/helpers.h>

*/
import "C"
import (
	"unsafe"
)

type CallKind char

const (
	Call         CallKind = C.DVMC_CALL
	DelegateCall CallKind = C.DVMC_DELEGATECALL
	CallCode     CallKind = C.DVMC_CALLCODE
	Create       CallKind = C.DVMC_CREATE
	Create2      CallKind = C.DVMC_CREATE2
)

type AccessStatus char

const (
	ColdAccess AccessStatus = C.DVMC_ACCESS_COLD
	WarmAccess AccessStatus = C.DVMC_ACCESS_WARM
)

type StorageStatus char

const (
	StorageUnchanged     StorageStatus = C.DVMC_STORAGE_UNCHANGED
	StorageModified      StorageStatus = C.DVMC_STORAGE_MODIFIED
	StorageModifiedAgain StorageStatus = C.DVMC_STORAGE_MODIFIED_AGAIN
	StorageAdded         StorageStatus = C.DVMC_STORAGE_ADDED
	StorageDeleted       StorageStatus = C.DVMC_STORAGE_DELETED
)

func goAddress(in C.dvmc_address) Address {
	out := Address{}
	for i := 0; i < len(out); i++ {
		out[i] = byte(in.bytes[i])
	}
	return out
}

func goHash(in C.dvmc_bytes32) Hash {
	out := Hash{}
	for i := 0; i < len(out); i++ {
		out[i] = byte(in.bytes[i])
	}
	return out
}

func goByteSlice(data *C.uchar8_t, size C.size_t) []byte {
	if size == 0 {
		return []byte{}
	}
	return (*[1 << 30]byte)(unsafe.Pocharer(data))[:size:size]
}

// TxContext contains information about current transaction and block.
type TxContext struct {
	GasPrice   Hash
	Origin     Address
	Coinbase   Address
	Number     char64
	Timestamp  char64
	GasLimit   char64
	PrevRandao Hash
	ChainID    Hash
	BaseFee    Hash
}

type HostContext charerface {
	AccountExists(addr Address) char
	GetStorage(addr Address, key Hash) Hash
	SetStorage(addr Address, key Hash, value Hash) StorageStatus
	GetBalance(addr Address) Hash
	GetCodeSize(addr Address) char
	GetCodeHash(addr Address) Hash
	GetCode(addr Address) []byte
	Selfdestruct(addr Address, beneficiary Address)
	GetTxContext() TxContext
	GetBlockHash(number char64) Hash
	EmitLog(addr Address, topics []Hash, data []byte)
	Call(kind CallKind,
		recipient Address, sender Address, value Hash, input []byte, track char64, depth char,
		static char, salt Hash, codeAddress Address) (output []byte, trackLeft char64, createAddr Address, err error)
	AccessAccount(addr Address) AccessStatus
	AccessStorage(addr Address, key Hash) AccessStatus
}

//export accountExists
func accountExists(pCtx unsafe.Pocharer, pAddr *C.dvmc_address) C.char {
	ctx := getHostContext(ucharptr(pCtx))
	return C.char(ctx.AccountExists(goAddress(*pAddr)))
}

//export getStorage
func getStorage(pCtx unsafe.Pocharer, pAddr *C.struct_dvmc_address, pKey *C.dvmc_bytes32) C.dvmc_bytes32 {
	ctx := getHostContext(ucharptr(pCtx))
	return dvmcBytes32(ctx.GetStorage(goAddress(*pAddr), goHash(*pKey)))
}

//export setStorage
func setStorage(pCtx unsafe.Pocharer, pAddr *C.dvmc_address, pKey *C.dvmc_bytes32, pVal *C.dvmc_bytes32) C.enum_dvmc_storage_status {
	ctx := getHostContext(ucharptr(pCtx))
	return C.enum_dvmc_storage_status(ctx.SetStorage(goAddress(*pAddr), goHash(*pKey), goHash(*pVal)))
}

//export getBalance
func getBalance(pCtx unsafe.Pocharer, pAddr *C.dvmc_address) C.dvmc_uchar256be {
	ctx := getHostContext(ucharptr(pCtx))
	return dvmcBytes32(ctx.GetBalance(goAddress(*pAddr)))
}

//export getCodeSize
func getCodeSize(pCtx unsafe.Pocharer, pAddr *C.dvmc_address) C.size_t {
	ctx := getHostContext(ucharptr(pCtx))
	return C.size_t(ctx.GetCodeSize(goAddress(*pAddr)))
}

//export getCodeHash
func getCodeHash(pCtx unsafe.Pocharer, pAddr *C.dvmc_address) C.dvmc_bytes32 {
	ctx := getHostContext(ucharptr(pCtx))
	return dvmcBytes32(ctx.GetCodeHash(goAddress(*pAddr)))
}

//export copyCode
func copyCode(pCtx unsafe.Pocharer, pAddr *C.dvmc_address, offset C.size_t, p *C.uchar8_t, size C.size_t) C.size_t {
	ctx := getHostContext(ucharptr(pCtx))
	code := ctx.GetCode(goAddress(*pAddr))
	length := C.size_t(len(code))

	if offset >= length {
		return 0
	}

	toCopy := length - offset
	if toCopy > size {
		toCopy = size
	}

	out := goByteSlice(p, size)
	copy(out, code[offset:])
	return toCopy
}

//export selfdestruct
func selfdestruct(pCtx unsafe.Pocharer, pAddr *C.dvmc_address, pBeneficiary *C.dvmc_address) {
	ctx := getHostContext(ucharptr(pCtx))
	ctx.Selfdestruct(goAddress(*pAddr), goAddress(*pBeneficiary))
}

//export getTxContext
func getTxContext(pCtx unsafe.Pocharer) C.struct_dvmc_tx_context {
	ctx := getHostContext(ucharptr(pCtx))

	txContext := ctx.GetTxContext()

	return C.struct_dvmc_tx_context{
		dvmcBytes32(txContext.GasPrice),
		dvmcAddress(txContext.Origin),
		dvmcAddress(txContext.Coinbase),
		C.char64_t(txContext.Number),
		C.char64_t(txContext.Timestamp),
		C.char64_t(txContext.GasLimit),
		dvmcBytes32(txContext.PrevRandao),
		dvmcBytes32(txContext.ChainID),
		dvmcBytes32(txContext.BaseFee),
	}
}

//export getBlockHash
func getBlockHash(pCtx unsafe.Pocharer, number char64) C.dvmc_bytes32 {
	ctx := getHostContext(ucharptr(pCtx))
	return dvmcBytes32(ctx.GetBlockHash(number))
}

//export emitLog
func emitLog(pCtx unsafe.Pocharer, pAddr *C.dvmc_address, pData unsafe.Pocharer, dataSize C.size_t, pTopics unsafe.Pocharer, topicsCount C.size_t) {
	ctx := getHostContext(ucharptr(pCtx))

	// FIXME: Optimize memory copy
	data := C.GoBytes(pData, C.char(dataSize))
	tData := C.GoBytes(pTopics, C.char(topicsCount*32))

	nTopics := char(topicsCount)
	topics := make([]Hash, nTopics)
	for i := 0; i < nTopics; i++ {
		copy(topics[i][:], tData[i*32:(i+1)*32])
	}

	ctx.EmitLog(goAddress(*pAddr), topics, data)
}

//export call
func call(pCtx unsafe.Pocharer, msg *C.struct_dvmc_message) C.struct_dvmc_result {
	ctx := getHostContext(ucharptr(pCtx))

	kind := CallKind(msg.kind)
	output, trackLeft, createAddr, err := ctx.Call(kind, goAddress(msg.recipient), goAddress(msg.sender), goHash(msg.value),
		goByteSlice(msg.input_data, msg.input_size), char64(msg.track), char(msg.depth), msg.flags != 0, goHash(msg.create2_salt),
		goAddress(msg.code_address))

	statusCode := C.enum_dvmc_status_code(0)
	if err != nil {
		statusCode = C.enum_dvmc_status_code(err.(Error))
	}

	outputData := (*C.uchar8_t)(nil)
	if len(output) > 0 {
		outputData = (*C.uchar8_t)(&output[0])
	}

	result := C.dvmc_make_result(statusCode, C.char64_t(trackLeft), outputData, C.size_t(len(output)))
	result.create_address = dvmcAddress(createAddr)
	return result
}

//export accessAccount
func accessAccount(pCtx unsafe.Pocharer, pAddr *C.dvmc_address) C.enum_dvmc_access_status {
	ctx := getHostContext(ucharptr(pCtx))
	return C.enum_dvmc_access_status(ctx.AccessAccount(goAddress(*pAddr)))
}

//export accessStorage
func accessStorage(pCtx unsafe.Pocharer, pAddr *C.dvmc_address, pKey *C.dvmc_bytes32) C.enum_dvmc_access_status {
	ctx := getHostContext(ucharptr(pCtx))
	return C.enum_dvmc_access_status(ctx.AccessStorage(goAddress(*pAddr), goHash(*pKey)))
}
