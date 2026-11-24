# 设计 · mimage

## 1. 边界

M1 负责 8-bit 交错图像：

* `Image` 容器：`rows × cols × channels`，单块引用计数存储；
* 编解码：`imread` / `imdecode` / `imwrite`（stb_image / stb_image_write）；
* 双线性缩放：`resize` 与 raw-pointer 入口；
* 绘制：矩形/圆/线/文字与字体位图；
* 可注入日志接口。

**不包含**：

* YUV 旋转、仿射变换（`mat_pixel_rotate.cpp` / `mat_pixel_affine.cpp`）；
* Vulkan / GPU 图像。

这些留给 M2。

## 2. Image 存储

```
data       rows*cols*channels 字节，交错存储
refcount   位于对齐后的尾部，4 字节
rows cols c
```

* 分配：`mmemory::fast_malloc(total + sizeof(int32))`；
* 引用计数：`std::atomic_ref<int32>::fetch_add(±1, acq_rel)`；
* 外部数据构造：`refcount == nullptr`，析构不释放；
* `create` 对非法尺寸直接返回空图，分配失败清空尺寸。

## 3. 编解码

* `imread` / `imdecode` 用 stb_image，支持 JPEG/PNG/BMP/PNM；
* 解码后统一转成 BGR/BGRA（与 ncnn simpleocv 一致）；
* `imwrite` 先 clone，再 BGR→RGB，按扩展名调用 stb 写 JPEG/PNG/BMP；
* 每个失败点调用 `MIMAGE_LOGE`，包含路径、通道数或 stb 原因。

## 4. 缩放与绘制

`resize_bilinear_c*` 与 `draw_*` 是 ncnn 的 raw-pointer 算法，只依赖
`std::uint8_t*` 和尺寸，不依赖 `Mat`。`Image` 层做通道分发与参数校验。

绘制使用 `std::uint32_t color` 的字节序打包，保持 ncnn 行为。

## 5. 日志接口

```cpp
using LogHandler = void (*)(LogLevel, const char* message, void* user_data);
void set_log_handler(LogHandler, void*) noexcept;
```

* 两个 `std::atomic` 保存 handler 与 user_data，默认 `nullptr`；
* `log_format` 用 `vsnprintf` 写入 1024 字节栈缓冲；
* 不分配、不抛异常；
* 测试通过捕获 handler 验证级别、文本与 user_data。

## 6. 依赖与分层

```
mimage → mmemory → mplatform
```

依赖方向 `mimage → mmat → mmemory → mplatform`。`pixel.hpp` 的转换函数
直接返回/接收 `mmat::Mat`，ncnn 的 `Mat` 派生类可以零拷贝转发。

## 7. 像素转换

`pixel.hpp` 复刻 ncnn `Mat::PixelType` 的枚举值与语义：

* `from_pixels*` 把交错像素转成 `mmat::Mat` 的 planar 布局（3D，elemsize 4）；
* `to_pixels*` 反向转换，支持 stride 与目标缩放；
* `yuv420sp2rgb*` 保留 ncnn 的近似公式；
* allocator 参数类型为 `mmemory::Allocator*`，ncnn 的 `Allocator*` 直接传入。

## 8. ncnn 兼容

`Image` 的公开字段与方法对齐 ncnn `simpleocv::Mat`：

```
data refcount rows cols c
create/release/clone/empty/channels/type/total/ptr/operator()(Rect)
```

因此 ncnn 侧可以把 `cv::Mat` 替换为 `mimage::Image`，函数签名保持不变。
