# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

include(GNUInstallDirs)

if(MSVC AND CMAKE_C_COMPILER_VERSION VERSION_LESS 19.20)
    # For Visual Studio _before_ 2019
    # allow non-constant aggregate initializer.
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C>:/wd4204>
        $<$<COMPILE_LANGUAGE:C>:/wd4221>
    )
endif()

add_subdirectory(trans_log_vm)
add_subdirectory(trans_log_precompiles_vm)

add_library(dvmc-trans_log-host STATIC trans_log_host.cpp)
target_link_libraries(dvmc-trans_log-host PRIVATE dvmc::dvmc_cpp)

add_executable(dvmc-trans_log-static trans_log.c)
target_compile_features(dvmc-trans_log-static PRIVATE c_std_99)
target_link_libraries(dvmc-trans_log-static PRIVATE dvmc-trans_log-host dvmc::trans_log-vm-static dvmc::dvmc)
target_compile_definitions(dvmc-trans_log-static PRIVATE STATICALLY_LINKED_EXAMPLE)

add_executable(dvmc-trans_log trans_log.c)
target_compile_features(dvmc-trans_log PRIVATE c_std_99)
target_link_libraries(dvmc-trans_log PRIVATE dvmc-trans_log-host dvmc::loader)
