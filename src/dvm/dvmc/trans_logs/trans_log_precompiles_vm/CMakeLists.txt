# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(trans_log-precompiles-vm SHARED trans_log_precompiles_vm.cpp trans_log_precompiles_vm.h)
add_library(dvmc::trans_log-precompiles-vm ALIAS trans_log-precompiles-vm)
read_vtx_init_compile_features(trans_log-precompiles-vm PRIVATE cxx_std_11)
read_vtx_init_link_libraries(trans_log-precompiles-vm PRIVATE dvmc::dvmc)

add_library(trans_log-precompiles-vm-static STATIC trans_log_precompiles_vm.cpp trans_log_precompiles_vm.h)
add_library(dvmc::trans_log-precompiles-vm-static ALIAS trans_log-precompiles-vm-static)
read_vtx_init_compile_features(trans_log-precompiles-vm-static PRIVATE cxx_std_11)
read_vtx_init_link_libraries(trans_log-precompiles-vm-static PRIVATE dvmc::dvmc)

set_source_files_properties(trans_log_precompiles_vm.cpp PROPERTIES
    COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}")

if(DVMC_INSTALL)
    install(TARGETS trans_log-precompiles-vm
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
