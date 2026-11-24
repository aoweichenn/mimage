# 需求 · 图像容器与基础处理

## REQ-001 · Image 容器与空语义

**契约**

* 默认构造得到 `data == nullptr`、`rows/cols/c == 0` 的空图。
* `create(rows, cols, channels)` 校验正数；非法尺寸保持空图。
* `empty()` 在 `data == nullptr` 或 `total() == 0` 时为真；`total() = rows*cols*c`。

**验收**

`Image.DefaultIsEmpty`、`Image.CreateAllocatesRefcountedStorage`、
`Image.RejectsInvalidDimensions`。

## REQ-002 · 引用计数共享

**契约**

* 拷贝构造/赋值共享存储，引用计数加一。
* 最后一个引用析构时通过 `mmemory::fast_free` 释放。
* 控制块位于数据尾部，按 4 字节对齐。

**验收**

`Image.CreateAllocatesRefcountedStorage`。

## REQ-003 · 外部数据不拥有

**契约**

* 外部数据构造不设置 `refcount`，析构不释放调用方内存。
* `data` 指向调用方缓冲区。

**验收**

`Image.ExternalDataIsNotOwned`。

## REQ-004 · 深拷贝与 ROI

**契约**

* `clone()` 分配新存储并复制全部像素。
* `operator()(Rect)` 返回 ROI 的深拷贝，不共享源存储。

**验收**

`Image.CloneIsDeepCopy`、`Image.RoiIsDeepCopy`。

## REQ-005 · 标量填充

**契约**

* `image = Scalar(...)` 按通道填充所有像素。
* 支持 1/3/4 通道。

**验收**

`Image.ScalarAssignmentFillsAllChannels`。

## REQ-006 · 图像编解码

**契约**

* `imread` / `imdecode` 支持 JPEG/PNG/BMP/PNM，返回 BGR(A) 或灰度。
* `imwrite` 根据扩展名选择编码器，RGB 与 BGR 互换。
* 往返后像素一致。

**验收**

`Image.PngRoundTripPreservesPixels`。

## REQ-007 · 解码失败诊断

**契约**

* 解码失败返回空 `Image`，并通过日志接口记录路径与 stb 失败原因。
* 未知 flags、空输入、未知通道数同样记录。

**验收**

`Image.ImreadLogsMissingFileThroughHandler`。

## REQ-008 · 双线性缩放

**契约**

* `resize` 支持 1/3/4 通道，目标尺寸由 `Size` 或比例计算。
* 支持 stride 版本与 YUV420SP 版本。
* 不支持的通道数记录错误并保持 `dst` 不变。

**验收**

`Image.ResizeProducesRequestedDimensions`。

## REQ-009 · 绘制原语

**契约**

* `rectangle` / `circle` / `line` / `putText` 在 1/3/4 通道图上工作。
* `getTextSize` / `get_text_drawing_size` 返回正尺寸。
* `imshow` 保存 PNG，`waitKey` 为 stub。

**验收**

`Image.DrawingWritesExpectedPixels`、`Image.TextSizeAndTextDrawing`、
`Image.ShowAndWaitKeyAreStubs`。

## REQ-010 · 可注入日志接口

**契约**

* `set_log_handler(handler, user_data)` 安装/清除诊断回调。
* 默认静默；消息包含级别、格式化文本与 `user_data`。
* 内部格式化不分配、不抛异常。

**验收**

`Log.IsSilentWhenNoHandlerInstalled`、`Log.ForwardsLevelMessageAndUserData`、
`Log.CanBeReplacedAndCleared`。
