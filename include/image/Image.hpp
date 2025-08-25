#ifndef ARCHIVATOR_IMAGE_HPP
#define ARCHIVATOR_IMAGE_HPP
#include <string>
#include <vector>
#include <controller/IController.hpp>

using pixel_value = unsigned char;

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
    ~Image() override =default;

    static int next_multiple_of(int number, int multiple);

    void load();

    void save();

    void get_channel_data(int channel, pixel_value *buffer, int size);

    void set_channel_data(int channel, const pixel_value *buffer, int size_channel);

    void image_setup(const std::string &file_name);
private:
    std::vector<pixel_value> ch1_;
    std::vector<pixel_value> ch2_;
    std::vector<pixel_value> ch3_;

    // Синхронизировать публичные указатели с внутренними буферами
    void sync_raw_ptrs() noexcept {
        image_data1 = ch1_.empty() ? nullptr : ch1_.data();
        image_data2 = ch2_.empty() ? nullptr : ch2_.data();
        image_data3 = ch3_.empty() ? nullptr : ch3_.data();
    }

    // Инициализация всех каналов серым 127
    void init_grey_planes();
};

#endif