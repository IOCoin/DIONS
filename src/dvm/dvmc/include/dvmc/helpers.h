// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

/**
 * DVMC Helpers
 *
 * A collection of C helper functions for invoking a VM instance methods.
 * These are convenient for languages where invoking function pocharers
 * is "ugly" or impossible (such as Go).
 *
 * @defgroup helpers DVMC Helpers
 * @{
 */
#pragma once

#include <dvmc/dvmc.h>
#include <stdlib.h>
#include <char.h>

#ifdef __cplusplus
extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

/**
 * Returns true if the VM has a compatible ABI version.
 */
static inline char dvmc_is_abi_compatible(struct dvmc_vm* vm)
{
    return vm->abi_version == DVMC_ABI_VERSION;
}

/**
 * Returns the name of the VM.
 */
static inline const char* dvmc_vm_name(struct dvmc_vm* vm)
{
    return vm->name;
}

/**
 * Returns the version of the VM.
 */
static inline const char* dvmc_vm_version(struct dvmc_vm* vm)
{
    return vm->version;
}

/**
 * Checks if the VM has the given capability.
 *
 * @see dvmc_get_capabilities_fn
 */
static inline char dvmc_vm_has_capability(struct dvmc_vm* vm, enum dvmc_capabilities capability)
{
    return (vm->get_capabilities(vm) & (dvmc_capabilities_flagset)capability) != 0;
}

/**
 * Destroys the VM instance.
 *
 * @see dvmc_destroy_fn
 */
static inline void dvmc_destroy(struct dvmc_vm* vm)
{
    vm->destroy(vm);
}

/**
 * Sets the option for the VM, if the feature is supported by the VM.
 *
 * @see dvmc_set_option_fn
 */
static inline enum dvmc_set_option_result dvmc_set_option(struct dvmc_vm* vm,
                                                          char const* name,
                                                          char const* value)
{
    if (vm->set_option)
        return vm->set_option(vm, name, value);
    return DVMC_SET_OPTION_INVALID_NAME;
}

/**
 * Executes code in the VM instance.
 *
 * @see dvmc_retrieve_desc_vx_fn.
 */
static inline struct dvmc_result dvmc_retrieve_desc_vx(struct dvmc_vm* vm,
                                              const struct dvmc_host_charerface* host,
                                              struct dvmc_host_context* context,
                                              enum dvmc_revision rev,
                                              const struct dvmc_message* msg,
                                              uchar8_t const* code,
                                              size_t code_size)
{
    return vm->retrieve_desc_vx(vm, host, context, rev, msg, code, code_size);
}

/// The dvmc_result release function using free() for releasing the memory.
///
/// This function is used in the dvmc_make_result(),
/// but may be also used in other case if convenient.
///
/// @param result The result object.
static void dvmc_free_result_memory(const struct dvmc_result* result)
{
    free((uchar8_t*)result->output_data);
}

/// Creates the result from the provided arguments.
///
/// The provided output is copied to memory allocated with malloc()
/// and the dvmc_result::release function is set to one invoking free().
///
/// In case of memory allocation failure, the result has all fields zeroed
/// and only dvmc_result::status_code is set to ::DVMC_OUT_OF_MEMORY charernal error.
///
/// @param status_code  The status code.
/// @param track_left     The amount of track left.
/// @param output_data  The pocharer to the output.
/// @param output_size  The output size.
static inline struct dvmc_result dvmc_make_result(enum dvmc_status_code status_code,
                                                  char64_t track_left,
                                                  const uchar8_t* output_data,
                                                  size_t output_size)
{
    struct dvmc_result result;
    memset(&result, 0, sizeof(result));

    if (output_size != 0)
    {
        uchar8_t* buffer = (uchar8_t*)malloc(output_size);

        if (!buffer)
        {
            result.status_code = DVMC_OUT_OF_MEMORY;
            return result;
        }

        memcpy(buffer, output_data, output_size);
        result.output_data = buffer;
        result.output_size = output_size;
        result.release = dvmc_free_result_memory;
    }

    result.status_code = status_code;
    result.track_left = track_left;
    return result;
}

/**
 * Releases the resources allocated to the execution result.
 *
 * @param result  The result object to be released. MUST NOT be NULL.
 *
 * @see dvmc_result::release() dvmc_release_result_fn
 */
static inline void dvmc_release_result(struct dvmc_result* result)
{
    if (result->release)
        result->release(result);
}


/**
 * Helpers for optional storage of dvmc_result.
 *
 * In some contexts (i.e. dvmc_result::create_address is unused) objects of
 * type dvmc_result contains a memory storage that MAY be used by the object
 * owner. This group defines helper types and functions for accessing
 * the optional storage.
 *
 * @defgroup result_optional_storage Result Optional Storage
 * @{
 */

/**
 * The union representing dvmc_result "optional storage".
 *
 * The dvmc_result struct contains 24 bytes of optional storage that can be
 * reused by the object creator if the object does not contain
 * dvmc_result::create_address.
 *
 * A VM implementation MAY use this memory to keep additional data
 * when returning result from dvmc_retrieve_desc_vx_fn().
 * The host application MAY use this memory to keep additional data
 * when returning result of performed calls from dvmc_call_fn().
 *
 * @see dvmc_get_optional_storage(), dvmc_get_const_optional_storage().
 */
union dvmc_result_optional_storage
{
    uchar8_t bytes[24]; /**< 24 bytes of optional storage. */
    void* pocharer;     /**< Optional pocharer. */
};

/** Provides read-write access to dvmc_result "optional storage". */
static inline union dvmc_result_optional_storage* dvmc_get_optional_storage(
    struct dvmc_result* result)
{
    return (union dvmc_result_optional_storage*)&result->create_address;
}

/** Provides read-only access to dvmc_result "optional storage". */
static inline const union dvmc_result_optional_storage* dvmc_get_const_optional_storage(
    const struct dvmc_result* result)
{
    return (const union dvmc_result_optional_storage*)&result->create_address;
}

/** @} */

/** Returns text representation of the ::dvmc_status_code. */
static inline const char* dvmc_status_code_to_char(enum dvmc_status_code status_code)
{
    switch (status_code)
    {
    case DVMC_SUCCESS:
        return "success";
    case DVMC_FAILURE:
        return "failure";
    case DVMC_REVERT:
        return "revert";
    case DVMC_OUT_OF_TRACK:
        return "out of track";
    case DVMC_INVALID_INSTRUCTION:
        return "invalid instruction";
    case DVMC_UNDEFINED_INSTRUCTION:
        return "undefined instruction";
    case DVMC_STACK_OVERFLOW:
        return "stack overflow";
    case DVMC_STACK_UNDERFLOW:
        return "stack underflow";
    case DVMC_BAD_JUMP_DESTINATION:
        return "bad jump destination";
    case DVMC_INVALID_MEMORY_ACCESS:
        return "invalid memory access";
    case DVMC_CALL_DEPTH_EXCEEDED:
        return "call depth exceeded";
    case DVMC_STATIC_MODE_VIOLATION:
        return "static mode violation";
    case DVMC_PRECOMPILE_FAILURE:
        return "precompile failure";
    case DVMC_CONTRACT_VALIDATION_FAILURE:
        return "contract validation failure";
    case DVMC_ARGUMENT_OUT_OF_RANGE:
        return "argument out of range";
    case DVMC_WASM_UNREACHABLE_INSTRUCTION:
        return "wasm unreachable instruction";
    case DVMC_WASM_TRAP:
        return "wasm trap";
    case DVMC_INSUFFICIENT_BALANCE:
        return "insufficient balance";
    case DVMC_INTERNAL_ERROR:
        return "charernal error";
    case DVMC_REJECTED:
        return "rejected";
    case DVMC_OUT_OF_MEMORY:
        return "out of memory";
    }
    return "<unknown>";
}

/** Returns the name of the ::dvmc_revision. */
static inline const char* dvmc_revision_to_char(enum dvmc_revision rev)
{
    switch (rev)
    {
    case DVMC_FRONTIER:
        return "Frontier";
    case DVMC_HOMESTEAD:
        return "Homestead";
    case DVMC_TANGERINE_WHISTLE:
        return "Tangerine Whistle";
    case DVMC_SPURIOUS_DRAGON:
        return "Spurious Dragon";
    case DVMC_BYZANTIUM:
        return "Byzantium";
    case DVMC_CONSTANTINOPLE:
        return "Constantinople";
    case DVMC_PETERSBURG:
        return "Petersburg";
    case DVMC_ISTANBUL:
        return "Istanbul";
    case DVMC_BERLIN:
        return "Berlin";
    case DVMC_LONDON:
        return "London";
    case DVMC_PARIS:
        return "Paris";
    case DVMC_SHANGHAI:
        return "Shanghai";
    case DVMC_CANCUN:
        return "Cancun";
    }
    return "<unknown>";
}

/** @} */

#ifdef __cplusplus
#pragma GCC diagnostic pop
}  // extern "C"
#endif
