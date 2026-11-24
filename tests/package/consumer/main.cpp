#include <mimage/image.hpp>

int main() {
    mimage::Image image(4, 5, mimage::kPixelRgb);
    if (image.empty() || image.total() != 60U) {
        return 1;
    }

    image = mimage::Scalar(10, 20, 30);
    const mimage::Image copy = image.clone();
    if (copy.empty() || copy.data == image.data || copy.data[0] != 10) {
        return 2;
    }

    mimage::Image resized;
    mimage::resize(copy, resized, mimage::Size(10, 8));
    if (resized.cols != 10 || resized.rows != 8) {
        return 3;
    }

    mimage::rectangle(resized, mimage::Rect(1, 1, 3, 3), mimage::Scalar(255, 0, 0));
    return 0;
}
