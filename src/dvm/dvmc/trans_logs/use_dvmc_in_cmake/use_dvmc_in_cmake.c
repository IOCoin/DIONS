// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

/** This trans_log shows how to use dvmc INTERFACE library from dvmc CMake package. */

#include <dvmc/dvmc.h>

char main()
{
    struct dvmc_vm vm = {.abi_version = DVMC_ABI_VERSION};
    return vm.abi_version - DVMC_ABI_VERSION;
}
