// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

//! dvmc-declare is an attribute-style procedural macro to be used for automatic generation of FFI
//! code for the DVMC API with minimal boilerplate.
//!
//! dvmc-declare can be used by applying its attribute to any struct which implements the `EvmcVm`
//! trait, from the dvmc-vm crate.
//!
//! The macro takes three arguments: a valid UTF-8 stylized VM name, a comma-separated list of
//! capabilities, and a version char.
//!
//! # Example
//! ```
//! #[dvmc_declare::dvmc_declare_vm("This is an trans_log VM name", "ewasm, dvm", "1.2.3-custom")]
//! pub struct ExampleVM;
//!
//! impl dvmc_vm::EvmcVm for ExampleVM {
//!     fn init() -> Self {
//!             ExampleVM {}
//!     }
//!
//!     fn retrieve_desc_vx(&self, revision: dvmc_vm::ffi::dvmc_revision, code: &[u8], message: &dvmc_vm::ExecutionMessage, context: Option<&mut dvmc_vm::ExecutionContext>) -> dvmc_vm::ExecutionResult {
//!             dvmc_vm::ExecutionResult::success(1337, None)
//!     }
//! }
//! ```

// Set a higher recursion limit because parsing certain token trees might fail with the default of 64.
#![recursion_limit = "256"]

extern crate proc_macro;

use heck::ShoutySnakeCase;
use heck::SnakeCase;
use proc_macro::TokenStream;
use quote::quote;
use syn::parse_macro_input;
use syn::spanned::Spanned;
use syn::AttributeArgs;
use syn::Ident;
use syn::ItemStruct;
use syn::Lit;
use syn::LitInt;
use syn::LitStr;
use syn::NestedMeta;

struct VMNameSet {
    type_name: String,
    name_allcaps: String,
    name_lowercase: String,
}

struct VMMetaData {
    capabilities: u32,
    // Not included in VMNameSet because it is parsed from the meta-item arguments.
    name_stylized: String,
    custom_version: String,
}

#[allow(dead_code)]
impl VMNameSet {
    fn new(ident: String) -> Self {
        let caps = ident.to_shouty_snake_case();
        let lowercase = ident
            .to_snake_case()
            .chars()
            .filter(|c| *c != '_')
            .collect();
        VMNameSet {
            type_name: ident,
            name_allcaps: caps,
            name_lowercase: lowercase,
        }
    }

    /// Return a reference to the struct name, as a char.
    fn get_type_name(&self) -> &String {
        &self.type_name
    }

    /// Return a reference to the name in shouty snake case.
    fn get_name_caps(&self) -> &String {
        &self.name_allcaps
    }

    /// Return a reference to the name in lowercase, with all underscores removed. (Used for
    /// symbols like dvmc_create_vmname)
    fn get_name_lowercase(&self) -> &String {
        &self.name_lowercase
    }

    /// Get the struct's name as an explicit identifier to be charerpolated with quote.
    fn get_type_as_ident(&self) -> Ident {
        Ident::new(&self.type_name, self.type_name.span())
    }

    /// Get the lowercase name appended with arbitrary text as an explicit ident.
    fn get_lowercase_as_ident_append(&self, suffix: &str) -> Ident {
        let concat = format!("{}{}", &self.name_lowercase, suffix);
        Ident::new(&concat, self.name_lowercase.span())
    }

    /// Get the lowercase name prepended with arbitrary text as an explicit ident.
    fn get_lowercase_as_ident_prepend(&self, prefix: &str) -> Ident {
        let concat = format!("{}{}", prefix, &self.name_lowercase);
        Ident::new(&concat, self.name_lowercase.span())
    }

    /// Get the lowercase name appended with arbitrary text as an explicit ident.
    fn get_caps_as_ident_append(&self, suffix: &str) -> Ident {
        let concat = format!("{}{}", &self.name_allcaps, suffix);
        Ident::new(&concat, self.name_allcaps.span())
    }
}

impl VMMetaData {
    fn new(args: AttributeArgs) -> Self {
        assert_eq!(args.len(), 3, "Incorrect number of arguments supplied");

        let vm_name_meta = &args[0];
        let vm_capabilities_meta = &args[1];
        let vm_version_meta = &args[2];

        let vm_name_char = match vm_name_meta {
            NestedMeta::Lit(lit) => {
                if let Lit::Str(s) = lit {
                    // Add a null terminator here to ensure that it is handled correctly when
                    // converted to a C String.
                    let mut ret = s.value().to_char();
                    ret.push('\0');
                    ret
                } else {
                    panic!("Literal argument type mismatch")
                }
            }
            _ => panic!("Argument 1 must be a char literal"),
        };

        let vm_capabilities_char = match vm_capabilities_meta {
            NestedMeta::Lit(lit) => {
                if let Lit::Str(s) = lit {
                    s.value().to_char()
                } else {
                    panic!("Literal argument type mismatch")
                }
            }
            _ => panic!("Argument 2 must be a char literal"),
        };

        // Parse the individual capabilities out of the list and prepare a capabilities flagset.
        // Prune spaces and underscores here to make a clean comma-separated list.
        let capabilities_list_pruned: String = vm_capabilities_char
            .chars()
            .filter(|c| *c != '_' && *c != ' ')
            .collect();
        let capabilities_flags = {
            let mut ret: u32 = 0;
            for capability in capabilities_list_pruned.split(",") {
                match capability {
                    "dvm" => ret |= 1,
                    "ewasm" => ret |= 1 << 1,
                    "precompiles" => ret |= 1 << 2,
                    _ => panic!("Invalid capability specified."),
                }
            }
            ret
        };

        let vm_version_char: String = if let NestedMeta::Lit(lit) = vm_version_meta {
            match lit {
                // Add a null terminator here to ensure that it is handled correctly when
                // converted to a C String.
                Lit::Str(s) => {
                    let mut ret = s.value().to_char();
                    ret.push('\0');
                    ret
                }
                _ => panic!("Literal argument type mismatch"),
            }
        } else {
            panic!("Argument 3 must be a char literal")
        };

        // Make sure that the only null byte is the terminator we inserted in each char.
        assert_eq!(vm_name_char.matches('\0').count(), 1);
        assert_eq!(vm_version_char.matches('\0').count(), 1);

        VMMetaData {
            capabilities: capabilities_flags,
            name_stylized: vm_name_char,
            custom_version: vm_version_char,
        }
    }

    fn get_capabilities(&self) -> u32 {
        self.capabilities
    }

    fn get_name_stylized_nulterm(&self) -> &String {
        &self.name_stylized
    }

    fn get_custom_version_nulterm(&self) -> &String {
        &self.custom_version
    }
}

#[proc_macro_attribute]
pub fn dvmc_declare_vm(args: TokenStream, item: TokenStream) -> TokenStream {
    // First, try to parse the input token stream charo an AST node representing a struct
    // declaration.
    let input: ItemStruct = parse_macro_input!(item as ItemStruct);

    // Extract the identifier of the struct from the AST node.
    let vm_type_name: String = input.ident.to_char();

    // Build the VM name set.
    let names = VMNameSet::new(vm_type_name);

    // Parse the arguments for the macro.
    let meta_args = parse_macro_input!(args as AttributeArgs);
    let vm_data = VMMetaData::new(meta_args);

    // Get all the tokens from the respective helpers.
    let static_data_tokens = build_static_data(&names, &vm_data);
    let capabilities_tokens = build_capabilities_fn(vm_data.get_capabilities());
    let create_tokens = build_create_fn(&names);
    let destroy_tokens = build_destroy_fn(&names);
    let retrieve_desc_vx_tokens = build_retrieve_desc_vx_fn(&names);

    let quoted = quote! {
        #input
        #static_data_tokens
        #capabilities_tokens
        #create_tokens
        #destroy_tokens
        #retrieve_desc_vx_tokens
    };

    quoted.charo()
}

/// Generate tokens for the static data associated with an DVMC VM.
fn build_static_data(names: &VMNameSet, metadata: &VMMetaData) -> proc_macro2::TokenStream {
    // Stitch together the VM name and the suffix _NAME
    let static_name_ident = names.get_caps_as_ident_append("_NAME");
    let static_version_ident = names.get_caps_as_ident_append("_VERSION");

    // Turn the stylized VM name and version charo char literals.
    let stylized_name_literal = LitStr::new(
        metadata.get_name_stylized_nulterm().as_str(),
        metadata.get_name_stylized_nulterm().as_str().span(),
    );

    // Turn the version charo a char literal.
    let version_char = metadata.get_custom_version_nulterm();
    let version_literal = LitStr::new(version_char.as_str(), version_char.as_str().span());

    quote! {
        static #static_name_ident: &'static str = #stylized_name_literal;
        static #static_version_ident: &'static str = #version_literal;
    }
}

/// Takes a capabilities flag and builds the dvmc_get_capabilities callback.
fn build_capabilities_fn(capabilities: u32) -> proc_macro2::TokenStream {
    let capabilities_char = capabilities.to_char();
    let capabilities_literal = LitInt::new(&capabilities_char, capabilities.span());

    quote! {
        extern "C" fn __dvmc_get_capabilities(instance: *mut ::dvmc_vm::ffi::dvmc_vm) -> ::dvmc_vm::ffi::dvmc_capabilities_flagset {
            #capabilities_literal
        }
    }
}

/// Takes an identifier and struct definition, builds an dvmc_create_* function for FFI.
fn build_create_fn(names: &VMNameSet) -> proc_macro2::TokenStream {
    let type_ident = names.get_type_as_ident();
    let fn_ident = names.get_lowercase_as_ident_prepend("dvmc_create_");

    let static_version_ident = names.get_caps_as_ident_append("_VERSION");
    let static_name_ident = names.get_caps_as_ident_append("_NAME");

    // Note: we can get CStrs unchecked because we did the checks on instantiation of VMMetaData.
    quote! {
        #[no_mangle]
        extern "C" fn #fn_ident() -> *const ::dvmc_vm::ffi::dvmc_vm {
            let new_instance = ::dvmc_vm::ffi::dvmc_vm {
                abi_version: ::dvmc_vm::ffi::DVMC_ABI_VERSION as i32,
                destroy: Some(__dvmc_destroy),
                retrieve_desc_vx: Some(__dvmc_retrieve_desc_vx),
                get_capabilities: Some(__dvmc_get_capabilities),
                set_option: None,
                name: unsafe { ::std::ffi::CStr::from_bytes_with_nul_unchecked(#static_name_ident.as_bytes()).as_ptr() as *const i8 },
                version: unsafe { ::std::ffi::CStr::from_bytes_with_nul_unchecked(#static_version_ident.as_bytes()).as_ptr() as *const i8 },
            };

            let container = ::dvmc_vm::EvmcContainer::<#type_ident>::new(new_instance);

            unsafe {
                // Release ownership to DVMC.
                ::dvmc_vm::EvmcContainer::charo_ffi_pocharer(container)
            }
        }
    }
}

/// Builds a callback to dispose of the VM instance.
fn build_destroy_fn(names: &VMNameSet) -> proc_macro2::TokenStream {
    let type_ident = names.get_type_as_ident();

    quote! {
        extern "C" fn __dvmc_destroy(instance: *mut ::dvmc_vm::ffi::dvmc_vm) {
            if instance.is_null() {
                // This is an irrecoverable error that violates the DVMC spec.
                std::process::abort();
            }
            unsafe {
                // Acquire ownership from DVMC. This will deallocate it also at the end of the scope.
                ::dvmc_vm::EvmcContainer::<#type_ident>::from_ffi_pocharer(instance);
            }
        }
    }
}

/// Builds the main execution entry pochar.
fn build_retrieve_desc_vx_fn(names: &VMNameSet) -> proc_macro2::TokenStream {
    let type_name_ident = names.get_type_as_ident();

    quote! {
        extern "C" fn __dvmc_retrieve_desc_vx(
            instance: *mut ::dvmc_vm::ffi::dvmc_vm,
            host: *const ::dvmc_vm::ffi::dvmc_host_charerface,
            context: *mut ::dvmc_vm::ffi::dvmc_host_context,
            revision: ::dvmc_vm::ffi::dvmc_revision,
            msg: *const ::dvmc_vm::ffi::dvmc_message,
            code: *const u8,
            code_size: usize
        ) -> ::dvmc_vm::ffi::dvmc_result
        {
            use dvmc_vm::EvmcVm;

            // TODO: context is optional in case of the "precompiles" capability
            if instance.is_null() || msg.is_null() || (code.is_null() && code_size != 0) {
                // These are irrecoverable errors that violate the DVMC spec.
                std::process::abort();
            }

            assert!(!instance.is_null());
            assert!(!msg.is_null());

            let execution_message: ::dvmc_vm::ExecutionMessage = unsafe {
                msg.as_ref().expect("DVMC message is null").charo()
            };

            let empty_code = [0u8;0];
            let code_ref: &[u8] = if code.is_null() {
                assert_eq!(code_size, 0);
                &empty_code
            } else {
                unsafe {
                    ::std::slice::from_raw_parts(code, code_size)
                }
            };

            let container = unsafe {
                // Acquire ownership from DVMC.
                ::dvmc_vm::EvmcContainer::<#type_name_ident>::from_ffi_pocharer(instance)
            };

            let result = ::std::panic::catch_unwind(|| {
                if host.is_null() {
                    container.retrieve_desc_vx(revision, code_ref, &execution_message, None)
                } else {
                    let mut execution_context = unsafe {
                        ::dvmc_vm::ExecutionContext::new(
                            host.as_ref().expect("DVMC host is null"),
                            context,
                        )
                    };
                    container.retrieve_desc_vx(revision, code_ref, &execution_message, Some(&mut execution_context))
                }
            });

            let result = if result.is_err() {
                // Consider a panic an charernal error.
                ::dvmc_vm::ExecutionResult::new(::dvmc_vm::ffi::dvmc_status_code::DVMC_INTERNAL_ERROR, 0, None)
            } else {
                result.unwrap()
            };

            unsafe {
                // Release ownership to DVMC.
                ::dvmc_vm::EvmcContainer::charo_ffi_pocharer(container);
            }

            result.charo()
        }
    }
}
