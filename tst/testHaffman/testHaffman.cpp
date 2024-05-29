#include "Haffman.h"
#include <cassert>

void testDecode(std::string source, std::string encoded, std::string decoded) {
    HuffmanAlgo alg;

    alg.encode(source);
    alg.decode(encoded);

    std::ifstream inputFile1(source, std::ios::binary);
    std::ifstream inputFile2(decoded, std::ios::binary);

    std::string buff1;
    std::string buff2;

    while (std::getline(inputFile1, buff1) && std::getline(inputFile2, buff2)) {
        assert(buff1 == buff2);
    }
}

int main() {
    testDecode("example0.wav", "example0.wav.hcf", "exampleDecoded0.wav");
    std::cout << "example0 done" << std::endl;

    testDecode("example1.bmp", "example1.bmp.hcf", "example1Decoded.bmp");
    std::cout << "example1 done" << std::endl;

    //testDecode("example2.wav", "example2.hcf", "example2Decoded.wav");
    //std::cout << "example2 done" << std::endl;
}