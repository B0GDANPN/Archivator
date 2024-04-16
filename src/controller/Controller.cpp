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
#include "../../lib/AudioPart/FlacAlgo.h"
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
                try {
                    if (arg.action_) {//encode
                        QuantizationAlgo::encode(arg.name_, arg.options_[0]);
                    } else {//decode
                        QuantizationAlgo::decode(arg.name_, arg.options_[0],arg.options_[1],arg.options_[2]);
                    }
                }
                catch (std::exception){
                    sendMesssage("Error, need correct options: " + toStr(arg));
                }
                break;
            case AlgorithmEnum::FRACTAL:
                try {
                    if (arg.action_) {//encode
                        auto it = std::find(arg.options_.begin(), arg.options_.end(), "-t");
                        std::string val = *(it + 1);
                        int num = stoi(val);
                        FractalAlgo::encode(arg.name_, num);
                    } else {//decode
                        auto it = std::find(arg.options_.begin(), arg.options_.end(), "-p");
                        std::string val = *(it + 1);
                        int num = stoi(val);
                        FractalAlgo::encode(arg.name_, num);
                    }
                }
                catch (std::exception){
                    sendMesssage("Error, need correct options: " + toStr(arg));
                }
                break;
            case AlgorithmEnum::FLAC:
                try{
                    if (arg.action_){//encode
                        FlacAlgo::encode(arg.name_,arg.options_[0]);
                    }
                    else{//decode
                        FlacAlgo::decode(arg.name_,arg.options_[0]);
                    }
                }
                catch(std::exception){
                    sendMesssage("Error, need correct options: " + toStr(arg));
                }
                break;
            case AlgorithmEnum::ERROR:
                sendMesssage("Error, non support exctension of" +arg.name_);
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