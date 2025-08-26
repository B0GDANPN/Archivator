// QuadTreeEncoder.cpp
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include <image/QuadTreeEncoder.hpp>

// encode: читает исходный Image по константной ссылке, возвращает владение Transforms через unique_ptr
std::unique_ptr<Transforms> QuadTreeEncoder::encode(const Image& source)
{
    // Подготовим «рабочие» метаданные (используются helpers базового Encoder)
    img.width        = source.width;
    img.height       = source.height;
    img.channels     = source.channels;
    img.original_size= img.width * img.height * img.channels;

    if (img.width <= 0 || img.height <= 0 || img.channels < 1 || img.channels > 3) {
        send_error_information("Error: QuadTreeEncoder::encode invalid image metadata\n");
        throw std::invalid_argument("invalid image");
    }

    auto transforms = std::make_unique<Transforms>();
    transforms->channels = img.channels;

    const int plane   = img.width * img.height;
    const int down_w  = img.width  / 2;
    const int down_h  = img.height / 2;

    for (int channel = 1; channel <= img.channels; ++channel) {
        // 1) Локальная копия канала (range-плоскость)
        std::vector<pixel_value> range(static_cast<size_t>(plane));
        // NB: get_channel_data пишет size байт; в проекте size == width*height
        const_cast<Image&>(source).get_channel_data(channel, range.data(), plane);

        std::unique_ptr<pixel_value[]> down(
            (IFSTransform::down_sample(range.data(), img.width, /*x*/0, /*y*/0, /*newWidth*/down_w).data())
        );
        if (!down) {
            send_error_information("Error: down_sample returned null\n");
            throw std::runtime_error("down_sample failed");
        }

        // 3) Обход всех range-блоков N x N
        for (int y = 0; y < img.height; y += BUFFER_SIZE) {
            for (int x = 0; x < img.width; x += BUFFER_SIZE) {
                find_matches_for(transforms->ch[channel - 1],
                                 x, y, BUFFER_SIZE,
                                 range.data(), img.width,
                                 down.get(),  down_w,
                                 img.height);
            }
        }
        // RAII: vector/unique_ptr освободят ресурсы по выходу из scope
    }

    return transforms;
}

void QuadTreeEncoder::find_matches_for(transform& out,
                                       int to_x, int to_y,
                                       int block_size,
                                       const pixel_value* range_plane, int range_stride,
                                       const pixel_value* down_plane,  int down_stride,
                                       int image_height)
{
    if (!range_plane || !down_plane) {
        send_error_information("Error: find_matches_for null plane\n");
        throw std::invalid_argument("null plane");
    }
    if (block_size <= 0 || range_stride <= 0 || down_stride <= 0) {
        send_error_information("Error: find_matches_for invalid strides/sizes\n");
        throw std::invalid_argument("invalid stride/size");
    }

    int best_x = 0;
    int best_y = 0;
    int best_offset = 0;
    IFSTransform::Sym best_symmetry = IFSTransform::SYM_NONE;
    double best_scale = 0.0;
    double best_error = 1e9;

    // Буфер под домен-блок после трансформации (N x N)
    std::vector<pixel_value> buffer(static_cast<size_t>(block_size) * block_size);

    // Среднее для range-блока
    const int range_avg = get_average_pixel(range_plane, range_stride,
                                            to_x, to_y, block_size);

    // Перебор всех домен-блоков в даунсэмпле (шаг = block_size по полной картинке → /2 в даунсэмпле)
    for (int y = 0; y < image_height; y += block_size * 2) {
        for (int x = 0; x < range_stride; x += block_size * 2) {
            const int dx = x / 2; // координаты в downsample-плоскости
            const int dy = y / 2;

            // Временный трансформ (RAII)
             constexpr auto symmetry_enum = IFSTransform::SYM_NONE;
            IFSTransform ifs(dx, dy, /*to_x*/0, /*to_y*/0,
                             block_size, symmetry_enum,
                             /*scale*/1.0, /*offset*/0);

            // Применяем трансформ: из down_plane (stride=down_stride) в buffer (stride=block_size)
            ifs.execute(down_plane, down_stride,
                        buffer.data(), block_size,
                        /*isDownSampled*/ true);

            const int domain_avg = get_average_pixel(buffer.data(), block_size, 0, 0, block_size);

            const double scale = get_scale_factor(range_plane,  range_stride, to_x, to_y, range_avg,
                                                  buffer.data(), block_size,   0,    0,   domain_avg,
                                                  block_size);

            const int offset = static_cast<int>(range_avg - scale * static_cast<double>(domain_avg));

            const double error = get_error(buffer.data(), block_size, 0, 0, domain_avg,
                                           range_plane,  range_stride, to_x, to_y, range_avg,
                                           block_size, scale);

            if (error < best_error) {
                best_error   = error;
                best_x       = x;
                best_y       = y;
                best_symmetry= symmetry_enum;
                best_scale   = scale;
                best_offset  = offset;
            }
        }
    }

    if (block_size > 2 && best_error >= static_cast<double>(quality_)) {
        // Рекурсивное деление на 4 подблока
        const int half = block_size / 2;
        find_matches_for(out, to_x,         to_y,         half, range_plane, range_stride, down_plane, down_stride, image_height);
        find_matches_for(out, to_x + half,  to_y,         half, range_plane, range_stride, down_plane, down_stride, image_height);
        find_matches_for(out, to_x,         to_y + half,  half, range_plane, range_stride, down_plane, down_stride, image_height);
        find_matches_for(out, to_x + half,  to_y + half,  half, range_plane, range_stride, down_plane, down_stride, image_height);
    } else {
        // Лист квадродерева — сохраняем лучшую трансформацию
        // NB: тип transform = std::vector<IFSTransform*>, поэтому пока raw-пойнтер.
        out.push_back(std::make_unique<IFSTransform>(best_x, best_y,
                                             to_x, to_y,
                                             block_size,
                                             best_symmetry,
                                             best_scale,
                                             best_offset));
    }
}
