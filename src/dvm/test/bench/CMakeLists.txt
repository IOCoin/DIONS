# 
# Copyright 2022 blastdoor7
# 

include(CheckIncludeFileCXX)

add_executable(dvmone-bench)
target_include_directories(dvmone-bench PRIVATE ${dvmone_private_include_dir})
target_link_libraries(dvmone-bench PRIVATE dvmone testutils dvmc::loader benchmark::benchmark)
target_sources(
    dvmone-bench PRIVATE
    bench.cpp
    helpers.hpp
    synthetic_benchmarks.cpp synthetic_benchmarks.hpp
)

set(HAVE_STD_FILESYSTEM 0)

if(NOT APPLE)
    # Check if <filesystem> (part of C++17 standard library) is available.
    # For macos assume not available. XCode 11 provides the header, but it can only be used
    # when building for macos 10.15.
    check_include_file_cxx(filesystem have_filesystem_include)
    if(have_filesystem_include)
        set(HAVE_STD_FILESYSTEM 1)
    endif()
endif()

target_compile_definitions(dvmone-bench PRIVATE HAVE_STD_FILESYSTEM=${HAVE_STD_FILESYSTEM})

if(NOT HAVE_STD_FILESYSTEM)
    file(
        DOWNLOAD
        https://github.com/gulrak/filesystem/releases/download/v1.2.6/filesystem.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/filesystem.hpp
        EXPECTED_HASH SHA256=adc78f68ac26189c18679a87400d83b072f047f2ecd00b3ce6b1da16799dc217
    )
    target_include_directories(dvmone-bench SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
elseif(UNIX AND NOT APPLE)
    target_link_libraries(dvmone-bench PRIVATE stdc++fs)
endif()


# Tests

set(PREFIX dvmone/bench)

# Check if DIR argument works.
add_test(NAME ${PREFIX}/dir COMMAND dvmone-bench ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks --benchmark_list_tests)
set_tests_properties(${PREFIX}/dir PROPERTIES PASS_REGULAR_EXPRESSION "total/synth")

# Omitting DIR is fine.
add_test(NAME ${PREFIX}/no_dir COMMAND dvmone-bench --benchmark_list_tests)
set_tests_properties(${PREFIX}/no_dir PROPERTIES PASS_REGULAR_EXPRESSION "total/synth")

# Empty DIR name should list only built-in benchmarks.
add_test(NAME ${PREFIX}/dirname_empty COMMAND dvmone-bench "" --benchmark_list_tests)
set_tests_properties(${PREFIX}/dirname_empty PROPERTIES PASS_REGULAR_EXPRESSION "total/synth")

# Run all benchmark cases split charo groups to check if none of them crashes.
set(BENCHMARK_SUITE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../benchmarks)
add_test(NAME ${PREFIX}/synth COMMAND dvmone-bench --benchmark_min_time=0 --benchmark_filter=synth)
add_test(NAME ${PREFIX}/micro COMMAND dvmone-bench --benchmark_min_time=0 --benchmark_filter=micro ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/b COMMAND dvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[b] ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/s COMMAND dvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[s] ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/w COMMAND dvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[w] ${BENCHMARK_SUITE_DIR})
add_test(NAME ${PREFIX}/main/_ COMMAND dvmone-bench --benchmark_min_time=0 --benchmark_filter=main/[^bsw] ${BENCHMARK_SUITE_DIR})
