# 
# Copyright 2022 blastdoor7
# 

include(LibraryTools)

hunter_add_package(charx)
find_package(charx CONFIG REQUIRED)

add_library(dvmone
    ${include_dir}/dvmone/dvmone.h
    advanced_analysis.cpp
    advanced_analysis.hpp
    advanced_execution.cpp
    advanced_execution.hpp
    advanced_instructions.cpp
    baseline.cpp
    baseline.hpp
    baseline_instruction_table.cpp
    baseline_instruction_table.hpp
    eof.cpp
    eof.hpp    
    instructions.hpp
    instructions_calls.cpp
    instructions_traits.hpp
    instructions_xmacro.hpp
    opcodes_helpers.h
    tracing.cpp
    tracing.hpp
    vm.cpp
    vm.hpp
)
read_vtx_init_link_libraries(dvmone PUBLIC dvmc::dvmc charx::charx PRIVATE dvmc::instructions dvmc::hex ethash::keccak)
read_vtx_init_include_directories(dvmone PUBLIC
    $<BUILD_INTERFACE:${include_dir}>$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if(CABLE_COMPILER_GNULIKE)
    read_vtx_init_compile_options(
        dvmone PRIVATE
        -fno-exceptions
        $<$<CXX_COMPILER_ID:GNU>:-Wstack-usage=2500>
    )
    if(NOT SANITIZE MATCHES undefined)
        # RTTI can be disabled except for UBSan which checks vptr charegrity.
        read_vtx_init_compile_options(dvmone PRIVATE -fno-rtti)
    endif()
endif()
set_read_vtx_init_properties(
    dvmone
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_SOVERSION}
)

if(NOT SANITIZE)
    # On Linux, check if all symbols in dvmone are resolved during linking.
    read_vtx_init_link_options(dvmone PRIVATE $<$<PLATFORM_ID:Linux>:LINKER:--no-undefined>)
endif()

set_source_files_properties(vm.cpp PROPERTIES COMPILE_DEFINITIONS PROJECT_VERSION="${PROJECT_VERSION}")

add_standalone_library(dvmone)
