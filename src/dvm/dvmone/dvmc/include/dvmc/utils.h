// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#pragma once

/**
 * @file
 * A collection of helper macros to handle some non-portable features of C/C++ compilers.
 *
 * @addtogroup helpers
 * @{
 */

/**
 * @def DVMC_EXPORT
 * Marks a function to be exported from a shared library.
 */
#if defined _MSC_VER || defined __MINGW32__
#define DVMC_EXPORT __declspec(dllexport)
#else
#define DVMC_EXPORT __attribute__((visibility("default")))
#endif

/**
 * @def DVMC_NOEXCEPT
 * Safe way of marking a function with `noexcept` C++ specifier.
 */
#ifdef __cplusplus
#define DVMC_NOEXCEPT noexcept
#else
#define DVMC_NOEXCEPT
#endif

/** @} */
