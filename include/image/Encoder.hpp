#ifndef ARCHIVATOR_ENCODER_HPP
#define ARCHIVATOR_ENCODER_HPP
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include "controller/IController.hpp"

class Encoder : public IController {
public:
    void send_common_information(const CommonInformation &common_information) override;

    void send_error_information(const std::string &error) override;

    Encoder(bool is_text_output, const std::string &output_file,std::ostringstream& ref_oss);

    ~Encoder() override = default;

    virtual Transforms *encode(Image *source) = 0;

    // These functions are helpers
    int get_average_pixel(const pixel_value *domain_data, int domain_width,
                        int domain_x, int domain_y, int size);

    double get_scale_factor(
            const pixel_value *domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
            const pixel_value *range_data, int range_width, int range_x, int range_y, int range_avg,
            int size);

    double get_error(
            const pixel_value *domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
            const pixel_value *range_data, int range_width, int range_x, int range_y, int range_avg,
            int size, double scale);

    Image img;
};

#endif
