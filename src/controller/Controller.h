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

#pragma once
namespace fs = std::filesystem;

// порядок аргументов enc/dec -o opt1 .. optN -f file1 .. fileM
// или текстовый файл с такими строками
class Controller : public IController {
public:
    explicit Controller(bool isTextOutput, const std::string &outputFile, std::ostringstream &ref_oss)
            : IController(isTextOutput, outputFile, ref_oss) {}

    //Controller() : IController(true, "") {}

    void start(const std::string &str) {
        fs::path dir = "storageEncoded";
        if (!fs::exists(dir))
            fs::create_directory(dir);
        dir = "storageDecoded";
        if (!fs::exists(dir))
            fs::create_directory(dir);
        fs::path currentPath = fs::current_path();
        fs::path toSaveEncodedPath = currentPath / "storageEncoded";
        fs::path toSaveDecodedPath = currentPath / "storageDecoded";


        std::vector<Dto> args = Parser::parse(str);
        for (const auto &arg: args) {
            if (arg.files_.empty()) continue;
            AlgorithmEnum algo = Selector::getAlgorithmFromDto(arg);
            //bool isTextOutput = true;
            //std::string outputFile;
            fs::current_path(toSaveEncodedPath);
            std::string debug_str=oss.str();
            switch (algo) {
                case AlgorithmEnum::QUANTIZATION:
                    try {
                        QuantizationAlgo quantizationAlgo{isTextOutput, outputFile, oss};
                        std::string videoName = arg.files_[0];
                        size_t pos = videoName.rfind(".mp4");
                        fs::path dirName = videoName.substr(0, pos);
                        if (arg.action_) {//encode
                            fs::create_directory(dirName);
                            fs::current_path(dirName);
                            quantizationAlgo.encode(videoName);
                        } else {//decode
                            fs::current_path(dirName);
                            quantizationAlgo.decode();
                        }
                        fs::current_path("..");
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
                            int quality = stoi(arg.options_[0]);
                            fractalAlgo.encode(argName, quality);
                        } else {//decode
                            int phases = stoi(arg.options_[0]);
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

};


#endif ARCHIVATOR_CONTROLLER_H
