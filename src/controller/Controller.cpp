//
// Created by bogdan on 11.03.24.
//
#include <vector>
#include "Controller.h"
#include <filesystem>
#include "../model/Selector.h"
#include "../model/Parser.h"

void Controller::run(int argc, char **argv) {
    std::vector<std::string> tokens;
    std::string token;
    std::vector<std::string> argsTxt;
    for (int i = 0; i < argc; ++i) {
        std::string line(argv[i]);
        if (std::filesystem::exists(line)) {
            if (std::filesystem::path(line).extension() == ".txt")
                argsTxt.push_back(line);
        }
    }
    std::vector<std::string> argsToParse = Parser::getVectorArgvFromTxt(argsTxt);
    std::vector<Dto> args = Parser::parse(argsToParse);
    for (const auto &arg: args) {
        AlgorithmEnum algo = Selector::getAlgorithmFromDto(arg);
    }

}
//непонятно какую я работу проделал, мало сделано в контроллере, парсере селекторе, по сути взял только либы с инета, сделать больше в остальной части
//подправить работу с либами
//Сделать в контроллере что сделал алгоритм и интерфейс для работы с опциями