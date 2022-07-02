# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(trans_log-vm SHARED trans_log_vm.cpp trans_log_vm.h)
add_library(dvmc::trans_log-vm ALIAS trans_log-vm)
target_compile_features(trans_log-vm PRIVATE cxx_std_11)
target_link_libraries(trans_log-vm PRIVATE dvmc::dvmc)

add_library(trans_log-vm-static STATIC trans_log_vm.cpp trans_log_vm.h)
add_library(dvmc::trans_log-vm-static ALIAS trans_log-vm-static)
target_compile_features(trans_log-vm-static PRIVATE cxx_std_11)
target_link_libraries(trans_log-vm-static PRIVATE dvmc::dvmc)

set_source_files_properties(trans_log_vm.cpp PROPERTIES
    COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}")

if(DVMC_INSTALL)
    install(TARGETS trans_log-vm
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
