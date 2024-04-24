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
    bool wasTxt = false;
    for (int i = 0; i < argc; ++i) {
        std::string line(argv[i]);
        if (std::filesystem::exists(line))
            if (std::filesystem::path(line).extension() == ".txt") {
                argsTxt.push_back(line);
                wasTxt = true;
            }

    }
    std::vector<std::string> argsToParse = Parser::getVectorArgvFromTxt(argsTxt);
    if (!wasTxt) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            oss << argv[i] << ' ';
        }
        std::string argCL = oss.str();
        argsToParse.push_back(argCL);
    }
    std::vector<Dto> args = Parser::parse(argsToParse);
    for (const auto &arg: args) {
        AlgorithmEnum algo = Selector::getAlgorithmFromDto(arg);
        bool isTextOutput = true;
        std::string outputFile;
        switch (algo) {
            case AlgorithmEnum::QUANTIZATION:
                try {
                    if (arg.action_) {//encode
                        QuantizationAlgo::encode(arg.name_, arg.options_[0], arg.options_[1], arg.options_[2],
                                                 arg.options_[3]);
                    } else {//decode
                        QuantizationAlgo::decode(arg.name_, arg.options_[0], arg.options_[1], arg.options_[2]);
                    }
                }
                catch (std::exception) {
                    sendErrorInformation("Error, need correct options: " + toStr(arg));
                }
                break;
            case AlgorithmEnum::FRACTAL:
                auto fractalAlgo = *new FractalAlgo(isTextOutput, outputFile);
                try {
                    if (arg.action_) {//encode
                        auto it = std::find(arg.options_.begin(), arg.options_.end(), "-t");
                        std::string val = *(it + 1);
                        int num = stoi(val);
                        fractalAlgo.encode(arg.name_, num);
                    } else {//decode
                        auto it = std::find(arg.options_.begin(), arg.options_.end(), "-p");
                        std::string val = *(it + 1);
                        int num = stoi(val);
                        fractalAlgo.encode(arg.name_, num);
                    }
                }
                catch (std::exception) {
                    sendErrorInformation("Error, need correct options: " + toStr(arg));
                }
                delete &fractalAlgo;
                break;
            case AlgorithmEnum::FLAC:
                auto flacAlgo = *new FlacAlgo(isTextOutput, outputFile);
                try {
                    if (arg.action_) {//encode
                        flacAlgo.encode(arg.name_, arg.options_[0]);
                    } else {//decode
                        flacAlgo.decode(arg.name_, arg.options_[0]);
                    }
                }
                catch (std::exception) {
                    sendErrorInformation("Error, need correct options: " + toStr(arg) + '\n');
                }
                delete &fractalAlgo;
                break;
            case AlgorithmEnum::ERROR:
                sendErrorInformation("Error, non support exctension of" + arg.name_ + '\n');
                break;
        }
    }

}
/*void Controller::setGui(const View &view) {
    view=view;
}*/
//непонятно какую я работу проделал, мало сделано в контроллере, парсере селекторе, по сути взял только либы с инета, сделать больше в остальной части
//подправить работу с либами
//Сделать в контроллере что сделал алгоритм и интерфейс для работы с опциями