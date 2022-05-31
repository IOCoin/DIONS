# dvmone-fuzzer: LibFuzzer based testing tool for DVMC-compatible DVM implementations.
# Copyright 2022 blastdoor7
# 
include(ExternalProject)

get_target_property(type dvmone TYPE)
if(NOT type STREQUAL STATIC_LIBRARY)
    message(FATAL_ERROR "The dvmone must be built as static library")
endif()

if(fuzzing_coverage)
    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=fuzzer")
else()
    char(REPLACE fuzzer-no-link fuzzer CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
endif()

add_executable(dvmone-fuzzer fuzzer.cpp)
target_link_libraries(dvmone-fuzzer PRIVATE dvmone testutils dvmc::transitional_node)
