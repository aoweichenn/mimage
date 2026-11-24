// Copyright 2019 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

// Pixel rotate (kanna) and affine transform entry points.

#ifndef MIMAGE_TRANSFORM_HPP
#define MIMAGE_TRANSFORM_HPP

#include <cstdint>

#include <mimage/config.hpp>

namespace mimage {

MIMAGE_EXPORT void kanna_rotate_c1(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                   std::int32_t w, std::int32_t h, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_c2(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                   std::int32_t w, std::int32_t h, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_c3(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                   std::int32_t w, std::int32_t h, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_c4(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                   std::int32_t w, std::int32_t h, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_c1(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                   std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                   std::int32_t stride, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_c2(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                   std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                   std::int32_t stride, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_c3(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                   std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                   std::int32_t stride, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_c4(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                   std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                   std::int32_t stride, std::int32_t type);
MIMAGE_EXPORT void kanna_rotate_yuv420sp(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                         std::uint8_t* dst, std::int32_t w, std::int32_t h, std::int32_t type);

MIMAGE_EXPORT void get_rotation_matrix(float angle, float scale, float dx, float dy, float* tm);
MIMAGE_EXPORT void get_affine_transform(const float* points_from, const float* points_to, std::int32_t num_point,
                                        float* tm);
MIMAGE_EXPORT void invert_affine_transform(const float* tm, float* tm_inv);
MIMAGE_EXPORT void warpaffine_bilinear_c1(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::uint8_t* dst, std::int32_t w, std::int32_t h, const float* tm,
                                          std::int32_t type = 0, std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_c2(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::uint8_t* dst, std::int32_t w, std::int32_t h, const float* tm,
                                          std::int32_t type = 0, std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_c3(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::uint8_t* dst, std::int32_t w, std::int32_t h, const float* tm,
                                          std::int32_t type = 0, std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_c4(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::uint8_t* dst, std::int32_t w, std::int32_t h, const float* tm,
                                          std::int32_t type = 0, std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_c1(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                          std::int32_t stride, const float* tm, std::int32_t type = 0,
                                          std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_c2(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                          std::int32_t stride, const float* tm, std::int32_t type = 0,
                                          std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_c3(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                          std::int32_t stride, const float* tm, std::int32_t type = 0,
                                          std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_c4(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                          std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                          std::int32_t stride, const float* tm, std::int32_t type = 0,
                                          std::uint32_t v = 0);
MIMAGE_EXPORT void warpaffine_bilinear_yuv420sp(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                                std::uint8_t* dst, std::int32_t w, std::int32_t h, const float* tm,
                                                std::int32_t type = 0, std::uint32_t v = 0);

}  // namespace mimage

#endif  // MIMAGE_TRANSFORM_HPP
