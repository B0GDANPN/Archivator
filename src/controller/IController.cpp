#include "dto/CommonInformation.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <controller/IController.hpp>


void IController::send_message(const std::string &message) const  {
        if (is_text_output) {
            oss << message;
        } else {
            std::ofstream file(output_file, std::ios::app);

            if (file.is_open()) {
                file << message;
                file.close();
            } else {
                exit(-1);
            }
        }
    };
void IController::send_common_information(const CommonInformation &common_information){
        oss << "Compression ratio: 1:" << common_information.compression_ratio << '\n' <<
            "Time: " << common_information.time << "ms \n" <<
            "Size input data: " << common_information.size_input_data << " bytes\n" <<
            "Size output data: " << common_information.size_output_data << " bytes\n";
        //std::string tmp = oss.str();
        //sendMessage(tmp);
    };
void IController::send_error_information(const std::string &error) {
        send_message(error);
    };
std::streamsize IController::get_filesize(const std::string &filename) {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        if (in) {
            std::streamsize size = in.tellg();
            in.close();
            return size;
        }
        return -1;
    }