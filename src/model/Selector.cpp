//
// Created by bogdan on 11.03.24.
//

#include "Selector.h"
#include <filesystem>

AlgorithmEnum Selector::getAlgorithmFromDto(const Dto &dto) {
    std::string name = dto.name_;
    AlgorithmEnum algorithm = getAlgorithmFromName(name);
    return algorithm;
}

AlgorithmEnum Selector::getAlgorithmFromName(const std::string &name) {
    std::string extension = std::filesystem::path(name).extension();
    if (extension == ".mp4") {
        return AlgorithmEnum::QUANTIZATION;
    } else if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp") {
        return AlgorithmEnum::FRACTAL;
    } else if (extension == ".mp3") {//TODO Другие будут *wav..
        return AlgorithmEnum::FLAC;
    }
    return AlgorithmEnum::ERROR;//mp4
}
