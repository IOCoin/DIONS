# 
# Copyright 2022 blastdoor7
# 

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

# The internal dvmone unit tests. The generic DVM ones are also built in.
add_executable(dvmone-unittests
    analysis_test.cpp
    pos_read_test.cpp
    eof_test.cpp
    eof_validation_test.cpp
    dvm_fixture.cpp
    dvm_fixture.hpp
    dvm_test.cpp
    dvm_calls_test.cpp
    dvm_eip2929_test.cpp
    dvm_eip3198_basefee_test.cpp
    dvm_eip3855_push0_test.cpp
    dvm_eof_test.cpp
    dvm_memory_test.cpp
    dvm_state_test.cpp
    dvm_other_test.cpp
    dvm_benchmark_test.cpp
    dvmone_test.cpp
    execution_state_test.cpp
    instructions_test.cpp
    tracing_test.cpp
    utils_test.cpp
)
target_link_libraries(dvmone-unittests PRIVATE dvmone testutils dvmc::instructions GTest::gtest GTest::gtest_main)
target_include_directories(dvmone-unittests PRIVATE ${dvmone_private_include_dir})

gtest_discover_tests(dvmone-unittests TEST_PREFIX ${PROJECT_NAME}/unittests/)

option(DVMONE_DVM_TEST_TOOL "Enable DVM unit testing tool for DVMC implementations (not maintained)" OFF)
if(DVMONE_DVM_TEST_TOOL)
    # The dvm-test tool that contains the all dvm-unittests and loads VMs as DVMC modules.
    add_executable(dvm-test main.cpp)
    target_link_libraries(dvm-test PRIVATE dvm-unittests testutils dvmc::dvmc dvmc::loader GTest::gtest)
endif()

# Provide the project version to selected source files.
set_source_files_properties(
    dvmone_test.cpp
    main.cpp
    PROPERTIES COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}"
)
