//
// Created by bogdan on 11.03.24.
//
#include <vector>
#include "Controller.h"
#include <filesystem>
#include <iostream>
#include "../model/Selector.h"
#include "../model/Parser.h"
#include "../../lib/ImagePart/FractalAlgo.h"
#include "../../lib/VideoPart/QuantizationAlgo.h"
#include "../../lib/ImagePart/FlacAlgo.h"
void Controller::run(int argc, char **argv) {
    std::vector<std::string> tokens;
    std::string token;
    std::vector<std::string> argsTxt;
    bool wasTxt= false;
    for (int i = 0; i < argc; ++i) {
        std::string line(argv[i]);
        if (std::filesystem::exists(line))
            if (std::filesystem::path(line).extension() == ".txt") {
                argsTxt.push_back(line);
                wasTxt = true;
            }

    }
    std::vector<std::string> argsToParse = Parser::getVectorArgvFromTxt(argsTxt);
    if (!wasTxt){
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            oss << argv[i]<<' ';
        }
        std::string argCL = oss.str();
        argsToParse.push_back(argCL);
    }
    std::vector<Dto> args = Parser::parse(argsToParse);
    for (const auto &arg: args) {
        AlgorithmEnum algo = Selector::getAlgorithmFromDto(arg);
        switch (algo) {
            case AlgorithmEnum::QUANTIZATION:
                if (arg.options_.empty() || arg.options_[0]=="default"){
                    std::ostringstream oss;
                    oss<<"Error, need correct options: "<<arg.t
                    argsToParse.push_back(oss.str());
                }
                break;
            case AlgorithmEnum::FRACTAL:
                break;
            case AlgorithmEnum::FLAC:
                break;
            case AlgorithmEnum::ERROR:

                break;
        }
    }

}

void Controller::sendMesssage(const std::string &message) {
    std::cout << message;

}
//непонятно какую я работу проделал, мало сделано в контроллере, парсере селекторе, по сути взял только либы с инета, сделать больше в остальной части
//подправить работу с либами
//Сделать в контроллере что сделал алгоритм и интерфейс для работы с опциями