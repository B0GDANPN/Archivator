#include <stdexcept>
#include <sstream>

#include <image/Image.hpp>
#include <image/Encoder.hpp>

void Encoder::send_common_information(const CommonInformation& common_information) {
    send_message("FractalAlgo{ ");
    IController::send_common_information(common_information);
    send_message("}\n");
}

void Encoder::send_error_information(const std::string& error) {
    IController::send_error_information("FractalAlgo{ " + error + "}\n");
}

Encoder::Encoder(bool is_text_output, const std::string& output_file, std::ostringstream& ref_oss)
    : IController(is_text_output, output_file, ref_oss)
    , img(is_text_output, output_file, ref_oss) {}

// ==== helpers (без UB, без delete/exit, расширенные типы счётчиков) ====

int Encoder::get_average_pixel(const pixel_value* domain_data,
                               int domain_width,
                               int domain_x, int domain_y,
                               int size)
{
    // Предусловия
    if (!domain_data) {
        send_error_information("Error: get_average_pixel null domain_data\n");
        throw std::invalid_argument("null domain_data");
    }
    if (domain_width <= 0 || size <= 0) {
        send_error_information("Error: get_average_pixel invalid domain_width/size\n");
        throw std::invalid_argument("invalid width/size");
    }
    if (domain_x < 0 || domain_y < 0) {
        send_error_information("Error: get_average_pixel negative coords\n");
        throw std::out_of_range("negative coords");
    }

    // Аккуратная проверка правой/нижней границы потребует знать высоту плоскости.
    // Здесь считаем, что вызывающий гарантирует валидный прямоугольник.
    // Для устойчивости проверим только переполнение индексов в пределах int.
    const long long last_x = static_cast<long long>(domain_x) + size - 1LL;
    if (last_x >= domain_width) {
        send_error_information("Error: get_average_pixel x-range out of bounds\n");
        throw std::out_of_range("x out of bounds");
    }

    // 64-битный аккумулятор — никакого переполнения
    long long sum = 0;
    const long long denom = static_cast<long long>(size) * size;

    // Построчный проход
    for (int y = 0; y < size; ++y) {
        const int row = (domain_y + y) * domain_width;
        for (int x = 0; x < size; ++x) {
            sum += static_cast<unsigned int>(domain_data[row + (domain_x + x)]);
        }
    }

    // Среднее по полуинтегральному округлению вниз (как в исходнике)
    const long long avg = sum / denom;
    // пиксели хранятся в [0..255] — приводим безопасно
    return static_cast<int>(avg);
}

double Encoder::get_scale_factor(const pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
                                 const pixel_value* range_data,  int range_width,  int range_x,  int range_y,  int range_avg,
                                 int size)
{
    if (!domain_data || !range_data) {
        send_error_information("Error: get_scale_factor null plane pointer(s)\n");
        throw std::invalid_argument("null plane");
    }
    if (domain_width <= 0 || range_width <= 0 || size <= 0) {
        send_error_information("Error: get_scale_factor invalid width/size\n");
        throw std::invalid_argument("invalid width/size");
    }
    if (domain_x < 0 || domain_y < 0 || range_x < 0 || range_y < 0) {
        send_error_information("Error: get_scale_factor negative coords\n");
        throw std::out_of_range("negative coords");
    }
    if (domain_x + size > domain_width || range_x + size > range_width) {
        send_error_information("Error: get_scale_factor x-range out of bounds\n");
        throw std::out_of_range("x out of bounds");
    }

    // Используем 64-битные суммы, чтобы не словить переполнение
    long long sum_rd = 0; // Σ (R*D)
    long long sum_dd = 0; // Σ (D*D)

    for (int y = 0; y < size; ++y) {
        const int drow = (domain_y + y) * domain_width + domain_x;
        const int rrow = (range_y  + y) * range_width  + range_x;
        for (int x = 0; x < size; ++x) {
            const int d = static_cast<int>(domain_data[drow + x]) - domain_avg;
            const int r = static_cast<int>(range_data [rrow + x]) - range_avg;
            sum_rd += static_cast<long long>(r) * d;
            sum_dd += static_cast<long long>(d) * d;
        }
    }

    if (sum_dd == 0) return 0.0; // масштаб нулевой — нет вариации домена
    return static_cast<double>(sum_rd) / static_cast<double>(sum_dd);
}

double Encoder::get_error(const pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
                          const pixel_value* range_data,  int range_width,  int range_x,  int range_y,  int range_avg,
                          int size, double scale)
{
    if (!domain_data || !range_data) {
        send_error_information("Error: get_error null plane pointer(s)\n");
        throw std::invalid_argument("null plane");
    }
    if (domain_width <= 0 || range_width <= 0 || size <= 0) {
        send_error_information("Error: get_error invalid width/size\n");
        throw std::invalid_argument("invalid width/size");
    }
    if (domain_x < 0 || domain_y < 0 || range_x < 0 || range_y < 0) {
        send_error_information("Error: get_error negative coords\n");
        throw std::out_of_range("negative coords");
    }
    if (domain_x + size > domain_width || range_x + size > range_width) {
        send_error_information("Error: get_error x-range out of bounds\n");
        throw std::out_of_range("x out of bounds");
    }

    // Считаем среднеквадратичную ошибку: Σ (scale*D - R)^2 / (size*size)
    // Используем double, чтобы исключить переполнения и получить устойчивую метрику.
    double sum = 0.0;
    const double denom = static_cast<double>(size) * static_cast<double>(size);

    for (int y = 0; y < size; ++y) {
        const int drow = (domain_y + y) * domain_width + domain_x;
        const int rrow = (range_y  + y) * range_width  + range_x;
        for (int x = 0; x < size; ++x) {
            const int d0 = static_cast<int>(domain_data[drow + x]) - domain_avg;
            const int r0 = static_cast<int>(range_data [rrow + x]) - range_avg;
            const double diff = scale * static_cast<double>(d0) - static_cast<double>(r0);
            sum += diff * diff;
        }
    }

    return sum / denom;
}
