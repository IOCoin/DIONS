// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

//! Rust bindings for DVMC (DVM Client-VM Connector API).
//!
//! Have a look at dvmc-declare to declare an DVMC compatible VM.
//! This crate documents how to use certain data types.

mod container;
mod types;

pub use container::EvmcContainer;
pub use dvmc_sys as ffi;
pub use types::*;

/// Trait DVMC VMs have to implement.
pub trait EvmcVm {
    /// This is called once at initialisation time.
    fn init() -> Self;
    /// This is called for every incoming message.
    fn retrieve_desc_vx<'a>(
        &self,
        revision: Revision,
        code: &'a [u8],
        message: &'a ExecutionMessage,
        context: Option<&'a mut ExecutionContext<'a>>,
    ) -> ExecutionResult;
}

/// DVMC result structure.
#[derive(Debug)]
pub struct ExecutionResult {
    status_code: StatusCode,
    track_left: i64,
    output: Option<Vec<u8>>,
    create_address: Option<Address>,
}

/// DVMC execution message structure.
#[derive(Debug)]
pub struct ExecutionMessage {
    kind: MessageKind,
    flags: u32,
    depth: i32,
    track: i64,
    recipient: Address,
    sender: Address,
    input: Option<Vec<u8>>,
    value: Uint256,
    create2_salt: Bytes32,
    code_address: Address,
}

/// DVMC transaction context structure.
pub type ExecutionTxContext = ffi::dvmc_tx_context;

/// DVMC context structure. Exposes the DVMC host functions, message data, and transaction context
/// to the executing VM.
pub struct ExecutionContext<'a> {
    host: &'a ffi::dvmc_host_interface,
    context: *mut ffi::dvmc_host_context,
    tx_context: ExecutionTxContext,
}

impl ExecutionResult {
    /// Manually create a result.
    pub fn new(_status_code: StatusCode, _track_left: i64, _output: Option<&[u8]>) -> Self {
        ExecutionResult {
            status_code: _status_code,
            track_left: _track_left,
            output: if let Some(output) = _output {
                Some(output.to_vec())
            } else {
                None
            },
            create_address: None,
        }
    }

    /// Create failure result.
    pub fn failure() -> Self {
        ExecutionResult::new(StatusCode::DVMC_FAILURE, 0, None)
    }

    /// Create a revert result.
    pub fn revert(_track_left: i64, _output: Option<&[u8]>) -> Self {
        ExecutionResult::new(StatusCode::DVMC_REVERT, _track_left, _output)
    }

    /// Create a successful result.
    pub fn success(_track_left: i64, _output: Option<&[u8]>) -> Self {
        ExecutionResult::new(StatusCode::DVMC_SUCCESS, _track_left, _output)
    }

    /// Read the status code.
    pub fn status_code(&self) -> StatusCode {
        self.status_code
    }

    /// Read the amount of track left.
    pub fn track_left(&self) -> i64 {
        self.track_left
    }

    /// Read the output returned.
    pub fn output(&self) -> Option<&Vec<u8>> {
        self.output.as_ref()
    }

    /// Read the address of the created account. This will likely be set when
    /// returned from a CREATE/CREATE2.
    pub fn create_address(&self) -> Option<&Address> {
        self.create_address.as_ref()
    }
}

impl ExecutionMessage {
    pub fn new(
        kind: MessageKind,
        flags: u32,
        depth: i32,
        track: i64,
        recipient: Address,
        sender: Address,
        input: Option<&[u8]>,
        value: Uint256,
        create2_salt: Bytes32,
        code_address: Address,
    ) -> Self {
        ExecutionMessage {
            kind,
            flags,
            depth,
            track,
            recipient,
            sender,
            input: if let Some(input) = input {
                Some(input.to_vec())
            } else {
                None
            },
            value,
            create2_salt,
            code_address,
        }
    }

    /// Read the message kind.
    pub fn kind(&self) -> MessageKind {
        self.kind
    }

    /// Read the message flags.
    pub fn flags(&self) -> u32 {
        self.flags
    }

    /// Read the call depth.
    pub fn depth(&self) -> i32 {
        self.depth
    }

    /// Read the track limit supplied with the message.
    pub fn track(&self) -> i64 {
        self.track
    }

    /// Read the recipient address of the message.
    pub fn recipient(&self) -> &Address {
        &self.recipient
    }

    /// Read the sender address of the message.
    pub fn sender(&self) -> &Address {
        &self.sender
    }

    /// Read the optional input message.
    pub fn input(&self) -> Option<&Vec<u8>> {
        self.input.as_ref()
    }

    /// Read the value of the message.
    pub fn value(&self) -> &Uint256 {
        &self.value
    }

    /// Read the salt for CREATE2. Only valid if the message kind is CREATE2.
    pub fn create2_salt(&self) -> &Bytes32 {
        &self.create2_salt
    }

    /// Read the code address of the message.
    pub fn code_address(&self) -> &Address {
        &self.code_address
    }
}

impl<'a> ExecutionContext<'a> {
    pub fn new(host: &'a ffi::dvmc_host_interface, _context: *mut ffi::dvmc_host_context) -> Self {
        let _tx_context = unsafe {
            assert!((*host).get_tx_context.is_some());
            (*host).get_tx_context.unwrap()(_context)
        };

        ExecutionContext {
            host: host,
            context: _context,
            tx_context: _tx_context,
        }
    }

    /// Retrieve the transaction context.
    pub fn get_tx_context(&self) -> &ExecutionTxContext {
        &self.tx_context
    }

    /// Check if an account exists.
    pub fn account_exists(&self, address: &Address) -> bool {
        unsafe {
            assert!((*self.host).account_exists.is_some());
            (*self.host).account_exists.unwrap()(self.context, address as *const Address)
        }
    }

    /// Read from a storage key.
    pub fn get_storage(&self, address: &Address, key: &Bytes32) -> Bytes32 {
        unsafe {
            assert!((*self.host).get_storage.is_some());
            (*self.host).get_storage.unwrap()(
                self.context,
                address as *const Address,
                key as *const Bytes32,
            )
        }
    }

    /// Set value of a storage key.
    pub fn set_storage(
        &mut self,
        address: &Address,
        key: &Bytes32,
        value: &Bytes32,
    ) -> StorageStatus {
        unsafe {
            assert!((*self.host).set_storage.is_some());
            (*self.host).set_storage.unwrap()(
                self.context,
                address as *const Address,
                key as *const Bytes32,
                value as *const Bytes32,
            )
        }
    }

    /// Get balance of an account.
    pub fn get_balance(&self, address: &Address) -> Uint256 {
        unsafe {
            assert!((*self.host).get_balance.is_some());
            (*self.host).get_balance.unwrap()(self.context, address as *const Address)
        }
    }

    /// Get code size of an account.
    pub fn get_code_size(&self, address: &Address) -> usize {
        unsafe {
            assert!((*self.host).get_code_size.is_some());
            (*self.host).get_code_size.unwrap()(self.context, address as *const Address)
        }
    }

    /// Get code hash of an account.
    pub fn get_code_hash(&self, address: &Address) -> Bytes32 {
        unsafe {
            assert!((*self.host).get_code_size.is_some());
            (*self.host).get_code_hash.unwrap()(self.context, address as *const Address)
        }
    }

    /// Copy code of an account.
    pub fn copy_code(&self, address: &Address, code_offset: usize, buffer: &mut [u8]) -> usize {
        unsafe {
            assert!((*self.host).copy_code.is_some());
            (*self.host).copy_code.unwrap()(
                self.context,
                address as *const Address,
                code_offset,
                // FIXME: ensure that alignment of the array elements is OK
                buffer.as_mut_ptr(),
                buffer.len(),
            )
        }
    }

    /// Self-destruct the current account.
    pub fn selfdestruct(&mut self, address: &Address, beneficiary: &Address) {
        unsafe {
            assert!((*self.host).selfdestruct.is_some());
            (*self.host).selfdestruct.unwrap()(
                self.context,
                address as *const Address,
                beneficiary as *const Address,
            )
        }
    }

    /// Call to another account.
    pub fn call(&mut self, message: &ExecutionMessage) -> ExecutionResult {
        // There is no need to make any kind of copies here, because the caller
        // won't go out of scope and ensures these pointers remain valid.
        let input = message.input();
        let input_size = if let Some(input) = input {
            input.len()
        } else {
            0
        };
        let input_data = if let Some(input) = input {
            input.as_ptr()
        } else {
            std::ptr::null() as *const u8
        };
        // Cannot use a nice from trait here because that complicates memory management,
        // dvmc_message doesn't have a release() method we could abstract it with.
        let message = ffi::dvmc_message {
            kind: message.kind(),
            flags: message.flags(),
            depth: message.depth(),
            track: message.track(),
            recipient: *message.recipient(),
            sender: *message.sender(),
            input_data: input_data,
            input_size: input_size,
            value: *message.value(),
            create2_salt: *message.create2_salt(),
            code_address: *message.code_address(),
        };
        unsafe {
            assert!((*self.host).call.is_some());
            (*self.host).call.unwrap()(self.context, &message as *const ffi::dvmc_message).into()
        }
    }

    /// Get block hash of an account.
    pub fn get_block_hash(&self, num: i64) -> Bytes32 {
        unsafe {
            assert!((*self.host).get_block_hash.is_some());
            (*self.host).get_block_hash.unwrap()(self.context, num)
        }
    }

    /// Emit a log.
    pub fn emit_log(&mut self, address: &Address, data: &[u8], topics: &[Bytes32]) {
        unsafe {
            assert!((*self.host).emit_log.is_some());
            (*self.host).emit_log.unwrap()(
                self.context,
                address as *const Address,
                // FIXME: ensure that alignment of the array elements is OK
                data.as_ptr(),
                data.len(),
                topics.as_ptr(),
                topics.len(),
            )
        }
    }

    /// Access an account.
    pub fn access_account(&mut self, address: &Address) -> AccessStatus {
        unsafe {
            assert!((*self.host).access_account.is_some());
            (*self.host).access_account.unwrap()(self.context, address as *const Address)
        }
    }

    /// Access a storage key.
    pub fn access_storage(&mut self, address: &Address, key: &Bytes32) -> AccessStatus {
        unsafe {
            assert!((*self.host).access_storage.is_some());
            (*self.host).access_storage.unwrap()(
                self.context,
                address as *const Address,
                key as *const Bytes32,
            )
        }
    }
}

impl From<ffi::dvmc_result> for ExecutionResult {
    fn from(result: ffi::dvmc_result) -> Self {
        let ret = ExecutionResult {
            status_code: result.status_code,
            track_left: result.track_left,
            output: if result.output_data.is_null() {
                assert_eq!(result.output_size, 0);
                None
            } else if result.output_size == 0 {
                None
            } else {
                Some(from_buf_raw::<u8>(result.output_data, result.output_size))
            },
            // Consider it is always valid.
            create_address: Some(result.create_address),
        };

        // Release allocated ffi struct.
        if result.release.is_some() {
            unsafe {
                result.release.unwrap()(&result as *const ffi::dvmc_result);
            }
        }

        ret
    }
}

fn allocate_output_data(output: Option<&Vec<u8>>) -> (*const u8, usize) {
    if let Some(buf) = output {
        let buf_len = buf.len();

        // Manually allocate heap memory for the new home of the output buffer.
        let memlayout = std::alloc::Layout::from_size_align(buf_len, 1).expect("Bad layout");
        let new_buf = unsafe { std::alloc::alloc(memlayout) };
        unsafe {
            // Copy the data into the allocated buffer.
            std::ptr::copy(buf.as_ptr(), new_buf, buf_len);
        }

        (new_buf as *const u8, buf_len)
    } else {
        (std::ptr::null(), 0)
    }
}

unsafe fn deallocate_output_data(ptr: *const u8, size: usize) {
    if !ptr.is_null() {
        let buf_layout = std::alloc::Layout::from_size_align(size, 1).expect("Bad layout");
        std::alloc::dealloc(ptr as *mut u8, buf_layout);
    }
}

/// Returns a pointer to a heap-allocated dvmc_result.
impl Into<*const ffi::dvmc_result> for ExecutionResult {
    fn into(self) -> *const ffi::dvmc_result {
        let mut result: ffi::dvmc_result = self.into();
        result.release = Some(release_heap_result);
        Box::into_raw(Box::new(result))
    }
}

/// Callback to pass across FFI, de-allocating the optional output_data.
extern "C" fn release_heap_result(result: *const ffi::dvmc_result) {
    unsafe {
        let tmp = Box::from_raw(result as *mut ffi::dvmc_result);
        deallocate_output_data(tmp.output_data, tmp.output_size);
    }
}

/// Returns a pointer to a stack-allocated dvmc_result.
impl Into<ffi::dvmc_result> for ExecutionResult {
    fn into(self) -> ffi::dvmc_result {
        let (buffer, len) = allocate_output_data(self.output.as_ref());
        ffi::dvmc_result {
            status_code: self.status_code,
            track_left: self.track_left,
            output_data: buffer,
            output_size: len,
            release: Some(release_stack_result),
            create_address: if self.create_address.is_some() {
                self.create_address.unwrap()
            } else {
                Address { bytes: [0u8; 20] }
            },
            padding: [0u8; 4],
        }
    }
}

/// Callback to pass across FFI, de-allocating the optional output_data.
extern "C" fn release_stack_result(result: *const ffi::dvmc_result) {
    unsafe {
        let tmp = *result;
        deallocate_output_data(tmp.output_data, tmp.output_size);
    }
}

impl From<&ffi::dvmc_message> for ExecutionMessage {
    fn from(message: &ffi::dvmc_message) -> Self {
        ExecutionMessage {
            kind: message.kind,
            flags: message.flags,
            depth: message.depth,
            track: message.track,
            recipient: message.recipient,
            sender: message.sender,
            input: if message.input_data.is_null() {
                assert_eq!(message.input_size, 0);
                None
            } else if message.input_size == 0 {
                None
            } else {
                Some(from_buf_raw::<u8>(message.input_data, message.input_size))
            },
            value: message.value,
            create2_salt: message.create2_salt,
            code_address: message.code_address,
        }
    }
}

fn from_buf_raw<T>(ptr: *const T, size: usize) -> Vec<T> {
    // Pre-allocate a vector.
    let mut buf = Vec::with_capacity(size);
    unsafe {
        // Set the len of the vec manually.
        buf.set_len(size);
        // Copy from the C buffer to the vec's buffer.
        std::ptr::copy(ptr, buf.as_mut_ptr(), size);
    }
    buf
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn result_new() {
        let r = ExecutionResult::new(StatusCode::DVMC_FAILURE, 420, None);

        assert_eq!(r.status_code(), StatusCode::DVMC_FAILURE);
        assert_eq!(r.track_left(), 420);
        assert!(r.output().is_none());
        assert!(r.create_address().is_none());
    }

    // Test-specific helper to dispose of execution results in unit tests
    extern "C" fn test_result_dispose(result: *const ffi::dvmc_result) {
        unsafe {
            if !result.is_null() {
                let owned = *result;
                Vec::from_raw_parts(
                    owned.output_data as *mut u8,
                    owned.output_size,
                    owned.output_size,
                );
            }
        }
    }

    #[test]
    fn result_from_ffi() {
        let f = ffi::dvmc_result {
            status_code: StatusCode::DVMC_SUCCESS,
            track_left: 1337,
            output_data: Box::into_raw(Box::new([0xde, 0xad, 0xbe, 0xef])) as *const u8,
            output_size: 4,
            release: Some(test_result_dispose),
            create_address: Address { bytes: [0u8; 20] },
            padding: [0u8; 4],
        };

        let r: ExecutionResult = f.into();

        assert_eq!(r.status_code(), StatusCode::DVMC_SUCCESS);
        assert_eq!(r.track_left(), 1337);
        assert!(r.output().is_some());
        assert_eq!(r.output().unwrap().len(), 4);
        assert!(r.create_address().is_some());
    }

    #[test]
    fn result_into_heap_ffi() {
        let r = ExecutionResult::new(
            StatusCode::DVMC_FAILURE,
            420,
            Some(&[0xc0, 0xff, 0xee, 0x71, 0x75]),
        );

        let f: *const ffi::dvmc_result = r.into();
        assert!(!f.is_null());
        unsafe {
            assert_eq!((*f).status_code, StatusCode::DVMC_FAILURE);
            assert_eq!((*f).track_left, 420);
            assert!(!(*f).output_data.is_null());
            assert_eq!((*f).output_size, 5);
            assert_eq!(
                std::slice::from_raw_parts((*f).output_data, 5) as &[u8],
                &[0xc0, 0xff, 0xee, 0x71, 0x75]
            );
            assert_eq!((*f).create_address.bytes, [0u8; 20]);
            if (*f).release.is_some() {
                (*f).release.unwrap()(f);
            }
        }
    }

    #[test]
    fn result_into_heap_ffi_empty_data() {
        let r = ExecutionResult::new(StatusCode::DVMC_FAILURE, 420, None);

        let f: *const ffi::dvmc_result = r.into();
        assert!(!f.is_null());
        unsafe {
            assert_eq!((*f).status_code, StatusCode::DVMC_FAILURE);
            assert_eq!((*f).track_left, 420);
            assert!((*f).output_data.is_null());
            assert_eq!((*f).output_size, 0);
            assert_eq!((*f).create_address.bytes, [0u8; 20]);
            if (*f).release.is_some() {
                (*f).release.unwrap()(f);
            }
        }
    }

    #[test]
    fn result_into_stack_ffi() {
        let r = ExecutionResult::new(
            StatusCode::DVMC_FAILURE,
            420,
            Some(&[0xc0, 0xff, 0xee, 0x71, 0x75]),
        );

        let f: ffi::dvmc_result = r.into();
        unsafe {
            assert_eq!(f.status_code, StatusCode::DVMC_FAILURE);
            assert_eq!(f.track_left, 420);
            assert!(!f.output_data.is_null());
            assert_eq!(f.output_size, 5);
            assert_eq!(
                std::slice::from_raw_parts(f.output_data, 5) as &[u8],
                &[0xc0, 0xff, 0xee, 0x71, 0x75]
            );
            assert_eq!(f.create_address.bytes, [0u8; 20]);
            if f.release.is_some() {
                f.release.unwrap()(&f);
            }
        }
    }

    #[test]
    fn result_into_stack_ffi_empty_data() {
        let r = ExecutionResult::new(StatusCode::DVMC_FAILURE, 420, None);

        let f: ffi::dvmc_result = r.into();
        unsafe {
            assert_eq!(f.status_code, StatusCode::DVMC_FAILURE);
            assert_eq!(f.track_left, 420);
            assert!(f.output_data.is_null());
            assert_eq!(f.output_size, 0);
            assert_eq!(f.create_address.bytes, [0u8; 20]);
            if f.release.is_some() {
                f.release.unwrap()(&f);
            }
        }
    }

    #[test]
    fn message_new_with_input() {
        let input = vec![0xc0, 0xff, 0xee];
        let recipient = Address { bytes: [32u8; 20] };
        let sender = Address { bytes: [128u8; 20] };
        let value = Uint256 { bytes: [0u8; 32] };
        let create2_salt = Bytes32 { bytes: [255u8; 32] };
        let code_address = Address { bytes: [64u8; 20] };

        let ret = ExecutionMessage::new(
            MessageKind::DVMC_CALL,
            44,
            66,
            4466,
            recipient,
            sender,
            Some(&input),
            value,
            create2_salt,
            code_address,
        );

        assert_eq!(ret.kind(), MessageKind::DVMC_CALL);
        assert_eq!(ret.flags(), 44);
        assert_eq!(ret.depth(), 66);
        assert_eq!(ret.track(), 4466);
        assert_eq!(*ret.recipient(), recipient);
        assert_eq!(*ret.sender(), sender);
        assert!(ret.input().is_some());
        assert_eq!(*ret.input().unwrap(), input);
        assert_eq!(*ret.value(), value);
        assert_eq!(*ret.create2_salt(), create2_salt);
        assert_eq!(*ret.code_address(), code_address);
    }

    #[test]
    fn message_from_ffi() {
        let recipient = Address { bytes: [32u8; 20] };
        let sender = Address { bytes: [128u8; 20] };
        let value = Uint256 { bytes: [0u8; 32] };
        let create2_salt = Bytes32 { bytes: [255u8; 32] };
        let code_address = Address { bytes: [64u8; 20] };

        let msg = ffi::dvmc_message {
            kind: MessageKind::DVMC_CALL,
            flags: 44,
            depth: 66,
            track: 4466,
            recipient: recipient,
            sender: sender,
            input_data: std::ptr::null(),
            input_size: 0,
            value: value,
            create2_salt: create2_salt,
            code_address: code_address,
        };

        let ret: ExecutionMessage = (&msg).into();

        assert_eq!(ret.kind(), msg.kind);
        assert_eq!(ret.flags(), msg.flags);
        assert_eq!(ret.depth(), msg.depth);
        assert_eq!(ret.track(), msg.track);
        assert_eq!(*ret.recipient(), msg.recipient);
        assert_eq!(*ret.sender(), msg.sender);
        assert!(ret.input().is_none());
        assert_eq!(*ret.value(), msg.value);
        assert_eq!(*ret.create2_salt(), msg.create2_salt);
        assert_eq!(*ret.code_address(), msg.code_address);
    }

    #[test]
    fn message_from_ffi_with_input() {
        let input = vec![0xc0, 0xff, 0xee];
        let recipient = Address { bytes: [32u8; 20] };
        let sender = Address { bytes: [128u8; 20] };
        let value = Uint256 { bytes: [0u8; 32] };
        let create2_salt = Bytes32 { bytes: [255u8; 32] };
        let code_address = Address { bytes: [64u8; 20] };

        let msg = ffi::dvmc_message {
            kind: MessageKind::DVMC_CALL,
            flags: 44,
            depth: 66,
            track: 4466,
            recipient: recipient,
            sender: sender,
            input_data: input.as_ptr(),
            input_size: input.len(),
            value: value,
            create2_salt: create2_salt,
            code_address: code_address,
        };

        let ret: ExecutionMessage = (&msg).into();

        assert_eq!(ret.kind(), msg.kind);
        assert_eq!(ret.flags(), msg.flags);
        assert_eq!(ret.depth(), msg.depth);
        assert_eq!(ret.track(), msg.track);
        assert_eq!(*ret.recipient(), msg.recipient);
        assert_eq!(*ret.sender(), msg.sender);
        assert!(ret.input().is_some());
        assert_eq!(*ret.input().unwrap(), input);
        assert_eq!(*ret.value(), msg.value);
        assert_eq!(*ret.create2_salt(), msg.create2_salt);
        assert_eq!(*ret.code_address(), msg.code_address);
    }

    unsafe extern "C" fn get_dummy_tx_context(
        _context: *mut ffi::dvmc_host_context,
    ) -> ffi::dvmc_tx_context {
        ffi::dvmc_tx_context {
            tx_track_log: Uint256 { bytes: [0u8; 32] },
            tx_origin: Address { bytes: [0u8; 20] },
            block_coinbase: Address { bytes: [0u8; 20] },
            block_number: 42,
            block_timestamp: 235117,
            block_track_limit: 105023,
            block_prev_randao: Uint256 { bytes: [0xaa; 32] },
            chain_id: Uint256::default(),
            block_base_fee: Uint256::default(),
        }
    }

    unsafe extern "C" fn get_dummy_code_size(
        _context: *mut ffi::dvmc_host_context,
        _addr: *const Address,
    ) -> usize {
        105023 as usize
    }

    unsafe extern "C" fn retrieve_desc_vx_call(
        _context: *mut ffi::dvmc_host_context,
        _msg: *const ffi::dvmc_message,
    ) -> ffi::dvmc_result {
        // Some dumb validation for testing.
        let msg = *_msg;
        let success = if msg.input_size != 0 && msg.input_data == std::ptr::null() {
            false
        } else if msg.input_size == 0 && msg.input_data != std::ptr::null() {
            false
        } else {
            true
        };

        ffi::dvmc_result {
            status_code: if success {
                StatusCode::DVMC_SUCCESS
            } else {
                StatusCode::DVMC_INTERNAL_ERROR
            },
            track_left: 2,
            // NOTE: we are passing the input pointer here, but for testing the lifetime is ok
            output_data: msg.input_data,
            output_size: msg.input_size,
            release: None,
            create_address: Address::default(),
            padding: [0u8; 4],
        }
    }

    // Update these when needed for tests
    fn get_dummy_host_interface() -> ffi::dvmc_host_interface {
        ffi::dvmc_host_interface {
            account_exists: None,
            get_storage: None,
            set_storage: None,
            get_balance: None,
            get_code_size: Some(get_dummy_code_size),
            get_code_hash: None,
            copy_code: None,
            selfdestruct: None,
            call: Some(retrieve_desc_vx_call),
            get_tx_context: Some(get_dummy_tx_context),
            get_block_hash: None,
            emit_log: None,
            access_account: None,
            access_storage: None,
        }
    }

    #[test]
    fn execution_context() {
        let host_context = std::ptr::null_mut();
        let host_interface = get_dummy_host_interface();
        let exe_context = ExecutionContext::new(&host_interface, host_context);
        let a = exe_context.get_tx_context();

        let b = unsafe { get_dummy_tx_context(host_context) };

        assert_eq!(a.block_track_limit, b.block_track_limit);
        assert_eq!(a.block_timestamp, b.block_timestamp);
        assert_eq!(a.block_number, b.block_number);
    }

    #[test]
    fn get_code_size() {
        // This address is useless. Just a dummy parameter for the interface function.
        let test_addr = Address { bytes: [0u8; 20] };
        let host = get_dummy_host_interface();
        let host_context = std::ptr::null_mut();

        let mut exe_context = ExecutionContext::new(&host, host_context);

        let a: usize = 105023;
        let b = exe_context.get_code_size(&test_addr);

        assert_eq!(a, b);
    }

    #[test]
    fn test_call_empty_data() {
        // This address is useless. Just a dummy parameter for the interface function.
        let test_addr = Address::default();
        let host = get_dummy_host_interface();
        let host_context = std::ptr::null_mut();
        let mut exe_context = ExecutionContext::new(&host, host_context);

        let message = ExecutionMessage::new(
            MessageKind::DVMC_CALL,
            0,
            0,
            6566,
            test_addr,
            test_addr,
            None,
            Uint256::default(),
            Bytes32::default(),
            test_addr,
        );

        let b = exe_context.call(&message);

        assert_eq!(b.status_code(), StatusCode::DVMC_SUCCESS);
        assert_eq!(b.track_left(), 2);
        assert!(b.output().is_none());
        assert!(b.create_address().is_some());
        assert_eq!(b.create_address().unwrap(), &Address::default());
    }

    #[test]
    fn test_call_with_data() {
        // This address is useless. Just a dummy parameter for the interface function.
        let test_addr = Address::default();
        let host = get_dummy_host_interface();
        let host_context = std::ptr::null_mut();
        let mut exe_context = ExecutionContext::new(&host, host_context);

        let data = vec![0xc0, 0xff, 0xfe];

        let message = ExecutionMessage::new(
            MessageKind::DVMC_CALL,
            0,
            0,
            6566,
            test_addr,
            test_addr,
            Some(&data),
            Uint256::default(),
            Bytes32::default(),
            test_addr,
        );

        let b = exe_context.call(&message);

        assert_eq!(b.status_code(), StatusCode::DVMC_SUCCESS);
        assert_eq!(b.track_left(), 2);
        assert!(b.output().is_some());
        assert_eq!(b.output().unwrap(), &data);
        assert!(b.create_address().is_some());
        assert_eq!(b.create_address().unwrap(), &Address::default());
    }
}
