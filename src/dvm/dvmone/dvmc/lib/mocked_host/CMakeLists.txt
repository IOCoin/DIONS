# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

add_library(mocked_host INTERFACE)
target_sources(mocked_host INTERFACE $<BUILD_INTERFACE:${DVMC_INCLUDE_DIR}/dvmc/mocked_host.hpp>)

add_library(dvmc::mocked_host ALIAS mocked_host)
target_link_libraries(mocked_host INTERFACE dvmc::dvmc_cpp)

if(DVMC_INSTALL)
    install(TARGETS mocked_host EXPORT dvmcTargets)
endif()
