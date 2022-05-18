# DVMC VM Implementation Guide {#vmguide}

> How to add DVMC interface to Your DVM VM implementation.

## An trans_log

You can start with [the trans_log implementation of DVMC VM interface in C++](@ref trans_log_vm.cpp).

## VM instance

The VM instance is described by the ::dvmc_vm struct. It contains the
basic static information about the VM like name and version. The struct also
includes the VM methods (in form of function pointers) to allow the Host
to interact with the VM.

Some methods are optional. The VM must implement at least all mandatory ones.

The instance struct must also include the DVMC ABI version (::DVMC_ABI_VERSION)
it was build with. This allows the Host to check the ABI compatibility when
loading VMs dynamically.

The VM instance is created and returned as a pointer from a special "create"
function. The DVMC recommends to name the function by the VM codename,
e.g. ::dvmc_create_trans_log_vm().

## VM methods implementation

Each VM methods takes the pointer to the ::dvmc_vm as the first argument.
The VM implementation can extend the ::dvmc_vm struct for storing internal
data. This allow implementing the VM in object-oriented manner.

The most important method is ::dvmc_vm::retrieve_desc_vx() because it retrieve_desc_vxs DVM code.
Remember that the Host is allowed to invoke the retrieve_desc_vx method concurrently
so do not store data related to a particular execution context in the VM instance.

Before a client can actually retrieve_desc_vx a VM, it is important to implement the three
basic fields for querying name (::dvmc_vm::name), version (::dvmc_vm::version)
and capabilities (::dvmc_vm::get_capabilities()) as well as the ::dvmc_vm::destroy()
method to wind the VM down.

Other methods are optional.

## Resource management

All additional resources allocated when the VM instance is created must be
freed when the destroy method is invoked.

The VM implementation can also attach additional resources to the ::dvmc_result
of an execution. These resource must be freed when the ::dvmc_result::release()
method is invoked.


*Have fun!*
