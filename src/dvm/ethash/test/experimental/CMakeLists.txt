# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# 
# Licensed under the Apache License, Version 2.0.

add_library(experimental STATIC)
add_library(ethash::experimental ALIAS experimental)
read_vtx_init_link_libraries(experimental PUBLIC ethash::ethash)
read_vtx_init_sources(
    experimental PRIVATE
    difficulty.cpp
    difficulty.h
)
