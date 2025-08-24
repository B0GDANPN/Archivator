//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_COMMONINFORMATION_HPP
#define ARCHIVATOR_COMMONINFORMATION_HPP
#include <cstddef>

struct CommonInformation {
    double compressionRatio;
    size_t time, sizeInputData, sizeOutputData;
};
#endif