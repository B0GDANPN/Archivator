#include <vector>
#include <algorithm>
#include <image/IFSTransform.hpp>

std::vector<pixel_value>
IFSTransform::down_sample(const pixel_value* src,
                          int src_width,
                          int start_x,
                          int start_y,
                          int target_size)
{
    // caller обязан гарантировать валидность прямоугольника
    const int ts = target_size;
    std::vector<pixel_value> dest(static_cast<std::size_t>(ts) * ts);

    int dest_y = 0;
    for (int y = start_y; y < start_y + ts * 2; y += 2, ++dest_y) {
        int dest_x = 0;
        for (int x = start_x; x < start_x + ts * 2; x += 2, ++dest_x) {
            int pixel = 0;
            pixel += src[y * src_width + x];
            pixel += src[y * src_width + (x + 1)];
            pixel += src[(y + 1) * src_width + x];
            pixel += src[(y + 1) * src_width + (x + 1)];
            pixel /= 4;
            dest[static_cast<std::size_t>(dest_y) * ts + dest_x] = static_cast<pixel_value>(pixel);
        }
    }
    return dest;
}

bool IFSTransform::is_positive_x() const noexcept {
    return (
        symmetry == SYM_NONE ||
        symmetry == SYM_R90  ||
        symmetry == SYM_VFLIP||
        symmetry == SYM_RDFLIP
    );
}

bool IFSTransform::is_positive_y() const noexcept {
    return (
        symmetry == SYM_NONE ||
        symmetry == SYM_R270 ||
        symmetry == SYM_HFLIP||
        symmetry == SYM_RDFLIP
    );
}

bool IFSTransform::is_scanline_order() const noexcept {
    return (
        symmetry == SYM_NONE ||
        symmetry == SYM_R180 ||
        symmetry == SYM_HFLIP||
        symmetry == SYM_VFLIP
    );
}

void IFSTransform::execute(const pixel_value* src, int src_width,
                           pixel_value* dest, int dest_width,
                           bool downsampled) const
{
    // Локальные (знаковые) координаты для удобства инкрементов/декрементов
    int from_x_i = static_cast<int>(this->from_x);
    int from_y_i = static_cast<int>(this->from_y);
    int d_x = 1;
    int d_y = 1;
    const bool in_order = is_scanline_order();

    // Если источник не даунсэмплен — сделаем временный 2x-даунсэмпл блока
    std::vector<pixel_value> tmp_down;
    if (!downsampled) {
        tmp_down = down_sample(src, src_width,
                               static_cast<int>(this->from_x),
                               static_cast<int>(this->from_y),
                               static_cast<int>(this->size));
        src       = tmp_down.data();
        src_width = static_cast<int>(this->size);
        from_x_i  = 0;
        from_y_i  = 0;
    } else {
        // если plane уже даунсэмплен, но из исходных координат
        from_x_i /= 2;
        from_y_i /= 2;
    }

    if (!is_positive_x()) {
        from_x_i += static_cast<int>(size) - 1;
        d_x = -1;
    }
    if (!is_positive_y()) {
        from_y_i += static_cast<int>(size) - 1;
        d_y = -1;
    }

    const int start_x = from_x_i;
    const int start_y = from_y_i;

    for (std::size_t ty = this->to_y; ty < (this->to_y + this->size); ++ty) {
        for (std::size_t tx = this->to_x; tx < (this->to_x + this->size); ++tx) {

            int pixel = src[from_y_i * src_width + from_x_i];
            pixel = static_cast<int>(scale * pixel) + offset;

            if (pixel < 0)   pixel = 0;
            if (pixel > 255) pixel = 255;

            dest[ty * static_cast<std::size_t>(dest_width) + tx] = static_cast<pixel_value>(pixel);

            if (in_order) from_x_i += d_x;
            else          from_y_i += d_y;
        }

        if (in_order) {
            from_x_i = start_x;
            from_y_i += d_y;
        } else {
            from_y_i = start_y;
            from_x_i += d_x;
        }
    }
}
