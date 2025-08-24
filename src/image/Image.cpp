
#include <sstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cmath>
#include <filesystem>
#include <image/Image.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "image/stb_image.h"
#include "image/stb_image_write.h"
void Image::send_info_modified_image(int new_width, int new_height) const{
  std::ostringstream oss;
  oss << "Modifying image to (width=" << new_width << " height=" << new_height << ")" << '\n';
  std::string str = oss.str();
  send_message(str);
}
void Image::send_common_information(const CommonInformation& common_information){
  send_message("FractalAlgo{ ");
  IController::send_common_information(common_information);
  send_message("}\n");
}
void Image::send_error_information(const std::string& error){
  IController::send_error_information("FractalAlgo{ " + error + "}\n");
}
Image::~Image(){
  if (image_data1 != nullptr) {
    delete[]image_data1;
    image_data1 = nullptr;
  }
  if (image_data2 != nullptr) {
    delete[]image_data2;
    image_data2 = nullptr;
  }
  if (image_data3 != nullptr) {
    delete[]image_data3;
    image_data3 = nullptr;
  }
}
int Image::next_multiple_of(int number, int multiple) {
  return ((number + multiple - 1) / multiple) * multiple;
}
void Image::load() {
  std::string full_name =(this->file_name + '.' + this->extension);
        std::string cur_dir=std::filesystem::current_path().string();
        unsigned char *original_data = stbi_load(full_name.c_str(), &width, &height, &channels, 0);
        if (!original_data) {
            send_error_information("Error: Could not load image\n");
            exit(-1);
        }
        original_size = width * height * channels;
        int new_width = next_multiple_of(width, 32);
        int new_height = next_multiple_of(height, 32);
        if (new_height > new_width)
            new_width = new_height;
        if (new_width > new_height)
            new_height = new_width;
        if (new_width != width || new_height != height) {
            send_info_modified_image(new_width, new_height);
            original_size = new_width * new_height * channels;
            auto *new_data = new unsigned char[original_size];
            memset(new_data, 0, original_size);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    for (int c = 0; c < channels; ++c) {
                        new_data[(y * new_width + x) * channels + c] = original_data[(y * width + x) * channels + c];
                    }
                }
            }
            width = new_width;
            height = new_height;
            stbi_image_free(original_data);
            original_data = new_data;
        }

        // Convert to image data type
        if (image_data1 != nullptr) {
            delete[]image_data1;
            delete[]image_data2;
            delete[]image_data3;
        }
        int size_channel = original_size / 3;
        image_data1 = new pixel_value[size_channel];
        image_data2 = new pixel_value[size_channel];
        image_data3 = new pixel_value[size_channel];
        for (int i = 0; i < size_channel; i++) {
            pixel_value r = original_data[i * 3];
            pixel_value g = original_data[i * 3 + 1];
            pixel_value b = original_data[i * 3 + 2];
            image_data1[i] = r;
            image_data2[i] = g;
            image_data3[i] = b;
        }
        delete[]original_data;

}
  void Image::save(){
    std::ostringstream oss;
    oss << "Writing image (width=" << width << " height=" << height << " channels=" << channels << ")" << '\n';
    std::string str = oss.str();
    send_message(str);

    auto *chunk = new unsigned char[original_size];

    // Convert from image data type
    for (int i = 0; i < width * height; i++) {
      if (channels == 3) {
        pixel_value r = image_data1[i], g = image_data2[i], b = image_data3[i];
        chunk[i * 3 + 0] = static_cast<unsigned char> (r);
        chunk[i * 3 + 1] = static_cast<unsigned char> (g);
        chunk[i * 3 + 2] = static_cast<unsigned char> (b);
      } else if (channels == 1) {
        chunk[i] = image_data1[i];
      }
    }
    std::string full_name = (this->file_name + '.' + this->extension);
    if (this->extension == "bmp") {
      stbi_write_bmp(full_name.c_str(), width, height, channels, chunk);
    } else if (this->extension == "tga") {
      stbi_write_tga(full_name.c_str(), width, height, channels, chunk);
    } else if (this->extension == "jpg" || this->extension == "jpeg") {
      stbi_write_jpg(full_name.c_str(), width, height, channels, chunk, 100);
    } else {
      send_error_information("Error: Non supported extension " + this->extension + '\n');
      exit(-1);
    }
    delete[]chunk;
  }
  void Image::get_channel_data(int channel, pixel_value* buffer, int size){

    if ((channel == 1 && image_data1 == nullptr) ||
        (channel == 2 && image_data2 == nullptr) ||
        (channel == 3 && image_data3 == nullptr)) {
      send_error_information("Error: Image data was not loaded yet.\n");
      delete[]buffer;
      exit(-1);
        }
    if (width * height != size) {
      send_error_information("Error: Image data size mismatch.\n");
      delete[]buffer;
      exit(-1);
    }
    if (channel > channels || channel <= 0) {
      send_error_information("Error: Image channel out of bounds.\n");
      delete[]buffer;
      exit(-1);
    }

    if (channel == 1)
      memcpy(buffer, image_data1, size);
    else if (channel == 2)
      memcpy(buffer, image_data2, size);
    else if (channel == 3)
      memcpy(buffer, image_data3, size);
  }
  void Image::set_channel_data(int channel, const pixel_value* buffer, int size_channel){
    pixel_value *image_data_temp = nullptr;


    if (channel > channels)
      channels = channel;

    if (channel == 1) {
      image_data_temp = image_data1;
      image_data1 = new pixel_value[size_channel];
      memcpy(image_data1, buffer, size_channel);
    } else if (channel == 2) {
      image_data_temp = image_data2;
      image_data2 = new pixel_value[size_channel];
      memcpy(image_data2, buffer, size_channel);
    } else if (channel == 3) {
      image_data_temp = image_data3;
      image_data3 = new pixel_value[size_channel];
      memcpy(image_data3, buffer, size_channel);
    }
    delete[]image_data_temp;
  }
  void Image::image_setup(const std::string& file_name) {

    size_t last_dot_index = file_name.rfind('.');
    this->file_name = file_name.substr(0, last_dot_index);
    this->extension = file_name.substr(last_dot_index + 1);
  }