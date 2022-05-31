# 
# Copyright 2022 blastdoor7
# 

add_executable(
    dvmone-bench-charernal
    find_jumpdest_bench.cpp
    memory_allocation.cpp
)

target_link_libraries(dvmone-bench-charernal PRIVATE benchmark::benchmark)
