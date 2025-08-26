#ifndef ARCHIVATOR_IFS_TRANSFORM_HPP
#define ARCHIVATOR_IFS_TRANSFORM_HPP

#include <vector>
#include <memory>
#include <cstddef>
#include <image/Image.hpp> // pixel_value

class IFSTransform {
public:
    enum Sym {
        SYM_NONE = 0,
        SYM_R90,
        SYM_R180,
        SYM_R270,
        SYM_HFLIP,
        SYM_VFLIP,
        SYM_RDFLIP,
    };

    // Пространственные параметры (координаты блока в исходном/целевом пространстве)
    std::size_t from_x;
    std::size_t from_y;
    std::size_t to_x;
    std::size_t to_y;
    std::size_t size;

    // Симметрия и интенсивность
    Sym    symmetry;
    double scale;
    int    offset;

    // Даунсэмпл 2x (возвращает target_size x target_size)
    static std::vector<pixel_value>
    down_sample(const pixel_value* src,
                int src_width,
                int start_x,
                int start_y,
                int target_size);

    IFSTransform(int from_x, int from_y, int to_x, int to_y, int size,
                 Sym symmetry, double scale, int offset) noexcept
        : from_x(static_cast<std::size_t>(from_x))
        , from_y(static_cast<std::size_t>(from_y))
        , to_x(static_cast<std::size_t>(to_x))
        , to_y(static_cast<std::size_t>(to_y))
        , size(static_cast<std::size_t>(size))
        , symmetry(symmetry)
        , scale(scale)
        , offset(offset) {}

    ~IFSTransform() = default;

    // Если downsampled=false, выполняется внутренний даунсэмпл источника (во временный буфер).
    void execute(const pixel_value* src, int src_width,
                 pixel_value* dest, int dest_width,
                 bool downsampled) const;

private:
    bool is_scanline_order() const noexcept;
    bool is_positive_x()    const noexcept;
    bool is_positive_y()    const noexcept;
};

// RAII-хранилище трансформов
using transform = std::vector<std::unique_ptr<IFSTransform>>;

struct Transforms {
    int channels{0};
    transform ch[3];             // по каналам

    Transforms() = default;
    ~Transforms() = default;     // unique_ptr всё освободит

    // Возвращает суммарное число трансформов (ранее название было "get_size")
    int get_size() const noexcept {
        return static_cast<int>(ch[0].size() + ch[1].size() + ch[2].size());
    }
};

#endif
