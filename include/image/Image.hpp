#ifndef ARCHIVATOR_IMAGE_HPP
#define ARCHIVATOR_IMAGE_HPP
#include <string>
#include <controller/IController.hpp>
typedef unsigned char pixel_value;


class Image final : public IController {
public:
    int width = 0;
    int height = 0;
    int channels = 0;
    pixel_value *image_data1 = nullptr;
    pixel_value *image_data2 = nullptr;
    pixel_value *image_data3 = nullptr;
    std::string file_name;
    std::string extension;
    int original_size = 0;
    Image(const bool is_text_output, const std::string &output_file,std::ostringstream& ref_oss) : IController(is_text_output, output_file,ref_oss) {};

    void send_info_modified_image(int new_width, int new_height) const;

    void send_common_information(const CommonInformation &common_information) override ;

    void send_error_information(const std::string &error) override ;
    ~Image() override;

    static int next_multiple_of(int number, int multiple);

    void load();

    void save();

    void get_channel_data(int channel, pixel_value *buffer, int size);

    void set_channel_data(int channel, const pixel_value *buffer, int size_channel);

    void image_setup(const std::string &file_name);
};

#endif