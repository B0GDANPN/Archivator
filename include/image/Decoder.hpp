#ifndef ARCHIVATOR_DECODER_HPP
#define ARCHIVATOR_DECODER_HPP



#include <memory>
#include <string>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>

class Decoder {
public:

    Decoder(int width, int height, int channels, bool is_text_output, std::string output_file,
            std::ostringstream &ref_oss);

   Decoder(const Decoder&) = delete;
   Decoder& operator=(const Decoder&) = delete;
   Decoder(Decoder&&) noexcept = default;
   Decoder& operator=(Decoder&&) noexcept = delete;
   ~Decoder() = default;


    void decode(const Transforms&transforms);
    std::unique_ptr<Image> make_image(const std::string& file_name, int channel = 0) const;
    Image* get_new_image(const std::string& file_name, int channel = 0) const {
      return make_image(file_name, channel).release();
    }
private:
  bool is_text_output_;
  std::string output_file_;
  std::ostringstream& ref_oss_;

  // Рабочее изображение (буферы каналов внутри — RAII)
  Image img_;

  // Инициализация всех задействованных каналов серым (127)
  void init_grey_channels();

  // Убедиться, что выделены буферы под 1..n каналов (если увеличилось число каналов)
  void ensure_channels(int required_channels);
};

#endif


