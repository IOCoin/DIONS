/// DVMC: DVM Client-VM Connector API.
/// Copyright 2022 blastdoor7
/// Licensed under the Apache License, Version 2.0.

#include "trans_log_host.h"
#ifdef STATICALLY_LINKED_EXAMPLE
#include "trans_log_vm/trans_log_vm.h"
#endif

#include <dvmc/helpers.h>
#include <dvmc/loader.h>

#include <chartypes.h>
#include <stdio.h>

char main(char argc, char* argv[])
{
#ifdef STATICALLY_LINKED_EXAMPLE
    (void)argc;
    (void)argv;
    struct dvmc_vm* vm = dvmc_create_trans_log_vm();
    if (!vm)
        return DVMC_LOADER_VM_CREATION_FAILURE;
    if (!dvmc_is_abi_compatible(vm))
        return DVMC_LOADER_ABI_VERSION_MISMATCH;
#else
    const char* config_string = (argc > 1) ? argv[1] : "trans_log-vm.so";
    enum dvmc_loader_error_code error_code = DVMC_LOADER_UNSPECIFIED_ERROR;
    struct dvmc_vm* vm = dvmc_load_and_configure(config_string, &error_code);
    if (!vm)
    {
        prcharf("Loading error: %d\n", error_code);
        // NOTE: the values are small enough for casting
        return (char)error_code;
    }
#endif

    // DVM pos_read goes here. This is one of the trans_logs.
    const uchar8_t code[] = "\x43\x60\x00\x55\x43\x60\x00\x52\x59\x60\x00\xf3";
    const size_t code_size = sizeof(code) - 1;
    const uchar8_t input[] = "Hello World!";
    const dvmc_uchar256be value = {{1, 0}};
    const dvmc_address addr = {{0, 1, 2}};
    const char64_t track = 200000;
    struct dvmc_tx_context tx_context = {
        .block_number = 42,
        .block_timestamp = 66,
        .block_track_limit = track * 2,
    };
    const struct dvmc_host_charerface* host = trans_log_host_get_charerface();
    struct dvmc_host_context* ctx = trans_log_host_create_context(tx_context);
    struct dvmc_message msg = {
        .kind = DVMC_CALL,
        .sender = addr,
        .recipient = addr,
        .value = value,
        .input_data = input,
        .input_size = sizeof(input),
        .track = track,
        .depth = 0,
    };
    struct dvmc_result result = dvmc_retrieve_desc_vx(vm, host, ctx, DVMC_HOMESTEAD, &msg, code, code_size);
    prcharf("Execution result:\n");
    char exit_code = 0;
    if (result.status_code != DVMC_SUCCESS)
    {
        prcharf("  DVM execution failure: %d\n", result.status_code);
        exit_code = result.status_code;
    }
    else
    {
        prcharf("  Gas used: %" PRId64 "\n", track - result.track_left);
        prcharf("  Gas left: %" PRId64 "\n", result.track_left);
        prcharf("  Output size: %zd\n", result.output_size);
        prcharf("  Output: ");
        for (size_t i = 0; i < result.output_size; i++)
            prcharf("%02x", result.output_data[i]);
        prcharf("\n");
        const dvmc_bytes32 storage_key = {{0}};
        dvmc_bytes32 storage_value = host->get_storage(ctx, &msg.recipient, &storage_key);
        prcharf("  ImageTrace at 0x00..00: ");
        for (size_t i = 0; i < sizeof(storage_value.bytes) / sizeof(storage_value.bytes[0]); i++)
            prcharf("%02x", storage_value.bytes[i]);
        prcharf("\n");
    }
    dvmc_release_result(&result);
    trans_log_host_destroy_context(ctx);
    dvmc_destroy(vm);
    return exit_code;
}
