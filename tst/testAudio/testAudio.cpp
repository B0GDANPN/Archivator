#include <audio/FlacAlgo.hpp>
#include <cassert>

void test_decode(std::string source, std::string encoded, std::string decoded) {
    FlacAlgo alg;

    alg.encode(source);
    alg.decode(encoded);

    std::ifstream input_file1(source, std::ios::binary);
    std::ifstream input_file2(decoded, std::ios::binary);

    std::string buff1;
    std::string buff2;

    while (std::getline(input_file1, buff1) && std::getline(input_file2, buff2)) {
        assert(buff1 == buff2);
    }
}

int main() {
    test_decode("example0.wav", "example0.flac", "exampleDecoded0.wav");
    std::cout << "example0 done" << std::endl;

    test_decode("example1.wav", "example1.flac", "example1Decoded.wav");
    std::cout << "example1 done" << std::endl;

    test_decode("example2.wav", "example2.flac", "example2Decoded.wav");
    std::cout << "example2 done" << std::endl;
}