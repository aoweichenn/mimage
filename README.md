# mimage

`mcnn` 的图像容器与基础处理模块：交错 8-bit `Image`、编解码、双线性缩放、
矩形/圆/线/文字绘制，以及可注入的日志接口。C++20，无外部运行时依赖。

**范围刻意很窄**：不包含 `mmat::Mat` 的 planar/packed 像素转换、YUV 旋转/仿射、
Vulkan 图像。这些留给 M2。

## 能力

| 项 | 状态 |
| --- | --- |
| `Image`（rows × cols × channels，引用计数） | ✅ |
| clone / ROI / 标量填充 | ✅ |
| `imread` / `imdecode` / `imwrite`（JPEG/PNG/BMP/PNM） | ✅ |
| 双线性缩放（1/3/4 通道，stride，YUV420SP） | ✅ |
| 矩形/圆/线/文字绘制 | ✅ |
| 可注入日志 handler（默认静默） | ✅ |

## 快速开始

```cpp
#include <mimage/mimage.hpp>

mimage::Image image = mimage::imread("input.png");
mimage::rectangle(image, mimage::Rect(10, 10, 100, 50), mimage::Scalar(0, 0, 255), 2);
mimage::imwrite("output.png", image);
```

接入应用日志：

```cpp
mimage::set_log_handler(
    [](mimage::LogLevel level, const char* message, void* user_data) {
        // level / message / user_data 交给应用 logger
    },
    &app_state);
```

默认不安装 handler 时库完全静默；失败仍然通过返回值表达。

## 构建

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -Dmimage_build_tests=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build
```

作为子项目：

```cmake
add_subdirectory(path/to/mimage)
target_link_libraries(app PRIVATE mimage::mimage)
```

默认从 `../mmemory` 引入分配层，`mmemory` 再引入 `../mplatform`。

## 质量门

```bash
scripts/check-all.sh
```

包含 format、固定宽度类型、头文件自包含、分层、无直接输出、无 throw、具名常量、
追溯矩阵、API 覆盖、用例断言、零告警、单元测试、安装包测试。

## 与 mcnn 的关系

`Image` 的 `data/refcount/rows/cols/c` 与 ncnn `simpleocv::Mat` 对齐，便于后续
把 ncnn 的 `cv::Mat` / `mat_pixel_*` 替换为本模块。

许可证见 `LICENSE`，第三方声明见 `THIRD_PARTY_NOTICES.md`。
