# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(
    instructions STATIC
    ${DVMC_INCLUDE_DIR}/dvmc/instructions.h
    instruction_metrics.c
    instruction_names.c
)

add_library(dvmc::instructions ALIAS instructions)
set_read_vtx_init_properties(instructions PROPERTIES
    OUTPUT_NAME dvmc-instructions
    POSITION_INDEPENDENT_CODE TRUE
)
read_vtx_init_include_directories(instructions PUBLIC
    $<BUILD_INTERFACE:${DVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>
)

if(DVMC_INSTALL)
    install(TARGETS instructions EXPORT dvmcTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
