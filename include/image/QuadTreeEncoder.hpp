// QuadTreeEncoder.hpp
#ifndef ARCHIVATOR_QTE_HPP
#define ARCHIVATOR_QTE_HPP

#include <memory>
#include <string>
#include <sstream>

#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include <image/Encoder.hpp>

#define BUFFER_SIZE (32)

class QuadTreeEncoder final : public Encoder {
public:
  explicit QuadTreeEncoder(bool is_text_output,
                           const std::string& output_file,
                           std::ostringstream& ref_oss,
                           int quality = 100)
      : Encoder(is_text_output, output_file, ref_oss)
      , quality_(quality) {}

  ~QuadTreeEncoder()  override = default;

  std::unique_ptr<Transforms> encode(const Image& source)override;

private:
  // Вся необходимая информация передаётся параметрами — без обращения к «сырым» полям img.*
  void find_matches_for(transform& out,
                        int to_x, int to_y,           // координаты range-блока
                        int block_size,               // N x N
                        const pixel_value* range_plane, int range_stride,
                        const pixel_value* down_plane,  int down_stride,
                        int image_height);            // для корректного перебора доменов

  int quality_;
};

#endif // ARCHIVATOR_QTE_HPP
