# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

include(GNUInstallDirs)

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

# Disable support for std::tr1::tuple in GTest. This causes problems in Visual Studio 2015.
set_target_properties(GTest::gtest PROPERTIES INTERFACE_COMPILE_DEFINITIONS GTEST_HAS_TR1_TUPLE=0)

add_executable(dvmc-vmtester vmtester.hpp vmtester.cpp tests.cpp)
target_link_libraries(dvmc-vmtester PRIVATE dvmc::loader dvmc::mocked_host GTest::gtest)
set_source_files_properties(vmtester.cpp PROPERTIES COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}")
add_executable(dvmc::dvmc-vmtester ALIAS dvmc-vmtester)

if(DVMC_INSTALL)
    install(TARGETS dvmc-vmtester EXPORT dvmcTargets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
