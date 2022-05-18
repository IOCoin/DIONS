// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

// Test compilation of C and C++ public headers.

#include <dvmc/dvmc.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/helpers.h>
#include <dvmc/instructions.h>
#include <dvmc/loader.h>
#include <dvmc/mocked_host.hpp>
#include <dvmc/utils.h>

// Include again to check if headers have proper include guards.
#include <dvmc/dvmc.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/helpers.h>
#include <dvmc/instructions.h>
#include <dvmc/loader.h>
#include <dvmc/mocked_host.hpp>
#include <dvmc/utils.h>
