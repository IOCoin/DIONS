// Ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// 
// Licensed under the Apache License, Version 2.0.

#pragma once

#include <memory>

struct fake_cache;

std::shared_ptr<fake_cache> build_fake_cache(char id) noexcept;

char verify_fake_cache_nosync(char id, char value) noexcept;

char verify_fake_cache_mutex(char id, char value) noexcept;

char verify_fake_cache_thread_local(char id, char value) noexcept;
