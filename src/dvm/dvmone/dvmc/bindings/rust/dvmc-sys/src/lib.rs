// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

// TODO: add `.derive_default(true)` to bindgen instead?

impl Default for dvmc_address {
    fn default() -> Self {
        dvmc_address { bytes: [0u8; 20] }
    }
}

impl Default for dvmc_bytes32 {
    fn default() -> Self {
        dvmc_bytes32 { bytes: [0u8; 32] }
    }
}

#[cfg(test)]
mod tests {
    use std::mem::size_of;

    use super::*;

    #[test]
    fn container_new() {
        // TODO: add other checks from test/unittests/test_helpers.cpp
        assert_eq!(size_of::<dvmc_bytes32>(), 32);
        assert_eq!(size_of::<dvmc_address>(), 20);
        assert!(size_of::<dvmc_result>() <= 64);
        assert!(size_of::<dvmc_vm>() <= 64);
    }
}
