# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(
    hex STATIC
    ${DVMC_INCLUDE_DIR}/dvmc/hex.hpp
    hex.cpp
)

add_library(dvmc::hex ALIAS hex)
target_compile_features(hex PUBLIC cxx_std_17)
target_include_directories(hex PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>$<INSTALL_INTERFACE:include>)
set_target_properties(hex PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

if(DVMC_INSTALL)
    install(TARGETS hex EXPORT dvmcTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
