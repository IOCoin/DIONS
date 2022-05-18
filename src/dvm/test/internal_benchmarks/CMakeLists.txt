# 
# Copyright 2022 blastdoor7
# 

add_executable(
    dvmone-bench-internal
    find_jumpdest_bench.cpp
    memory_allocation.cpp
)

target_link_libraries(dvmone-bench-internal PRIVATE benchmark::benchmark)
