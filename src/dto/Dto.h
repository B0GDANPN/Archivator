//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_DTO_H
#define ARCHIVATOR_DTO_H
#pragma once

#include <string>
#include "AlgorithmEnum.h"
#include <vector>
#include <sstream>

struct Dto {
    std::string name_;
    bool action_;
    std::vector<std::string> options_;

    explicit Dto(std::string name, bool action, std::vector<std::string> options);
};

std::string toStr(const Dto &dto) {
    std::ostringstream oss;
    oss << dto.name_ << " action:" << dto.action_ << ' ';
    for (const auto& option: dto.options_)
        oss << option << ' ';
    return oss.str();
}

#endif //ARCHIVATOR_DTO_H
