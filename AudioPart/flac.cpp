#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>

static constexpr size_t order = 10; // Порядок LPC модели
static constexpr int k = 20; // Параметр кода Райса

struct BitStream {
    std::vector<uint8_t> data;
    int bitIndex;

    BitStream() : bitIndex(0) {}

    BitStream(std::vector<uint8_t> data) : bitIndex(0), data(data) {}

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

struct WAVHeader { 
    char chunkID[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char subchunk2ID[4];
    uint32_t subchunk2Size;
};

bool readWAVHeader(const std::string& filename, WAVHeader& header) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (std::string(header.chunkID, 4) != "RIFF" || std::string(header.format, 4) != "WAVE" || 
            header.audioFormat != 1 || header.numChannels != 1) {
        std::cerr << "Invalid WAV file format." << std::endl;
        file.close();
        return false;
    }

    if (std::string(header.subchunk2ID) != "data") {
        file.seekg(header.subchunk2Size, std::ios::cur);
        file.read(reinterpret_cast<char*>(&(header.subchunk2ID)), sizeof(header.subchunk2ID));
        file.read(reinterpret_cast<char*>(&(header.subchunk2Size)), sizeof(int32_t));
    }
    file.close();

    std::cout << header.subchunk2Size << " " << header.numChannels << "\n";
    std::cout << header.sampleRate << " " << header.byteRate << "\n";
    std::cout << header.blockAlign << " " << header.bitsPerSample << "\n" << std::endl;

    return true;
}

std::vector<int16_t> readWAVData(const std::string& filename, const WAVHeader& header) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return {};
    }

    file.seekg(sizeof(WAVHeader), std::ios::beg);


    if (header.subchunk2Size % sizeof(int16_t) != 0) {
        std::cerr << "Invalid data size in WAV file." << std::endl;
        return {};
    }

    std::vector<int16_t> audioData(header.subchunk2Size / sizeof(int16_t));
    file.read(reinterpret_cast<char*>(audioData.data()), header.subchunk2Size);
    file.close();
    return audioData;
}

class LPC {
public:
    std::vector<double> coeffs; // Коэффициенты LPC

    LPC() {}
    LPC(std::vector<double> coeffs) : coeffs(coeffs) {}

    void train(const std::vector<int16_t>& input) {
        int N = input.size();
        coeffs.resize(order + 1, 0);

        std::vector<double> r(order + 1, 0); // Автокорреляционная последовательность

        for (int i = 0; i <= order; ++i) {
            for (int j = 0; j < N - i; ++j) {
                r[i] += input[j] * input[j + i];
            }
        }

        // Выполняем метод Левинсона-Дурбина для нахождения коэффициентов LPC
        std::vector<double> alpha(order + 1, 0);
        std::vector<double> kappa(order + 1, 0);
        std::vector<double> Am1(order + 1, 0);


        Am1[0] = 1;
        double Em, km = 0;
        
        Am1[0] = 1;
        alpha[0] = 1.0;
        coeffs[0] = alpha[0];
        double Em1 = r[0];

        for (int m = 1; m <= order; ++m) {
            double sum = 0;
            for (int j = 1; j <= m - 1; ++j) {
                sum += Am1[j] * r[m - j];
            }
            km = (r[m] - sum) / Em1;
            kappa[m-1] = -float(km);
            alpha[m] = (float)km;

            for (int j = 1; j <= m - 1; ++j) {
                alpha[j] = Am1[j] - km * Am1[m - j];
            }
            Em = (1 - km * km) * Em1;
            for(int s = 0; s <= order; ++s) {
                Am1[s] = alpha[s];
            }
            coeffs[m] = alpha[m];
            Em1 = Em;
        }

        for(int s = 0; s <= order; ++s) {
            coeffs[s] = alpha[s];
        }
    }

    int16_t predict(const std::vector<int16_t>& input, size_t index) {
        double prediction = coeffs[0];
        for (int i = 1; i <= order; ++i) {
            if (i <= index) {
                prediction += coeffs[i] * input[index - i];
            }
        }
        return (int16_t)prediction;
    }

};

void riceEncode(BitStream& stream, int num) {
    if (num < 0) {
        stream.addBit(1);
        num *= -1;
    } else {
        stream.addBit(0);
    }
    int q = num >> k;
    for (int i = 0; i < q; ++i) {
        stream.addBit(1);
    }
    stream.addBit(0);
    for (int i = 0; i < k; ++i) {
        stream.addBit((num >> (k - 1 - i)) & 1);
    }
}

int riceDecode(BitStream& stream) {
    int sgn = 1;
    if (stream.getBit()) {
        sgn = -1;
    }
    int q = 0;
    while (stream.getBit() == 1) {
        q++;
    }
    int num = q << k;
    for (int j = 0; j < k; ++j) {
        if (stream.getBit()) {
            num |= (1 << (k - 1 - j));
        }
    }
    return sgn * num;
}

std::vector<uint8_t> encodeVector(const std::vector<int16_t>& vec) {
    BitStream stream;
    for (int16_t num : vec) {
        riceEncode(stream, num);
    }
    return stream.data;
}

std::vector<int16_t> decodeVector(std::vector<uint8_t> data) {
    BitStream stream(data);
    std::vector<int16_t> decoded;
    while (stream.bitIndex < stream.data.size() * 8 - 7) {
        decoded.push_back(riceDecode(stream));
    }
    return decoded;
}

std::vector<uint8_t> convertToFLAC(const std::vector<int16_t>& pcmData) {
    LPC lpc;
    lpc.train(pcmData);

    std::vector<int16_t> data;
    data.reserve(pcmData.size() + (order + 1) * 4);


    for (size_t i = 0; i < order + 1; ++i) {
        int16_t arr[4] = {};
        std::memcpy(arr, &lpc.coeffs[i], sizeof(double));
        data.push_back(arr[0]);
        data.push_back(arr[1]);
        data.push_back(arr[2]);
        data.push_back(arr[3]);
    }

    std::cout << pcmData.size() << " "<< std::endl;
    for (size_t i = 0; i < pcmData.size(); ++i) {
        data.push_back(pcmData[i] - lpc.predict(pcmData, i));
    }

    return encodeVector(data);
}

std::vector<int16_t> convertFromFLAC(const std::vector<uint8_t>& flacData) {
    std::vector<int16_t> data = decodeVector(flacData);

    std::vector<double> lpccoeffs;
    lpccoeffs.reserve(order + 1);
    for (size_t i = 0; i < order + 1; ++i) {
        double coefficient;
        std::memcpy(&coefficient, &data[4*i], sizeof(double));
        lpccoeffs.push_back(coefficient);
    }

    LPC lpc(lpccoeffs);

    std::vector<int16_t> pcmData;
    int shift = (order + 1) * 4;
    std::cout << data.size() - shift << " "<< std::endl;
    pcmData.reserve(data.size() - shift);
    for (size_t i = 0; i < data.size() - shift; ++i) {
        pcmData.push_back(lpc.predict(pcmData, i) + data[i + shift]);
    }

    return pcmData;
}

int main() {
    std::string inputFilename = "example.wav";
    std::string outputFilename = "example.flac";
    std::string outputFilename2 = "exampleAfter.wav";

    WAVHeader header;
    std::vector<int16_t> pcmData;
    if (readWAVHeader(inputFilename, header)) {
        pcmData = readWAVData(inputFilename, header);
    } else {
        std::cerr << "Failed to read WAV file." << std::endl;
        return 1;
    }

    //------------------------------------------------------------------------
    std::vector<uint8_t> flacData = convertToFLAC(pcmData);

    std::ofstream outputFile(outputFilename, std::ios::binary);
    if (outputFile.is_open()) {
        outputFile.write(reinterpret_cast<const char*>(flacData.data()), flacData.size());
        outputFile.close();
        std::cout << "FLAC data saved to: " << outputFilename << std::endl;
    } else {
        std::cerr << "Failed to write FLAC file." << std::endl;
        return 1;
    }

    //------------------------------------------------------------------------
    std::vector<int16_t> pcmData2 = convertFromFLAC(flacData);

    std::ofstream outputFile2(outputFilename2, std::ios::binary);
    if (outputFile2.is_open()) {
        outputFile2.write(reinterpret_cast<const char*>(&header), sizeof(header));
        outputFile2.write(reinterpret_cast<const char*>(pcmData2.data()), pcmData2.size() * sizeof(int16_t));
        outputFile2.close();
        std::cout << "FLAC data saved to: " << outputFilename2 << std::endl;
    } else {
        std::cerr << "Failed to write FLAC file." << std::endl;
        return 1;
    }

    return 0;
}
