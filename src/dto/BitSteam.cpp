#include <dto/BitSteam.hpp>

#include <vector>
void ::BitStream::add_bit(bool bit) {
    if (bit_index % 8 == 0) {
        data.push_back(0);
    }
    if (bit) {
        data.back() |= (1 << (7 - bit_index % 8));
    }
    bit_index++;
}
void ::BitStream::add_byte(const unsigned char byte) {
    for (int i = 7; i >= 0; i--){
        this->add_bit((byte >> i) & 1);
    }
}
bool ::BitStream::get_bit() {
    bool bit = (data[bit_index >>3] >> (7 - bit_index & (8-1))) & 1;
    bit_index++;
    return bit;
}
unsigned char ::BitStream::get_byte() {
    unsigned char byte = 0;
    for (int i = 0; i < 8; i++){
        byte = (byte << 1) | this->get_bit();
    }
    return byte;
}