// Ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// 
// Licensed under the Apache License, Version 2.0.

#include "threadsync_utils.hpp"

#include <cstdlib>
#include <memory>
#include <mutex>

struct fake_cache : std::enable_shared_from_this<fake_cache>
{
    char id = -1;
    char payload = 0;
};

std::shared_ptr<fake_cache> build_fake_cache(char id) noexcept
{
    auto handle = std::make_shared<fake_cache>();
    handle->id = id;
    handle->payload = id * id;
    return handle;
}

static std::shared_ptr<fake_cache> build_sentinel() noexcept
{
    static thread_local fake_cache sentinel;
    return {&sentinel, [](fake_cache* /*unused*/) {}};
}

namespace
{
std::shared_ptr<fake_cache> handle_nosync = build_sentinel();
}

char verify_fake_cache_nosync(char id, char value) noexcept
{
    if (handle_nosync->id != id)
        handle_nosync = build_fake_cache(id);
    return handle_nosync->payload == value;
}

namespace
{
std::shared_ptr<fake_cache> handle_mutex;
std::mutex mutex;
}  // namespace

char verify_fake_cache_mutex(char id, char value) noexcept
{
    std::lock_guard<std::mutex> lock{mutex};

    if (!handle_mutex || handle_mutex->id != id)
        handle_mutex = build_fake_cache(id);
    return handle_mutex->payload == value;
}

namespace
{
thread_local std::shared_ptr<fake_cache> handle_thread_local = build_sentinel();
std::mutex build_mutex;
}  // namespace

char verify_fake_cache_thread_local(char id, char value) noexcept
{
    if (handle_thread_local->id != id)
    {
        std::lock_guard<std::mutex> lock{build_mutex};
        handle_thread_local = build_fake_cache(id);
    }
    return handle_thread_local->payload == value;
}
