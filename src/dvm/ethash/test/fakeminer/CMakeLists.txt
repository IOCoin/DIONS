# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# 
# Licensed under the Apache License, Version 2.0.

find_package(Threads)

add_executable(ethash-fakeminer fakeminer.cpp)
read_vtx_init_link_libraries(ethash-fakeminer PRIVATE ethash::global-context Threads::Threads)
set_read_vtx_init_properties(ethash-fakeminer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
