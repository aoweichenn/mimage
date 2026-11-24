# 质量策略 · mimage

沿用 mzip/mplatform/mmemory/mmat 的本地优先、零告警基线。

## 闸门

| 闸门 | 脚本 |
| --- | --- |
| 格式 | `format.sh --check` |
| 固定宽度类型 | `check-types.py` |
| 头文件自包含/尖括号 | `check-headers.py` |
| 分层 | `check-layers.sh` |
| 无直接输出 | `check-no-printf.py` |
| 无 throw | `check-no-throw.py` |
| 具名常量 | `check-magic-numbers.py` |
| 追溯矩阵 | `check-traceability.py` |
| API 覆盖 | `check-api-coverage.py` |
| `[[nodiscard]]` | `check-nodiscard.py` |
| 用例断言 | `check-test-assertions.py` |
| 零告警 | `build.sh test` |
| 单元测试 | `ctest` |
| 安装包 | `package-test.sh` |

## 警告基线

* **新增代码**（`src/log.cpp`、公共头、测试）使用完整严格集。
* **移植的 codec/resize/drawing**（`src/image.cpp`、`src/resize.cpp`、
  `src/drawing.cpp`）使用 ncnn 上游同级警告集，仍要求 0 warning。
* `third_party/stb_*` 的告警不计入本库基线。
* 每个 target 单独设置，禁止 `add_compile_options()`。

## 具名常量例外

`resize.cpp`、`drawing.cpp`、`mat_pixel_drawing_font.h` 头部标注：

```
// check-magic-numbers: allow-file <reason>
```

这些文件里大量字面量是像素几何、字体位图或采样表数据，提成常量不会提高
可读性。`check-magic-numbers.py` 会跳过整个文件；新增代码仍逐行检查。

## 静态分析范围

`clang-tidy` 只分析新增代码（`src/log.cpp`）与公共头。移植的 codec/resize/drawing
由编译器告警、ASan/UBSan 和 valgrind 覆盖；这些文件里的 int 运算、SIMD
intrinsic 与动态像素游标会产生大量等价改写型告警，逐条 NOLINT 没有收益。

## 日志

`mimage/log.hpp` 提供可注入的 handler 接口，默认静默；codec/drawing 的失败
路径必须通过该接口记录上下文。新增诊断不得使用 `printf` / `std::cout`。
