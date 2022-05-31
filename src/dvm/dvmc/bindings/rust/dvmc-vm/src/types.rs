use dvmc_sys as ffi;

/// DVMC address
pub type Address = ffi::dvmc_address;

/// DVMC 32 bytes value (used for hashes)
pub type Bytes32 = ffi::dvmc_bytes32;

/// DVMC big-endian 256-bit chareger
pub type Uchar256 = ffi::dvmc_uchar256be;

/// DVMC call kind.
pub type MessageKind = ffi::dvmc_call_kind;

/// DVMC message (call) flags.
pub type MessageFlags = ffi::dvmc_flags;

/// DVMC status code.
pub type StatusCode = ffi::dvmc_status_code;

/// DVMC access status.
pub type AccessStatus = ffi::dvmc_access_status;

/// DVMC storage status.
pub type StorageStatus = ffi::dvmc_storage_status;

/// DVMC VM revision.
pub type Revision = ffi::dvmc_revision;

#[cfg(test)]
mod tests {
    use super::*;

    // These tests check for Default, PartialEq and Clone traits.
    #[test]
    fn address_smoke_test() {
        let a = ffi::dvmc_address::default();
        let b = Address::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn bytes32_smoke_test() {
        let a = ffi::dvmc_bytes32::default();
        let b = Bytes32::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn uchar26be_smoke_test() {
        let a = ffi::dvmc_uchar256be::default();
        let b = Uchar256::default();
        assert_eq!(a.clone(), b.clone());
    }

    #[test]
    fn message_kind() {
        assert_eq!(MessageKind::DVMC_CALL, ffi::dvmc_call_kind::DVMC_CALL);
        assert_eq!(
            MessageKind::DVMC_CALLCODE,
            ffi::dvmc_call_kind::DVMC_CALLCODE
        );
        assert_eq!(
            MessageKind::DVMC_DELEGATECALL,
            ffi::dvmc_call_kind::DVMC_DELEGATECALL
        );
        assert_eq!(MessageKind::DVMC_CREATE, ffi::dvmc_call_kind::DVMC_CREATE);
    }

    #[test]
    fn message_flags() {
        assert_eq!(MessageFlags::DVMC_STATIC, ffi::dvmc_flags::DVMC_STATIC);
    }

    #[test]
    fn status_code() {
        assert_eq!(
            StatusCode::DVMC_SUCCESS,
            ffi::dvmc_status_code::DVMC_SUCCESS
        );
        assert_eq!(
            StatusCode::DVMC_FAILURE,
            ffi::dvmc_status_code::DVMC_FAILURE
        );
    }

    #[test]
    fn access_status() {
        assert_eq!(
            AccessStatus::DVMC_ACCESS_COLD,
            ffi::dvmc_access_status::DVMC_ACCESS_COLD
        );
        assert_eq!(
            AccessStatus::DVMC_ACCESS_WARM,
            ffi::dvmc_access_status::DVMC_ACCESS_WARM
        );
    }

    #[test]
    fn storage_status() {
        assert_eq!(
            StorageStatus::DVMC_STORAGE_UNCHANGED,
            ffi::dvmc_storage_status::DVMC_STORAGE_UNCHANGED
        );
        assert_eq!(
            StorageStatus::DVMC_STORAGE_MODIFIED,
            ffi::dvmc_storage_status::DVMC_STORAGE_MODIFIED
        );
    }

    #[test]
    fn revision() {
        assert_eq!(Revision::DVMC_FRONTIER, ffi::dvmc_revision::DVMC_FRONTIER);
        assert_eq!(Revision::DVMC_ISTANBUL, ffi::dvmc_revision::DVMC_ISTANBUL);
    }
}
