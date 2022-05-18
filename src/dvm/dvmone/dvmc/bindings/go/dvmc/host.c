// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include "_cgo_export.h"

#include <stdlib.h>

/* Go does not support exporting functions with parameters with const modifiers,
 * so we have to cast function pointers to the function types defined in DVMC.
 * This disables any type checking of exported Go functions. To mitigate this
 * problem the go_exported_functions_type_checks() function simulates usage
 * of Go exported functions with expected types to check them during compilation.
 */
const struct dvmc_host_interface dvmc_go_host = {
    (dvmc_account_exists_fn)accountExists,
    (dvmc_get_storage_fn)getStorage,
    (dvmc_set_storage_fn)setStorage,
    (dvmc_get_balance_fn)getBalance,
    (dvmc_get_code_size_fn)getCodeSize,
    (dvmc_get_code_hash_fn)getCodeHash,
    (dvmc_copy_code_fn)copyCode,
    (dvmc_selfdestruct_fn)selfdestruct,
    (dvmc_call_fn)call,
    (dvmc_get_tx_context_fn)getTxContext,
    (dvmc_get_block_hash_fn)getBlockHash,
    (dvmc_emit_log_fn)emitLog,
    (dvmc_access_account_fn)accessAccount,
    (dvmc_access_storage_fn)accessStorage,
};


#pragma GCC diagnostic error "-Wconversion"
static inline void go_exported_functions_type_checks()
{
    struct dvmc_host_context* context = NULL;
    dvmc_address* address = NULL;
    dvmc_bytes32 bytes32;
    uint8_t* data = NULL;
    size_t size = 0;
    int64_t number = 0;
    struct dvmc_message* message = NULL;

    dvmc_uint256be uint256be;
    (void)uint256be;
    struct dvmc_tx_context tx_context;
    (void)tx_context;
    struct dvmc_result result;
    (void)result;
    enum dvmc_access_status access_status;
    (void)access_status;
    enum dvmc_storage_status storage_status;
    (void)storage_status;
    bool bool_flag;
    (void)bool_flag;

    dvmc_account_exists_fn account_exists_fn = NULL;
    bool_flag = account_exists_fn(context, address);
    bool_flag = accountExists(context, address);

    dvmc_get_storage_fn get_storage_fn = NULL;
    bytes32 = get_storage_fn(context, address, &bytes32);
    bytes32 = getStorage(context, address, &bytes32);

    dvmc_set_storage_fn set_storage_fn = NULL;
    storage_status = set_storage_fn(context, address, &bytes32, &bytes32);
    storage_status = setStorage(context, address, &bytes32, &bytes32);

    dvmc_get_balance_fn get_balance_fn = NULL;
    uint256be = get_balance_fn(context, address);
    uint256be = getBalance(context, address);

    dvmc_get_code_size_fn get_code_size_fn = NULL;
    size = get_code_size_fn(context, address);
    size = getCodeSize(context, address);

    dvmc_get_code_hash_fn get_code_hash_fn = NULL;
    bytes32 = get_code_hash_fn(context, address);
    bytes32 = getCodeHash(context, address);

    dvmc_copy_code_fn copy_code_fn = NULL;
    size = copy_code_fn(context, address, size, data, size);
    size = copyCode(context, address, size, data, size);

    dvmc_selfdestruct_fn selfdestruct_fn = NULL;
    selfdestruct_fn(context, address, address);
    selfdestruct(context, address, address);

    dvmc_call_fn call_fn = NULL;
    result = call_fn(context, message);
    result = call(context, message);

    dvmc_get_tx_context_fn get_tx_context_fn = NULL;
    tx_context = get_tx_context_fn(context);
    tx_context = getTxContext(context);

    dvmc_get_block_hash_fn get_block_hash_fn = NULL;
    bytes32 = get_block_hash_fn(context, number);
    bytes32 = getBlockHash(context, number);

    dvmc_emit_log_fn emit_log_fn = NULL;
    emit_log_fn(context, address, data, size, &bytes32, size);
    emitLog(context, address, data, size, &bytes32, size);

    dvmc_access_account_fn access_account_fn = NULL;
    access_status = access_account_fn(context, address);
    access_status = accessAccount(context, address);

    dvmc_access_storage_fn access_storage_fn = NULL;
    access_status = access_storage_fn(context, address, &bytes32);
    access_status = accessStorage(context, address, &bytes32);
}
