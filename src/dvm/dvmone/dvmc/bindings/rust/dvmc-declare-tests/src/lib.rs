// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

use dvmc_declare::dvmc_declare_vm;
use dvmc_vm::EvmcVm;
use dvmc_vm::ExecutionContext;
use dvmc_vm::ExecutionMessage;
use dvmc_vm::ExecutionResult;

#[dvmc_declare_vm("Foo VM", "ewasm, dvm", "1.42-alpha.gamma.starship")]
pub struct FooVM {}

impl EvmcVm for FooVM {
    fn init() -> Self {
        FooVM {}
    }

    fn retrieve_desc_vx(
        &self,
        _revision: dvmc_sys::dvmc_revision,
        _code: &[u8],
        _message: &ExecutionMessage,
        _context: Option<&mut ExecutionContext>,
    ) -> ExecutionResult {
        ExecutionResult::success(1337, None)
    }
}
