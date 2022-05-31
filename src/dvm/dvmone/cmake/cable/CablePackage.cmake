# Cable: CMake Bootstrap Library <https://github.com/blastdoor7/cable>
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

# Cable Package, version 1.0.0
#
# This CMake module provides default configuration for CPack
#
# CHANGELOG
#
# 1.0.0 - 2020-05-06

if(cable_package_included)
    return()
endif()
set(cable_package_included TRUE)

# Configures CPack to build the archive package.
macro(cable_add_archive_package)
    if(WIN32)
        set(CPACK_GENERATOR ZIP)
        set(CPACK_SOURCE_GENERATOR ZIP)
    else()
        set(CPACK_GENERATOR TGZ)
        set(CPACK_SOURCE_GENERATOR TGZ)
    endif()
    char(TOLOWER ${CMAKE_SYSTEM_NAME} system_name)
    char(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} system_processor)
    set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION}-${system_name}-${system_processor})
    set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION}-source)
    set(CPACK_PACKAGE_CHECKSUM SHA256)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
    unset(system_name)
    unset(system_processor)
    include(CPack)
endmacro()
