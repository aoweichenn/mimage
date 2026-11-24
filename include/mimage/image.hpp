// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

// ncnn-compatible interleaved 8-bit image container.
//
// Image stores rows * cols * channels bytes in one refcounted allocation.
// Pixel conversions that operate on mmat::Mat live in mimage/pixel.hpp
// (future milestone); this header only covers the image container, codecs,
// resize and drawing.

#ifndef MIMAGE_IMAGE_HPP
#define MIMAGE_IMAGE_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <atomic>
#include <string>
#include <vector>

#include <mimage/config.hpp>
#include <mmemory/allocator.hpp>

namespace mimage {

// Pixel flags kept compatible with ncnn / simpleocv.
inline constexpr std::int32_t kPixelGray = 1;
inline constexpr std::int32_t kPixelRgb = 3;
inline constexpr std::int32_t kPixelRgba = 4;

enum ImreadModes : std::int32_t {
    IMREAD_UNCHANGED = -1,
    IMREAD_GRAYSCALE = 0,
    IMREAD_COLOR = 1,
};

enum ImwriteFlags : std::int32_t {
    IMWRITE_JPEG_QUALITY = 1,
};

struct Scalar {
    Scalar() : v{0, 0, 0, 0} {}

    explicit Scalar(std::uint8_t v0) : v{v0, 0, 0, 0} {}

    Scalar(std::uint8_t v0, std::uint8_t v1, std::uint8_t v2) : v{v0, v1, v2, 0} {}

    Scalar(std::uint8_t v0, std::uint8_t v1, std::uint8_t v2, std::uint8_t v3) : v{v0, v1, v2, v3} {}

    std::uint8_t operator[](std::int32_t index) const { return this->v[index]; }

    std::uint8_t& operator[](std::int32_t index) { return this->v[index]; }

    std::uint8_t v[4];
};

struct Point {
    Point() : x(0), y(0) {}

    Point(std::int32_t x_value, std::int32_t y_value) : x(x_value), y(y_value) {}

    std::int32_t x;
    std::int32_t y;
};

struct Size {
    Size() : width(0), height(0) {}

    Size(std::int32_t width_value, std::int32_t height_value) : width(width_value), height(height_value) {}

    std::int32_t width;
    std::int32_t height;
};

struct Rect {
    Rect() : x(0), y(0), width(0), height(0) {}

    Rect(std::int32_t x_value, std::int32_t y_value, std::int32_t width_value, std::int32_t height_value)
            : x(x_value), y(y_value), width(width_value), height(height_value) {}

    Rect(Point point, Size size) : x(point.x), y(point.y), width(size.width), height(size.height) {}

    std::int32_t area() const { return this->width * this->height; }

    std::int32_t x;
    std::int32_t y;
    std::int32_t width;
    std::int32_t height;
};

class MIMAGE_EXPORT Image {
public:
    Image() : data(nullptr), refcount(nullptr), rows(0), cols(0), c(0) {}

    Image(std::int32_t rows_value, std::int32_t cols_value, std::int32_t channels_value)
            : data(nullptr), refcount(nullptr), rows(0), cols(0), c(0) {
        this->create(rows_value, cols_value, channels_value);
    }

    // External storage: does not take ownership, refcount stays null.
    Image(std::int32_t rows_value, std::int32_t cols_value, std::int32_t channels_value, void* external_data)
            : data(static_cast<std::uint8_t*>(external_data)),
              refcount(nullptr),
              rows(rows_value),
              cols(cols_value),
              c(channels_value) {}

    Image(const Image& other)
            : data(other.data), refcount(other.refcount), rows(other.rows), cols(other.cols), c(other.c) {
        if (this->refcount != nullptr) {
            std::atomic_ref<std::int32_t>(*this->refcount).fetch_add(1, std::memory_order_acq_rel);
        }
    }

    ~Image() { this->release(); }

    Image& operator=(const Image& other) {
        if (this == &other) {
            return *this;
        }
        if (other.refcount != nullptr) {
            std::atomic_ref<std::int32_t>(*other.refcount).fetch_add(1, std::memory_order_acq_rel);
        }
        this->release();
        this->data = other.data;
        this->refcount = other.refcount;
        this->rows = other.rows;
        this->cols = other.cols;
        this->c = other.c;
        return *this;
    }

    Image& operator=(const Scalar& value) {
        if (this->total() > 0) {
            auto* ptr = this->data;
            const auto count = static_cast<std::size_t>(this->cols) * static_cast<std::size_t>(this->rows);
            for (std::size_t index = 0; index < count; ++index) {
                for (std::int32_t channel = 0; channel < this->c; ++channel) {
                    *ptr++ = value[channel];
                }
            }
        }
        return *this;
    }

    void create(std::int32_t rows_value, std::int32_t cols_value, std::int32_t channels_value) {
        this->release();
        if (rows_value <= 0 || cols_value <= 0 || channels_value <= 0) {
            return;
        }

        this->rows = rows_value;
        this->cols = cols_value;
        this->c = channels_value;

        const std::size_t total_size = this->total();
        if (total_size == 0) {
            return;
        }
        const std::size_t aligned_size = (total_size + 3U) / 4U * 4U;
        this->data = static_cast<std::uint8_t*>(mmemory::fast_malloc(aligned_size + sizeof(std::int32_t)));
        if (this->data == nullptr) {
            this->rows = 0;
            this->cols = 0;
            this->c = 0;
            return;
        }
        this->refcount = reinterpret_cast<std::int32_t*>(this->data + aligned_size);
        *this->refcount = 1;
    }

    void release() {
        if (this->refcount != nullptr &&
            std::atomic_ref<std::int32_t>(*this->refcount).fetch_add(-1, std::memory_order_acq_rel) == 1) {
            mmemory::fast_free(this->data);
        }
        this->data = nullptr;
        this->refcount = nullptr;
        this->rows = 0;
        this->cols = 0;
        this->c = 0;
    }

    [[nodiscard]] Image clone() const {
        if (this->empty()) {
            return Image();
        }
        Image result(this->rows, this->cols, this->c);
        if (result.total() > 0) {
            std::memcpy(result.data, this->data, this->total());
        }
        return result;
    }

    [[nodiscard]] bool empty() const { return this->data == nullptr || this->total() == 0; }

    [[nodiscard]] std::int32_t channels() const { return this->c; }

    [[nodiscard]] std::int32_t type() const { return this->c; }

    [[nodiscard]] std::size_t total() const {
        if (this->rows <= 0 || this->cols <= 0 || this->c <= 0) {
            return 0;
        }
        return static_cast<std::size_t>(this->cols) * static_cast<std::size_t>(this->rows) *
               static_cast<std::size_t>(this->c);
    }

    [[nodiscard]] const std::uint8_t* ptr(std::int32_t y) const {
        return this->data +
               static_cast<std::size_t>(y) * static_cast<std::size_t>(this->cols) * static_cast<std::size_t>(this->c);
    }

    [[nodiscard]] std::uint8_t* ptr(std::int32_t y) {
        return this->data +
               static_cast<std::size_t>(y) * static_cast<std::size_t>(this->cols) * static_cast<std::size_t>(this->c);
    }

    // Region of interest: deep copy, matching the ncnn simpleocv behaviour.
    [[nodiscard]] Image operator()(const Rect& roi) const {
        if (this->empty()) {
            return Image();
        }
        Image result(roi.height, roi.width, this->c);
        if (result.empty()) {
            return result;
        }
        std::int32_t source_y = roi.y;
        for (std::int32_t y = 0; y < roi.height; ++y) {
            const auto* source =
                this->ptr(source_y) + static_cast<std::size_t>(roi.x) * static_cast<std::size_t>(this->c);
            auto* destination = result.ptr(y);
            std::memcpy(destination, source, static_cast<std::size_t>(roi.width) * static_cast<std::size_t>(this->c));
            ++source_y;
        }
        return result;
    }

    std::uint8_t* data;
    // Null when the image wraps caller-owned memory.
    std::int32_t* refcount;

    std::int32_t rows;
    std::int32_t cols;
    std::int32_t c;
};

// Codecs. Return an empty Image on failure; the reason is available through
// the installed log handler.
MIMAGE_EXPORT Image imread(const std::string& path, std::int32_t flags = IMREAD_COLOR);
MIMAGE_EXPORT Image imdecode(const std::vector<std::uint8_t>& buffer, std::int32_t flags = IMREAD_COLOR);
MIMAGE_EXPORT Image imdecode(const std::uint8_t* encoded_data, std::size_t encoded_size,
                             std::int32_t flags = IMREAD_COLOR);
MIMAGE_EXPORT bool imwrite(const std::string& path, const Image& image, const std::vector<std::int32_t>& params = {});
MIMAGE_EXPORT void imshow(const std::string& name, const Image& image);
MIMAGE_EXPORT std::int32_t waitKey(std::int32_t delay = 0);

// Resize raw pixel buffers (ncnn-compatible entry points).
MIMAGE_EXPORT void resize_bilinear_c1(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                      std::int32_t w, std::int32_t h);
MIMAGE_EXPORT void resize_bilinear_c2(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                      std::int32_t w, std::int32_t h);
MIMAGE_EXPORT void resize_bilinear_c3(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                      std::int32_t w, std::int32_t h);
MIMAGE_EXPORT void resize_bilinear_c4(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch, std::uint8_t* dst,
                                      std::int32_t w, std::int32_t h);
MIMAGE_EXPORT void resize_bilinear_c1(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                      std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                      std::int32_t stride);
MIMAGE_EXPORT void resize_bilinear_c2(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                      std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                      std::int32_t stride);
MIMAGE_EXPORT void resize_bilinear_c3(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                      std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                      std::int32_t stride);
MIMAGE_EXPORT void resize_bilinear_c4(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                      std::int32_t srcstride, std::uint8_t* dst, std::int32_t w, std::int32_t h,
                                      std::int32_t stride);
MIMAGE_EXPORT void resize_bilinear_yuv420sp(const std::uint8_t* src, std::int32_t srcw, std::int32_t srch,
                                            std::uint8_t* dst, std::int32_t w, std::int32_t h);

MIMAGE_EXPORT void resize(const Image& src, Image& dst, const Size& size, float scale_w = 0.0F, float scale_h = 0.0F,
                          std::int32_t flags = 0);

// Drawing raw entry points (ncnn-compatible).
MIMAGE_EXPORT void draw_rectangle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx,
                                     std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color,
                                     std::int32_t thickness);
MIMAGE_EXPORT void draw_rectangle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx,
                                     std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color,
                                     std::int32_t thickness);
MIMAGE_EXPORT void draw_rectangle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx,
                                     std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color,
                                     std::int32_t thickness);
MIMAGE_EXPORT void draw_rectangle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx,
                                     std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color,
                                     std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx,
                                  std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx,
                                  std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx,
                                  std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx,
                                  std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                                std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                                std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                                std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                                std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_text_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                                std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color);
MIMAGE_EXPORT void draw_text_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                                std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color);
MIMAGE_EXPORT void draw_text_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                                std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color);
MIMAGE_EXPORT void draw_text_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                                std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color);
MIMAGE_EXPORT void get_text_drawing_size(const char* text, std::int32_t fontpixelsize, std::int32_t* w,
                                         std::int32_t* h);

MIMAGE_EXPORT void draw_rectangle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                     std::int32_t rx, std::int32_t ry, std::int32_t rw, std::int32_t rh,
                                     std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_rectangle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                     std::int32_t rx, std::int32_t ry, std::int32_t rw, std::int32_t rh,
                                     std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_rectangle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                     std::int32_t rx, std::int32_t ry, std::int32_t rw, std::int32_t rh,
                                     std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_rectangle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                     std::int32_t rx, std::int32_t ry, std::int32_t rw, std::int32_t rh,
                                     std::uint32_t color, std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                  std::int32_t cx, std::int32_t cy, std::int32_t radius, std::uint32_t color,
                                  std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                  std::int32_t cx, std::int32_t cy, std::int32_t radius, std::uint32_t color,
                                  std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                  std::int32_t cx, std::int32_t cy, std::int32_t radius, std::uint32_t color,
                                  std::int32_t thickness);
MIMAGE_EXPORT void draw_circle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                  std::int32_t cx, std::int32_t cy, std::int32_t radius, std::uint32_t color,
                                  std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color,
                                std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color,
                                std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color,
                                std::int32_t thickness);
MIMAGE_EXPORT void draw_line_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color,
                                std::int32_t thickness);
MIMAGE_EXPORT void draw_text_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                const char* text, std::int32_t x, std::int32_t y, std::int32_t fontpixelsize,
                                std::uint32_t color);
MIMAGE_EXPORT void draw_text_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                const char* text, std::int32_t x, std::int32_t y, std::int32_t fontpixelsize,
                                std::uint32_t color);
MIMAGE_EXPORT void draw_text_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                const char* text, std::int32_t x, std::int32_t y, std::int32_t fontpixelsize,
                                std::uint32_t color);
MIMAGE_EXPORT void draw_text_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride,
                                const char* text, std::int32_t x, std::int32_t y, std::int32_t fontpixelsize,
                                std::uint32_t color);

MIMAGE_EXPORT void rectangle(Image& image, Point pt1, Point pt2, const Scalar& color, std::int32_t thickness = 1);

MIMAGE_EXPORT void rectangle(Image& image, Rect rect, const Scalar& color, std::int32_t thickness = 1);
MIMAGE_EXPORT void circle(Image& image, Point center, std::int32_t radius, const Scalar& color,
                          std::int32_t thickness = 1);
MIMAGE_EXPORT void line(Image& image, Point p0, Point p1, const Scalar& color, std::int32_t thickness = 1);
MIMAGE_EXPORT void putText(Image& image, const std::string& text, Point org, std::int32_t font_face, double font_scale,
                           Scalar color, std::int32_t thickness = 1);
MIMAGE_EXPORT Size getTextSize(const std::string& text, std::int32_t font_face, double font_scale,
                               std::int32_t thickness, std::int32_t* base_line);

}  // namespace mimage

#endif  // MIMAGE_IMAGE_HPP
