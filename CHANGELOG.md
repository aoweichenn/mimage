# Changelog

## 0.1.0 — 2026-11-24

首个独立版本。

* `mimage::Image`：交错 8-bit 图像容器，引用计数、clone、ROI、标量填充。
* `imread` / `imdecode` / `imwrite`：JPEG/PNG/BMP/PNM，BGR(A) 约定。
* `resize` 与 `resize_bilinear_c1/c2/c3/c4`（含 stride / yuv420sp）。
* 矩形/圆/线/文字绘制与字体尺寸查询。
* 像素布局转换：`from_pixels*` / `to_pixels*` / YUV420SP，返回 `mmat::Mat`。
* kanna 旋转与 warpaffine 仿射变换。
* 可注入日志接口 `mimage::set_log_handler`，默认静默。
* 独立测试、安装包与 mzip 风格质量闸门。
