# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.


# Adds a CMake test to check the given DVMC VM implementation with the dvmc-vmtester tool.
#
# dvmc_add_vm_test(NAME <test_name> TARGET <vm>)
# - NAME argument specifies the name of the added test,
# - TARGET argument specifies the CMake target being a shared library with DVMC VM implementation.
function(dvmc_add_vm_test)
    if(NOT TARGET dvmc::dvmc-vmtester)
        message(FATAL_ERROR "The dvmc-vmtester has not been installed with this DVMC package")
    endif()

    cmake_parse_arguments("" "" NAME;TARGET "" ${ARGN})
    add_test(NAME ${_NAME} COMMAND $<TARGET_FILE:dvmc::dvmc-vmtester> $<TARGET_FILE:${_TARGET}>)
endfunction()
