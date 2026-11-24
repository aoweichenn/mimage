# 第三方声明

## ncnn image / simpleocv

`src/image.cpp`、`src/resize.cpp`、`src/drawing.cpp`、`src/pixel.cpp`、
`src/mat_pixel_drawing_font.h` 的图像容器、编解码封装、双线性缩放、绘制和
像素布局转换算法
来自 ncnn（Tencent，BSD-3-Clause，Copyright 2017-2023 Tencent）。

本仓库的改动：

* 容器改名为 `mimage::Image`，namespace 改为 `mimage`；
* 分配改用 `mmemory::fast_malloc/fast_free`；
* 类型改为固定宽度（`std::int32_t` / `std::uint8_t` / `std::int16_t`）；
* 失败路径接入 `mimage::LogHandler` 可注入日志；
* resize/drawing 的临时 `ncnn::Mat` 缓冲改为 `std::vector`；
* 增加独立测试、安装包与质量闸门。

## stb_image / stb_image_write

`third_party/stb_image.h` 与 `third_party/stb_image_write.h` 来自
[stb](https://github.com/nothings/stb)，public domain。
