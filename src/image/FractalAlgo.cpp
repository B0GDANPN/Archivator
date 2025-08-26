#include <vector>
#include <string>
#include <fstream>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include <image/QuadTreeEncoder.hpp>
#include <image/Decoder.hpp>
#include <chrono>
#include <controller/IController.hpp>
#include <huffman/HuffmanAlgo.hpp>
#include <image/FractalAlgo.hpp>
namespace fs = std::filesystem;
void ::FractalAlgo::send_error_information(const std::string& error){
  IController::send_error_information("FractalAlgo{ " + error + "}\n");
}
void FractalAlgo::send_common_information(const CommonInformation& common_information){
  send_message("FractalAlgo{ ");
  IController::send_common_information(common_information);
  send_message("}\n");
}


void ::FractalAlgo::send_encoded_information(int width, int height, int num_transforms) const  {
  std::stringstream oss;
  oss << "Reading image (width=" << width << " height=" << height << ")\n" <<
      "Number of transforms: " << num_transforms << "\n";
  std::string tmp = oss.str();
  send_message(tmp);
}
void ::FractalAlgo::send_decoded_information(int width, int height, int phases) const {
  std::stringstream oss;
  oss << "Created image (width=" << width << " height=" << height << ")\n" <<
      "Number of phases: " << phases << "\n";
  std::string tmp = oss.str();
  send_message(tmp);
}
void ::FractalAlgo::encode(const std::string& input_filename, int quality) const {
        auto start = std::chrono::high_resolution_clock::now();
        int size_input = static_cast<int>(get_filesize(input_filename));
        send_message("\nEncoding:\n");
        size_t last_slash_pos = input_filename.find_last_of('/');
        std::string tmp_input_filename =
                last_slash_pos != std::string::npos ? input_filename.substr(last_slash_pos + 1) : input_filename;


        size_t pos = tmp_input_filename.rfind('.');
        auto source = Image{is_text_output, output_file, oss};
        source.image_setup(input_filename);
        auto enc =  QuadTreeEncoder{is_text_output, output_file, oss, quality};
        source.load();

        int width = source.width;
        int height = source.height;
        auto transforms = enc.encode(source);
        size_t num_transforms = transforms->ch[0].size() +
                               transforms->ch[1].size() + transforms->ch[2].size();
        send_encoded_information(width, height, static_cast<int>(num_transforms));
        auto dec = Decoder{width, height, transforms->channels, is_text_output, output_file, oss};
        for (int phase = 1; phase <= 5; phase++) {
            dec.decode(*transforms);
        }
        std::string output_filename = "storageEncoded/" + tmp_input_filename.substr(0, pos) +'.' +source.extension;// путь сохранения
        auto producer = dec.get_new_image(output_filename, 0);
        producer->save();
        HuffmanAlgo huffman_algo{is_text_output, output_file, oss};
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        huffman_algo.encode(output_filename,"Fractal",size_input,duration);
        remove(output_filename.c_str());
    }
