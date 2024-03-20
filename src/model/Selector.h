#ifndef ARCHIVATOR_SELECTOR_H
#define ARCHIVATOR_SELECTOR_H
#pragma once

#include <string>
#include <vector>
#include "../dto/Dto.h"

class Selector {
public:
    static AlgorithmEnum getAlgorithmFromDto(const Dto &dto);

private:
    static AlgorithmEnum getAlgorithmFromName(const std::string &name);
};


#endif //ARCHIVATOR_SELECTOR_H
