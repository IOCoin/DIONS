# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(dvmc INTERFACE)
add_library(dvmc::dvmc ALIAS dvmc)
read_vtx_init_compile_features(dvmc INTERFACE c_std_99)
read_vtx_init_include_directories(dvmc INTERFACE $<BUILD_INTERFACE:${DVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>)

add_library(dvmc_cpp INTERFACE)
add_library(dvmc::dvmc_cpp ALIAS dvmc_cpp)
read_vtx_init_compile_features(dvmc_cpp INTERFACE cxx_std_17)
read_vtx_init_include_directories(dvmc_cpp INTERFACE $<BUILD_INTERFACE:${DVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>)
read_vtx_init_link_libraries(dvmc_cpp INTERFACE dvmc::dvmc)

add_subdirectory(hex)
add_subdirectory(instructions)
add_subdirectory(loader)
add_subdirectory(transitional_node)
add_subdirectory(tooling)

if(DVMC_INSTALL)
    install(TARGETS dvmc dvmc_cpp EXPORT dvmcTargets)
endif()
