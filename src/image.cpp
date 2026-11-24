// Copyright 2017 Tencent
// Copyright 2026 mcnn authors
// SPDX-License-Identifier: BSD-3-Clause

#include <mimage/image.hpp>

#include <cmath>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <utility>
#include <vector>

#include <mimage/log.hpp>

#define STB_IMAGE_IMPLEMENTATION
#if defined(__ARM_NEON)
#define STBI_NEON
#endif
#define STBI_NO_THREAD_LOCALS
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_PNM
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace mimage {
namespace {

constexpr std::int32_t kStbUnchangedChannels = 0;
constexpr std::int32_t kStbGrayChannels = 1;
constexpr std::int32_t kStbRgbChannels = 3;
constexpr std::int32_t kStbRgbaChannels = 4;
constexpr std::int32_t kDefaultJpegQuality = 95;
constexpr double kFontPixelScale = 20.0;

std::int32_t desired_channels(std::int32_t flags) {
    if (flags == IMREAD_UNCHANGED) {
        return kStbUnchangedChannels;
    }
    if (flags == IMREAD_GRAYSCALE) {
        return kStbGrayChannels;
    }
    if (flags == IMREAD_COLOR) {
        return kStbRgbChannels;
    }
    MIMAGE_LOGE("imread: unknown flags %d", flags);
    return -1;
}

Image take_decoded(std::uint8_t* pixeldata, std::int32_t width, std::int32_t height, std::int32_t channels,
                   std::int32_t wanted_channels) {
    const std::int32_t effective_channels = wanted_channels != 0 ? wanted_channels : channels;
    Image decoded;

    if (effective_channels == kStbGrayChannels) {
        decoded.create(height, width, kPixelGray);
        if (decoded.empty()) {
            stbi_image_free(pixeldata);
            MIMAGE_LOGE("imread: allocation failed for %dx%dx1", width, height);
            return Image();
        }
        std::memcpy(decoded.data, pixeldata, decoded.total());
    } else if (effective_channels == kStbRgbChannels) {
        decoded.create(height, width, kPixelRgb);
        if (decoded.empty()) {
            stbi_image_free(pixeldata);
            MIMAGE_LOGE("imread: allocation failed for %dx%dx3", width, height);
            return Image();
        }
        std::memcpy(decoded.data, pixeldata, decoded.total());
        auto* pixel = decoded.data;
        for (std::int32_t index = 0; index < width * height; ++index) {
            std::swap(pixel[0], pixel[2]);
            pixel += kStbRgbChannels;
        }
    } else if (effective_channels == kStbRgbaChannels) {
        decoded.create(height, width, kPixelRgba);
        if (decoded.empty()) {
            stbi_image_free(pixeldata);
            MIMAGE_LOGE("imread: allocation failed for %dx%dx4", width, height);
            return Image();
        }
        std::memcpy(decoded.data, pixeldata, decoded.total());
        auto* pixel = decoded.data;
        for (std::int32_t index = 0; index < width * height; ++index) {
            std::swap(pixel[0], pixel[2]);
            pixel += kStbRgbaChannels;
        }
    } else {
        stbi_image_free(pixeldata);
        MIMAGE_LOGE("imread: unsupported channel count %d", effective_channels);
        return Image();
    }

    stbi_image_free(pixeldata);
    return decoded;
}
}  // namespace

Image imread(const std::string& path, std::int32_t flags) {
    const std::int32_t wanted_channels = desired_channels(flags);
    if (wanted_channels < 0) {
        return Image();
    }

    // stb_image exposes int-typed dimensions.
    int width = 0;     // check-types: allow stb_image ABI
    int height = 0;    // check-types: allow stb_image ABI
    int channels = 0;  // check-types: allow stb_image ABI
    auto* pixeldata = stbi_load(path.c_str(), &width, &height, &channels, wanted_channels);
    if (pixeldata == nullptr) {
        MIMAGE_LOGE("imread: failed to decode '%s': %s", path.c_str(), stbi_failure_reason());
        return Image();
    }

    return take_decoded(pixeldata, static_cast<std::int32_t>(width), static_cast<std::int32_t>(height),
                        static_cast<std::int32_t>(channels), wanted_channels);
}

Image imdecode(const std::uint8_t* encoded_data, std::size_t encoded_size, std::int32_t flags) {
    if (encoded_data == nullptr || encoded_size == 0) {
        MIMAGE_LOGE("imdecode: empty input");
        return Image();
    }

    const std::int32_t wanted_channels = desired_channels(flags);
    if (wanted_channels < 0) {
        return Image();
    }

    int width = 0;     // check-types: allow stb_image ABI
    int height = 0;    // check-types: allow stb_image ABI
    int channels = 0;  // check-types: allow stb_image ABI
    auto* pixeldata = stbi_load_from_memory(encoded_data, static_cast<std::int32_t>(encoded_size), &width, &height,
                                            &channels, wanted_channels);
    if (pixeldata == nullptr) {
        MIMAGE_LOGE("imdecode: failed to decode %zu bytes: %s", encoded_size, stbi_failure_reason());
        return Image();
    }

    return take_decoded(pixeldata, static_cast<std::int32_t>(width), static_cast<std::int32_t>(height),
                        static_cast<std::int32_t>(channels), wanted_channels);
}

Image imdecode(const std::vector<std::uint8_t>& buffer, std::int32_t flags) {
    return imdecode(buffer.data(), buffer.size(), flags);
}

bool imwrite(const std::string& path, const Image& image, const std::vector<std::int32_t>& params) {
    const char* extension = std::strrchr(path.c_str(), '.');
    if (extension == nullptr) {
        MIMAGE_LOGE("imwrite: missing extension in '%s'", path.c_str());
        return false;
    }

    const std::string ext(extension);
    Image rgb = image.clone();
    if (rgb.empty()) {
        MIMAGE_LOGE("imwrite: empty image for '%s'", path.c_str());
        return false;
    }

    std::int32_t channels = 0;
    if (rgb.type() == kPixelGray) {
        channels = kStbGrayChannels;
    } else if (rgb.type() == kPixelRgb) {
        channels = kStbRgbChannels;
        auto* pixel = rgb.data;
        for (std::int32_t index = 0; index < rgb.cols * rgb.rows; ++index) {
            std::swap(pixel[0], pixel[2]);
            pixel += kStbRgbChannels;
        }
    } else if (rgb.type() == kPixelRgba) {
        channels = kStbRgbaChannels;
        auto* pixel = rgb.data;
        for (std::int32_t index = 0; index < rgb.cols * rgb.rows; ++index) {
            std::swap(pixel[0], pixel[2]);
            pixel += kStbRgbaChannels;
        }
    } else {
        MIMAGE_LOGE("imwrite: unsupported channel count %d for '%s'", rgb.type(), path.c_str());
        return false;
    }

    if (ext == ".jpg" || ext == ".jpeg" || ext == ".JPG" || ext == ".JPEG") {
        std::int32_t quality = kDefaultJpegQuality;
        for (std::size_t index = 0; index + 1 < params.size(); index += 2) {
            if (params[index] == IMWRITE_JPEG_QUALITY) {
                quality = params[index + 1];
                break;
            }
        }
        return stbi_write_jpg(path.c_str(), rgb.cols, rgb.rows, channels, rgb.data, quality) != 0;
    }
    if (ext == ".png" || ext == ".PNG") {
        return stbi_write_png(path.c_str(), rgb.cols, rgb.rows, channels, rgb.data, 0) != 0;
    }
    if (ext == ".bmp" || ext == ".BMP") {
        return stbi_write_bmp(path.c_str(), rgb.cols, rgb.rows, channels, rgb.data) != 0;
    }

    MIMAGE_LOGE("imwrite: unsupported extension '%s'", ext.c_str());
    return false;
}

void imshow(const std::string& name, const Image& image) {
    MIMAGE_LOGI("imshow: saving '%s.png'", name.c_str());
    (void)imwrite(name + ".png", image);
}

std::int32_t waitKey(std::int32_t delay) {
    MIMAGE_LOGW("waitKey is a stub; delay %d ignored", delay);
    return -1;
}

void resize(const Image& src, Image& dst, const Size& size, float scale_w, float scale_h, std::int32_t flags) {
    (void)flags;

    const std::int32_t source_width = src.cols;
    const std::int32_t source_height = src.rows;
    std::int32_t width = size.width;
    std::int32_t height = size.height;

    if (width == 0 || height == 0) {
        width = static_cast<std::int32_t>(static_cast<float>(source_width) * scale_w);
        height = static_cast<std::int32_t>(static_cast<float>(source_height) * scale_h);
    }
    if (width <= 0 || height <= 0) {
        MIMAGE_LOGW("resize: invalid target size %dx%d", width, height);
        return;
    }
    if (width == source_width && height == source_height) {
        dst = src.clone();
        return;
    }

    Image resized(height, width, src.c);
    if (resized.empty()) {
        MIMAGE_LOGE("resize: allocation failed for %dx%dx%d", width, height, src.c);
        return;
    }

    if (src.c == kPixelGray) {
        resize_bilinear_c1(src.data, source_width, source_height, resized.data, width, height);
    } else if (src.c == kPixelRgb) {
        resize_bilinear_c3(src.data, source_width, source_height, resized.data, width, height);
    } else if (src.c == kPixelRgba) {
        resize_bilinear_c4(src.data, source_width, source_height, resized.data, width, height);
    } else {
        MIMAGE_LOGE("resize: unsupported channel count %d", src.c);
        return;
    }

    dst = resized;
}

static std::uint32_t pack_color(const Scalar& scalar, std::int32_t channels) {
    std::uint32_t color = 0;
    auto* bytes = reinterpret_cast<std::uint8_t*>(&color);
    for (std::int32_t channel = 0; channel < channels; ++channel) {
        bytes[channel] = scalar[channel];
    }
    return color;
}

void rectangle(Image& image, Point pt1, Point pt2, const Scalar& color, std::int32_t thickness) {
    Rect rect;
    rect.x = std::min(pt1.x, pt2.x);
    rect.y = std::min(pt1.y, pt2.y);
    rect.width = std::max(pt1.x, pt2.x) - rect.x;
    rect.height = std::max(pt1.y, pt2.y) - rect.y;
    rectangle(image, rect, color, thickness);
}

void rectangle(Image& image, Rect rect, const Scalar& color, std::int32_t thickness) {
    const std::uint32_t packed = pack_color(color, image.c);
    if (image.c == kPixelGray) {
        draw_rectangle_c1(image.data, image.cols, image.rows, rect.x, rect.y, rect.width, rect.height, packed,
                          thickness);
    } else if (image.c == kPixelRgb) {
        draw_rectangle_c3(image.data, image.cols, image.rows, rect.x, rect.y, rect.width, rect.height, packed,
                          thickness);
    } else if (image.c == kPixelRgba) {
        draw_rectangle_c4(image.data, image.cols, image.rows, rect.x, rect.y, rect.width, rect.height, packed,
                          thickness);
    } else {
        MIMAGE_LOGE("rectangle: unsupported channel count %d", image.c);
    }
}

void circle(Image& image, Point center, std::int32_t radius, const Scalar& color, std::int32_t thickness) {
    const std::uint32_t packed = pack_color(color, image.c);
    if (image.c == kPixelGray) {
        draw_circle_c1(image.data, image.cols, image.rows, center.x, center.y, radius, packed, thickness);
    } else if (image.c == kPixelRgb) {
        draw_circle_c3(image.data, image.cols, image.rows, center.x, center.y, radius, packed, thickness);
    } else if (image.c == kPixelRgba) {
        draw_circle_c4(image.data, image.cols, image.rows, center.x, center.y, radius, packed, thickness);
    } else {
        MIMAGE_LOGE("circle: unsupported channel count %d", image.c);
    }
}

void line(Image& image, Point p0, Point p1, const Scalar& color, std::int32_t thickness) {
    const std::uint32_t packed = pack_color(color, image.c);
    if (image.c == kPixelGray) {
        draw_line_c1(image.data, image.cols, image.rows, p0.x, p0.y, p1.x, p1.y, packed, thickness);
    } else if (image.c == kPixelRgb) {
        draw_line_c3(image.data, image.cols, image.rows, p0.x, p0.y, p1.x, p1.y, packed, thickness);
    } else if (image.c == kPixelRgba) {
        draw_line_c4(image.data, image.cols, image.rows, p0.x, p0.y, p1.x, p1.y, packed, thickness);
    } else {
        MIMAGE_LOGE("line: unsupported channel count %d", image.c);
    }
}

void putText(Image& image, const std::string& text, Point org, std::int32_t font_face, double font_scale, Scalar color,
             std::int32_t thickness) {
    (void)font_face;
    (void)thickness;

    const auto font_pixel_size = static_cast<std::int32_t>(kFontPixelScale * font_scale);
    const std::uint32_t packed = pack_color(color, image.c);
    if (image.c == kPixelGray) {
        draw_text_c1(image.data, image.cols, image.rows, text.c_str(), org.x, org.y - font_pixel_size * 2,
                     font_pixel_size, packed);
    } else if (image.c == kPixelRgb) {
        draw_text_c3(image.data, image.cols, image.rows, text.c_str(), org.x, org.y - font_pixel_size * 2,
                     font_pixel_size, packed);
    } else if (image.c == kPixelRgba) {
        draw_text_c4(image.data, image.cols, image.rows, text.c_str(), org.x, org.y - font_pixel_size * 2,
                     font_pixel_size, packed);
    } else {
        MIMAGE_LOGE("putText: unsupported channel count %d", image.c);
    }
}

Size getTextSize(const std::string& text, std::int32_t font_face, double font_scale, std::int32_t thickness,
                 std::int32_t* base_line) {
    (void)font_face;
    (void)thickness;

    const auto font_pixel_size = static_cast<std::int32_t>(kFontPixelScale * font_scale);
    std::int32_t width = 0;
    std::int32_t height = 0;
    get_text_drawing_size(text.c_str(), font_pixel_size, &width, &height);
    if (base_line != nullptr) {
        *base_line = 0;
    }
    return Size(width, height);
}

}  // namespace mimage
