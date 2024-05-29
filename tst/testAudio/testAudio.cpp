#include "FlacAlgo.h"
#include <cassert>

void testDecode(std::string source, std::string encoded, std::string decoded) {
    FlacAlgo alg;

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
    testDecode("example0.wav", "example0.flac", "exampleDecoded0.wav");
    std::cout << "example0 done" << std::endl;

    testDecode("example1.wav", "example1.flac", "example1Decoded.wav");
    std::cout << "example1 done" << std::endl;

    testDecode("example2.wav", "example2.flac", "example2Decoded.wav");
    std::cout << "example2 done" << std::endl;
}