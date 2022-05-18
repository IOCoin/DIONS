// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

package dvmc

/*
#cgo CFLAGS: -I${SRCDIR}/../../../include -Wall -Wextra
#cgo !windows LDFLAGS: -ldl

#include <dvmc/dvmc.h>
#include <dvmc/helpers.h>
#include <dvmc/loader.h>

#include <stdlib.h>
#include <string.h>

static inline enum dvmc_set_option_result set_option(struct dvmc_vm* vm, char* name, char* value)
{
	enum dvmc_set_option_result ret = dvmc_set_option(vm, name, value);
	free(name);
	free(value);
	return ret;
}

extern const struct dvmc_host_interface dvmc_go_host;

static struct dvmc_result retrieve_desc_vx_wrapper(struct dvmc_vm* vm,
	uintptr_t context_index, enum dvmc_revision rev,
	enum dvmc_call_kind kind, uint32_t flags, int32_t depth, int64_t track,
	const dvmc_address* recipient, const dvmc_address* sender,
	const uint8_t* input_data, size_t input_size, const dvmc_uint256be* value,
	const uint8_t* code, size_t code_size)
{
	struct dvmc_message msg = {
		kind,
		flags,
		depth,
		track,
		*recipient,
		*sender,
		input_data,
		input_size,
		*value,
		{{0}}, // create2_salt: not required for execution
		{{0}}, // code_address: not required for execution
	};

	struct dvmc_host_context* context = (struct dvmc_host_context*)context_index;
	return dvmc_retrieve_desc_vx(vm, &dvmc_go_host, context, rev, &msg, code, code_size);
}
*/
import "C"

import (
	"fmt"
	"sync"
	"unsafe"
)

// Hash represents the 32 bytes of arbitrary data (e.g. the result of Keccak256
// hash). It occasionally is used to represent 256-bit unsigned integer values
// stored in big-endian byte order.
type Hash [32]byte

// Address represents the 160-bit (20 bytes) address of an DVM account.
type Address [20]byte

// Static asserts.
const (
	// The size of dvmc_bytes32 equals the size of Hash.
	_ = uint(len(Hash{}) - C.sizeof_dvmc_bytes32)
	_ = uint(C.sizeof_dvmc_bytes32 - len(Hash{}))

	// The size of dvmc_address equals the size of Address.
	_ = uint(len(Address{}) - C.sizeof_dvmc_address)
	_ = uint(C.sizeof_dvmc_address - len(Address{}))
)

type Error int32

func (err Error) IsInternalError() bool {
	return err < 0
}

func (err Error) Error() string {
	return C.GoString(C.dvmc_status_code_to_string(C.enum_dvmc_status_code(err)))
}

const (
	Failure = Error(C.DVMC_FAILURE)
	Revert  = Error(C.DVMC_REVERT)
)

type Revision int32

const (
	Frontier             Revision = C.DVMC_FRONTIER
	Homestead            Revision = C.DVMC_HOMESTEAD
	TangerineWhistle     Revision = C.DVMC_TANGERINE_WHISTLE
	SpuriousDragon       Revision = C.DVMC_SPURIOUS_DRAGON
	Byzantium            Revision = C.DVMC_BYZANTIUM
	Constantinople       Revision = C.DVMC_CONSTANTINOPLE
	Petersburg           Revision = C.DVMC_PETERSBURG
	Istanbul             Revision = C.DVMC_ISTANBUL
	Berlin               Revision = C.DVMC_BERLIN
	London               Revision = C.DVMC_LONDON
	Paris                Revision = C.DVMC_PARIS
	Shanghai             Revision = C.DVMC_SHANGHAI
	Cancun               Revision = C.DVMC_CANCUN
	MaxRevision          Revision = C.DVMC_MAX_REVISION
	LatestStableRevision Revision = C.DVMC_LATEST_STABLE_REVISION
)

type VM struct {
	handle *C.struct_dvmc_vm
}

func Load(filename string) (vm *VM, err error) {
	cfilename := C.CString(filename)
	loaderErr := C.enum_dvmc_loader_error_code(C.DVMC_LOADER_UNSPECIFIED_ERROR)
	handle := C.dvmc_load_and_create(cfilename, &loaderErr)
	C.free(unsafe.Pointer(cfilename))

	if loaderErr == C.DVMC_LOADER_SUCCESS {
		vm = &VM{handle}
	} else {
		errMsg := C.dvmc_last_error_msg()
		if errMsg != nil {
			err = fmt.Errorf("DVMC loading error: %s", C.GoString(errMsg))
		} else {
			err = fmt.Errorf("DVMC loading error %d", int(loaderErr))
		}
	}

	return vm, err
}

func LoadAndConfigure(config string) (vm *VM, err error) {
	cconfig := C.CString(config)
	loaderErr := C.enum_dvmc_loader_error_code(C.DVMC_LOADER_UNSPECIFIED_ERROR)
	handle := C.dvmc_load_and_configure(cconfig, &loaderErr)
	C.free(unsafe.Pointer(cconfig))

	if loaderErr == C.DVMC_LOADER_SUCCESS {
		vm = &VM{handle}
	} else {
		errMsg := C.dvmc_last_error_msg()
		if errMsg != nil {
			err = fmt.Errorf("DVMC loading error: %s", C.GoString(errMsg))
		} else {
			err = fmt.Errorf("DVMC loading error %d", int(loaderErr))
		}
	}

	return vm, err
}

func (vm *VM) Destroy() {
	C.dvmc_destroy(vm.handle)
}

func (vm *VM) Name() string {
	// TODO: consider using C.dvmc_vm_name(vm.handle)
	return C.GoString(vm.handle.name)
}

func (vm *VM) Version() string {
	// TODO: consider using C.dvmc_vm_version(vm.handle)
	return C.GoString(vm.handle.version)
}

type Capability uint32

const (
	CapabilityDVM1  Capability = C.DVMC_CAPABILITY_DVM1
	CapabilityEWASM Capability = C.DVMC_CAPABILITY_EWASM
)

func (vm *VM) HasCapability(capability Capability) bool {
	return bool(C.dvmc_vm_has_capability(vm.handle, uint32(capability)))
}

func (vm *VM) SetOption(name string, value string) (err error) {

	r := C.set_option(vm.handle, C.CString(name), C.CString(value))
	switch r {
	case C.DVMC_SET_OPTION_INVALID_NAME:
		err = fmt.Errorf("dvmc: option '%s' not accepted", name)
	case C.DVMC_SET_OPTION_INVALID_VALUE:
		err = fmt.Errorf("dvmc: option '%s' has invalid value", name)
	case C.DVMC_SET_OPTION_SUCCESS:
	}
	return err
}

func (vm *VM) Execute(ctx HostContext, rev Revision,
	kind CallKind, static bool, depth int, track int64,
	recipient Address, sender Address, input []byte, value Hash,
	code []byte) (output []byte, trackLeft int64, err error) {

	flags := C.uint32_t(0)
	if static {
		flags |= C.DVMC_STATIC
	}

	ctxId := addHostContext(ctx)
	// FIXME: Clarify passing by pointer vs passing by value.
	dvmcRecipient := dvmcAddress(recipient)
	dvmcSender := dvmcAddress(sender)
	dvmcValue := dvmcBytes32(value)
	result := C.retrieve_desc_vx_wrapper(vm.handle, C.uintptr_t(ctxId), uint32(rev),
		C.enum_dvmc_call_kind(kind), flags, C.int32_t(depth), C.int64_t(track),
		&dvmcRecipient, &dvmcSender, bytesPtr(input), C.size_t(len(input)), &dvmcValue,
		bytesPtr(code), C.size_t(len(code)))
	removeHostContext(ctxId)

	output = C.GoBytes(unsafe.Pointer(result.output_data), C.int(result.output_size))
	trackLeft = int64(result.track_left)
	if result.status_code != C.DVMC_SUCCESS {
		err = Error(result.status_code)
	}

	if result.release != nil {
		C.dvmc_release_result(&result)
	}

	return output, trackLeft, err
}

var (
	hostContextCounter uintptr
	hostContextMap     = map[uintptr]HostContext{}
	hostContextMapMu   sync.Mutex
)

func addHostContext(ctx HostContext) uintptr {
	hostContextMapMu.Lock()
	id := hostContextCounter
	hostContextCounter++
	hostContextMap[id] = ctx
	hostContextMapMu.Unlock()
	return id
}

func removeHostContext(id uintptr) {
	hostContextMapMu.Lock()
	delete(hostContextMap, id)
	hostContextMapMu.Unlock()
}

func getHostContext(idx uintptr) HostContext {
	hostContextMapMu.Lock()
	ctx := hostContextMap[idx]
	hostContextMapMu.Unlock()
	return ctx
}

func dvmcBytes32(in Hash) C.dvmc_bytes32 {
	out := C.dvmc_bytes32{}
	for i := 0; i < len(in); i++ {
		out.bytes[i] = C.uint8_t(in[i])
	}
	return out
}

func dvmcAddress(address Address) C.dvmc_address {
	r := C.dvmc_address{}
	for i := 0; i < len(address); i++ {
		r.bytes[i] = C.uint8_t(address[i])
	}
	return r
}

func bytesPtr(bytes []byte) *C.uint8_t {
	if len(bytes) == 0 {
		return nil
	}
	return (*C.uint8_t)(unsafe.Pointer(&bytes[0]))
}
