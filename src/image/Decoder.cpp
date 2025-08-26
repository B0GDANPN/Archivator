#include <utility>
#include <vector>
#include <stdexcept>
#include <image/Decoder.hpp>
#include <image/IFSTransform.hpp>

// Конструктор: задаём метаданные и инициализируем буферы каналов серым
Decoder::Decoder(int width,
                 int height,
                 int channels,
                 bool is_text_output,
                 std::string  output_file,
                 std::ostringstream& ref_oss)
    : is_text_output_(is_text_output)
    , output_file_(std::move(output_file))
    , ref_oss_(ref_oss)
    , img_(is_text_output_, output_file_, ref_oss_)
{
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("Decoder: width/height must be > 0");
    if (channels < 1 || channels > 3)
        throw std::invalid_argument("Decoder: channels must be in [1..3]");

    img_.width  = width;
    img_.height = height;
    img_.channels = channels;
    img_.original_size = width * height * channels;

    init_grey_channels();
}

void Decoder::init_grey_channels() {
    const int plane = img_.width * img_.height;
    std::vector<pixel_value> grey(static_cast<size_t>(plane), static_cast<pixel_value>(127));
    // Гарантируем заполнение всех заявленных каналов (1..channels)
    for (int c = 1; c <= img_.channels; ++c) {
        img_.set_channel_data(c, grey.data(), plane);
    }
}

void Decoder::ensure_channels(int required_channels) {
    if (required_channels < 1 || required_channels > 3)
        throw std::invalid_argument("ensure_channels_: channels out of range");
    if (img_.channels >= required_channels) return;

    const int plane = img_.width * img_.height;
    std::vector<pixel_value> grey(static_cast<size_t>(plane), static_cast<pixel_value>(127));
    for (int c = img_.channels + 1; c <= required_channels; ++c) {
        img_.set_channel_data(c, grey.data(), plane);
    }
    img_.channels = required_channels;
    img_.original_size = img_.width * img_.height * img_.channels;
}

void Decoder::decode(const Transforms& transforms)
{
    // Синхронизируем число каналов, если Transforms его задаёт
    if (transforms.channels != 0) {
        ensure_channels(transforms.channels);
    }

    // Применяем трансформы к каждому задействованному каналу (1..channels)
    for (int channel = 1; channel <= img_.channels; ++channel) {
        pixel_value* plane = nullptr;
        if (channel == 1)      plane = img_.image_data1;
        else if (channel == 2) plane = img_.image_data2;
        else                   plane = img_.image_data3;

        // Если по какой-то причине плоскость не инициализирована — это ошибка API
        if (!plane) throw std::runtime_error("Decoder: channel plane is null");

        // Прогоняем цепочку трансформов
        // Ожидается, что transforms.ch[channel-1] — контейнер IFSTransform*

        for (const auto& t  : transforms.ch[channel - 1]) {
            if (!t) continue; // или assert(t && "IFSTransform must not be null");
            t->execute(plane, img_.width, plane, img_.width, /*some flag*/ false);
        }
    }
}

std::unique_ptr<Image>
Decoder::make_image(const std::string& file_name, int channel) const
{
    // Собираем новое Image с такими же логгер-параметрами
    auto out = std::make_unique<Image>(is_text_output_, output_file_, ref_oss_);
    out->image_setup(file_name);

    out->width  = img_.width;
    out->height = img_.height;

    const int plane = img_.width * img_.height;

    if (channel == 0) {
        // Все каналы, как есть
        out->channels = img_.channels;
        out->original_size = out->width * out->height * out->channels;

        if (img_.channels >= 1 && img_.image_data1)
            out->set_channel_data(1, img_.image_data1, plane);
        if (img_.channels >= 2 && img_.image_data2)
            out->set_channel_data(2, img_.image_data2, plane);
        if (img_.channels >= 3 && img_.image_data3)
            out->set_channel_data(3, img_.image_data3, plane);
    } else {
        // Только один указанный канал -> выводим как одно-канальное изображение
        if (channel < 1 || channel > img_.channels)
            throw std::out_of_range("make_image: channel out of range");

        out->channels = 1;
        out->original_size = out->width * out->height * out->channels;

        const pixel_value* src = nullptr;
        if (channel == 1)      src = img_.image_data1;
        else if (channel == 2) src = img_.image_data2;
        else                   src = img_.image_data3;

        if (!src) throw std::runtime_error("make_image: source channel is null");

        out->set_channel_data(1, src, plane);
    }

    return out;
}
