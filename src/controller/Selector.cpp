#include <controller/Selector.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <dto/Dto.hpp>

AlgorithmEnum Selector::get_algorithm_from_dto(const Dto &dto) {
    std::string name = dto.files_[0];
    AlgorithmEnum algorithm = get_algorithm_from_name(name, dto.action_);
    return algorithm;
}

AlgorithmEnum Selector::get_algorithm_from_name(const std::string &name, bool action) {
    std::filesystem::path file_path(name);
    std::string extension = file_path.extension().string();
    if (action) {
        if (extension == ".mp4")
            return AlgorithmEnum::QUANTIZATION;
        if (extension == ".tga" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp")
            return AlgorithmEnum::FRACTAL;
        if (extension == ".wav")
            return AlgorithmEnum::FLAC;
        return AlgorithmEnum::HUFFMAN;
    }
    if (extension.empty())
        return AlgorithmEnum::QUANTIZATION;
    if (extension == ".flac")
        return AlgorithmEnum::FLAC;
    if (extension == ".hcf")
        return AlgorithmEnum::HUFFMAN;
    return AlgorithmEnum::ERROR;
}
