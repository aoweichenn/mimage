# 验证 · M1

## 环境

| 项 | 值 |
| --- | --- |
| 主机 | Linux x86_64（Fedora 44） |
| CPU | 支持 AVX2 / AVX-VNNI，无 AVX-512 |
| GCC | 16.2.1 |
| Clang | 22.1.8 |
| CMake | 4.3.0 |
| GTest | 1.17.0（CMake config） |
| 依赖 | `../mmemory`、`../mplatform` |

## 1. 质量门

```bash
scripts/check-all.sh
```

**全部通过**：

| 项 | 结果 |
| --- | --- |
| format / types / headers / layers | 通过 |
| no-printf / no-throw / magic | 通过（移植文件走 allow-file） |
| traceability | 12 条需求、36 个用例 |
| API coverage | 43/43 |
| nodiscard | 0/0 |
| 用例断言 | 36/36 |
| GCC 零告警 | 0（stb 第三方不计入） |
| 单元测试 | 36/36（4 个 suite） |
| 安装包消费方 | 通过 |

## 2. 双编译器

```bash
scripts/build.sh test
scripts/build.sh test --clang
```

两者均 `零告警 ✓`，Clang 下 36/36 通过。

## 3. 内存、线程与静态分析

```bash
scripts/sanitize.sh
scripts/tsan.sh
scripts/analyze.sh tidy
scripts/analyze.sh memcheck
```

| 项 | 结果 |
| --- | --- |
| ASan + UBSan | 通过 |
| TSan | 通过 |
| clang-tidy | 通过（新代码与公共头） |
| valgrind memcheck | 通过，无 definite leak |

## 4. 安装包

```bash
scripts/package-test.sh
```

`libmimage.a` 安装到临时前缀，外部消费方通过 `find_package(mimage CONFIG)`
构建并运行；`ldd` 审计确认没有额外共享库依赖。

## 5. 日志接口验证

* 默认无 handler 时静默；
* 安装 handler 后收到 `LogLevel`、格式化文本与 `user_data`；
* 替换/清除 handler 生效；
* `imread` 缺失文件时消息包含路径。

## 6. 未验证项

| 项 | 原因 |
| --- | --- |

| JPEG/BMP 往返 | 只验证 PNG；三者走同一 stb 路径 |
| ncnn simpleocv 编译 | 已在 `NCNN_SIMPLEOCV=ON` 下通过；examples 未全量构建 |
| 非 x86 运行时 | NEON/SSE 分支只做编译期与代码审查 |
| 真实 OOM | 未做 RLIMIT_AS 子进程注入 |
