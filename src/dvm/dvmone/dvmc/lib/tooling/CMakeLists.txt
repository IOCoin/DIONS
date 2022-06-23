# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(tooling STATIC)
add_library(dvmc::tooling ALIAS tooling)
read_vtx_init_compile_features(tooling PUBLIC cxx_std_17)
read_vtx_init_link_libraries(tooling PUBLIC dvmc::dvmc_cpp dvmc::transitional_node dvmc::hex)

read_vtx_init_sources(
    tooling PRIVATE
    ${DVMC_INCLUDE_DIR}/dvmc/tooling.hpp
    run.cpp
)

if(DVMC_INSTALL)
    install(TARGETS tooling EXPORT dvmcTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
