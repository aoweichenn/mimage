// Copyright 2021 Tencent
// Copyright 2026 mcnn authors
// check-magic-numbers: allow-file ported ncnn drawing geometry and font sizing
// SPDX-License-Identifier: BSD-3-Clause

#include <cctype>
#include <climits>
#include <cmath>

#include <vector>

#include <mimage/image.hpp>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif  // __ARM_NEON

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

#include <mat_pixel_drawing_font.h>

namespace mimage {

void draw_rectangle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx, std::int32_t ry,
                       std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    return draw_rectangle_c1(pixels, w, h, w, rx, ry, rw, rh, color, thickness);
}

void draw_rectangle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx, std::int32_t ry,
                       std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    return draw_rectangle_c2(pixels, w, h, w * 2, rx, ry, rw, rh, color, thickness);
}

void draw_rectangle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx, std::int32_t ry,
                       std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    return draw_rectangle_c3(pixels, w, h, w * 3, rx, ry, rw, rh, color, thickness);
}

void draw_rectangle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t rx, std::int32_t ry,
                       std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    return draw_rectangle_c4(pixels, w, h, w * 4, rx, ry, rw, rh, color, thickness);
}

void draw_rectangle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t rx,
                       std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = ry; y < ry + rh; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx; x < rx + rw; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x] = pen_color[0];
            }
        }

        return;
    }

    const std::int32_t t0 = thickness / 2;
    const std::int32_t t1 = thickness - t0;

    // draw top
    {
        for (std::int32_t y = ry - t0; y < ry + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x] = pen_color[0];
            }
        }
    }

    // draw bottom
    {
        for (std::int32_t y = ry + rh - t0; y < ry + rh + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x] = pen_color[0];
            }
        }
    }

    // draw left
    for (std::int32_t x = rx - t0; x < rx + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x] = pen_color[0];
        }
    }

    // draw right
    for (std::int32_t x = rx + rw - t0; x < rx + rw + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x] = pen_color[0];
        }
    }
}

void draw_rectangle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t rx,
                       std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = ry; y < ry + rh; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx; x < rx + rw; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 2 + 0] = pen_color[0];
                p[x * 2 + 1] = pen_color[1];
            }
        }

        return;
    }

    const std::int32_t t0 = thickness / 2;
    const std::int32_t t1 = thickness - t0;

    // draw top
    {
        for (std::int32_t y = ry - t0; y < ry + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 2 + 0] = pen_color[0];
                p[x * 2 + 1] = pen_color[1];
            }
        }
    }

    // draw bottom
    {
        for (std::int32_t y = ry + rh - t0; y < ry + rh + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 2 + 0] = pen_color[0];
                p[x * 2 + 1] = pen_color[1];
            }
        }
    }

    // draw left
    for (std::int32_t x = rx - t0; x < rx + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x * 2 + 0] = pen_color[0];
            p[x * 2 + 1] = pen_color[1];
        }
    }

    // draw right
    for (std::int32_t x = rx + rw - t0; x < rx + rw + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x * 2 + 0] = pen_color[0];
            p[x * 2 + 1] = pen_color[1];
        }
    }
}

void draw_rectangle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t rx,
                       std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = ry; y < ry + rh; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx; x < rx + rw; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 3 + 0] = pen_color[0];
                p[x * 3 + 1] = pen_color[1];
                p[x * 3 + 2] = pen_color[2];
            }
        }

        return;
    }

    const std::int32_t t0 = thickness / 2;
    const std::int32_t t1 = thickness - t0;

    // draw top
    {
        for (std::int32_t y = ry - t0; y < ry + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 3 + 0] = pen_color[0];
                p[x * 3 + 1] = pen_color[1];
                p[x * 3 + 2] = pen_color[2];
            }
        }
    }

    // draw bottom
    {
        for (std::int32_t y = ry + rh - t0; y < ry + rh + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 3 + 0] = pen_color[0];
                p[x * 3 + 1] = pen_color[1];
                p[x * 3 + 2] = pen_color[2];
            }
        }
    }

    // draw left
    for (std::int32_t x = rx - t0; x < rx + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x * 3 + 0] = pen_color[0];
            p[x * 3 + 1] = pen_color[1];
            p[x * 3 + 2] = pen_color[2];
        }
    }

    // draw right
    for (std::int32_t x = rx + rw - t0; x < rx + rw + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x * 3 + 0] = pen_color[0];
            p[x * 3 + 1] = pen_color[1];
            p[x * 3 + 2] = pen_color[2];
        }
    }
}

void draw_rectangle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t rx,
                       std::int32_t ry, std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = ry; y < ry + rh; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx; x < rx + rw; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 4 + 0] = pen_color[0];
                p[x * 4 + 1] = pen_color[1];
                p[x * 4 + 2] = pen_color[2];
                p[x * 4 + 3] = pen_color[3];
            }
        }

        return;
    }

    const std::int32_t t0 = thickness / 2;
    const std::int32_t t1 = thickness - t0;

    // draw top
    {
        for (std::int32_t y = ry - t0; y < ry + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 4 + 0] = pen_color[0];
                p[x * 4 + 1] = pen_color[1];
                p[x * 4 + 2] = pen_color[2];
                p[x * 4 + 3] = pen_color[3];
            }
        }
    }

    // draw bottom
    {
        for (std::int32_t y = ry + rh - t0; y < ry + rh + t1; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = rx - t0; x < rx + rw + t1; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                p[x * 4 + 0] = pen_color[0];
                p[x * 4 + 1] = pen_color[1];
                p[x * 4 + 2] = pen_color[2];
                p[x * 4 + 3] = pen_color[3];
            }
        }
    }

    // draw left
    for (std::int32_t x = rx - t0; x < rx + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x * 4 + 0] = pen_color[0];
            p[x * 4 + 1] = pen_color[1];
            p[x * 4 + 2] = pen_color[2];
            p[x * 4 + 3] = pen_color[3];
        }
    }

    // draw right
    for (std::int32_t x = rx + rw - t0; x < rx + rw + t1; x++) {
        if (x < 0) {
            continue;
        }

        if (x >= w) {
            break;
        }

        for (std::int32_t y = ry + t1; y < ry + rh - t0; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            p[x * 4 + 0] = pen_color[0];
            p[x * 4 + 1] = pen_color[1];
            p[x * 4 + 2] = pen_color[2];
            p[x * 4 + 3] = pen_color[3];
        }
    }
}

void draw_rectangle_yuv420sp(std::uint8_t* yuv420sp, std::int32_t w, std::int32_t h, std::int32_t rx, std::int32_t ry,
                             std::int32_t rw, std::int32_t rh, std::uint32_t color, std::int32_t thickness) {
    // assert w % 2 == 0
    // assert h % 2 == 0
    // assert rx % 2 == 0
    // assert ry % 2 == 0
    // assert rw % 2 == 0
    // assert rh % 2 == 0
    // assert thickness % 2 == 0

    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint32_t v_y;
    std::uint32_t v_uv;
    std::uint8_t* pen_color_y = (std::uint8_t*)&v_y;
    std::uint8_t* pen_color_uv = (std::uint8_t*)&v_uv;
    pen_color_y[0] = pen_color[0];
    pen_color_uv[0] = pen_color[1];
    pen_color_uv[1] = pen_color[2];

    std::uint8_t* Y = yuv420sp;
    draw_rectangle_c1(Y, w, h, rx, ry, rw, rh, v_y, thickness);

    std::uint8_t* UV = yuv420sp + w * h;
    std::int32_t thickness_uv = thickness == -1 ? thickness : std::max(thickness / 2, 1);
    draw_rectangle_c2(UV, w / 2, h / 2, rx / 2, ry / 2, rw / 2, rh / 2, v_uv, thickness_uv);
}

static inline bool distance_lessequal(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, float r) {
    std::int32_t dx = x0 - x1;
    std::int32_t dy = y0 - y1;
    std::int32_t q = dx * dx + dy * dy;
    return q <= r * r;
}

static inline bool distance_inrange(std::int32_t x0, std::int32_t y0, std::int32_t x1, std::int32_t y1, float r0,
                                    float r1) {
    std::int32_t dx = x0 - x1;
    std::int32_t dy = y0 - y1;
    std::int32_t q = dx * dx + dy * dy;
    return q >= r0 * r0 && q < r1 * r1;
}

void draw_circle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx, std::int32_t cy,
                    std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    return draw_circle_c1(pixels, w, h, w, cx, cy, radius, color, thickness);
}

void draw_circle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx, std::int32_t cy,
                    std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    return draw_circle_c2(pixels, w, h, w * 2, cx, cy, radius, color, thickness);
}

void draw_circle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx, std::int32_t cy,
                    std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    return draw_circle_c3(pixels, w, h, w * 3, cx, cy, radius, color, thickness);
}

void draw_circle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t cx, std::int32_t cy,
                    std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    return draw_circle_c4(pixels, w, h, w * 4, cx, cy, radius, color, thickness);
}

void draw_circle_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t cx,
                    std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = cy - (radius - 1); y < cy + radius; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = cx - (radius - 1); x < cx + radius; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                // distance from cx cy
                if (distance_lessequal(x, y, cx, cy, radius)) {
                    p[x] = pen_color[0];
                }
            }
        }

        return;
    }

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    for (std::int32_t y = cy - (radius - 1) - t0; y < cy + radius + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = cx - (radius - 1) - t0; x < cx + radius + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from cx cy
            if (distance_inrange(x, y, cx, cy, radius - t0, radius + t1)) {
                p[x] = pen_color[0];
            }
        }
    }
}

void draw_circle_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t cx,
                    std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = cy - (radius - 1); y < cy + radius; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = cx - (radius - 1); x < cx + radius; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                // distance from cx cy
                if (distance_lessequal(x, y, cx, cy, radius)) {
                    p[x * 2 + 0] = pen_color[0];
                    p[x * 2 + 1] = pen_color[1];
                }
            }
        }

        return;
    }

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    for (std::int32_t y = cy - radius - t0; y < cy + radius + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = cx - radius - t0; x < cx + radius + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from cx cy
            if (distance_inrange(x, y, cx, cy, radius - t0, radius + t1)) {
                p[x * 2 + 0] = pen_color[0];
                p[x * 2 + 1] = pen_color[1];
            }
        }
    }
}

void draw_circle_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t cx,
                    std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = cy - (radius - 1); y < cy + radius; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = cx - (radius - 1); x < cx + radius; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                // distance from cx cy
                if (distance_lessequal(x, y, cx, cy, radius)) {
                    p[x * 3 + 0] = pen_color[0];
                    p[x * 3 + 1] = pen_color[1];
                    p[x * 3 + 2] = pen_color[2];
                }
            }
        }

        return;
    }

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    for (std::int32_t y = cy - radius - t0; y < cy + radius + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = cx - radius - t0; x < cx + radius + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from cx cy
            if (distance_inrange(x, y, cx, cy, radius - t0, radius + t1)) {
                p[x * 3 + 0] = pen_color[0];
                p[x * 3 + 1] = pen_color[1];
                p[x * 3 + 2] = pen_color[2];
            }
        }
    }
}

void draw_circle_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t cx,
                    std::int32_t cy, std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    if (thickness == -1) {
        // filled
        for (std::int32_t y = cy - (radius - 1); y < cy + radius; y++) {
            if (y < 0) {
                continue;
            }

            if (y >= h) {
                break;
            }

            std::uint8_t* p = pixels + stride * y;

            for (std::int32_t x = cx - (radius - 1); x < cx + radius; x++) {
                if (x < 0) {
                    continue;
                }

                if (x >= w) {
                    break;
                }

                // distance from cx cy
                if (distance_lessequal(x, y, cx, cy, radius)) {
                    p[x * 4 + 0] = pen_color[0];
                    p[x * 4 + 1] = pen_color[1];
                    p[x * 4 + 2] = pen_color[2];
                    p[x * 4 + 3] = pen_color[3];
                }
            }
        }

        return;
    }

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    for (std::int32_t y = cy - (radius - 1) - t0; y < cy + radius + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = cx - (radius - 1) - t0; x < cx + radius + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from cx cy
            if (distance_inrange(x, y, cx, cy, radius - t0, radius + t1)) {
                p[x * 4 + 0] = pen_color[0];
                p[x * 4 + 1] = pen_color[1];
                p[x * 4 + 2] = pen_color[2];
                p[x * 4 + 3] = pen_color[3];
            }
        }
    }
}

void draw_circle_yuv420sp(std::uint8_t* yuv420sp, std::int32_t w, std::int32_t h, std::int32_t cx, std::int32_t cy,
                          std::int32_t radius, std::uint32_t color, std::int32_t thickness) {
    // assert w % 2 == 0
    // assert h % 2 == 0
    // assert cx % 2 == 0
    // assert cy % 2 == 0
    // assert radius % 2 == 0
    // assert thickness % 2 == 0

    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint32_t v_y;
    std::uint32_t v_uv;
    std::uint8_t* pen_color_y = (std::uint8_t*)&v_y;
    std::uint8_t* pen_color_uv = (std::uint8_t*)&v_uv;
    pen_color_y[0] = pen_color[0];
    pen_color_uv[0] = pen_color[1];
    pen_color_uv[1] = pen_color[2];

    std::uint8_t* Y = yuv420sp;
    draw_circle_c1(Y, w, h, cx, cy, radius, v_y, thickness);

    std::uint8_t* UV = yuv420sp + w * h;
    std::int32_t thickness_uv = thickness == -1 ? thickness : std::max(thickness / 2, 1);
    draw_circle_c2(UV, w / 2, h / 2, cx / 2, cy / 2, radius / 2, v_uv, thickness_uv);
}

static inline bool distance_lessthan(std::int32_t x, std::int32_t y, std::int32_t x0, std::int32_t y0, std::int32_t x1,
                                     std::int32_t y1, float t) {
    std::int32_t dx01 = x1 - x0;
    std::int32_t dy01 = y1 - y0;
    std::int32_t dx0 = x - x0;
    std::int32_t dy0 = y - y0;

    float r = (float)(dx0 * dx01 + dy0 * dy01) / (dx01 * dx01 + dy01 * dy01);

    if (r < 0 || r > 1) {
        return false;
    }

    float px = x0 + dx01 * r;
    float py = y0 + dy01 * r;
    float dx = x - px;
    float dy = y - py;
    float p = dx * dx + dy * dy;
    return p < t;
}

void draw_line_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                  std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    draw_line_c1(pixels, w, h, w, x0, y0, x1, y1, color, thickness);
}

void draw_line_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                  std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    draw_line_c2(pixels, w, h, w * 2, x0, y0, x1, y1, color, thickness);
}

void draw_line_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                  std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    draw_line_c3(pixels, w, h, w * 3, x0, y0, x1, y1, color, thickness);
}

void draw_line_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                  std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    draw_line_c4(pixels, w, h, w * 4, x0, y0, x1, y1, color, thickness);
}

void draw_line_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t x0,
                  std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    std::int32_t x_min = std::min(x0, x1);
    std::int32_t x_max = std::max(x0, x1);
    std::int32_t y_min = std::min(y0, y1);
    std::int32_t y_max = std::max(y0, y1);

    for (std::int32_t y = y_min - t0; y < y_max + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = x_min - t0; x < x_max + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from line
            if (distance_lessthan(x, y, x0, y0, x1, y1, t1)) {
                p[x] = pen_color[0];
            }
        }
    }
}

void draw_line_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t x0,
                  std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    std::int32_t x_min = std::min(x0, x1);
    std::int32_t x_max = std::max(x0, x1);
    std::int32_t y_min = std::min(y0, y1);
    std::int32_t y_max = std::max(y0, y1);

    for (std::int32_t y = y_min - t0; y < y_max + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = x_min - t0; x < x_max + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from line
            if (distance_lessthan(x, y, x0, y0, x1, y1, t1)) {
                p[x * 2 + 0] = pen_color[0];
                p[x * 2 + 1] = pen_color[1];
            }
        }
    }
}

void draw_line_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t x0,
                  std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    std::int32_t x_min = std::min(x0, x1);
    std::int32_t x_max = std::max(x0, x1);
    std::int32_t y_min = std::min(y0, y1);
    std::int32_t y_max = std::max(y0, y1);

    for (std::int32_t y = y_min - t0; y < y_max + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = x_min - t0; x < x_max + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from line
            if (distance_lessthan(x, y, x0, y0, x1, y1, t1)) {
                p[x * 3 + 0] = pen_color[0];
                p[x * 3 + 1] = pen_color[1];
                p[x * 3 + 2] = pen_color[2];
            }
        }
    }
}

void draw_line_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, std::int32_t x0,
                  std::int32_t y0, std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    const float t0 = thickness / 2.f;
    const float t1 = thickness - t0;

    std::int32_t x_min = std::min(x0, x1);
    std::int32_t x_max = std::max(x0, x1);
    std::int32_t y_min = std::min(y0, y1);
    std::int32_t y_max = std::max(y0, y1);

    for (std::int32_t y = y_min - t0; y < y_max + t1; y++) {
        if (y < 0) {
            continue;
        }

        if (y >= h) {
            break;
        }

        std::uint8_t* p = pixels + stride * y;

        for (std::int32_t x = x_min - t0; x < x_max + t1; x++) {
            if (x < 0) {
                continue;
            }

            if (x >= w) {
                break;
            }

            // distance from line
            if (distance_lessthan(x, y, x0, y0, x1, y1, t1)) {
                p[x * 4 + 0] = pen_color[0];
                p[x * 4 + 1] = pen_color[1];
                p[x * 4 + 2] = pen_color[2];
                p[x * 4 + 3] = pen_color[3];
            }
        }
    }
}

void draw_line_yuv420sp(std::uint8_t* yuv420sp, std::int32_t w, std::int32_t h, std::int32_t x0, std::int32_t y0,
                        std::int32_t x1, std::int32_t y1, std::uint32_t color, std::int32_t thickness) {
    // assert w % 2 == 0
    // assert h % 2 == 0
    // assert x0 % 2 == 0
    // assert y0 % 2 == 0
    // assert x1 % 2 == 0
    // assert y1 % 2 == 0
    // assert thickness % 2 == 0

    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint32_t v_y;
    std::uint32_t v_uv;
    std::uint8_t* pen_color_y = (std::uint8_t*)&v_y;
    std::uint8_t* pen_color_uv = (std::uint8_t*)&v_uv;
    pen_color_y[0] = pen_color[0];
    pen_color_uv[0] = pen_color[1];
    pen_color_uv[1] = pen_color[2];

    std::uint8_t* Y = yuv420sp;
    draw_line_c1(Y, w, h, x0, y0, x1, y1, v_y, thickness);

    std::uint8_t* UV = yuv420sp + w * h;
    std::int32_t thickness_uv = thickness == -1 ? thickness : std::max(thickness / 2, 1);
    draw_line_c2(UV, w / 2, h / 2, x0 / 2, y0 / 2, x1 / 2, y1 / 2, v_uv, thickness_uv);
}

void get_text_drawing_size(const char* text, std::int32_t fontpixelsize, std::int32_t* w, std::int32_t* h) {
    *w = 0;
    *h = 0;

    const std::int32_t n = strlen(text);

    std::int32_t line_w = 0;
    for (std::int32_t i = 0; i < n; i++) {
        char ch = text[i];

        if (ch == '\n') {
            // newline
            *w = std::max(*w, line_w);
            *h += fontpixelsize * 2;
            line_w = 0;
        }

        if (isprint(ch) != 0) {
            line_w += fontpixelsize;
        }
    }

    *w = std::max(*w, line_w);
    *h += fontpixelsize * 2;
}

void draw_text_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                  std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    return draw_text_c1(pixels, w, h, w, text, x, y, fontpixelsize, color);
}

void draw_text_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                  std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    return draw_text_c2(pixels, w, h, w * 2, text, x, y, fontpixelsize, color);
}

void draw_text_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                  std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    return draw_text_c3(pixels, w, h, w * 3, text, x, y, fontpixelsize, color);
}

void draw_text_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                  std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    return draw_text_c4(pixels, w, h, w * 4, text, x, y, fontpixelsize, color);
}

void resize_bilinear_font(const std::uint8_t* font_bitmap, std::uint8_t* resized_font_bitmap,
                          std::int32_t fontpixelsize) {
    const std::int32_t INTER_RESIZE_COEF_BITS = 11;
    const std::int32_t INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;

    const std::int32_t srcw = 20;
    const std::int32_t srch = 40;
    const std::int32_t w = fontpixelsize;
    const std::int32_t h = fontpixelsize * 2;

    double scale = (double)srcw / w;

    std::int32_t* buf = new std::int32_t[w + h + w + h];

    std::int32_t* xofs = buf;      // new std::int32_t[w];
    std::int32_t* yofs = buf + w;  // new std::int32_t[h];

    std::int16_t* ialpha = (std::int16_t*)(buf + w + h);     // new std::int16_t[w * 2];
    std::int16_t* ibeta = (std::int16_t*)(buf + w + h + w);  // new std::int16_t[h * 2];

    float fx;
    float fy;
    std::int32_t sx;
    std::int32_t sy;

#define SATURATE_CAST_SHORT(X) \
    (std::int16_t)::std::min(::std::max((std::int32_t)(X + (X >= 0.f ? 0.5f : -0.5f)), SHRT_MIN), SHRT_MAX);

    for (std::int32_t dx = 0; dx < w; dx++) {
        fx = (float)((dx + 0.5) * scale - 0.5);
        sx = static_cast<std::int32_t>(floor(fx));
        fx -= sx;

        xofs[dx] = sx;

        float a0 = (1.f - fx) * INTER_RESIZE_COEF_SCALE;
        float a1 = fx * INTER_RESIZE_COEF_SCALE;

        ialpha[dx * 2] = SATURATE_CAST_SHORT(a0);
        ialpha[dx * 2 + 1] = SATURATE_CAST_SHORT(a1);
    }

    for (std::int32_t dy = 0; dy < h; dy++) {
        fy = (float)((dy + 0.5) * scale - 0.5);
        sy = static_cast<std::int32_t>(floor(fy));
        fy -= sy;

        yofs[dy] = sy;

        float b0 = (1.f - fy) * INTER_RESIZE_COEF_SCALE;
        float b1 = fy * INTER_RESIZE_COEF_SCALE;

        ibeta[dy * 2] = SATURATE_CAST_SHORT(b0);
        ibeta[dy * 2 + 1] = SATURATE_CAST_SHORT(b1);
    }

#undef SATURATE_CAST_SHORT

    // loop body
    std::vector<std::int16_t> rowsbuf0(static_cast<std::size_t>(w));
    std::vector<std::int16_t> rowsbuf1(static_cast<std::size_t>(w));
    std::int16_t* rows0 = rowsbuf0.data();
    std::int16_t* rows1 = rowsbuf1.data();

    {
        std::int16_t* rows1p = rows1;
        for (std::int32_t dx = 0; dx < w; dx++) {
            rows1p[dx] = 0;
        }
    }

    std::int32_t prev_sy1 = -2;

    for (std::int32_t dy = 0; dy < h; dy++) {
        sy = yofs[dy];

        if (sy == prev_sy1) {
            // reuse all rows
        } else if (sy == prev_sy1 + 1) {
            // hresize one row
            std::int16_t* rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            const std::uint8_t* S1 = font_bitmap + 10 * (sy + 1);

            if (sy >= srch - 1) {
                std::int16_t* rows1p = rows1;
                for (std::int32_t dx = 0; dx < w; dx++) {
                    rows1p[dx] = 0;
                }
            } else {
                const std::int16_t* ialphap = ialpha;
                std::int16_t* rows1p = rows1;
                for (std::int32_t dx = 0; dx < w; dx++) {
                    sx = xofs[dx];
                    std::int16_t a0 = ialphap[0];
                    std::int16_t a1 = ialphap[1];

                    std::uint8_t S1p0;
                    std::uint8_t S1p1;

                    if (sx < 0) {
                        S1p0 = 0;
                        S1p1 = S1[0] & 0x0f;
                    } else if (sx >= srcw - 1) {
                        S1p0 = (S1[9] & 0xf0) >> 4;
                        S1p1 = 0;
                    } else {
                        S1p0 = sx % 2 == 0 ? S1[sx / 2] & 0x0f : (S1[sx / 2] & 0xf0) >> 4;
                        S1p1 = sx % 2 == 0 ? (S1[sx / 2] & 0xf0) >> 4 : S1[sx / 2 + 1] & 0x0f;
                    }
                    rows1p[dx] = (S1p0 * a0 + S1p1 * a1) * 17 >> 4;

                    ialphap += 2;
                }
            }
        } else {
            // hresize two rows
            const std::uint8_t* S0 = font_bitmap + 10 * (sy);
            const std::uint8_t* S1 = font_bitmap + 10 * (sy + 1);

            if (sy >= srch - 1) {
                const std::int16_t* ialphap = ialpha;
                std::int16_t* rows0p = rows0;
                std::int16_t* rows1p = rows1;
                for (std::int32_t dx = 0; dx < w; dx++) {
                    sx = xofs[dx];
                    std::int16_t a0 = ialphap[0];
                    std::int16_t a1 = ialphap[1];

                    std::uint8_t S0p0;
                    std::uint8_t S0p1;

                    if (sx < 0) {
                        S0p0 = 0;
                        S0p1 = S0[0] & 0x0f;
                    } else if (sx >= srcw - 1) {
                        S0p0 = (S0[9] & 0xf0) >> 4;
                        S0p1 = 0;
                    } else {
                        S0p0 = sx % 2 == 0 ? S0[sx / 2] & 0x0f : (S0[sx / 2] & 0xf0) >> 4;
                        S0p1 = sx % 2 == 0 ? (S0[sx / 2] & 0xf0) >> 4 : S0[sx / 2 + 1] & 0x0f;
                    }
                    rows0p[dx] = (S0p0 * a0 + S0p1 * a1) * 17 >> 4;
                    rows1p[dx] = 0;

                    ialphap += 2;
                }
            } else {
                const std::int16_t* ialphap = ialpha;
                std::int16_t* rows0p = rows0;
                std::int16_t* rows1p = rows1;
                for (std::int32_t dx = 0; dx < w; dx++) {
                    sx = xofs[dx];
                    std::int16_t a0 = ialphap[0];
                    std::int16_t a1 = ialphap[1];

                    std::uint8_t S0p0;
                    std::uint8_t S0p1;
                    std::uint8_t S1p0;
                    std::uint8_t S1p1;

                    if (sx < 0) {
                        S0p0 = 0;
                        S0p1 = S0[0] & 0x0f;
                        S1p0 = 0;
                        S1p1 = S1[0] & 0x0f;
                    } else if (sx >= srcw - 1) {
                        S0p0 = (S0[9] & 0xf0) >> 4;
                        S0p1 = 0;
                        S1p0 = (S1[9] & 0xf0) >> 4;
                        S1p1 = 0;
                    } else {
                        S0p0 = sx % 2 == 0 ? S0[sx / 2] & 0x0f : (S0[sx / 2] & 0xf0) >> 4;
                        S0p1 = sx % 2 == 0 ? (S0[sx / 2] & 0xf0) >> 4 : S0[sx / 2 + 1] & 0x0f;
                        S1p0 = sx % 2 == 0 ? S1[sx / 2] & 0x0f : (S1[sx / 2] & 0xf0) >> 4;
                        S1p1 = sx % 2 == 0 ? (S1[sx / 2] & 0xf0) >> 4 : S1[sx / 2 + 1] & 0x0f;
                    }
                    rows0p[dx] = (S0p0 * a0 + S0p1 * a1) * 17 >> 4;
                    rows1p[dx] = (S1p0 * a0 + S1p1 * a1) * 17 >> 4;

                    ialphap += 2;
                }
            }
        }

        prev_sy1 = sy;

        if (dy + 1 < h && yofs[dy + 1] == sy) {
            // vresize for two rows
            std::int16_t b0 = ibeta[0];
            std::int16_t b1 = ibeta[1];
            std::int16_t b2 = ibeta[2];
            std::int16_t b3 = ibeta[3];

            std::int16_t* rows0p = rows0;
            std::int16_t* rows1p = rows1;
            std::uint8_t* Dp0 = resized_font_bitmap + w * (dy);
            std::uint8_t* Dp1 = resized_font_bitmap + w * (dy + 1);

            std::int32_t dx = 0;
#if defined(__ARM_NEON)
            int16x8_t _b0 = vdupq_n_s16(b0);
            int16x8_t _b1 = vdupq_n_s16(b1);
            int16x8_t _b2 = vdupq_n_s16(b2);
            int16x8_t _b3 = vdupq_n_s16(b3);
            for (; dx + 15 < w; dx += 16) {
                int16x8_t _r00 = vld1q_s16(rows0p);
                int16x8_t _r01 = vld1q_s16(rows0p + 8);
                int16x8_t _r10 = vld1q_s16(rows1p);
                int16x8_t _r11 = vld1q_s16(rows1p + 8);
                int16x8_t _acc00 = vaddq_s16(vqdmulhq_s16(_r00, _b0), vqdmulhq_s16(_r10, _b1));
                int16x8_t _acc01 = vaddq_s16(vqdmulhq_s16(_r01, _b0), vqdmulhq_s16(_r11, _b1));
                int16x8_t _acc10 = vaddq_s16(vqdmulhq_s16(_r00, _b2), vqdmulhq_s16(_r10, _b3));
                int16x8_t _acc11 = vaddq_s16(vqdmulhq_s16(_r01, _b2), vqdmulhq_s16(_r11, _b3));
                uint8x16_t _Dp0 = vcombine_u8(vqrshrun_n_s16(_acc00, 3), vqrshrun_n_s16(_acc01, 3));
                uint8x16_t _Dp1 = vcombine_u8(vqrshrun_n_s16(_acc10, 3), vqrshrun_n_s16(_acc11, 3));
                vst1q_u8(Dp0, _Dp0);
                vst1q_u8(Dp1, _Dp1);
                Dp0 += 16;
                Dp1 += 16;
                rows0p += 16;
                rows1p += 16;
            }
            for (; dx + 7 < w; dx += 8) {
                int16x8_t _r0 = vld1q_s16(rows0p);
                int16x8_t _r1 = vld1q_s16(rows1p);
                int16x8_t _acc0 = vaddq_s16(vqdmulhq_s16(_r0, _b0), vqdmulhq_s16(_r1, _b1));
                int16x8_t _acc1 = vaddq_s16(vqdmulhq_s16(_r0, _b2), vqdmulhq_s16(_r1, _b3));
                uint8x8_t _Dp0 = vqrshrun_n_s16(_acc0, 3);
                uint8x8_t _Dp1 = vqrshrun_n_s16(_acc1, 3);
                vst1_u8(Dp0, _Dp0);
                vst1_u8(Dp1, _Dp1);
                Dp0 += 8;
                Dp1 += 8;
                rows0p += 8;
                rows1p += 8;
            }
#endif  // __ARM_NEON
#if defined(__SSE2__)
            __m128i _b0 = _mm_set1_epi16(b0);
            __m128i _b1 = _mm_set1_epi16(b1);
            __m128i _b2 = _mm_set1_epi16(b2);
            __m128i _b3 = _mm_set1_epi16(b3);
            __m128i _v2 = _mm_set1_epi16(2);
            for (; dx + 15 < w; dx += 16) {
                __m128i _r00 = _mm_loadu_si128((const __m128i*)rows0p);
                __m128i _r01 = _mm_loadu_si128((const __m128i*)(rows0p + 8));
                __m128i _r10 = _mm_loadu_si128((const __m128i*)rows1p);
                __m128i _r11 = _mm_loadu_si128((const __m128i*)(rows1p + 8));
                __m128i _acc00 = _mm_add_epi16(_mm_mulhi_epi16(_r00, _b0), _mm_mulhi_epi16(_r10, _b1));
                __m128i _acc01 = _mm_add_epi16(_mm_mulhi_epi16(_r01, _b0), _mm_mulhi_epi16(_r11, _b1));
                __m128i _acc10 = _mm_add_epi16(_mm_mulhi_epi16(_r00, _b2), _mm_mulhi_epi16(_r10, _b3));
                __m128i _acc11 = _mm_add_epi16(_mm_mulhi_epi16(_r01, _b2), _mm_mulhi_epi16(_r11, _b3));
                _acc00 = _mm_srai_epi16(_mm_add_epi16(_acc00, _v2), 2);
                _acc01 = _mm_srai_epi16(_mm_add_epi16(_acc01, _v2), 2);
                _acc10 = _mm_srai_epi16(_mm_add_epi16(_acc10, _v2), 2);
                _acc11 = _mm_srai_epi16(_mm_add_epi16(_acc11, _v2), 2);
                __m128i _Dp0 = _mm_packus_epi16(_acc00, _acc01);
                __m128i _Dp1 = _mm_packus_epi16(_acc10, _acc11);
                _mm_storeu_si128((__m128i*)Dp0, _Dp0);
                _mm_storeu_si128((__m128i*)Dp1, _Dp1);
                Dp0 += 16;
                Dp1 += 16;
                rows0p += 16;
                rows1p += 16;
            }
            for (; dx + 7 < w; dx += 8) {
                __m128i _r0 = _mm_loadu_si128((const __m128i*)rows0p);
                __m128i _r1 = _mm_loadu_si128((const __m128i*)rows1p);
                __m128i _acc0 = _mm_add_epi16(_mm_mulhi_epi16(_r0, _b0), _mm_mulhi_epi16(_r1, _b1));
                __m128i _acc1 = _mm_add_epi16(_mm_mulhi_epi16(_r0, _b2), _mm_mulhi_epi16(_r1, _b3));
                _acc0 = _mm_srai_epi16(_mm_add_epi16(_acc0, _v2), 2);
                _acc1 = _mm_srai_epi16(_mm_add_epi16(_acc1, _v2), 2);
                __m128i _Dp0 = _mm_packus_epi16(_acc0, _acc0);
                __m128i _Dp1 = _mm_packus_epi16(_acc1, _acc1);
                _mm_storel_epi64((__m128i*)Dp0, _Dp0);
                _mm_storel_epi64((__m128i*)Dp1, _Dp1);
                Dp0 += 8;
                Dp1 += 8;
                rows0p += 8;
                rows1p += 8;
            }
#endif  // __SSE2__
            for (; dx < w; dx++) {
                std::int16_t s0 = *rows0p++;
                std::int16_t s1 = *rows1p++;

                *Dp0++ = (std::uint8_t)(((std::int16_t)((b0 * s0) >> 16) + (std::int16_t)((b1 * s1) >> 16) + 2) >> 2);
                *Dp1++ = (std::uint8_t)(((std::int16_t)((b2 * s0) >> 16) + (std::int16_t)((b3 * s1) >> 16) + 2) >> 2);
            }

            ibeta += 4;
            dy += 1;
        } else {
            // vresize
            std::int16_t b0 = ibeta[0];
            std::int16_t b1 = ibeta[1];

            std::int16_t* rows0p = rows0;
            std::int16_t* rows1p = rows1;
            std::uint8_t* Dp = resized_font_bitmap + w * (dy);

            std::int32_t dx = 0;
#if defined(__ARM_NEON)
            int16x8_t _b0 = vdupq_n_s16(b0);
            int16x8_t _b1 = vdupq_n_s16(b1);
            for (; dx + 15 < w; dx += 16) {
                int16x8_t _r00 = vld1q_s16(rows0p);
                int16x8_t _r01 = vld1q_s16(rows0p + 8);
                int16x8_t _r10 = vld1q_s16(rows1p);
                int16x8_t _r11 = vld1q_s16(rows1p + 8);
                int16x8_t _acc0 = vaddq_s16(vqdmulhq_s16(_r00, _b0), vqdmulhq_s16(_r10, _b1));
                int16x8_t _acc1 = vaddq_s16(vqdmulhq_s16(_r01, _b0), vqdmulhq_s16(_r11, _b1));
                uint8x16_t _Dp = vcombine_u8(vqrshrun_n_s16(_acc0, 3), vqrshrun_n_s16(_acc1, 3));
                vst1q_u8(Dp, _Dp);
                Dp += 16;
                rows0p += 16;
                rows1p += 16;
            }
            for (; dx + 7 < w; dx += 8) {
                int16x8_t _r0 = vld1q_s16(rows0p);
                int16x8_t _r1 = vld1q_s16(rows1p);
                int16x8_t _acc = vaddq_s16(vqdmulhq_s16(_r0, _b0), vqdmulhq_s16(_r1, _b1));
                uint8x8_t _Dp = vqrshrun_n_s16(_acc, 3);
                vst1_u8(Dp, _Dp);
                Dp += 8;
                rows0p += 8;
                rows1p += 8;
            }
#endif  // __ARM_NEON
#if defined(__SSE2__)
            __m128i _b0 = _mm_set1_epi16(b0);
            __m128i _b1 = _mm_set1_epi16(b1);
            __m128i _v2 = _mm_set1_epi16(2);
            for (; dx + 15 < w; dx += 16) {
                __m128i _r00 = _mm_loadu_si128((const __m128i*)rows0p);
                __m128i _r01 = _mm_loadu_si128((const __m128i*)(rows0p + 8));
                __m128i _r10 = _mm_loadu_si128((const __m128i*)rows1p);
                __m128i _r11 = _mm_loadu_si128((const __m128i*)(rows1p + 8));
                __m128i _acc0 = _mm_add_epi16(_mm_mulhi_epi16(_r00, _b0), _mm_mulhi_epi16(_r10, _b1));
                __m128i _acc1 = _mm_add_epi16(_mm_mulhi_epi16(_r01, _b0), _mm_mulhi_epi16(_r11, _b1));
                _acc0 = _mm_srai_epi16(_mm_add_epi16(_acc0, _v2), 2);
                _acc1 = _mm_srai_epi16(_mm_add_epi16(_acc1, _v2), 2);
                __m128i _Dp = _mm_packus_epi16(_acc0, _acc1);
                _mm_storeu_si128((__m128i*)Dp, _Dp);
                Dp += 16;
                rows0p += 16;
                rows1p += 16;
            }
            for (; dx + 7 < w; dx += 8) {
                __m128i _r0 = _mm_loadu_si128((const __m128i*)rows0p);
                __m128i _r1 = _mm_loadu_si128((const __m128i*)rows1p);
                __m128i _acc = _mm_add_epi16(_mm_mulhi_epi16(_r0, _b0), _mm_mulhi_epi16(_r1, _b1));
                _acc = _mm_srai_epi16(_mm_add_epi16(_acc, _v2), 2);
                __m128i _Dp = _mm_packus_epi16(_acc, _acc);
                _mm_storel_epi64((__m128i*)Dp, _Dp);
                Dp += 8;
                rows0p += 8;
                rows1p += 8;
            }
#endif  // __SSE2__
            for (; dx < w; dx++) {
                std::int16_t s0 = *rows0p++;
                std::int16_t s1 = *rows1p++;

                *Dp++ = (std::uint8_t)(((std::int16_t)((b0 * s0) >> 16) + (std::int16_t)((b1 * s1) >> 16) + 2) >> 2);
            }

            ibeta += 2;
        }
    }

    delete[] buf;
}

void draw_text_c1(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, const char* text,
                  std::int32_t x, std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint8_t* resized_font_bitmap = new std::uint8_t[fontpixelsize * fontpixelsize * 2];

    const std::int32_t n = strlen(text);

    std::int32_t cursor_x = x;
    std::int32_t cursor_y = y;
    for (std::int32_t i = 0; i < n; i++) {
        char ch = text[i];

        if (ch == '\n') {
            // newline
            cursor_x = x;
            cursor_y += fontpixelsize * 2;
            continue;
        }

        if (ch == ' ') {
            cursor_x += fontpixelsize;
            continue;
        }

        if (isprint(ch) != 0) {
            const std::uint8_t* font_bitmap = mono_font_data[ch - '!'];

            // draw resized character
            resize_bilinear_font(font_bitmap, resized_font_bitmap, fontpixelsize);

            const std::int32_t ystart = std::max(cursor_y, 0);
            const std::int32_t yend = std::min(cursor_y + fontpixelsize * 2, h);
            const std::int32_t xstart = std::max(cursor_x, 0);
            const std::int32_t xend = std::min(cursor_x + fontpixelsize, w);

            for (std::int32_t j = ystart; j < yend; j++) {
                const std::uint8_t* palpha = resized_font_bitmap + (j - cursor_y) * fontpixelsize + xstart - cursor_x;
                std::uint8_t* p = pixels + stride * j + xstart;

                for (std::int32_t k = xstart; k < xend; k++) {
                    std::uint8_t alpha = *palpha++;

                    p[0] = (p[0] * (255 - alpha) + pen_color[0] * alpha) / 255;
                    p += 1;
                }
            }

            cursor_x += fontpixelsize;
        }
    }

    delete[] resized_font_bitmap;
}

void draw_text_c2(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, const char* text,
                  std::int32_t x, std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint8_t* resized_font_bitmap = new std::uint8_t[fontpixelsize * fontpixelsize * 2];

    const std::int32_t n = strlen(text);

    std::int32_t cursor_x = x;
    std::int32_t cursor_y = y;
    for (std::int32_t i = 0; i < n; i++) {
        char ch = text[i];

        if (ch == '\n') {
            // newline
            cursor_x = x;
            cursor_y += fontpixelsize * 2;
            continue;
        }

        if (ch == ' ') {
            cursor_x += fontpixelsize;
            continue;
        }

        if (isprint(ch) != 0) {
            const std::uint8_t* font_bitmap = mono_font_data[ch - '!'];

            // draw resized character
            resize_bilinear_font(font_bitmap, resized_font_bitmap, fontpixelsize);

            const std::int32_t ystart = std::max(cursor_y, 0);
            const std::int32_t yend = std::min(cursor_y + fontpixelsize * 2, h);
            const std::int32_t xstart = std::max(cursor_x, 0);
            const std::int32_t xend = std::min(cursor_x + fontpixelsize, w);

            for (std::int32_t j = ystart; j < yend; j++) {
                const std::uint8_t* palpha = resized_font_bitmap + (j - cursor_y) * fontpixelsize + xstart - cursor_x;
                std::uint8_t* p = pixels + stride * j + xstart * 2;

                for (std::int32_t k = xstart; k < xend; k++) {
                    std::uint8_t alpha = *palpha++;

                    p[0] = (p[0] * (255 - alpha) + pen_color[0] * alpha) / 255;
                    p[1] = (p[1] * (255 - alpha) + pen_color[1] * alpha) / 255;
                    p += 2;
                }
            }

            cursor_x += fontpixelsize;
        }
    }

    delete[] resized_font_bitmap;
}

void draw_text_c3(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, const char* text,
                  std::int32_t x, std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint8_t* resized_font_bitmap = new std::uint8_t[fontpixelsize * fontpixelsize * 2];

    const std::int32_t n = strlen(text);

    std::int32_t cursor_x = x;
    std::int32_t cursor_y = y;
    for (std::int32_t i = 0; i < n; i++) {
        char ch = text[i];

        if (ch == '\n') {
            // newline
            cursor_x = x;
            cursor_y += fontpixelsize * 2;
            continue;
        }

        if (ch == ' ') {
            cursor_x += fontpixelsize;
            continue;
        }

        if (isprint(ch) != 0) {
            const std::uint8_t* font_bitmap = mono_font_data[ch - '!'];

            // draw resized character
            resize_bilinear_font(font_bitmap, resized_font_bitmap, fontpixelsize);

            const std::int32_t ystart = std::max(cursor_y, 0);
            const std::int32_t yend = std::min(cursor_y + fontpixelsize * 2, h);
            const std::int32_t xstart = std::max(cursor_x, 0);
            const std::int32_t xend = std::min(cursor_x + fontpixelsize, w);

            for (std::int32_t j = ystart; j < yend; j++) {
                const std::uint8_t* palpha = resized_font_bitmap + (j - cursor_y) * fontpixelsize + xstart - cursor_x;
                std::uint8_t* p = pixels + stride * j + xstart * 3;

                for (std::int32_t k = xstart; k < xend; k++) {
                    std::uint8_t alpha = *palpha++;

                    p[0] = (p[0] * (255 - alpha) + pen_color[0] * alpha) / 255;
                    p[1] = (p[1] * (255 - alpha) + pen_color[1] * alpha) / 255;
                    p[2] = (p[2] * (255 - alpha) + pen_color[2] * alpha) / 255;
                    p += 3;
                }
            }

            cursor_x += fontpixelsize;
        }
    }

    delete[] resized_font_bitmap;
}

void draw_text_c4(std::uint8_t* pixels, std::int32_t w, std::int32_t h, std::int32_t stride, const char* text,
                  std::int32_t x, std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint8_t* resized_font_bitmap = new std::uint8_t[fontpixelsize * fontpixelsize * 2];

    const std::int32_t n = strlen(text);

    std::int32_t cursor_x = x;
    std::int32_t cursor_y = y;
    for (std::int32_t i = 0; i < n; i++) {
        char ch = text[i];

        if (ch == '\n') {
            // newline
            cursor_x = x;
            cursor_y += fontpixelsize * 2;
            continue;
        }

        if (ch == ' ') {
            cursor_x += fontpixelsize;
            continue;
        }

        if (isprint(ch) != 0) {
            const std::uint8_t* font_bitmap = mono_font_data[ch - '!'];

            // draw resized character
            resize_bilinear_font(font_bitmap, resized_font_bitmap, fontpixelsize);

            const std::int32_t ystart = std::max(cursor_y, 0);
            const std::int32_t yend = std::min(cursor_y + fontpixelsize * 2, h);
            const std::int32_t xstart = std::max(cursor_x, 0);
            const std::int32_t xend = std::min(cursor_x + fontpixelsize, w);

            for (std::int32_t j = ystart; j < yend; j++) {
                const std::uint8_t* palpha = resized_font_bitmap + (j - cursor_y) * fontpixelsize + xstart - cursor_x;
                std::uint8_t* p = pixels + stride * j + xstart * 4;

                for (std::int32_t k = xstart; k < xend; k++) {
                    std::uint8_t alpha = *palpha++;

                    p[0] = (p[0] * (255 - alpha) + pen_color[0] * alpha) / 255;
                    p[1] = (p[1] * (255 - alpha) + pen_color[1] * alpha) / 255;
                    p[2] = (p[2] * (255 - alpha) + pen_color[2] * alpha) / 255;
                    p[3] = (p[3] * (255 - alpha) + pen_color[3] * alpha) / 255;
                    p += 4;
                }
            }

            cursor_x += fontpixelsize;
        }
    }

    delete[] resized_font_bitmap;
}

void draw_text_yuv420sp(std::uint8_t* yuv420sp, std::int32_t w, std::int32_t h, const char* text, std::int32_t x,
                        std::int32_t y, std::int32_t fontpixelsize, std::uint32_t color) {
    // assert w % 2 == 0
    // assert h % 2 == 0
    // assert x % 2 == 0
    // assert y % 2 == 0
    // assert fontpixelsize % 2 == 0

    const std::uint8_t* pen_color = (const std::uint8_t*)&color;

    std::uint32_t v_y;
    std::uint32_t v_uv;
    std::uint8_t* pen_color_y = (std::uint8_t*)&v_y;
    std::uint8_t* pen_color_uv = (std::uint8_t*)&v_uv;
    pen_color_y[0] = pen_color[0];
    pen_color_uv[0] = pen_color[1];
    pen_color_uv[1] = pen_color[2];

    std::uint8_t* Y = yuv420sp;
    draw_text_c1(Y, w, h, text, x, y, fontpixelsize, v_y);

    std::uint8_t* UV = yuv420sp + w * h;
    draw_text_c2(UV, w / 2, h / 2, text, x / 2, y / 2, std::max(fontpixelsize / 2, 1), v_uv);
}

}  // namespace mimage
