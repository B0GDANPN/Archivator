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
#include "../model/Selector.h"
#include "../model/Parser.h"
#include "../../lib/ImagePart/FractalAlgo.h"
#include "../../lib/VideoPart/QuantizationAlgo.h"
#include "../../lib/AudioPart/FlacAlgo.h"
#include "../../lib/Huffman/HuffmanAlgo.h"

#pragma once

// порядок аргументов enc/dec -o opt1 .. optN -f file1 .. fileM
// или текстовый файл с такими строками
class Controller : public IController {
public:
    explicit Controller(bool isTextOutput, const std::string &outputFile, std::ostringstream &ref_oss)
            : IController(isTextOutput, outputFile, ref_oss) {}

    void start(const std::string &str) {
        fs::path dir = "storageEncoded";
        if (!fs::exists(dir))
            fs::create_directory(dir);
        dir = "storageDecoded";
        if (!fs::exists(dir))
            fs::create_directory(dir);
        std::vector<std::string> tokens;
        std::string token;
        std::vector<std::string> argsToParse;
        for (const auto &line: argv) {
            if (fs::exists(line)) {
                if (fs::path(line).extension() == ".txt") {
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
            //std::string debug_str = oss.str();
            switch (algo) {
                case AlgorithmEnum::QUANTIZATION:
                    try {
                        QuantizationAlgo quantizationAlgo{isTextOutput, outputFile, oss};
                        std::string videoName = arg.files_[0];
                        size_t lastSlashPos = videoName.find_last_of('/');
                        std::string dirName =
                                lastSlashPos != std::string::npos ? videoName.substr(lastSlashPos + 1) : videoName;
                        size_t pos = dirName.rfind(".mp4");
                        dirName = dirName.substr(0, pos);
                        if (arg.action_) {//encode
                            quantizationAlgo.encode(videoName);
                        } else {//decode
                            quantizationAlgo.decode(dirName);
                        }
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg));
                    }
                    break;
                case AlgorithmEnum::FRACTAL:
                    try {
                        FractalAlgo fractalAlgo{isTextOutput, outputFile, oss};
                        std::string argName = arg.files_[0];
                        if (arg.action_) {//encode
                            int quality = 600;
                            if (!arg.options_.empty()) quality = stoi(arg.options_[0]);
                            fractalAlgo.encode(argName, quality);
                        } else {//decode
                            int phases = 4;
                            if (!arg.options_.empty()) phases = stoi(arg.options_[0]);
                            fractalAlgo.decode(argName, phases);
                        }
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg));
                    }
                    break;
                case AlgorithmEnum::FLAC:
                    try {
                        FlacAlgo flacAlgo{isTextOutput, outputFile, oss};
                        std::string argName = arg.files_[0];
                        if (arg.action_) {//encode
                            flacAlgo.encode(argName);
                        } else {//decode
                            flacAlgo.decode(argName);
                        }
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg) + '\n');
                    }
                    break;
                case AlgorithmEnum::HUFFMAN:
                    try {
                        HuffmanAlgo huffmanAlgo{isTextOutput, outputFile, oss};
                        std::string argName = arg.files_[0];
                        if (arg.action_) {//encode
                            huffmanAlgo.encode(argName);
                        } else {//decode
                            huffmanAlgo.decode(argName);
                        }
                    }
                    catch (std::exception) {
                        sendErrorInformation("Error, need correct options: " + toStr(arg) + '\n');
                    }
                    break;
                case AlgorithmEnum::ERROR:
                    sendErrorInformation("Error, need correct options: " + toStr(arg) + '\n');
                    break;
            }
        }
    };

};

#endif ARCHIVATOR_CONTROLLER_H
