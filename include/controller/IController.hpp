//
// Created by bogdan on 24.04.24.
//

#ifndef ARCHIVATOR_ICONTROLLER_HPP
#define ARCHIVATOR_ICONTROLLER_HPP

#include <dto/CommonInformation.hpp>
#include <string>
#include <fstream>
#include <utility>



class IController {
public:
    virtual ~IController() = default;

    bool is_text_output;
    std::string output_file;
    std::ostringstream &oss;

    explicit IController(const bool is_text_output, std::string output_file, std::ostringstream &ref_oss) : is_text_output(
            is_text_output),
                                                                                                   output_file(std::move(
                                                                                                           output_file)),
                                                                                                   oss(ref_oss) {}

    void send_message(const std::string &message) const;

    virtual void send_common_information(const CommonInformation &common_information);

    virtual void send_error_information(const std::string &error);

    static std::streamsize get_filesize(const std::string &filename);

};

#endif
