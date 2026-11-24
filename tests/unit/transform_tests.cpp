// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <cstdint>
#include <cstring>

#include <algorithm>
#include <array>

#include <gtest/gtest.h>
#include <mimage/transform.hpp>

TEST(Transform, KannaRotateIdentityKeepsPixels) {
    const std::uint8_t source[4] = {1, 2, 3, 4};
    std::uint8_t gray[4] = {};
    mimage::kanna_rotate_c1(source, 2, 2, gray, 2, 2, 1);
    EXPECT_EQ(std::memcmp(source, gray, 4), 0);

    std::uint8_t gray2[2] = {7, 8};
    std::uint8_t out2[2] = {};
    mimage::kanna_rotate_c2(gray2, 1, 1, out2, 1, 1, 1);
    EXPECT_EQ(std::memcmp(gray2, out2, 2), 0);

    std::uint8_t rgb[3] = {1, 2, 3};
    std::uint8_t rgb_out[3] = {};
    mimage::kanna_rotate_c3(rgb, 1, 1, rgb_out, 1, 1, 1);
    EXPECT_EQ(std::memcmp(rgb, rgb_out, 3), 0);

    std::uint8_t rgba[4] = {1, 2, 3, 4};
    std::uint8_t rgba_out[4] = {};
    mimage::kanna_rotate_c4(rgba, 1, 1, rgba_out, 1, 1, 1);
    EXPECT_EQ(std::memcmp(rgba, rgba_out, 4), 0);
}

TEST(Transform, KannaRotateChangesDimensions) {
    const std::uint8_t source[6] = {1, 2, 3, 4, 5, 6};
    std::uint8_t rotated[6] = {};
    mimage::kanna_rotate_c1(source, 2, 3, rotated, 3, 2, 5);
    std::array<std::uint8_t, 6> sorted_source = {1, 2, 3, 4, 5, 6};
    std::array<std::uint8_t, 6> sorted_rotated = {rotated[0], rotated[1], rotated[2],
                                                  rotated[3], rotated[4], rotated[5]};
    std::sort(sorted_source.begin(), sorted_source.end());
    std::sort(sorted_rotated.begin(), sorted_rotated.end());
    EXPECT_EQ(sorted_source, sorted_rotated);
}

TEST(Transform, KannaRotateYuvIdentity) {
    const std::uint8_t yuv[6] = {128, 128, 128, 128, 128, 128};
    std::uint8_t output[6] = {};
    mimage::kanna_rotate_yuv420sp(yuv, 2, 2, output, 2, 2, 1);
    EXPECT_EQ(std::memcmp(yuv, output, sizeof(yuv)), 0);
}

TEST(Transform, RotationMatrixIsIdentityAtZero) {
    std::array<float, 6> matrix = {};
    mimage::get_rotation_matrix(0.0F, 1.0F, 0.0F, 0.0F, matrix.data());
    EXPECT_FLOAT_EQ(matrix[0], 1.0F);
    EXPECT_FLOAT_EQ(matrix[1], 0.0F);
    EXPECT_FLOAT_EQ(matrix[2], 0.0F);
    EXPECT_FLOAT_EQ(matrix[3], 0.0F);
    EXPECT_FLOAT_EQ(matrix[4], 1.0F);
    EXPECT_FLOAT_EQ(matrix[5], 0.0F);

    std::array<float, 6> inverse = {};
    mimage::invert_affine_transform(matrix.data(), inverse.data());
    for (std::size_t index = 0; index < matrix.size(); ++index) {
        EXPECT_FLOAT_EQ(inverse[index], matrix[index]);
    }
}

TEST(Transform, AffineTransformFromTranslationPoints) {
    const float from[4] = {0.0F, 0.0F, 1.0F, 0.0F};
    const float to[4] = {1.0F, 2.0F, 2.0F, 2.0F};
    std::array<float, 6> matrix = {};
    mimage::get_affine_transform(from, to, 2, matrix.data());
    EXPECT_FLOAT_EQ(matrix[0], 1.0F);
    EXPECT_FLOAT_EQ(matrix[1], 0.0F);
    EXPECT_FLOAT_EQ(matrix[2], 1.0F);
    EXPECT_FLOAT_EQ(matrix[3], 0.0F);
    EXPECT_FLOAT_EQ(matrix[4], 1.0F);
    EXPECT_FLOAT_EQ(matrix[5], 2.0F);
}

TEST(Transform, WarpAffineIdentityKeepsPixels) {
    const std::uint8_t source[4] = {10, 20, 30, 40};
    std::uint8_t gray[4] = {};
    const float identity[6] = {1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F};
    mimage::warpaffine_bilinear_c1(source, 2, 2, gray, 2, 2, identity);
    EXPECT_EQ(std::memcmp(source, gray, 4), 0);

    std::uint8_t c2_in[2] = {1, 2};
    std::uint8_t c2_out[2] = {};
    mimage::warpaffine_bilinear_c2(c2_in, 1, 1, c2_out, 1, 1, identity);
    EXPECT_EQ(std::memcmp(c2_in, c2_out, 2), 0);

    std::uint8_t c3_in[3] = {1, 2, 3};
    std::uint8_t c3_out[3] = {};
    mimage::warpaffine_bilinear_c3(c3_in, 1, 1, c3_out, 1, 1, identity);
    EXPECT_EQ(std::memcmp(c3_in, c3_out, 3), 0);

    std::uint8_t c4_in[4] = {1, 2, 3, 4};
    std::uint8_t c4_out[4] = {};
    mimage::warpaffine_bilinear_c4(c4_in, 1, 1, c4_out, 1, 1, identity);
    EXPECT_EQ(std::memcmp(c4_in, c4_out, 4), 0);
}

TEST(Transform, WarpAffineYuvIsCallable) {
    const std::uint8_t yuv[6] = {128, 128, 128, 128, 128, 128};
    std::uint8_t output[6] = {};
    const float identity[6] = {1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F};
    mimage::warpaffine_bilinear_yuv420sp(yuv, 2, 2, output, 2, 2, identity);
    EXPECT_EQ(std::memcmp(yuv, output, sizeof(yuv)), 0);
}
