// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

// Pixel layout conversion between interleaved images and mmat::Mat planes.
//
// The enum values and function behaviour match ncnn's Mat::PixelType and
// Mat::from_pixels/to_pixels so the ncnn layer can forward without changing
// callers.

#ifndef MIMAGE_PIXEL_HPP
#define MIMAGE_PIXEL_HPP

#include <cstdint>

#include <mimage/config.hpp>
#include <mmat/mat.hpp>
#include <mmemory/allocator.hpp>

namespace mimage {

enum PixelType {
    PIXEL_CONVERT_SHIFT = 16,
    PIXEL_FORMAT_MASK = 0x0000ffff,
    PIXEL_CONVERT_MASK = 0xffff0000,

    PIXEL_RGB = 1,
    PIXEL_BGR = 2,
    PIXEL_GRAY = 3,
    PIXEL_RGBA = 4,
    PIXEL_BGRA = 5,

    PIXEL_RGB2BGR = PIXEL_RGB | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
    PIXEL_RGB2GRAY = PIXEL_RGB | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
    PIXEL_RGB2RGBA = PIXEL_RGB | (PIXEL_RGBA << PIXEL_CONVERT_SHIFT),
    PIXEL_RGB2BGRA = PIXEL_RGB | (PIXEL_BGRA << PIXEL_CONVERT_SHIFT),

    PIXEL_BGR2RGB = PIXEL_BGR | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
    PIXEL_BGR2GRAY = PIXEL_BGR | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
    PIXEL_BGR2RGBA = PIXEL_BGR | (PIXEL_RGBA << PIXEL_CONVERT_SHIFT),
    PIXEL_BGR2BGRA = PIXEL_BGR | (PIXEL_BGRA << PIXEL_CONVERT_SHIFT),

    PIXEL_GRAY2RGB = PIXEL_GRAY | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
    PIXEL_GRAY2BGR = PIXEL_GRAY | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
    PIXEL_GRAY2RGBA = PIXEL_GRAY | (PIXEL_RGBA << PIXEL_CONVERT_SHIFT),
    PIXEL_GRAY2BGRA = PIXEL_GRAY | (PIXEL_BGRA << PIXEL_CONVERT_SHIFT),

    PIXEL_RGBA2RGB = PIXEL_RGBA | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
    PIXEL_RGBA2BGR = PIXEL_RGBA | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
    PIXEL_RGBA2GRAY = PIXEL_RGBA | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
    PIXEL_RGBA2BGRA = PIXEL_RGBA | (PIXEL_BGRA << PIXEL_CONVERT_SHIFT),

    PIXEL_BGRA2RGB = PIXEL_BGRA | (PIXEL_RGB << PIXEL_CONVERT_SHIFT),
    PIXEL_BGRA2BGR = PIXEL_BGRA | (PIXEL_BGR << PIXEL_CONVERT_SHIFT),
    PIXEL_BGRA2GRAY = PIXEL_BGRA | (PIXEL_GRAY << PIXEL_CONVERT_SHIFT),
    PIXEL_BGRA2RGBA = PIXEL_BGRA | (PIXEL_RGBA << PIXEL_CONVERT_SHIFT),
};

MIMAGE_EXPORT mmat::Mat from_pixels(const std::uint8_t* pixels, std::int32_t type, std::int32_t w, std::int32_t h,
                                    mmemory::Allocator* allocator = nullptr);
MIMAGE_EXPORT mmat::Mat from_pixels(const std::uint8_t* pixels, std::int32_t type, std::int32_t w, std::int32_t h,
                                    std::int32_t stride, mmemory::Allocator* allocator = nullptr);
MIMAGE_EXPORT mmat::Mat from_pixels_resize(const std::uint8_t* pixels, std::int32_t type, std::int32_t w,
                                           std::int32_t h, std::int32_t target_width, std::int32_t target_height,
                                           mmemory::Allocator* allocator = nullptr);
MIMAGE_EXPORT mmat::Mat from_pixels_resize(const std::uint8_t* pixels, std::int32_t type, std::int32_t w,
                                           std::int32_t h, std::int32_t stride, std::int32_t target_width,
                                           std::int32_t target_height, mmemory::Allocator* allocator = nullptr);
MIMAGE_EXPORT mmat::Mat from_pixels_roi(const std::uint8_t* pixels, std::int32_t type, std::int32_t w, std::int32_t h,
                                        std::int32_t roix, std::int32_t roiy, std::int32_t roiw, std::int32_t roih,
                                        mmemory::Allocator* allocator = nullptr);
MIMAGE_EXPORT mmat::Mat from_pixels_roi(const std::uint8_t* pixels, std::int32_t type, std::int32_t w, std::int32_t h,
                                        std::int32_t stride, std::int32_t roix, std::int32_t roiy, std::int32_t roiw,
                                        std::int32_t roih, mmemory::Allocator* allocator = nullptr);
MIMAGE_EXPORT mmat::Mat from_pixels_roi_resize(const std::uint8_t* pixels, std::int32_t type, std::int32_t w,
                                               std::int32_t h, std::int32_t roix, std::int32_t roiy, std::int32_t roiw,
                                               std::int32_t roih, std::int32_t target_width, std::int32_t target_height,
                                               mmemory::Allocator* allocator = nullptr);
MIMAGE_EXPORT mmat::Mat from_pixels_roi_resize(const std::uint8_t* pixels, std::int32_t type, std::int32_t w,
                                               std::int32_t h, std::int32_t stride, std::int32_t roix,
                                               std::int32_t roiy, std::int32_t roiw, std::int32_t roih,
                                               std::int32_t target_width, std::int32_t target_height,
                                               mmemory::Allocator* allocator = nullptr);

MIMAGE_EXPORT void to_pixels(const mmat::Mat& mat, std::uint8_t* pixels, std::int32_t type);
MIMAGE_EXPORT void to_pixels(const mmat::Mat& mat, std::uint8_t* pixels, std::int32_t type, std::int32_t stride);
MIMAGE_EXPORT void to_pixels_resize(const mmat::Mat& mat, std::uint8_t* pixels, std::int32_t type,
                                    std::int32_t target_width, std::int32_t target_height);
MIMAGE_EXPORT void to_pixels_resize(const mmat::Mat& mat, std::uint8_t* pixels, std::int32_t type,
                                    std::int32_t target_width, std::int32_t target_height, std::int32_t target_stride);

MIMAGE_EXPORT void yuv420sp2rgb(const std::uint8_t* yuv420sp, std::int32_t w, std::int32_t h, std::uint8_t* rgb);
MIMAGE_EXPORT void yuv420sp2rgb_nv12(const std::uint8_t* yuv420sp, std::int32_t w, std::int32_t h, std::uint8_t* rgb);
MIMAGE_EXPORT void yuv420sp2rgb_half(const std::uint8_t* yuv, std::int32_t w, std::int32_t h, std::uint8_t* rgb);

}  // namespace mimage

#endif  // MIMAGE_PIXEL_HPP
