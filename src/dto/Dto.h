//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_DTO_H
#define ARCHIVATOR_DTO_H
#pragma once

#include <string>
#include "AlgorithmEnum.h"
#include <vector>

struct Dto {
    std::string name_;
    bool action_;
    std::vector<std::string> options_;

    explicit Dto(std::string name, bool action, std::vector<std::string> options);
};
std::string toStr(const Dto& dto){
    dto.options_.
}
#endif //ARCHIVATOR_DTO_H
