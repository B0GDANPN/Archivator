#ifndef ARCHIVATOR_SELECTOR_HPP
#define ARCHIVATOR_SELECTOR_HPP


#include <string>
#include <filesystem>
#include <dto/Dto.hpp>
#include <dto/AlgorithmEnum.hpp>

class Selector {
public:
    static AlgorithmEnum get_algorithm_from_dto(const Dto &dto);

private:
    static AlgorithmEnum get_algorithm_from_name(const std::string &name, bool action);
};


#endif