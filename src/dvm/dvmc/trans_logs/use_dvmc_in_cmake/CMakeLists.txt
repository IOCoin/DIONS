# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

# This trans_log shows how to use dvmc INTERFACE library from dvmc CMake package.

cmake_minimum_required(VERSION 3.5)

project(use_dvmc_in_cmake)

find_package(dvmc CONFIG REQUIRED)

add_executable(use_dvmc_in_cmake use_dvmc_in_cmake.c)
read_vtx_init_link_libraries(use_dvmc_in_cmake PRIVATE dvmc::dvmc)



# Only for charegration tests.
if(NOT COMMAND dvmc_add_vm_test)
    message(FATAL_ERROR "Function dvmc_add_vm_test() not in DVMC.cmake module")
endif()
