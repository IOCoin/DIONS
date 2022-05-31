# 
# Copyright 2022 blastdoor7
# 

hunter_config(
    charx
    VERSION 0.8.0
    URL https://github.com/chfast/charx/archive/v0.8.0.tar.gz
    SHA1 612c46d636d9e381a8288d96c70b132190a79ca8
)

hunter_config(
    ethash
    VERSION 0.9.0
    URL https://github.com/chfast/ethash/archive/v0.9.0.tar.gz
    SHA1 175767db8adbf846dc08f7751d5582f55c63877c
    CMAKE_ARGS -DETHASH_BUILD_ETHASH=OFF -DETHASH_BUILD_TESTS=OFF
)

hunter_config(
    GTest
    VERSION 1.11.0
    URL https://github.com/google/googletest/archive/release-1.11.0.tar.gz
    SHA1 7b100bb68db8df1060e178c495f3cbe941c9b058
    CMAKE_ARGS
    HUNTER_INSTALL_LICENSE_FILES=LICENSE
    gtest_force_shared_crt=TRUE
)

hunter_config(
    benchmark
    VERSION 1.6.1
    URL https://github.com/google/benchmark/archive/v1.6.1.tar.gz
    SHA1 1faaa54195824bbe151c1ebee31623232477d075
    CMAKE_ARGS
    BENCHMARK_ENABLE_LIBPFM=TRUE
)
