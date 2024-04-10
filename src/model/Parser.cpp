//
// Created by bogdan on 11.03.24.
//

#include "Parser.h"
#include <filesystem>
#include <fstream>
#include "../controller/Controller.h"
std::vector<Dto> Parser::parse(const std::vector<std::string> &lines) {
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
        std::string nameOfFileOrDirectory = tokens[0];
        if (!isExist(nameOfFileOrDirectory)) {
            
            Controller::sendMesssage("File or directory doesn't exist\n");
            continue;
        }
        bool action = tokens[1] == "encode";///add exception input
        std::vector<std::string> option;
        if (!isDirectory(nameOfFileOrDirectory)) {
            if (tokens.size() > 2) {
                for (int j = 2; j < tokens.size(); j++) {
                    option.push_back(tokens[j]);
                }
            }
            if (option.empty()) {
                option.emplace_back("default");
            }
        } else {
            option = {"default"};
        }
        result.emplace_back(nameOfFileOrDirectory, action, option);
        tokens.clear();
    }
    return result;
}

bool Parser::isDirectory(const std::string &line) {
    bool isDir = std::filesystem::is_directory(line);
    return isDir;
}

bool Parser::isExist(const std::string &line) {
    bool exist = std::filesystem::exists(line);
    return exist;
}

std::vector<std::string> Parser::getVectorArgvFromTxt(const std::vector<std::string> &namesOfFiles) {
    std::vector<std::string> result;
    std::string line;
    for (const auto &nameOfFile: namesOfFiles) {
        std::ifstream in(nameOfFile); // окрываем файл для чтения
        if (in.is_open()) {
            while (std::getline(in, line))
                result.push_back(line);
        }
        in.close();
    }
    return result;
}
