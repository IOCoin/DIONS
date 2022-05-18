// 
// Copyright 2022 blastdoor7
// 

#ifndef DVMONE_H
#define DVMONE_H

#include <dvmc/dvmc.h>
#include <dvmc/utils.h>

#if __cplusplus
extern "C" {
#endif

DVMC_EXPORT struct dvmc_vm* dvmc_create_dvmone(void) DVMC_NOEXCEPT;

#if __cplusplus
}
#endif

#endif  // DVMONE_H
