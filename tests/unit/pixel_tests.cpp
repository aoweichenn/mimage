// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>

#include <vector>

#include <gtest/gtest.h>
#include <mimage/pixel.hpp>

namespace {

const std::uint8_t kRgb2x2[12] = {
    255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255,
};

}  // namespace

TEST(Pixel, FromRgbCreatesPlanarMat) {
    const mmat::Mat mat = mimage::from_pixels(kRgb2x2, mimage::PIXEL_RGB, 2, 2);
    ASSERT_FALSE(mat.empty());
    EXPECT_EQ(mat.dims, 3);
    EXPECT_EQ(mat.w, 2);
    EXPECT_EQ(mat.h, 2);
    EXPECT_EQ(mat.c, 3);

    const auto* red = static_cast<const float*>(mat.channel(0));
    const auto* green = static_cast<const float*>(mat.channel(1));
    const auto* blue = static_cast<const float*>(mat.channel(2));
    EXPECT_FLOAT_EQ(red[0], 255.0F);
    EXPECT_FLOAT_EQ(green[1], 255.0F);
    EXPECT_FLOAT_EQ(blue[2], 255.0F);
}

TEST(Pixel, FromPixelsWithStride) {
    const std::uint8_t padded[16] = {
        255, 0, 0, 0, 0, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 0,
    };
    const mmat::Mat mat = mimage::from_pixels(padded, mimage::PIXEL_RGB, 2, 2, 8);
    ASSERT_FALSE(mat.empty());
    EXPECT_EQ(mat.w, 2);
    EXPECT_EQ(mat.h, 2);
    const auto* red = static_cast<const float*>(mat.channel(0));
    EXPECT_FLOAT_EQ(red[0], 255.0F);
    EXPECT_FLOAT_EQ(red[1], 0.0F);
}

TEST(Pixel, FromPixelsResize) {
    const mmat::Mat mat = mimage::from_pixels_resize(kRgb2x2, mimage::PIXEL_RGB, 2, 2, 4, 3);
    ASSERT_FALSE(mat.empty());
    EXPECT_EQ(mat.w, 4);
    EXPECT_EQ(mat.h, 3);
    EXPECT_EQ(mat.c, 3);
}

TEST(Pixel, FromPixelsRoi) {
    const std::uint8_t rgb4x4[48] = {
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    };
    const mmat::Mat mat = mimage::from_pixels_roi(rgb4x4, mimage::PIXEL_RGB, 4, 4, 1, 1, 2, 2);
    ASSERT_FALSE(mat.empty());
    EXPECT_EQ(mat.w, 2);
    EXPECT_EQ(mat.h, 2);
    const auto* red = static_cast<const float*>(mat.channel(0));
    EXPECT_FLOAT_EQ(red[0], 16.0F);
}

TEST(Pixel, ToPixelsRoundTrip) {
    const mmat::Mat mat = mimage::from_pixels(kRgb2x2, mimage::PIXEL_RGB, 2, 2);
    std::vector<std::uint8_t> output(12, 0);
    mimage::to_pixels(mat, output.data(), mimage::PIXEL_RGB);
    EXPECT_EQ(std::memcmp(output.data(), kRgb2x2, sizeof(kRgb2x2)), 0);
}

TEST(Pixel, ToPixelsWritesGray) {
    const std::uint8_t gray_in[4] = {10, 20, 30, 40};
    const mmat::Mat mat = mimage::from_pixels(gray_in, mimage::PIXEL_GRAY, 2, 2);
    std::vector<std::uint8_t> gray_out(4, 0);
    mimage::to_pixels(mat, gray_out.data(), mimage::PIXEL_GRAY);
    EXPECT_EQ(gray_out[0], 10);
    EXPECT_EQ(gray_out[3], 40);
}

TEST(Pixel, ToPixelsResize) {
    const mmat::Mat mat = mimage::from_pixels(kRgb2x2, mimage::PIXEL_RGB, 2, 2);
    std::vector<std::uint8_t> output(4 * 3 * 3, 0);
    mimage::to_pixels_resize(mat, output.data(), mimage::PIXEL_RGB, 4, 3);
    EXPECT_GT(output[0], 0);
}

TEST(Pixel, Yuv420SpToRgb) {
    const std::uint8_t yuv[6] = {128, 128, 128, 128, 128, 128};
    std::uint8_t rgb[12] = {};
    mimage::yuv420sp2rgb(yuv, 2, 2, rgb);
    for (std::size_t index = 0; index < sizeof(rgb); ++index) {
        EXPECT_NEAR(rgb[index], 128, 3);
    }
}

TEST(Pixel, FromPixelsRoiResize) {
    const std::uint8_t rgb4x4[48] = {
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    };
    const mmat::Mat mat = mimage::from_pixels_roi_resize(rgb4x4, mimage::PIXEL_RGB, 4, 4, 1, 1, 2, 2, 3, 3);
    ASSERT_FALSE(mat.empty());
    EXPECT_EQ(mat.w, 3);
    EXPECT_EQ(mat.h, 3);
    EXPECT_EQ(mat.c, 3);
}

TEST(Pixel, ToPixelsWithStride) {
    const mmat::Mat mat = mimage::from_pixels(kRgb2x2, mimage::PIXEL_RGB, 2, 2);
    std::vector<std::uint8_t> output(16, 0);
    mimage::to_pixels(mat, output.data(), mimage::PIXEL_RGB, 8);
    EXPECT_EQ(output[0], 255);
    EXPECT_EQ(output[3], 0);
    EXPECT_EQ(output[8], 0);
}

TEST(Pixel, ToPixelsResizeWithStride) {
    const mmat::Mat mat = mimage::from_pixels(kRgb2x2, mimage::PIXEL_RGB, 2, 2);
    std::vector<std::uint8_t> output(4 * 3 * 4, 0);
    mimage::to_pixels_resize(mat, output.data(), mimage::PIXEL_RGB, 4, 3, 12);
    EXPECT_GT(output[0], 0);
}

TEST(Pixel, Yuv420SpNv12ToRgb) {
    const std::uint8_t yuv[6] = {128, 128, 128, 128, 128, 128};
    std::uint8_t rgb[12] = {};
    mimage::yuv420sp2rgb_nv12(yuv, 2, 2, rgb);
    for (std::size_t index = 0; index < sizeof(rgb); ++index) {
        EXPECT_NEAR(rgb[index], 128, 3);
    }
}

TEST(Pixel, Yuv420SpHalfToRgb) {
    const std::uint8_t yuv[24] = {
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    };
    std::uint8_t rgb[12] = {};
    mimage::yuv420sp2rgb_half(yuv, 4, 4, rgb);
    for (std::size_t index = 0; index < sizeof(rgb); ++index) {
        EXPECT_NEAR(rgb[index], 128, 3);
    }
}
