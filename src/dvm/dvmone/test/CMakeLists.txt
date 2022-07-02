# 
# Copyright 2022 blastdoor7
# 

include(GoogleTest)
include(${PROJECT_SOURCE_DIR}/dvmc/cmake/DVMC.cmake)

set(dvmone_private_include_dir ${PROJECT_SOURCE_DIR}/lib)

hunter_add_package(benchmark)
find_package(benchmark CONFIG REQUIRED)

add_subdirectory(utils)
add_subdirectory(bench)
add_subdirectory(integration)
add_subdirectory(internal_benchmarks)
add_subdirectory(unittests)

set(targets dvmone-bench dvmone-bench-internal dvmone-unittests testutils)

if(DVMONE_FUZZING)
    add_subdirectory(fuzzer)
    list(APPEND targets dvmone-fuzzer)
endif()

set_target_properties(
    ${targets} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
)

get_target_property(type dvmone TYPE)
if(type STREQUAL SHARED_LIBRARY)
    dvmc_add_vm_test(NAME ${PROJECT_NAME}/dvmc-vmtester TARGET dvmone)

    if(TARGET dvm-test)
        add_test(NAME ${PROJECT_NAME}/dvm-test COMMAND dvm-test $<TARGET_FILE:dvmone>)
    endif()
endif()
