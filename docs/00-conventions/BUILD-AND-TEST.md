# 构建与测试 · mimage

## 依赖

* CMake ≥ 3.24、Ninja、GCC/Clang（C++20）。
* 同工作区的 `../mmemory`（会引入 `../mplatform`）。
* 测试需要 GTest ≥ 1.17 的 CMake config。
* `third_party/` 内的 stb_image / stb_image_write（public domain）。

## 构建

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -Dmimage_build_tests=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

脚本：

```bash
scripts/build.sh test          # GCC，构建 + 零告警
scripts/build.sh test --clang  # Clang
scripts/test.sh
scripts/sanitize.sh
scripts/tsan.sh
scripts/analyze.sh
scripts/package-test.sh
scripts/check-all.sh
```

## 选项

| 选项 | 默认 | 说明 |
| --- | --- | --- |
| `mimage_build_tests` | 顶层为 `ON` | 测试 |
| `MIMAGE_SANITIZE` | `OFF` | ASan + UBSan |
| `MIMAGE_MMEMORY_DIR` | `../mmemory` | mmemory 源码目录 |

## 安装包

`libmimage.a` 合并 `mimage_objs`、`mmemory_objs`、`mplatform_objs`，
并安装 `mmemory` / `mplatform` 的公共头。消费方只需：

```cmake
find_package(mimage CONFIG REQUIRED)
target_link_libraries(app PRIVATE mimage::mimage)
```

## ncnn 集成

本次 M1 先完成独立模块；ncnn `simpleocv` / `mat_pixel_*` 的替换在后续步骤，
接口保持兼容：`Image` 的 `data/refcount/rows/cols/c` 与 `simpleocv` 的 `Mat` 一致。
