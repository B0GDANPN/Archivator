#ifndef ARCHIVATOR_DECODER_HPP
#define ARCHIVATOR_DECODER_HPP



#include <string>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>

class Decoder {
    Image img_;
public:

    Decoder(int width, int height, int channels, bool is_text_output, const std::string &output_file,
            std::ostringstream &ref_oss);

    ~Decoder() = default;

    void decode(Transforms *transforms);

    Image* get_new_image(const std::string &file_name, int channel) const;

    bool is_text_output;
    std::string output_file;
    std::ostringstream &ref_oss;
};

#endif


