# DVMC VM Tester {#vmtester}

The DVMC project contains a DVMC-compatibility testing tool for VM implementations.

The tool is called `dvmc-vmtester` and to include it in the DVMC build
add `-DDVMC_TESTING=ON` CMake option to the project configuration step.

Usage is simple as

```sh
dvmc-vmtester [vm]
```

where `[vm]` is a path to a shared library with VM implementation.

For more information check `dvmc-vmtester --help`.
