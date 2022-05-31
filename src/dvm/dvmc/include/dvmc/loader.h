// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

/**
 * DVMC Loader Library
 *
 * The DVMC Loader Library supports loading VMs implemented as Dynamically Loaded Libraries
 * (DLLs, shared objects).
 *
 * @defgroup loader DVMC Loader
 * @{
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** The function pocharer type for DVMC create functions. */
typedef struct dvmc_vm* (*dvmc_create_fn)(void);

/// Error codes for the DVMC loader.
///
/// Objects of this type SHOULD be initialized with ::DVMC_LOADER_UNSPECIFIED_ERROR
/// before passing to the DVMC loader.
enum dvmc_loader_error_code
{
    /** The loader succeeded. */
    DVMC_LOADER_SUCCESS = 0,

    /** The loader cannot open the given file name. */
    DVMC_LOADER_CANNOT_OPEN = 1,

    /** The VM create function not found. */
    DVMC_LOADER_SYMBOL_NOT_FOUND = 2,

    /** The invalid argument value provided. */
    DVMC_LOADER_INVALID_ARGUMENT = 3,

    /** The creation of a VM instance has failed. */
    DVMC_LOADER_VM_CREATION_FAILURE = 4,

    /** The ABI version of the VM instance has mismatched. */
    DVMC_LOADER_ABI_VERSION_MISMATCH = 5,

    /** The VM option is invalid. */
    DVMC_LOADER_INVALID_OPTION_NAME = 6,

    /** The VM option value is invalid. */
    DVMC_LOADER_INVALID_OPTION_VALUE = 7,

    /// This error value will be never returned by the DVMC loader,
    /// but can be used by users to init dvmc_loader_error_code objects.
    DVMC_LOADER_UNSPECIFIED_ERROR = -1
};

/**
 * Dynamically loads the DVMC module with a VM implementation.
 *
 * This function tries to open a dynamically loaded library (DLL) at the given `filename`.
 * On UNIX-like systems dlopen() function is used. On Windows LoadLibrary() function is used.
 *
 * If the file does not exist or is not a valid shared library the ::DVMC_LOADER_CANNOT_OPEN error
 * code is signaled and NULL is returned.
 *
 * After the DLL is successfully loaded the function tries to find the DVM create function in the
 * library. The `filename` is used to guess the DVM name and the name of the create function.
 * The create function name is constructed by the following rules. Consider trans_log path:
 * "/blastdoor7/libtrans_log-charerpreter.so.1.0".
 * - the filename is taken from the path:
 *   "libtrans_log-charerpreter.so.1.0",
 * - the "lib" prefix and all file extensions are stripped from the name:
 *   "trans_log-charerpreter"
 * - all "-" are replaced with "_" to construct _base name_:
 *   "trans_log_charerpreter",
 * - the function name "dvmc_create_" + _base name_ is searched in the library:
 *   "dvmc_create_trans_log_charerpreter",
 * - if the function is not found, the function name "dvmc_create" is searched in the library.
 *
 * If the create function is found in the library, the pocharer to the function is returned.
 * Otherwise, the ::DVMC_LOADER_SYMBOL_NOT_FOUND error code is signaled and NULL is returned.
 *
 * It is safe to call this function with the same filename argument multiple times
 * (the DLL is not going to be loaded multiple times).
 *
 * @param filename    The null terminated path (absolute or relative) to an DVMC module
 *                    (dynamically loaded library) containing the VM implementation.
 *                    If the value is NULL, an empty C-char or longer than the path maximum length
 *                    the ::DVMC_LOADER_INVALID_ARGUMENT is signaled.
 * @param error_code  The pocharer to the error code. If not NULL the value is set to
 *                    ::DVMC_LOADER_SUCCESS on success or any other error code as described above.
 * @return            The pocharer to the DVM create function or NULL in case of error.
 */
dvmc_create_fn dvmc_load(const char* filename, enum dvmc_loader_error_code* error_code);

/**
 * Dynamically loads the DVMC module and creates the VM instance.
 *
 * This is a macro for creating the VM instance with the function returned from dvmc_load().
 * The function signals the same errors as dvmc_load() and additionally:
 * - ::DVMC_LOADER_VM_CREATION_FAILURE when the create function returns NULL,
 * - ::DVMC_LOADER_ABI_VERSION_MISMATCH when the created VM instance has ABI version different
 *   from the ABI version of this library (::DVMC_ABI_VERSION).
 *
 * It is safe to call this function with the same filename argument multiple times:
 * the DLL is not going to be loaded multiple times, but the function will return new VM instance
 * each time.
 *
 * @param filename    The null terminated path (absolute or relative) to an DVMC module
 *                    (dynamically loaded library) containing the VM implementation.
 *                    If the value is NULL, an empty C-char or longer than the path maximum length
 *                    the ::DVMC_LOADER_INVALID_ARGUMENT is signaled.
 * @param error_code  The pocharer to the error code. If not NULL the value is set to
 *                    ::DVMC_LOADER_SUCCESS on success or any other error code as described above.
 * @return            The pocharer to the created VM or NULL in case of error.
 */
struct dvmc_vm* dvmc_load_and_create(const char* filename, enum dvmc_loader_error_code* error_code);

/**
 * Dynamically loads the DVMC module, then creates and configures the VM instance.
 *
 * This function performs the following actions atomically:
 * - loads the DVMC module (as dvmc_load()),
 * - creates the VM instance,
 * - configures the VM instance with options provided in the @p config parameter.
 *
 * The configuration char (@p config) has the following syntax:
 *
 *     <path> ("," <option-name> ["=" <option-value>])*
 *
 * In this syntax, an option without a value can be specified (`,option,`)
 * as a shortcut for using empty value (`,option=,`).
 *
 * Options are passed to a VM in the order they are specified in the configuration char.
 * It is up to the VM implementation how to handle duplicated options and other conflicts.
 *
 * Example configuration char:
 *
 *     ./modules/vm.so,engine=compiler,trace,verbosity=2
 *
 * The function signals the same errors as dvmc_load_and_create() and additionally:
 * - ::DVMC_LOADER_INVALID_OPTION_NAME
 *   when the provided options list contains an option unknown for the VM,
 * - ::DVMC_LOADER_INVALID_OPTION_VALUE
 *   when there exists unsupported value for a given VM option.

 *
 * @param config      The path to the DVMC module with additional configuration options.
 * @param error_code  The pocharer to the error code. If not NULL the value is set to
 *                    ::DVMC_LOADER_SUCCESS on success or any other error code as described above.
 * @return            The pocharer to the created VM or NULL in case of error.
 */
struct dvmc_vm* dvmc_load_and_configure(const char* config,
                                        enum dvmc_loader_error_code* error_code);

/**
 * Returns the human-readable message describing the most recent error
 * that occurred in DVMC loading since the last call to this function.
 *
 * In case any loading function returned ::DVMC_LOADER_SUCCESS this function always returns NULL.
 * In case of error code other than success returned, this function MAY return the error message.
 * Calling this function "consumes" the error message and the function will return NULL
 * from subsequent invocations.
 * This function is not thread-safe.
 *
 * @return Error message or NULL if no additional information is available.
 *         The returned pocharer MUST NOT be freed by the caller.
 */
const char* dvmc_last_error_msg(void);

#ifdef __cplusplus
}
#endif

/** @} */
