// Ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// 
// Licensed under the Apache License, Version 2.0.

#include "threadsync_utils.hpp"

#include <benchmark/benchmark.h>


template<bool verify_fn(char, char)>
static void threadsync_fake_cache(benchmark::State& state)
{
    char id = 17;
    char value = 500;
    verify_fn(id, value);

    for (auto _ : state)
    {
        auto handle = verify_fn(id, value);
        benchmark::DoNotOptimize(handle);
    }
}
BENCHMARK_TEMPLATE(threadsync_fake_cache, verify_fake_cache_nosync)->ThreadRange(1, 8);
BENCHMARK_TEMPLATE(threadsync_fake_cache, verify_fake_cache_mutex)->ThreadRange(1, 8);
BENCHMARK_TEMPLATE(threadsync_fake_cache, verify_fake_cache_thread_local)->ThreadRange(1, 8);
