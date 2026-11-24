# 实现 · M1

## 文件地图

```
include/mimage/
  image.hpp        Image/Scalar/Point/Size/Rect + codec/resize/drawing 声明
  pixel.hpp        像素布局枚举 + from_pixels/to_pixels/YUV 声明
  transform.hpp    旋转与仿射声明
  log.hpp          可注入日志接口 + MIMAGE_LOG* 宏
  mimage.hpp       伞头

src/
  image.cpp        编解码 + Image 高层 resize/drawing 包装
  pixel.cpp        交错像素 <-> planar mmat::Mat + YUV420SP
  rotate.cpp       kanna 旋转
  affine.cpp       仿射矩阵 + warpaffine
  resize.cpp       resize_bilinear_c1/c2/c3/c4 + stride + yuv420sp
  drawing.cpp      矩形/圆/线/文字 raw 绘制
  log.cpp          handler 存储与 vsnprintf 格式化
  mat_pixel_drawing_font.h   移植的等宽字体位图

third_party/
  stb_image.h
  stb_image_write.h

tests/unit/image_tests.cpp
tests/unit/log_tests.cpp
tests/package/consumer/
```

## 移植方式

来自 ncnn：

| 源 | 目 | 处理 |
| --- | --- | --- |
| `src/simpleocv.h/.cpp` | `image.hpp` / `image.cpp` | 容器改名 `Image`，namespace `mimage`，`ncnn::fastMalloc`→`mmemory::fast_malloc`，失败点加日志 |
| `src/mat_pixel_resize.cpp` | `src/resize.cpp` | 原样算法，类型固定宽度 |
| `src/mat_pixel_drawing.cpp` | `src/drawing.cpp` | 原样算法，字体表作为私有头 |
| `src/mat_pixel.cpp` | `src/pixel.cpp` | 自由函数化，`Mat`→`mmat::Mat`，`Allocator*`→`mmemory::Allocator*` |
| `src/mat_pixel_rotate.cpp` | `src/rotate.cpp` | 原样 kanna 旋转，类型固定宽度 |
| `src/mat_pixel_affine.cpp` | `src/affine.cpp` | 原样 warpaffine，矩阵函数保留 |
| `src/mat_pixel_drawing_font.h` | `src/` | 数据表，`unsigned char`→`std::uint8_t` |
| `src/stb_image*.h` | `third_party/` | 原样 vendored |

主要机械转换：

* `int` → `std::int32_t`，`unsigned char` → `std::uint8_t`；
* `short` → `std::int16_t`，`size_t` → `std::size_t`；
* `ncnn::` → `mimage::`；
* `NCNN_LOGE` → `MIMAGE_LOGE`。

## 与原实现的差异

| 项 | ncnn simpleocv | mimage |
| --- | --- | --- |
| 容器名 | `cv::Mat` | `mimage::Image` |
| 分配 | `ncnn::fastMalloc` | `mmemory::fast_malloc` |
| 日志 | `NCNN_LOGE` | `MIMAGE_LOG*` 可注入 handler |
| resize 临时缓冲 | `ncnn::Mat` | `std::vector<std::int16_t>` |
| 失败诊断 | 部分静默 | 路径/通道/尺寸/stb 原因 |
| 字体/几何字面量 | 裸数字 | 保持裸数字（文件级 allow-file） |

## 日志接口

`src/log.cpp` 用两个 `std::atomic` 保存 handler 与 user_data，`log_format`
用 `vsnprintf` 写入固定缓冲。`MIMAGE_LOGE` 等在 codec 失败点记录：

* 未知 flags；
* 文件/内存解码失败（含 stb 原因）；
* 不支持通道数；
* 写文件扩展名缺失或不支持；
* resize 目标非法、分配失败、通道不支持；
* 绘制通道不支持。

## ncnn 兼容

`Image` 的 `data/refcount/rows/cols/c`、`create/release/clone/empty/total/ptr`、
`operator=(Scalar)`、`operator()(Rect)` 与 simpleocv 一致。ncnn 集成时把
`cv::Mat` 设为 `mimage::Image`，`cv::` 函数转发到 mimage；`ncnn::Mat` 的
`from_pixels/to_pixels` 转发到 `mimage::from_pixels/to_pixels`。
