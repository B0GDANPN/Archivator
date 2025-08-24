#ifndef ARCHIVATOR_BITSTEAM_HPP
#define ARCHIVATOR_BITSTEAM_HPP
#include <vector>
#include <cstdint>

struct BitStream {
    std::vector<uint8_t> data;
    size_t bitIndex;

    BitStream() : bitIndex(0) {}

    explicit BitStream(std::vector<uint8_t> data) : data(std::move(data)), bitIndex(0) {}

    void addBit(bool bit);

    void addByte(unsigned char byte);

    bool getBit();

    unsigned char getByte();
};

#endif
