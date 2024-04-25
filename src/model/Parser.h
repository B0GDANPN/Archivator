#ifndef ARCHIVATOR_PARSER_H
#define ARCHIVATOR_PARSER_H
#pragma once

#include "../dto/Dto.h"
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

// порядок аргументов enc/dec -o opt1 .. optN -f file1 .. fileM
// или текстовый файл с такими строками
class Parser {
public:
    static std::vector<Dto> parse(const std::vector<std::string> &lines) {
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
            bool action = tokens[0] == "enc";///add exception input
            std::vector<std::string> options;
            std::vector<std::string> files;
            bool collectOptions = false;
            bool collectFiles = false;
            for (int i = 1; i < tokens.size(); i++) {
                if (tokens[i] == "-o") {
                    collectOptions = true;
                    collectFiles = false;
                } else if (tokens[i] == "-f") {
                    collectOptions = false;
                    collectFiles = true;
                } else {
                    if (collectOptions) {
                        options.push_back(tokens[i]);
                    } else if (collectFiles) {
                        files.push_back(tokens[i]);
                    }
                }
            }
            result.emplace_back(action, options, files);
            tokens.clear();
        }
        return result;
    };

    static std::vector<std::string> getArgFromTxt(const std::string &nameOfFile) {
        std::vector<std::string> result;
        std::string line;
        std::ifstream in(nameOfFile); // окрываем файл для чтения
        if (in.is_open()) {
            while (std::getline(in, line))
                result.push_back(line);
        }
        in.close();
        return result;
    };

private:
    static bool isDirectory(const std::string &line) {
        bool isDir = std::filesystem::is_directory(line);
        return isDir;
    };

    static bool isExist(const std::string &line) {
        bool exist = std::filesystem::exists(line);
        return exist;
    };
};
/*
 *  строки ->Parser
 *  для каждой строки выбирает алгоритм парсинга и флаги
 *  возвращает в main массив структур {имя, алгоритм, флаги}
 *
 *
 *
 *
 *
 * */

#endif ARCHIVATOR_PARSER_H
