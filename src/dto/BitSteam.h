//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_BITSTEAM_H
#define ARCHIVATOR_BITSTEAM_H
#pragma once

#include <vector>
#include <cstdint>

struct BitStream {
    std::vector<uint8_t> data;
    int bitIndex;

    BitStream() : bitIndex(0) {}

    explicit BitStream(std::vector<uint8_t> data) : bitIndex(0), data(std::move(data)) {}

    void addBit(bool bit) {
        if (bitIndex % 8 == 0) {
            data.push_back(0);
        }
        if (bit) {
            data.back() |= (1 << (7 - bitIndex % 8));
        }
        bitIndex++;
    }

    bool getBit() {
        bool bit = (data[bitIndex / 8] >> (7 - bitIndex % 8)) & 1;
        bitIndex++;
        return bit;
    }
};

#endif ARCHIVATOR_BITSTEAM_H
