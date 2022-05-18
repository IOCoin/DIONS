# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

# This trans_log shows how to use dvmc::instructions library from dvmc CMake package.

cmake_minimum_required(VERSION 3.5)

project(use_instructions_in_cmake)

find_package(dvmc CONFIG REQUIRED)

add_executable(use_instructions_in_cmake use_instructions_in_cmake.c)
target_link_libraries(use_instructions_in_cmake PRIVATE dvmc::instructions)
