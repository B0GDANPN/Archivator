//
// Created by bogdan on 10.04.24.
//

#ifndef ARCHIVATOR_FRACTAL_ALGO_HPP
#define ARCHIVATOR_FRACTAL_ALGO_HPP

#include <string>
#include <image/Image.hpp>
#include <controller/IController.hpp>

namespace fs = std::filesystem;

class FractalAlgo final : public IController {
    void send_common_information(const CommonInformation &common_information) override;

    void send_error_information(const std::string &error) override ;

    void send_encoded_information(int width, int height, int num_transforms) const;

    void send_decoded_information(int width, int height, int phases) const;

public:
    explicit FractalAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
            : IController(is_text_output, output_file, ref_oss) {}

    void encode(const std::string &input_filename, int quality) const;
};


#endif