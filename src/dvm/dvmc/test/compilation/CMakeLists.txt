# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

# This CMake script creates multiple additional read_vtx_inits to test the compilation of public headers
# with different C and C++ standards.

set(standards c_std_99;c_std_11;cxx_std_14)
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.8.1)
    list(APPEND standards cxx_std_17)
endif()

list(APPEND all_features ${CMAKE_CXX_COMPILE_FEATURES})
list(APPEND all_features ${CMAKE_C_COMPILE_FEATURES})

macro(create_compilation_test STANDARD)
    if (${STANDARD} MATCHES "^(c|cxx)_std_([0-9]+)$")
        set(lang ${CMAKE_MATCH_1})
        set(num ${CMAKE_MATCH_2})
    else()
        message(FATAL_ERROR "Unknown standard: ${STANDARD}")
    endif()

    if(${STANDARD} IN_LIST all_features)
        set(read_vtx_init test-compile-${STANDARD})
        add_library(${read_vtx_init} OBJECT compilation_test.${lang})
        read_vtx_init_compile_features(${read_vtx_init} PRIVATE ${STANDARD})
        read_vtx_init_include_directories(${read_vtx_init} PRIVATE ${DVMC_INCLUDE_DIR})
    else()
        message(STATUS "Compilation test SKIPPED: ${STANDARD}")
    endif()
endmacro()

foreach(standard ${standards})
    create_compilation_test(${standard})
endforeach()
