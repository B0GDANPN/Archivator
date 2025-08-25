
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


// RAII для stbi_image_free
struct StbDeleter {
  void operator()(unsigned char* p) const noexcept { stbi_image_free(p); }
};
using stb_ptr = std::unique_ptr<unsigned char, StbDeleter>;

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
int Image::next_multiple_of(int number, int multiple) {
  return ((number + multiple - 1) / multiple) * multiple;
}
void Image::init_grey_planes() {
  const int plane = width * height;
  if (plane <= 0) {
    ch1_.clear(); ch2_.clear(); ch3_.clear();
    sync_raw_ptrs();
    return;
  }
  ch1_.assign(static_cast<size_t>(plane), static_cast<pixel_value>(127));
  ch2_.assign(static_cast<size_t>(plane), static_cast<pixel_value>(127));
  ch3_.assign(static_cast<size_t>(plane), static_cast<pixel_value>(127));
  sync_raw_ptrs();
}

void Image::load() {
    const std::string full_name = file_name + '.' + extension;

    int w = 0, h = 0, ch = 0;
    const stb_ptr original{ stbi_load(full_name.c_str(), &w, &h, &ch, 0) };
    if (!original) {
        send_error_information("Error: Could not load image: " + full_name + "\n");
        throw std::runtime_error("stbi_load failed");
    }

    width = w;
    height = h;
    channels = ch;                       // как есть из файла (обычно 1 или 3)
    original_size = width * height * channels;

    // Паддинг до кратности 32 по каждой стороне, затем выравнивание до квадрата (как в исходнике)
    int new_width  = next_multiple_of(width,  32);
    int new_height = next_multiple_of(height, 32);
    if (new_height > new_width)  new_width  = new_height;
    if (new_width  > new_height) new_height = new_width;

    std::vector<unsigned char> padded;   // interleaved RGB/Gray
    const unsigned char* src = original.get();
    int out_w = width, out_h = height;

    if (new_width != width || new_height != height) {
        send_info_modified_image(new_width, new_height);
        out_w = new_width; out_h = new_height;

        const size_t padded_size = static_cast<size_t>(out_w) * out_h * channels;
        padded.assign(padded_size, 0);

        // копируем исходное изображение в верхний левый угол
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                for (int c = 0; c < channels; ++c) {
                    const size_t dst_idx = (static_cast<size_t>(y) * out_w + x) * channels + c;
                    const size_t src_idx = (static_cast<size_t>(y) * width + x) * channels + c;
                    padded[dst_idx] = src[src_idx];
                }
            }
        }
        // обновляем метаданные
        width = out_w;
        height = out_h;
        original_size = width * height * channels;
        src = padded.data();
    }

    // Разворачиваем в по-канальные плоскости
    const size_t plane = static_cast<size_t>(width) * height;
    ch1_.assign(plane, 0);
    ch2_.assign(plane, 0);
    ch3_.assign(plane, 0);

    if (channels == 1) {
        // Градации серого: только ch1_ заполняем
        std::memcpy(ch1_.data(), src, plane);
    } else if (channels == 3) {
        for (size_t i = 0; i < plane; ++i) {
            ch1_[i] = src[i * 3 + 0];
            ch2_[i] = src[i * 3 + 1];
            ch3_[i] = src[i * 3 + 2];
        }
    } else {
        // Неподдерживаемое число каналов — безопаснее конвертнуть в 3 через stbi_load(..., 3),
        // но сейчас просто считаем это ошибкой API.
        send_error_information("Error: Unsupported channel count: " + std::to_string(channels) + "\n");
        throw std::runtime_error("Unsupported channels");
    }

    sync_raw_ptrs();
}

void Image::save() {
  std::ostringstream oss;
  oss << "Writing image (width=" << width << " height=" << height
      << " channels=" << channels << ")\n";
  send_message(oss.str());

  if (!(channels == 1 || channels == 3)) {
    send_error_information("Error: Unsupported channel count for save: " + std::to_string(channels) + "\n");
    throw std::runtime_error("Unsupported channels on save");
  }

  const size_t plane = static_cast<size_t>(width) * height;
  const size_t chunk_sz = plane * static_cast<size_t>(channels);
  std::vector<unsigned char> chunk(chunk_sz);

  if (channels == 3) {
    for (size_t i = 0; i < plane; ++i) {
      chunk[i * 3 + 0] = ch1_[i];
      chunk[i * 3 + 1] = ch2_[i];
      chunk[i * 3 + 2] = ch3_[i];
    }
  } else { // channels == 1
    std::memcpy(chunk.data(), ch1_.data(), plane);
  }

  const std::string full_name = file_name + '.' + extension;
  int ok = 0;
  if (extension == "bmp") {
    ok = stbi_write_bmp(full_name.c_str(), width, height, channels, chunk.data());
  } else if (extension == "tga") {
    ok = stbi_write_tga(full_name.c_str(), width, height, channels, chunk.data());
  } else if (extension == "jpg" || extension == "jpeg") {
    ok = stbi_write_jpg(full_name.c_str(), width, height, channels, chunk.data(), 100);
  } else if (extension == "png") {
    ok = stbi_write_png(full_name.c_str(), width, height, channels, chunk.data(), width * channels);
  } else {
    send_error_information("Error: Non supported extension " + extension + "\n");
    throw std::runtime_error("Unsupported extension");
  }

  if (ok == 0) {
    send_error_information("Error: Failed to write image: " + full_name + "\n");
    throw std::runtime_error("stbi_write_* failed");
  }
}
void Image::get_channel_data(int channel, pixel_value* buffer, int size) {
  // НИКОГДА не освобождаем чужой buffer!
  if (!buffer) {
    send_error_information("Error: get_channel_data null buffer\n");
    throw std::invalid_argument("null buffer");
  }
  if (channel <= 0 || channel > channels) {
    send_error_information("Error: Image channel out of bounds.\n");
    throw std::out_of_range("channel");
  }
  if (width * height != size) {
    send_error_information("Error: Image data size mismatch.\n");
    throw std::invalid_argument("size mismatch");
  }
  const auto n = static_cast<size_t>(size);
  const std::vector<pixel_value>* src = nullptr;
  if (channel == 1) src = &ch1_;
  else if (channel == 2) src = &ch2_;
  else if (channel == 3) src = &ch3_;
  if (!src || src->empty()) {
    send_error_information("Error: Image data was not loaded yet.\n");
    throw std::runtime_error("data not loaded");
  }
  std::memcpy(buffer, src->data(), n);
}
void Image::set_channel_data(int channel, const pixel_value* buffer, int size_channel) {
  if (channel <= 0 || channel > 3) {
    send_error_information("Error: set_channel_data channel out of bounds.\n");
    throw std::out_of_range("channel");
  }
  if (!buffer) {
    send_error_information("Error: set_channel_data null buffer.\n");
    throw std::invalid_argument("null buffer");
  }
  const int plane = width * height;
  if (plane <= 0 || size_channel != plane) {
    send_error_information("Error: set_channel_data size mismatch.\n");
    throw std::invalid_argument("size mismatch");
  }

  auto assign_plane = [&](std::vector<pixel_value>& dst) {
    dst.assign(static_cast<size_t>(size_channel), 0);
    std::memcpy(dst.data(), buffer, static_cast<size_t>(size_channel));
  };

  if (channel == 1) assign_plane(ch1_);
  else if (channel == 2) assign_plane(ch2_);
  else if (channel == 3) assign_plane(ch3_);

  if (channel > channels) channels = channel; // как в исходнике
  original_size = width * height * channels;

  sync_raw_ptrs();
}
void Image::image_setup(const std::string& file_name_in) {
  const size_t last_dot_index = file_name_in.rfind('.');
  file_name = file_name_in.substr(0, last_dot_index);
  extension = file_name_in.substr(last_dot_index + 1);
}
