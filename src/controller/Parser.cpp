#include <dto/Dto.hpp>
#include <controller/Parser.hpp>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

// порядок аргументов enc/dec -o opt1 .. optN -f file1 .. fileM
// или текстовый файл с такими строками
std::vector<Dto> Parser::parse(const std::vector<std::string> &lines){
    std::istringstream iss;
    std::vector<Dto> result;
    std::vector<std::string> tokens;
    std::string token;
    for (const auto &line: lines) {
        iss.clear();
        iss.str(line);
        while (iss >> token) {
            tokens.push_back(token);
        }
        if (tokens.empty() || (tokens[0] != "enc" && tokens[0] != "dec")) {
            tokens.clear();
            continue;
        }
        bool action = tokens[0] == "enc"; ///add exception input
        std::vector<std::string> options;
        std::vector<std::string> files;
        bool collect_options = false;
        bool collect_files = false;
        for (size_t i = 1; i < tokens.size(); i++) {
            if (tokens[i] == "-o") {
                collect_options = true;
                collect_files = false;
            } else if (tokens[i] == "-f") {
                collect_options = false;
                collect_files = true;
            } else {
                if (collect_options) {
                    options.push_back(tokens[i]);
                } else if (collect_files) {
                    files.push_back(tokens[i]);
                }
            }
        }
        result.emplace_back(action, options, files);
        tokens.clear();
    }
    return result;
}

std::vector<std::string> Parser::get_arg_from_txt(const std::string &name_of_file) {
    std::vector<std::string> result;
    std::ifstream in(name_of_file);
    if (in.is_open()) {
        std::string line;
        while (std::getline(in, line))
            result.push_back(line);
    }
    in.close();
    return result;
}
