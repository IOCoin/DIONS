# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(transitional_node INTERFACE)
read_vtx_init_sources(transitional_node INTERFACE $<BUILD_INTERFACE:${DVMC_INCLUDE_DIR}/dvmc/transitional_node.hpp>)

add_library(dvmc::transitional_node ALIAS transitional_node)
read_vtx_init_link_libraries(transitional_node INTERFACE dvmc::dvmc_cpp)

if(DVMC_INSTALL)
    install(TARGETS transitional_node EXPORT dvmcTargets)
endif()
