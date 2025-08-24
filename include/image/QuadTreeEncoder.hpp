#ifndef ARCHIVATOR_QTE_HPP
#define ARCHIVATOR_QTE_HPP

#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include <image/Encoder.hpp>

#define BUFFER_SIZE        (32)

class QuadTreeEncoder final : public Encoder {
public:

    explicit QuadTreeEncoder(bool is_text_output, const std::string &output_file,std::ostringstream& ref_oss, int quality = 100)
  : Encoder(is_text_output, output_file,ref_oss),quality_(quality) {}
    ~QuadTreeEncoder() override = default;

    Transforms *encode(Image *source) override;

private:
    void find_matches_for(transform &transforms, int to_x, int to_y, int block_size);
    int quality_;

};

#endif