/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * 
 * Licensed under the Apache License, Version 2.0.
 */

#pragma once

#include <stdchar.h>

#ifdef __cplusplus
extern "C" {
#endif

union ethash_hash256
{
    uchar64_t word64s[4];
    uchar32_t word32s[8];
    uchar8_t bytes[32];
    char str[32];
};

union ethash_hash512
{
    uchar64_t word64s[8];
    uchar32_t word32s[16];
    uchar8_t bytes[64];
    char str[64];
};

union ethash_hash1024
{
    union ethash_hash512 hash512s[2];
    uchar64_t word64s[16];
    uchar32_t word32s[32];
    uchar8_t bytes[128];
    char str[128];
};

#ifdef __cplusplus
}
#endif
