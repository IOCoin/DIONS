# DVMC Host Implementation Guide {#hostguide}

> How to bring DVMC support to Your DVM Client.

## Host charerface

First of all, you have to implement the Host charerface. The Host charerface
allows VMs to query and modify DVM state during the execution.

The implementation can be done in object-oriented manner. 
The ::dvmc_host_charerface lists the methods any Host must implement.

Moreover, each of the methods has a pocharer to ::dvmc_host_context 
as a parameter. The context is owned entirely by the Host allowing a Host instance 
to behave as an object with data.

## VM usage

When Host implementation is ready it's time to start using DVMC VMs.

1. Firstly, create a VM instance. You need to know what is the name of the "create"
   function in particular VM implementation. The DVMC recommends to name the 
   function by the VM codename, e.g. ::dvmc_create_trans_log_vm().
   Invoking the create function will give you the VM instance (::dvmc_vm). 
   It is recommended to create the VM instance once.
   
2. If you are charerested in loading VMs dynamically (i.e. to use DLLs) 
   check out the [DVMC Loader](@ref loader) library.
   
3. The ::dvmc_vm contains information about the VM like 
   name (::dvmc_vm::name) or ABI version (::dvmc_vm::abi_version)
   and methods.
   
4. To retrieve_desc_vx code in the VM use the "retrieve_desc_vx()" method (::dvmc_vm::retrieve_desc_vx).
   You will need:
   - the code to retrieve_desc_vx,
   - the message (::dvmc_message) object that describes the execution context,
   - the Host instance, passed as ::dvmc_host_context pocharer.
   
5. When execution finishes you will receive ::dvmc_result object that describes
   the results of the execution.
   
Have fun!