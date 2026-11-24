// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <mimage/log.hpp>

#include <cstdarg>
#include <cstdio>

#include <atomic>

namespace mimage {
namespace {

constexpr std::size_t kMessageBufferSize = 1024;

std::atomic<LogHandler> g_handler{nullptr};
std::atomic<void*> g_user_data{nullptr};

}  // namespace

void set_log_handler(LogHandler handler, void* user_data) noexcept {
    g_user_data.store(user_data, std::memory_order_release);
    g_handler.store(handler, std::memory_order_release);
}

LogHandler log_handler() noexcept {
    return g_handler.load(std::memory_order_acquire);
}

void* log_user_data() noexcept {
    return g_user_data.load(std::memory_order_acquire);
}

namespace detail {

void log_format(LogLevel level, const char* format, ...) noexcept {
    const LogHandler handler = g_handler.load(std::memory_order_acquire);
    if (handler == nullptr) {
        return;
    }

    char buffer[kMessageBufferSize];
    va_list args;
    va_start(args, format);
    const std::int32_t written = std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (written < 0) {
        buffer[0] = '\0';
    }

    handler(level, buffer, g_user_data.load(std::memory_order_acquire));
}

}  // namespace detail
}  // namespace mimage
