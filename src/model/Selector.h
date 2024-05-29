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
        AlgorithmEnum algorithm = getAlgorithmFromName(name, dto.action_);
        return algorithm;
    };

private:
    static AlgorithmEnum getAlgorithmFromName(const std::string &name, bool action) {
        std::filesystem::path filePath(name);
        std::string extension = filePath.extension().string();
        if (action) {
            if (extension == ".mp4") {
                return AlgorithmEnum::QUANTIZATION;
            } else if (extension == ".tga" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp") {
                return AlgorithmEnum::FRACTAL;
            } else if (extension == ".wav") {
                return AlgorithmEnum::FLAC;
            }
            return AlgorithmEnum::HUFFMAN;
        } else {
            if (extension.empty()) {
                return AlgorithmEnum::QUANTIZATION;
            } else if (extension == ".flac") {
                return AlgorithmEnum::FLAC;
            } else if (extension == ".hcf") {
                return AlgorithmEnum::HUFFMAN;
            }
            return AlgorithmEnum::ERROR;
        }

    };
};


#endif ARCHIVATOR_SELECTOR_H
