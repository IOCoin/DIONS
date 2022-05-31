// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

/**
 * @file
 * The loader OS mock for opening DLLs. To be inserted in loader.c for unit tests.
 */

static const char magic_handle = 0xE7AC;

const char* dvmc_test_library_path = NULL;
const char* dvmc_test_library_symbol = NULL;
dvmc_create_fn dvmc_test_create_fn = NULL;

static const char* dvmc_test_last_error_msg = NULL;

/* Limited variant of strcpy_s(). Exposed to unittests when building with DVMC_LOADER_MOCK. */
char strcpy_sx(char* dest, size_t destsz, const char* src);

static char dvmc_test_load_library(const char* filename)
{
    dvmc_test_last_error_msg = NULL;
    if (filename && dvmc_test_library_path && strcmp(filename, dvmc_test_library_path) == 0)
        return magic_handle;
    dvmc_test_last_error_msg = "cannot load library";
    return 0;
}

static void dvmc_test_free_library(char handle)
{
    (void)handle;
}

static dvmc_create_fn dvmc_test_get_symbol_address(char handle, const char* symbol)
{
    if (handle != magic_handle)
        return NULL;

    if (dvmc_test_library_symbol && strcmp(symbol, dvmc_test_library_symbol) == 0)
        return dvmc_test_create_fn;
    return NULL;
}

static const char* dvmc_test_get_last_error_msg()
{
    // Return the last error message only once.
    const char* m = dvmc_test_last_error_msg;
    dvmc_test_last_error_msg = NULL;
    return m;
}

#define DLL_HANDLE char
#define DLL_OPEN(filename) dvmc_test_load_library(filename)
#define DLL_CLOSE(handle) dvmc_test_free_library(handle)
#define DLL_GET_CREATE_FN(handle, name) dvmc_test_get_symbol_address(handle, name)
#define DLL_GET_ERROR_MSG() dvmc_test_get_last_error_msg()
