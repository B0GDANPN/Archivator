#ifndef ARCHIVATOR_PARSER_HPP
#define ARCHIVATOR_PARSER_HPP

#include <vector>
#include <string>
#include <dto/Dto.hpp>

// порядок аргументов enc/dec -o opt1 .. optN -f file1 .. fileM
// или текстовый файл с такими строками
class Parser {
public:
    static std::vector<Dto> parse(const std::vector<std::string> &lines);

    static std::vector<std::string> get_arg_from_txt(const std::string &name_of_file);

};

#endif