// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <mimage/image.hpp>
#include <mimage/log.hpp>

namespace {

struct Capture {
    std::string message;
    std::int32_t calls = 0;
};

void capture_handler(mimage::LogLevel, const char* message, void* user_data) {
    auto* capture = static_cast<Capture*>(user_data);
    capture->message = message;
    ++capture->calls;
}

constexpr const char* kRoundTripPath = "/tmp/mimage_unit_roundtrip.png";

}  // namespace

TEST(Image, DefaultIsEmpty) {
    const mimage::Image image;
    EXPECT_TRUE(image.empty());
    EXPECT_EQ(image.rows, 0);
    EXPECT_EQ(image.cols, 0);
    EXPECT_EQ(image.c, 0);
    EXPECT_EQ(image.data, nullptr);
    EXPECT_EQ(image.refcount, nullptr);
    EXPECT_EQ(image.total(), 0U);
}

TEST(Image, CreateAllocatesRefcountedStorage) {
    mimage::Image image(4, 5, mimage::kPixelRgb);
    ASSERT_FALSE(image.empty());
    EXPECT_EQ(image.rows, 4);
    EXPECT_EQ(image.cols, 5);
    EXPECT_EQ(image.c, 3);
    EXPECT_EQ(image.total(), 60U);
    ASSERT_NE(image.refcount, nullptr);
    EXPECT_EQ(*image.refcount, 1);
    EXPECT_EQ(image.channels(), 3);
    EXPECT_EQ(image.type(), mimage::kPixelRgb);
    EXPECT_EQ(image.ptr(0), image.data);

    mimage::Image copy = image;
    EXPECT_EQ(copy.data, image.data);
    EXPECT_EQ(*image.refcount, 2);

    copy.release();
    EXPECT_EQ(*image.refcount, 1);
}

TEST(Image, RejectsInvalidDimensions) {
    mimage::Image image(0, 5, 3);
    EXPECT_TRUE(image.empty());

    image.create(-1, 5, 3);
    EXPECT_TRUE(image.empty());
}

TEST(Image, ExternalDataIsNotOwned) {
    std::vector<std::uint8_t> pixels(16, 7);
    {
        mimage::Image image(2, 2, 4, pixels.data());
        EXPECT_FALSE(image.empty());
        EXPECT_EQ(image.data, pixels.data());
        EXPECT_EQ(image.refcount, nullptr);
        EXPECT_EQ(image.total(), 16U);
    }
    EXPECT_EQ(pixels[0], 7);
}

TEST(Image, CloneIsDeepCopy) {
    mimage::Image image(2, 2, mimage::kPixelGray);
    ASSERT_FALSE(image.empty());
    image.data[0] = 11;
    image.data[3] = 22;

    const mimage::Image copy = image.clone();
    EXPECT_NE(copy.data, image.data);
    EXPECT_EQ(copy.data[0], 11);
    EXPECT_EQ(copy.data[3], 22);
}

TEST(Image, RoiIsDeepCopy) {
    mimage::Image image(4, 4, mimage::kPixelGray);
    ASSERT_FALSE(image.empty());
    for (std::int32_t y = 0; y < 4; ++y) {
        for (std::int32_t x = 0; x < 4; ++x) {
            image.data[y * 4 + x] = static_cast<std::uint8_t>(y * 4 + x);
        }
    }

    const mimage::Image roi = image(mimage::Rect(1, 1, 2, 2));
    ASSERT_FALSE(roi.empty());
    EXPECT_EQ(roi.rows, 2);
    EXPECT_EQ(roi.cols, 2);
    EXPECT_EQ(roi.data[0], 5);
    EXPECT_EQ(roi.data[3], 10);
    EXPECT_NE(roi.data, image.data);
}

TEST(Image, ScalarAssignmentFillsAllChannels) {
    mimage::Image image(2, 2, mimage::kPixelRgba);
    image = mimage::Scalar(1, 2, 3, 4);
    ASSERT_FALSE(image.empty());
    for (std::size_t index = 0; index < image.total(); index += 4) {
        EXPECT_EQ(image.data[index + 0], 1);
        EXPECT_EQ(image.data[index + 1], 2);
        EXPECT_EQ(image.data[index + 2], 3);
        EXPECT_EQ(image.data[index + 3], 4);
    }
}

TEST(Image, PngRoundTripPreservesPixels) {
    mimage::Image source(3, 4, mimage::kPixelRgb);
    ASSERT_FALSE(source.empty());
    for (std::int32_t y = 0; y < source.rows; ++y) {
        for (std::int32_t x = 0; x < source.cols; ++x) {
            auto* pixel =
                source.data +
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(source.cols) + static_cast<std::size_t>(x)) * 3;
            pixel[0] = static_cast<std::uint8_t>(10 + x);
            pixel[1] = static_cast<std::uint8_t>(20 + y);
            pixel[2] = static_cast<std::uint8_t>(30 + x + y);
        }
    }

    ASSERT_TRUE(mimage::imwrite(kRoundTripPath, source));

    std::ifstream stream(kRoundTripPath, std::ios::binary);
    const std::vector<std::uint8_t> encoded((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    const mimage::Image decoded = mimage::imdecode(encoded, mimage::IMREAD_COLOR);
    const mimage::Image decoded_from_file = mimage::imread(kRoundTripPath, mimage::IMREAD_COLOR);
    std::remove(kRoundTripPath);

    ASSERT_FALSE(decoded.empty());
    EXPECT_EQ(decoded.rows, source.rows);
    EXPECT_EQ(decoded.cols, source.cols);
    EXPECT_EQ(decoded.c, source.c);
    EXPECT_EQ(std::memcmp(decoded.data, source.data, source.total()), 0);
    ASSERT_FALSE(decoded_from_file.empty());
    EXPECT_EQ(std::memcmp(decoded_from_file.data, source.data, source.total()), 0);
}

TEST(Image, ImreadLogsMissingFileThroughHandler) {
    Capture capture;
    mimage::set_log_handler(capture_handler, &capture);

    const mimage::Image image = mimage::imread("/tmp/mimage_missing_file_12345.png");
    mimage::set_log_handler(nullptr, nullptr);

    EXPECT_TRUE(image.empty());
    EXPECT_EQ(capture.calls, 1);
    EXPECT_NE(capture.message.find("mimage_missing_file_12345.png"), std::string::npos);
}

TEST(Image, ResizeProducesRequestedDimensions) {
    mimage::Image source(2, 2, mimage::kPixelGray);
    ASSERT_FALSE(source.empty());
    source.data[0] = 0;
    source.data[1] = 100;
    source.data[2] = 100;
    source.data[3] = 200;

    mimage::Image resized;
    mimage::resize(source, resized, mimage::Size(4, 4));
    ASSERT_FALSE(resized.empty());
    EXPECT_EQ(resized.cols, 4);
    EXPECT_EQ(resized.rows, 4);
    EXPECT_EQ(resized.c, 1);
    EXPECT_EQ(resized.data[0], 0);
    EXPECT_EQ(resized.data[15], 200);
}

TEST(Image, DrawingWritesExpectedPixels) {
    mimage::Image image(8, 8, mimage::kPixelGray);
    ASSERT_FALSE(image.empty());
    image = mimage::Scalar(0);

    mimage::rectangle(image, mimage::Rect(1, 1, 4, 4), mimage::Scalar(255), 1);
    EXPECT_EQ(image.data[1 * 8 + 1], 255);
    EXPECT_EQ(image.data[1 * 8 + 4], 255);
    EXPECT_EQ(image.data[4 * 8 + 1], 255);
    EXPECT_EQ(image.data[2 * 8 + 2], 0);

    mimage::line(image, mimage::Point(0, 0), mimage::Point(7, 7), mimage::Scalar(128), 1);
    EXPECT_EQ(image.data[0], 128);
    EXPECT_EQ(image.data[7 * 8 + 7], 128);

    mimage::circle(image, mimage::Point(4, 4), 2, mimage::Scalar(64), 1);
    EXPECT_EQ(image.data[4 * 8 + 4], 128);
    EXPECT_EQ(image.data[4 * 8 + 2], 64);
}

TEST(Image, TextSizeAndTextDrawing) {
    std::int32_t baseline = -1;
    const mimage::Size size = mimage::getTextSize("hello", 0, 1.0, 1, &baseline);
    EXPECT_GT(size.width, 0);
    EXPECT_GT(size.height, 0);

    std::int32_t drawing_width = 0;
    std::int32_t drawing_height = 0;
    mimage::get_text_drawing_size("hello", 20, &drawing_width, &drawing_height);
    EXPECT_GT(drawing_width, 0);
    EXPECT_GT(drawing_height, 0);

    mimage::Image image(64, 128, mimage::kPixelGray);
    image = mimage::Scalar(0);
    mimage::putText(image, "hi", mimage::Point(2, 40), 0, 1.0, mimage::Scalar(255));
    std::int32_t lit = 0;
    for (std::size_t index = 0; index < image.total(); ++index) {
        lit += image.data[index] != 0 ? 1 : 0;
    }
    EXPECT_GT(lit, 0);
}

TEST(Image, ShowAndWaitKeyAreStubs) {
    mimage::Image image(2, 2, mimage::kPixelGray);
    image = mimage::Scalar(0);
    mimage::imshow("/tmp/mimage_unit_imshow", image);
    std::remove("/tmp/mimage_unit_imshow.png");
    EXPECT_EQ(mimage::waitKey(0), -1);
}
