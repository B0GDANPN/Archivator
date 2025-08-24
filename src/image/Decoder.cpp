
#include <string>
#include <vector>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include <image/Decoder.hpp>
 ::Decoder::Decoder(int width, int height, int channels, bool is_text_output, const std::string& output_file, std::ostringstream& ref_oss): img_(is_text_output, output_file, ref_oss),
                                           is_text_output(is_text_output), output_file(output_file),ref_oss(ref_oss) {
        img_.channels = channels;
        img_.width = width;
        img_.height = height;
        img_.original_size = width * height * channels;
        img_.image_data1 = new pixel_value[width * height];
        img_.image_data2 = new pixel_value[width * height];
        img_.image_data3 = new pixel_value[width * height];
      // Initialize to grey image
        std::fill_n(img_.image_data1,width * height,127);
        std::fill_n(img_.image_data2,width * height,127);
        std::fill_n(img_.image_data3,width * height,127);
    }
void Decoder::decode(Transforms* transforms) {

        img_.channels = transforms->channels;

        for (int channel = 1; channel <= img_.channels; channel++) {
            pixel_value *orig_image = img_.image_data1;
            if (channel == 2)
                orig_image = img_.image_data2;
            else if (channel == 3)
                orig_image = img_.image_data3;

            // Apple each transform at a time to this channel
            transform::iterator iter = transforms->ch[channel - 1].begin();
            for (; iter != transforms->ch[channel - 1].end(); ++iter) {
                iter[0]->execute(orig_image, img_.width, orig_image, img_.width, false);
            }
        }
    }
Image* Decoder::get_new_image(const std::string& file_name, int channel) const {
        auto temp = new Image(is_text_output, output_file, ref_oss);
        temp->image_setup(file_name);
        temp->channels = img_.channels;
        temp->width = img_.width;
        temp->height = img_.height;
        temp->original_size = img_.original_size;
        int size_channel = img_.width * img_.height;
        // Get according to channel number or all channels if number is zero
        if (img_.channels >= 1 && (!channel || channel == 1))
            temp->set_channel_data(1, img_.image_data1, size_channel);
        if (img_.channels >= 2 && (!channel || channel == 2))
            temp->set_channel_data((!channel ? 2 : 1), img_.image_data2, size_channel);
        if (img_.channels >= 3 && (!channel || channel == 3))
            temp->set_channel_data((!channel ? 3 : 1), img_.image_data3, size_channel);
        return temp;
    }