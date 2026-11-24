# 代码风格 · mimage

全局 C++ 约定见本机 `cpp-conventions` 规范；本文件记录本库的具体决定。

## 命名

| 类别 | 形式 | 例 |
| --- | --- | --- |
| 类型 | `PascalCase` | `Image`、`Scalar`、`Rect` |
| 函数/变量 | `snake_case` | `imread`、`resize_bilinear_c1` |
| 常量 | `kPascalCase` | `kPixelRgb`、`kDefaultJpegQuality` |
| 宏 | `MIMAGE_*` | `MIMAGE_LOGE`、`MIMAGE_EXPORT` |

## 兼容字段

`Image` 的公开字段沿用 ncnn/simpleocv 名称，便于 ncnn 层直接替换：

```
data refcount rows cols c
```

`rows/cols/c` 是图像语义，不改成 `height_/width_`。

## 日志接口

所有诊断都走 `mimage/log.hpp` 的可注入接口：

```cpp
mimage::set_log_handler(handler, user_data);
MIMAGE_LOGE("imread: failed to decode '%s': %s", path, reason);
```

约束：

* 默认未安装 handler 时完全静默；
* handler 必须线程安全；
* 内部格式化到固定 1024 字节栈缓冲，不分配、不抛异常；
* 返回值仍然表达失败；日志只补充上下文（路径、通道数、尺寸、stb 原因）。
* 日志文本使用英文，便于日志系统统一处理。

## 错误处理

* `imread` / `imdecode` 失败返回空 `Image`。
* `imwrite` 失败返回 `false`。
* `Image::create` 对非法尺寸返回空图；分配失败清空尺寸。
* `resize` 对不支持通道数记录错误并保持 `dst` 不变。
* 本库不包含 `throw` / `catch`。

## 移植代码例外

`src/image.cpp`、`src/resize.cpp`、`src/drawing.cpp`、`src/mat_pixel_drawing_font.h`
来自 ncnn 的 BSD-3-Clause 图像实现：

* 使用上游同级警告集（`-Wall -Wextra -Wpedantic` 等）；
* 文件头标注 `check-magic-numbers: allow-file`，因为像素几何/字体表里的
  字面量本身就是数据，提成常量不会提高可读性；
* 新增代码（`log.cpp`、公共头、测试）仍使用完整严格集与具名常量规则。
