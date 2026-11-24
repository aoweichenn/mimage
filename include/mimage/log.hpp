// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MIMAGE_LOG_HPP
#define MIMAGE_LOG_HPP

// Injectable diagnostic channel.
//
// Default: no handler installed, so the library stays silent. Image loading,
// encoding, resize and drawing report the failure through their return value;
// the log is for the context that the return value cannot carry (path, codec,
// dimensions).
//
// The host can route these messages into its own logger:
//
//     mimage::set_log_handler(
//         [](mimage::LogLevel level, const char* message, void* user_data) {
//             // forward to the application logger
//         },
//         &app_state);

#include <cstdint>

#include <mimage/config.hpp>

namespace mimage {

enum class LogLevel : std::uint8_t {
    kDebug = 0,
    kInfo,
    kWarning,
    kError,
};

// The handler must be thread-safe; codecs and drawing may log from any thread.
using LogHandler = void (*)(LogLevel level, const char* message, void* user_data);

void set_log_handler(LogHandler handler, void* user_data) noexcept;

[[nodiscard]] LogHandler log_handler() noexcept;
[[nodiscard]] void* log_user_data() noexcept;

namespace detail {

#if defined(__GNUC__) || defined(__clang__)
#define MIMAGE_PRINTF_FORMAT(format_index, argument_index) __attribute__((format(printf, format_index, argument_index)))
#else
#define MIMAGE_PRINTF_FORMAT(format_index, argument_index)
#endif

// Formats into a fixed stack buffer; never allocates and never throws.
void log_format(LogLevel level, const char* format, ...) noexcept MIMAGE_PRINTF_FORMAT(2, 3);

}  // namespace detail

}  // namespace mimage

#define MIMAGE_LOGD(...) ::mimage::detail::log_format(::mimage::LogLevel::kDebug, __VA_ARGS__)
#define MIMAGE_LOGI(...) ::mimage::detail::log_format(::mimage::LogLevel::kInfo, __VA_ARGS__)
#define MIMAGE_LOGW(...) ::mimage::detail::log_format(::mimage::LogLevel::kWarning, __VA_ARGS__)
#define MIMAGE_LOGE(...) ::mimage::detail::log_format(::mimage::LogLevel::kError, __VA_ARGS__)

#endif  // MIMAGE_LOG_HPP
