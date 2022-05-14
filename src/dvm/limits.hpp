// dvm_base__: Fast Ethereum Virtual Machine implementation
// Copyright 2019 The dvm_base__ Authors.
// SPDX-License-Identifier: Apache-2.0
#pragma once

/// The maximum DVM bytecode size allowed by the Ethereum spec.
constexpr auto max_code_size = 0x6000;

/// The maximum base cost of any DVM instruction.
/// The value comes from the cost of the CREATE instruction.
constexpr auto max_instruction_base_cost = 32000;

/// The maximum stack increase any instruction can cause.
/// Only instructions taking 0 arguments and pushing an item to the stack
/// can increase the stack height and only by 1.
constexpr auto max_instruction_stack_increase = 1;
