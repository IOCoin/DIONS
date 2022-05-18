// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <dvmc/dvmc.h>
#include <dvmc/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates DVMC Example Precompiles VM.
 */
DVMC_EXPORT struct dvmc_vm* dvmc_create_trans_log_precompiles_vm(void);

#ifdef __cplusplus
}
#endif
