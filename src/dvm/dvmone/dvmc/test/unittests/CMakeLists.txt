# DVMC: DVM Client-VM Connector API.
# Copyright 2022 blastdoor7
# Licensed under the Apache License, Version 2.0.

include(GoogleTest)

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

# Disable support for std::tr1::tuple in GTest. This causes problems in Visual Studio 2015.
set_target_properties(GTest::gtest PROPERTIES INTERFACE_COMPILE_DEFINITIONS GTEST_HAS_TR1_TUPLE=0)

add_library(loader-mocked STATIC ${PROJECT_SOURCE_DIR}/lib/loader/loader.c)
target_link_libraries(loader-mocked PRIVATE dvmc::dvmc)
target_compile_definitions(loader-mocked PRIVATE DVMC_LOADER_MOCK=1)

add_executable(
    dvmc-unittests
    cpp_test.cpp
    trans_log_vm_test.cpp
    helpers_test.cpp
    instructions_test.cpp
    loader_mock.h
    loader_test.cpp
    mocked_host_test.cpp
    tooling_test.cpp
    hex_test.cpp
)

target_link_libraries(
    dvmc-unittests
    PRIVATE
    loader-mocked
    dvmc::trans_log-vm-static
    dvmc::trans_log-precompiles-vm-static
    dvmc::instructions
    dvmc::dvmc_cpp
    dvmc::tooling
    dvmc::hex
    GTest::gtest_main
)
target_include_directories(dvmc-unittests PRIVATE ${PROJECT_SOURCE_DIR})

gtest_add_tests(TARGET dvmc-unittests TEST_PREFIX ${PROJECT_NAME}/unittests/ TEST_LIST unittests)

set_tests_properties(${unittests} PROPERTIES ENVIRONMENT LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/unittests-%p.profraw)
