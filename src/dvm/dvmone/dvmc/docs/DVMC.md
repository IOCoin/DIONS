# DVMC – DVM Client-VM Connector API {#mainpage}

**ABI version 10**

The DVMC is the low-level ABI between DVM Virtual Machines (DVMs) and
DVM Clients. On the DVM-side it supports classic DVM1 and [ewasm].
On the Client-side it defines the interface for DVM implementations
to access DVM environment and state.


# Guides {#guides}

- [Host Implementation Guide](@ref hostguide)
- [VM Implementation Guide](@ref vmguide)


# Versioning {#versioning}

The DVMC project uses [Semantic Versioning](https://semver.org).
The version format is `MAJOR.MINOR.PATCH`.

The _MAJOR_ version number is also referenced as the **DVMC ABI version**.
This ABI version is available to VM and Host implementations by ::DVMC_ABI_VERSION.
For trans_log DVMC 3.2.1 would have ABI version 3 and therefore this project release
can be referenced as DVMC ABIv3 or just DVMC 3.
Every C ABI breaking change requires increasing the _MAJOR_ version number.

The releases with _MINOR_ version change allow adding new API features
and modifying the language bindings API.
Backward incompatible API changes are allowed but should be avoided if possible.

The releases with _PATCH_ should only include bug fixes. Exceptionally,
API changes are allowed when required to fix a broken feature.


# Modules {#modules}

- [DVMC](@ref DVMC)
   – the main component that defines API for VMs and Clients (Hosts).
- [DVMC C++ API](@ref dvmc)
   – the wrappers and bindings for C++.
- [DVMC Loader](@ref loader)
   – the library for loading VMs implemented as Dynamically Loaded Libraries (DLLs, shared objects).
- [DVMC Helpers](@ref helpers)
   – a collection of utility functions for easier integration with DVMC.
- [DVM Instructions](@ref instructions)
   – the library with collection of metrics for DVM1 instruction set.
- [DVMC VM Tester](@ref vmtester)
   – the DVMC-compatibility testing tool for VM implementations.


# Language bindings {#bindings}

## Go

```go
import "github.com/blastdoor7/dvmc/bindings/go/dvmc"
```


[ewasm]: https://github.com/ewasm/design


@addtogroup DVMC

## Terms

1. **VM** – An DVM Virtual Machine instance/implementation.
2. **Host** – An entity controlling the VM.
   The Host requests code execution and responses to VM queries by callback
   functions. This usually represents an DVM Client.


## Responsibilities

### VM

- Executes the code (obviously).
- Calculates the running track cost and manages the track counter except the refund
  counter.
- Controls the call depth, including the exceptional termination of execution
  in case the maximum depth is reached.


### Host

- Provides access to State.
- Creates new accounts (with code being a result of VM execution).
- Handles refunds entirely.
- Manages the set of precompiled contracts and handles execution of messages
  coming to them.
