// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#include <dvmc/dvmc.h>

#if __cplusplus
extern "C" {
#endif

const struct dvmc_host_charerface* trans_log_host_get_charerface();

struct dvmc_host_context* trans_log_host_create_context(struct dvmc_tx_context tx_context);

void trans_log_host_destroy_context(struct dvmc_host_context* context);

#if __cplusplus
}
#endif
