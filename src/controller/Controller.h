//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_CONTROLLER_H
#define ARCHIVATOR_CONTROLLER_H

#include "IController.h"
#include "../dto/CommonInformation.h"
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <iostream>
#include <vector>
#include <filesystem>
#include "../model/Selector.h"
#include "../model/Parser.h"
#include "../../lib/ImagePart/FractalAlgo.h"
#include "../../lib/VideoPart/QuantizationAlgo.h"
#include "../../lib/AudioPart/FlacAlgo.h"
//#include "../view/View.h"
#pragma once

// порядок аргументов enc/dec -o opt1 .. optN -f file1 .. fileM
// или текстовый файл с такими строками
class Controller : public IController {
public:
    explicit Controller(bool isTextOutput, const std::string &outputFile)
            : IController(isTextOutput, outputFile) {
        //this->view = view;
    }

    void run(const std::vector<std::string> &argv) {
        std::vector<std::string> tokens;
        std::string token;
        std::vector<std::string> argsToParse;
        for (const auto &line: argv) {
            if (std::filesystem::exists(line)) {
                if (std::filesystem::path(line).extension() == ".txt") {
                    std::vector<std::string> tmp = Parser::getArgFromTxt(line);
                    argsToParse.insert(argsToParse.end(), tmp.begin(), tmp.end());
                } else {
                    argsToParse.emplace_back(line);
                }
            } else {
                argsToParse.emplace_back(line);
            }
        }
        std::vector<Dto> args = Parser::parse(argsToParse);
        for (const auto &arg: args) {
            if (arg.files_.empty()) continue;
            AlgorithmEnum algo = Selector::getAlgorithmFromDto(arg);
            bool isTextOutput = true;
            std::string outputFile;
            switch (algo) {
                case AlgorithmEnum::QUANTIZATION:
                    try {
                        auto quantizationAlgo = *new QuantizationAlgo(isTextOutput, outputFile);
                        if (arg.action_) {//encode
                            quantizationAlgo.encode(arg.files_[0], arg.files_[1], arg.files_[2], arg.files_[3],
                                                    arg.files_[4]);
                        } else {//decode
                            quantizationAlgo.encode(arg.files_[0], arg.files_[1], arg.files_[2], arg.files_[3],
                                                    arg.files_[4]);
                        }
                        delete &quantizationAlgo;
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg));
                    }
                    break;
                case AlgorithmEnum::FRACTAL:
                    try {
                        auto fractalAlgo = *new FractalAlgo(isTextOutput, outputFile);
                        if (arg.action_) {//encode
                            int quality = stoi(arg.options_[0]);
                            fractalAlgo.encode(arg.files_[0], arg.files_[1], quality);
                        } else {//decode
                            int phases = stoi(arg.options_[0]);
                            fractalAlgo.decode(arg.files_[0], arg.files_[1], phases);
                        }
                        delete &fractalAlgo;
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg));
                    }
                    break;
                case AlgorithmEnum::FLAC:
                    try {
                        auto flacAlgo = *new FlacAlgo(isTextOutput, outputFile);
                        if (arg.action_) {//encode
                            flacAlgo.encode(arg.files_[0], arg.files_[1]);
                        } else {//decode
                            flacAlgo.decode(arg.files_[0], arg.files_[1]);
                        }
                        delete &flacAlgo;
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg) + '\n');
                    }
                    break;
                case AlgorithmEnum::ERROR:
                    try {
                        sendErrorInformation("Error, non support exctension of" + arg.files_[0] + '\n');
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg) + '\n');
                    }
                    break;
            }
        }

    };
    /*void Controller::setGui(const View &view) {
        view=view;
    }*/

};


#endif //ARCHIVATOR_CONTROLLER_H
