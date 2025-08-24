#include <dto/BitSteam.hpp>

#include <vector>
void ::BitStream::addBit(bool bit) {
    if (bitIndex % 8 == 0) {
        data.push_back(0);
    }
    if (bit) {
        data.back() |= (1 << (7 - bitIndex % 8));
    }
    bitIndex++;
}
void ::BitStream::addByte(const unsigned char byte) {
    for (int i = 7; i >= 0; i--){
        this->addBit((byte >> i) & 1);
    }
}
bool ::BitStream::getBit() {
    bool bit = (data[bitIndex >>3] >> (7 - bitIndex & (8-1))) & 1;
    bitIndex++;
    return bit;
}
unsigned char ::BitStream::getByte() {
    unsigned char byte = 0;
    for (int i = 0; i < 8; i++){
        byte = (byte << 1) | this->getBit();
    }
    return byte;
}