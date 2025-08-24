//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_CONTROLLER_HPP
#define ARCHIVATOR_CONTROLLER_HPP

#include <controller/IController.hpp>


// порядок аргументов enc/dec -o opt1 .. optN -f file1 .. fileM
// или текстовый файл с такими строками
struct  Controller : public IController {
    explicit Controller(const bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
            : IController(is_text_output, output_file, ref_oss) {}

    void start(const std::string &str);

};

#endif