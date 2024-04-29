#ifndef ARCHIVATOR_SELECTOR_H
#define ARCHIVATOR_SELECTOR_H
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "../dto/Dto.h"

class Selector {
public:
    static AlgorithmEnum getAlgorithmFromDto(const Dto &dto) {
        std::string name = dto.files_[0];
        AlgorithmEnum algorithm = getAlgorithmFromName(name);
        return algorithm;
    };

private:
    static AlgorithmEnum getAlgorithmFromName(const std::string &name){
        std::filesystem::path filePath(name);
        std::string extension = filePath.extension().string();
        if (extension == ".mp4") {
            return AlgorithmEnum::QUANTIZATION;
        } else if (extension == ".tga" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp"|| extension=="json") {
            return AlgorithmEnum::FRACTAL;
        } else if (extension == ".flac" || extension == ".wav") {
            return AlgorithmEnum::FLAC;
        }
        return AlgorithmEnum::ERROR;//mp4
    };
};


#endif ARCHIVATOR_SELECTOR_H
