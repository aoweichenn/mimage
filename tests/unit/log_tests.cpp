// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>

#include <string>

#include <gtest/gtest.h>
#include <mimage/log.hpp>

namespace {

struct Capture {
    mimage::LogLevel level = mimage::LogLevel::kDebug;
    std::string message;
    void* user_data = nullptr;
    std::int32_t calls = 0;
};

void capture_handler(mimage::LogLevel level, const char* message, void* user_data) {
    auto* capture = static_cast<Capture*>(user_data);
    capture->level = level;
    capture->message = message;
    capture->user_data = user_data;
    ++capture->calls;
}

}  // namespace

TEST(Log, IsSilentWhenNoHandlerInstalled) {
    mimage::set_log_handler(nullptr, nullptr);
    EXPECT_EQ(mimage::log_handler(), nullptr);

    MIMAGE_LOGE("this message must not reach any handler");
    EXPECT_EQ(mimage::log_handler(), nullptr);
}

TEST(Log, ForwardsLevelMessageAndUserData) {
    Capture capture;
    mimage::set_log_handler(capture_handler, &capture);

    MIMAGE_LOGW("decode failed with code %d", 42);
    EXPECT_EQ(capture.level, mimage::LogLevel::kWarning);
    EXPECT_EQ(capture.message, "decode failed with code 42");
    EXPECT_EQ(capture.user_data, &capture);
    EXPECT_EQ(capture.calls, 1);

    mimage::set_log_handler(nullptr, nullptr);
}

TEST(Log, CanBeReplacedAndCleared) {
    Capture first;
    Capture second;
    mimage::set_log_handler(capture_handler, &first);
    MIMAGE_LOGI("first");
    mimage::set_log_handler(capture_handler, &second);
    MIMAGE_LOGI("second");

    EXPECT_EQ(first.message, "first");
    EXPECT_EQ(second.message, "second");
    EXPECT_EQ(first.calls, 1);
    EXPECT_EQ(second.calls, 1);

    mimage::set_log_handler(nullptr, nullptr);
    EXPECT_EQ(mimage::log_handler(), nullptr);
}
