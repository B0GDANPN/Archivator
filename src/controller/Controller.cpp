#include <controller/Controller.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <controller/Selector.hpp>
#include <controller/Parser.hpp>
#include <image/FractalAlgo.hpp>
#include <video/QuantizationAlgo.hpp>
#include <audio/FlacAlgo.hpp>
#include <huffman/HuffmanAlgo.hpp>


void ::Controller::start(const std::string &str) {
    std::istringstream iss(str);
    std::vector<std::string> argv;
    std::string line;
    while (std::getline(iss, line, '\n')) argv.push_back(line);
    fs::path dir = "storageEncoded";
    if (!fs::exists(dir))
        fs::create_directory(dir);
    dir = "storageDecoded";
    if (!fs::exists(dir))
        fs::create_directory(dir);
    std::vector<std::string> tokens;
    std::string token;
    std::vector<std::string> args_to_parse;
    for (const auto &item: argv) {
        if (fs::exists(item)) {
            if (fs::path(item).extension() == ".txt") {
                std::vector<std::string> tmp = Parser::get_arg_from_txt(item);
                args_to_parse.insert(args_to_parse.end(), tmp.begin(), tmp.end());
            } else {
                args_to_parse.emplace_back(item);
            }
        } else {
            args_to_parse.emplace_back(item);
        }
    }
    std::vector<Dto> args = Parser::parse(args_to_parse);

    for (const auto &arg: args) {
        if (arg.files_.empty()) continue;
        auto algo = Selector::get_algorithm_from_dto(arg);
        switch (algo) {
            case AlgorithmEnum::QUANTIZATION:
                try {
                    QuantizationAlgo quantization_algo{is_text_output, output_file, oss};
                    std::string video_name = arg.files_[0];
                    size_t last_slash_pos = video_name.find_last_of('/');
                    std::string dir_name =
                            last_slash_pos != std::string::npos ? video_name.substr(last_slash_pos + 1) : video_name;
                    size_t pos = dir_name.rfind(".mp4");
                    dir_name = dir_name.substr(0, pos);
                    if (arg.action_) {
                        //encode
                        quantization_algo.encode(video_name);
                    } else {
                        //decode
                        quantization_algo.decode(dir_name);
                    }
                } catch (std::exception) {
                    send_error_information("Error, need correct options: " + Dto::to_string(arg));
                }
                break;
            case AlgorithmEnum::FRACTAL:
                try {
                    FractalAlgo fractal_algo{is_text_output, output_file, oss};
                    std::string arg_name = arg.files_[0];
                    int quality = 600;
                    if (!arg.options_.empty()) quality = stoi(arg.options_[0]);
                    fractal_algo.encode(arg_name, quality);
                } catch (std::exception) {
                    send_error_information("Error, need correct options: " + Dto::to_string(arg));
                }
                break;
            case AlgorithmEnum::FLAC:
                try {
                    FlacAlgo flac_algo{is_text_output, output_file, oss};
                    std::string arg_name = arg.files_[0];
                    if (arg.action_) {
                        //encode
                        flac_algo.encode(arg_name);
                    } else {
                        //decode
                        flac_algo.decode(arg_name);
                    }
                } catch (std::exception) {
                    send_error_information("Error, need correct options: " + Dto::to_string(arg) + '\n');
                }
                break;
            case AlgorithmEnum::HUFFMAN:
                try {
                    HuffmanAlgo huffman_algo{is_text_output, output_file, oss};
                    std::string arg_name = arg.files_[0];
                    if (arg.action_) {
                        //encode
                        huffman_algo.encode(arg_name);
                    } else {
                        //decode
                        huffman_algo.decode(arg_name);
                    }
                } catch (std::exception) {
                    send_error_information("Error, need correct options: " + Dto::to_string(arg) + '\n');
                }
                break;
            case AlgorithmEnum::ERROR:
                send_error_information("Error, need correct options: " + Dto::to_string(arg) + '\n');
                break;
        }
    }
};
