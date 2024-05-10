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

    void addByte(unsigned char byte) {
        for (int i = 7; i >= 0; i--){
            this->addBit((byte >> i) & 1);
        }
    }

    bool getBit() {
        bool bit = (data[bitIndex / 8] >> (7 - bitIndex % 8)) & 1;
        bitIndex++;
        return bit;
    }

    unsigned char getByte() {
        unsigned char byte = 0;
        for (int i = 0; i < 8; i++){
            byte = (byte << 1) | this->getBit();
        }
        return byte;
    }
};

#endif ARCHIVATOR_BITSTEAM_H
