# 复审记录 · mimage

## RV-01 · 移植代码缺少 `<cmath>` / `<vector>`（已修，P1）

**发现**：`resize.cpp` / `drawing.cpp` 使用 `floor`，`drawing.cpp` 的字体缩放
还使用了 `ncnn::Mat` 作为临时行缓冲；直接移植后编译失败。

**修复**：补 `<cmath>`、`<vector>`；把 `Mat rowsbuf(w, 2u)` 改成
`std::vector<std::int16_t>`，语义不变且不依赖 mmat。

## RV-02 · SSE 头文件丢失（已修，P1）

**发现**：删除 `platform.h` 后 `__m128i` / `_mm_*` 未声明。

**修复**：在 `resize.cpp` 中按 `__SSE2__` 显式 include `<emmintrin.h>`。

## RV-03 · stride 重载声明重复（已修，P1）

**发现**：补充 stride 版本声明时插入了两份，`-Wredundant-decls` 报 16 条。

**修复**：删除重复块；保留 non-stride + stride 两组。

## RV-04 · 字体/几何字面量触发具名常量闸门（决定）

**背景**：`drawing.cpp` 与字体表有 60+ 个像素几何/位图字面量。

**决定**：这三个移植文件头部标注
`// check-magic-numbers: allow-file <reason>`，`check-magic-numbers.py` 跳过
整文件；新增代码仍逐行检查。理由与边界写入质量策略。

## RV-05 · stb 头在 `-Wmissing-field-initializers` 下告警（决定）

**决定**：`third_party/stb_*` 是 vendored public-domain 代码，告警不计入
自有基线；`check-warnings.py` 已把 `third_party/` 标为外部。

## RV-06 · 日志必须有接口（需求）

**背景**：simpleocv 的解码失败大多静默返回空图，定位只能靠猜。

**实现**：新增 `mimage/log.hpp`：

* 默认静默；
* handler 接收 level / message / user_data；
* codec、resize、drawing 的失败点全部记录上下文；
* 测试验证安装、替换、清除与消息内容。

## RV-07 · 固定宽度类型

**发现**：移植后仍有 `int` / `unsigned char`。

**修复**：批量改为 `std::int32_t` / `std::uint8_t` / `std::int16_t`；
stb 的 `int` 维度参数用 `// check-types: allow stb_image ABI` 标注。

## RV-08 · ncnn 集成范围

**决定**：M1 先完成独立模块与兼容容器；ncnn `simpleocv` 替换与
`Mat::from_pixels` 迁移放到 M2，避免同一轮同时改两套 API。

## RV-09 · valgrind 发现测试写出未初始化像素（已修，P1）

**发现**：`Image.ShowAndWaitKeyAreStubs` 创建 2×2 图像后直接 `imshow`，
stb 编码时读取未初始化像素，valgrind 报 conditional jump on uninitialised value。

**修复**：测试先 `image = Scalar(0)` 再编码。valgrind 恢复干净。

## RV-10 · clang-tidy 范围（决定）

**背景**：移植的 drawing/resize 在 analyzer 下产生大量动态游标越界误报，
stb 头也有同类告警。

**决定**：clang-tidy 只分析 `src/log.cpp` 与公共头；移植代码由编译器告警、
ASan/UBSan、valgrind 覆盖。范围写入 `QUALITY-GATES.md`。


---

## 复审结论

* 编译、测试、移植类型、临时缓冲、重复声明均已处理。
* 唯一有意偏差是移植文件的警告集与 allow-file 常量规则，已文档化。
* 日志接口已覆盖所有用户可见失败路径。
