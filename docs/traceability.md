# 追溯矩阵 · mimage

> `scripts/check-traceability.py --check` 校验：REQ-001..010 无缺号、每条有 `●`、
> 引用的文件存在、`Suite.Case` 真实存在、**每个 TEST 都被本文提及**。

## 1. 需求

| 需求 | 设计 | 实现 | 测试 |
| --- | --- | --- | --- |
| **REQ-001** Image 容器与空语义 | DES §2 | ● `include/mimage/image.hpp`<br>● `src/image.cpp` | ● `Image.DefaultIsEmpty`<br>● `Image.CreateAllocatesRefcountedStorage`<br>● `Image.RejectsInvalidDimensions` |
| **REQ-002** 引用计数共享 | DES §2 | ● `include/mimage/image.hpp` | ● `Image.CreateAllocatesRefcountedStorage` |
| **REQ-003** 外部数据不拥有 | DES §2 | ● `include/mimage/image.hpp` | ● `Image.ExternalDataIsNotOwned` |
| **REQ-004** 深拷贝与 ROI | DES §2 | ● `include/mimage/image.hpp` | ● `Image.CloneIsDeepCopy`<br>● `Image.RoiIsDeepCopy` |
| **REQ-005** 标量填充 | DES §2 | ● `include/mimage/image.hpp` | ● `Image.ScalarAssignmentFillsAllChannels` |
| **REQ-006** 图像编解码 | DES §3 | ● `src/image.cpp` | ● `Image.PngRoundTripPreservesPixels` |
| **REQ-007** 解码失败诊断 | DES §5 | ● `src/image.cpp` | ● `Image.ImreadLogsMissingFileThroughHandler` |
| **REQ-008** 双线性缩放 | DES §4 | ● `src/resize.cpp`<br>● `src/image.cpp` | ● `Image.ResizeProducesRequestedDimensions` |
| **REQ-009** 绘制原语 | DES §4 | ● `src/drawing.cpp`<br>● `src/image.cpp` | ● `Image.DrawingWritesExpectedPixels`<br>● `Image.TextSizeAndTextDrawing`<br>● `Image.ShowAndWaitKeyAreStubs` |
| **REQ-010** 可注入日志接口 | DES §5 | ● `include/mimage/log.hpp`<br>● `src/log.cpp` | ● `Log.IsSilentWhenNoHandlerInstalled`<br>● `Log.ForwardsLevelMessageAndUserData`<br>● `Log.CanBeReplacedAndCleared` |

## 2. 用例清单（反向检查）

● `Image.DefaultIsEmpty`
● `Image.CreateAllocatesRefcountedStorage`
● `Image.RejectsInvalidDimensions`
● `Image.ExternalDataIsNotOwned`
● `Image.CloneIsDeepCopy`
● `Image.RoiIsDeepCopy`
● `Image.ScalarAssignmentFillsAllChannels`
● `Image.PngRoundTripPreservesPixels`
● `Image.ImreadLogsMissingFileThroughHandler`
● `Image.ResizeProducesRequestedDimensions`
● `Image.DrawingWritesExpectedPixels`
● `Image.TextSizeAndTextDrawing`
● `Image.ShowAndWaitKeyAreStubs`
● `Log.IsSilentWhenNoHandlerInstalled`
● `Log.ForwardsLevelMessageAndUserData`
● `Log.CanBeReplacedAndCleared`

## 3. 未覆盖项

| 项 | 原因 |
| --- | --- |
| `mmat::Mat` 像素转换 | M2 范围：`from_pixels` / `to_pixels` / YUV 旋转/仿射 |
| ncnn simpleocv 替换 | M2 集成步骤 |
| 非 x86 运行时 | NEON/SSE 分支只做编译期与代码审查 |
| 真实 OOM | 未做 RLIMIT_AS 子进程注入 |
| 图像格式全覆盖 | 只验证 PNG 往返；JPEG/BMP 依赖 stb，走同一路径 |
