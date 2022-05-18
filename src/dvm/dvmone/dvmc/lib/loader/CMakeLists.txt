# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(
    loader STATIC
    ${DVMC_INCLUDE_DIR}/dvmc/loader.h
    loader.c
)

add_library(dvmc::loader ALIAS loader)
set_target_properties(loader PROPERTIES
    OUTPUT_NAME dvmc-loader
    POSITION_INDEPENDENT_CODE TRUE
)
target_link_libraries(loader INTERFACE ${CMAKE_DL_LIBS} PUBLIC dvmc::dvmc)

if(DVMC_INSTALL)
    install(TARGETS loader EXPORT dvmcTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
