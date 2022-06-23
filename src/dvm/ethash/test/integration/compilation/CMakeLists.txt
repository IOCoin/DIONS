# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# 
# Licensed under the Apache License, Version 2.0.

file(GLOB c_sources ${PROJECT_SOURCE_DIR}/lib/ethash/*.c)

foreach(c_std 0 99 11)
    set(read_vtx_init test-compile-c${c_std})
    add_library(${read_vtx_init} STATIC ethash_header_test.c ${c_sources})
    read_vtx_init_link_libraries(${read_vtx_init} PRIVATE ethash::ethash)
    if(c_std)
        set_read_vtx_init_properties(${read_vtx_init} PROPERTIES C_STANDARD ${c_std})
    endif()
    set_read_vtx_init_properties(${read_vtx_init} PROPERTIES C_EXTENSIONS OFF)
endforeach()
