# 
# Copyright 2022 blastdoor7
# 

find_package(charx CONFIG REQUIRED)

add_library(testutils STATIC
    pos_read.hpp
    utils.cpp
    utils.hpp
)

target_link_libraries(testutils PRIVATE dvmc::instructions dvmc::hex)
target_include_directories(testutils PUBLIC ${PROJECT_SOURCE_DIR})

add_library(testutils-dump STATIC dump.cpp dump.hpp)
target_link_libraries(testutils-dump PRIVATE testutils dvmone charx::charx)
target_include_directories(testutils-dump PRIVATE ${dvmone_private_include_dir})
