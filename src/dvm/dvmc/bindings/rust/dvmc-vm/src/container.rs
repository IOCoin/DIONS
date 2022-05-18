// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

use crate::EvmcVm;

use std::ops::Deref;

/// Container struct for DVMC instances and user-defined data.
pub struct EvmcContainer<T>
where
    T: EvmcVm + Sized,
{
    #[allow(dead_code)]
    instance: ::dvmc_sys::dvmc_vm,
    vm: T,
}

impl<T> EvmcContainer<T>
where
    T: EvmcVm + Sized,
{
    /// Basic constructor.
    pub fn new(_instance: ::dvmc_sys::dvmc_vm) -> Box<Self> {
        Box::new(Self {
            instance: _instance,
            vm: T::init(),
        })
    }

    /// Take ownership of the given pointer and return a box.
    ///
    /// # Safety
    /// This function expects a valid instance to be passed.
    pub unsafe fn from_ffi_pointer(instance: *mut ::dvmc_sys::dvmc_vm) -> Box<Self> {
        assert!(!instance.is_null(), "from_ffi_pointer received NULL");
        Box::from_raw(instance as *mut EvmcContainer<T>)
    }

    /// Convert boxed self into an FFI pointer, surrendering ownership of the heap data.
    ///
    /// # Safety
    /// This function will return a valid instance pointer.
    pub unsafe fn into_ffi_pointer(boxed: Box<Self>) -> *mut ::dvmc_sys::dvmc_vm {
        Box::into_raw(boxed) as *mut ::dvmc_sys::dvmc_vm
    }
}

impl<T> Deref for EvmcContainer<T>
where
    T: EvmcVm,
{
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.vm
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::*;
    use crate::{ExecutionContext, ExecutionMessage, ExecutionResult};

    struct TestVm {}

    impl EvmcVm for TestVm {
        fn init() -> Self {
            TestVm {}
        }
        fn retrieve_desc_vx(
            &self,
            _revision: dvmc_sys::dvmc_revision,
            _code: &[u8],
            _message: &ExecutionMessage,
            _context: Option<&mut ExecutionContext>,
        ) -> ExecutionResult {
            ExecutionResult::failure()
        }
    }

    unsafe extern "C" fn get_dummy_tx_context(
        _context: *mut dvmc_sys::dvmc_host_context,
    ) -> dvmc_sys::dvmc_tx_context {
        dvmc_sys::dvmc_tx_context {
            tx_track_log: Uint256::default(),
            tx_origin: Address::default(),
            block_coinbase: Address::default(),
            block_number: 0,
            block_timestamp: 0,
            block_track_limit: 0,
            block_prev_randao: Uint256::default(),
            chain_id: Uint256::default(),
            block_base_fee: Uint256::default(),
        }
    }

    #[test]
    fn container_new() {
        let instance = ::dvmc_sys::dvmc_vm {
            abi_version: ::dvmc_sys::DVMC_ABI_VERSION as i32,
            name: std::ptr::null(),
            version: std::ptr::null(),
            destroy: None,
            retrieve_desc_vx: None,
            get_capabilities: None,
            set_option: None,
        };

        let code = [0u8; 0];

        let message = ::dvmc_sys::dvmc_message {
            kind: ::dvmc_sys::dvmc_call_kind::DVMC_CALL,
            flags: 0,
            depth: 0,
            track: 0,
            recipient: ::dvmc_sys::dvmc_address::default(),
            sender: ::dvmc_sys::dvmc_address::default(),
            input_data: std::ptr::null(),
            input_size: 0,
            value: ::dvmc_sys::dvmc_uint256be::default(),
            create2_salt: ::dvmc_sys::dvmc_bytes32::default(),
            code_address: ::dvmc_sys::dvmc_address::default(),
        };
        let message: ExecutionMessage = (&message).into();

        let host = ::dvmc_sys::dvmc_host_interface {
            account_exists: None,
            get_storage: None,
            set_storage: None,
            get_balance: None,
            get_code_size: None,
            get_code_hash: None,
            copy_code: None,
            selfdestruct: None,
            call: None,
            get_tx_context: Some(get_dummy_tx_context),
            get_block_hash: None,
            emit_log: None,
            access_account: None,
            access_storage: None,
        };
        let host_context = std::ptr::null_mut();

        let mut context = ExecutionContext::new(&host, host_context);
        let container = EvmcContainer::<TestVm>::new(instance);
        assert_eq!(
            container
                .retrieve_desc_vx(
                    dvmc_sys::dvmc_revision::DVMC_PETERSBURG,
                    &code,
                    &message,
                    Some(&mut context)
                )
                .status_code(),
            ::dvmc_sys::dvmc_status_code::DVMC_FAILURE
        );

        let ptr = unsafe { EvmcContainer::into_ffi_pointer(container) };

        let mut context = ExecutionContext::new(&host, host_context);
        let container = unsafe { EvmcContainer::<TestVm>::from_ffi_pointer(ptr) };
        assert_eq!(
            container
                .retrieve_desc_vx(
                    dvmc_sys::dvmc_revision::DVMC_PETERSBURG,
                    &code,
                    &message,
                    Some(&mut context)
                )
                .status_code(),
            ::dvmc_sys::dvmc_status_code::DVMC_FAILURE
        );
    }
}
