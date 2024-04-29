//
// Created by bogdan on 15.04.24.
//

#ifndef ARCHIVATOR_FLACALGO_H
#define ARCHIVATOR_FLACALGO_H
#pragma once

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <chrono>
#include "../../src/controller/Controller.h"
#include "../../src/dto/BitSteam.h"
#include "../../src/dto/WAWHeader.h"
#include "../../src/dto/CommonInformation.h"
#include <sstream>

static constexpr int globalSizeBlocks = 16384 * 8; // Size of blocks (INT32_MAX for 1 block)
static constexpr int globalOrder = 10;            // LPC model globalOrder
static constexpr int globalK = 5;                // Rice code parameter
struct Information {
    double compression;
    std::chrono::milliseconds time;
    int globalSizeBlocks, globalOrder, k;
};

// Linear predictive coding

class LPC {
public:
    std::vector<double> coeffs; // Coefficients LPC

    LPC() = default;

    explicit LPC(std::vector<double> coeffs) : coeffs(std::move(coeffs)) {}

    void train(const std::vector<int16_t> &input) {
        int N = static_cast<int>(input.size());
        coeffs.resize(globalOrder + 1, 0);

        std::vector<double> r(globalOrder + 1, 0); // Autocorrelation sequence

        for (int i = 0; i <= globalOrder; ++i) {
            for (int j = 0; j < N - i; ++j) {
                r[i] += input[j] * input[j + i];
            }
        }

        // We perform the Levinson-Durbin method to find the LPC coefficients
        std::vector<double> alpha(globalOrder + 1, 0);
        std::vector<double> kappa(globalOrder + 1, 0);
        std::vector<double> Am1(globalOrder + 1, 0);


        Am1[0] = 1;
        double Em, km = 0;

        Am1[0] = 1;
        alpha[0] = 1.0;
        coeffs[0] = alpha[0];
        double Em1 = r[0];

        for (int m = 1; m <= globalOrder; ++m) {
            double sum = 0;
            for (int j = 1; j <= m - 1; ++j) {
                sum += Am1[j] * r[m - j];
            }
            km = (r[m] - sum) / Em1;
            kappa[m - 1] = -float(km);
            alpha[m] = (float) km;

            for (int j = 1; j <= m - 1; ++j) {
                alpha[j] = Am1[j] - km * Am1[m - j];
            }
            Em = (1 - km * km) * Em1;
            for (int s = 0; s <= globalOrder; ++s) {
                Am1[s] = alpha[s];
            }
            coeffs[m] = alpha[m];
            Em1 = Em;
        }

        for (int s = 0; s <= globalOrder; ++s) {
            coeffs[s] = alpha[s];
        }
    }

    int16_t predict(const std::vector<int16_t> &input, size_t index) {
        double prediction = coeffs[0];
        for (int i = 1; i <= globalOrder; ++i) {
            if (i <= index) {
                prediction += coeffs[i] * input[index - i];
            }
        }
        return (int16_t) prediction;
    }

};

class FlacAlgo : public IController {
    friend class LPC;

public:
    explicit FlacAlgo(bool isTextOutput, const std::string &outputFile)
            : IController(isTextOutput, outputFile) {
        //this->view = view;
    }

    void sendCommonInformation(const CommonInformation &commonInformation) override {
        sendMessage("FlacAlgo{ ");
        IController::sendCommonInformation(commonInformation);
        sendMessage("}\n");
    }

    void sendErrorInformation(const std::string &error) override {
        IController::sendErrorInformation("FlacAlgo{ " + error + "}\n");
    }

    void sendGlobalParams() {
        std::ostringstream oss;
        oss << "FlacAlgo: globalSizeBlocks: " << globalSizeBlocks << ", globalOrder: " << globalOrder << ", k: "
            << globalK << '\n';
        std::string str = oss.str();
        sendMessage(str);
    }

    void encode(const std::string &inputFilename) {
        auto start = std::chrono::high_resolution_clock::now();
        size_t pos = inputFilename.rfind('.');
        std::string outputFilename = inputFilename.substr(0, pos) + ".flac";// путь сохранения
        WAVHeader header{};
        if (!readWAVHeader(inputFilename, header)) {
            sendErrorInformation("Failed to read WAV file.\n");
            exit(-1);
        }
        BitStream stream;
        size_t size = 0;
        for (int i = 0; header.subchunk2Size / sizeof(int16_t) > i * globalSizeBlocks; ++i) {
            std::vector<int16_t> pcmData = readWAVData(inputFilename, header, globalSizeBlocks * i);
            LPC lpc = *new LPC();
            lpc.train(pcmData);

            for (size_t j = 0; j < globalOrder + 1; ++j) {
                int16_t arr[4] = {};
                std::memcpy(arr, &lpc.coeffs[j], sizeof(double));
                riceEncode(stream, arr[0]);
                riceEncode(stream, arr[1]);
                riceEncode(stream, arr[2]);
                riceEncode(stream, arr[3]);
            }

            for (size_t j = 0; j < pcmData.size(); ++j) {
                riceEncode(stream, pcmData[j] - lpc.predict(pcmData, j));
            }
            delete &lpc;
        }
        std::ofstream outputFile(outputFilename, std::ios::binary);
        if (outputFile.is_open()) {
            outputFile.write(reinterpret_cast<const char *>(&header), sizeof(WAVHeader));
            size = stream.data.size();
            outputFile.write(reinterpret_cast<const char *>(&size), sizeof(size_t));
            outputFile.write(reinterpret_cast<const char *>(stream.data.data()), size);
            outputFile.close();
            sendMessage("FLAC data saved to: " + outputFilename + '\n');
        } else {
            sendErrorInformation("Failed to write FLAC file.\n");
            exit(-1);
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto info = CommonInformation(static_cast<int>(header.subchunk2Size / size),
                                      duration.count(), size, header.subchunk2Size);
        sendCommonInformation(info);
        sendGlobalParams();
    }

    void decode(const std::string &inputFilename) {
        auto start = std::chrono::high_resolution_clock::now();
        size_t pos = inputFilename.rfind('.');
        fs::path outputFilename = "../storageDecoded" + inputFilename.substr(0, pos) + ".wav";// путь сохранения
        std::ofstream outputFile(outputFilename, std::ios::binary);
        std::ifstream inputFile(inputFilename, std::ios::binary);

        size_t size;
        WAVHeader header{};
        std::vector<double> lpccoeffs(globalOrder + 1);
        std::vector<uint8_t> flacData;
        if (inputFile.is_open()) {

            inputFile.read(reinterpret_cast<char *>(&header), sizeof(WAVHeader));

            if (outputFile.is_open()) {
                outputFile.write(reinterpret_cast<const char *>(&header), sizeof(WAVHeader));
            } else {
                sendErrorInformation("Failed to write WAV file.\n");
                exit(-1);
            }

            inputFile.read(reinterpret_cast<char *>(&size), sizeof(size_t));
            flacData.reserve(size);
            flacData.resize(size);
            inputFile.read(reinterpret_cast<char *>(flacData.data()), size);
            inputFile.close();

            std::vector<int16_t> data = decodeVector(flacData);

            for (int i = 0; header.subchunk2Size / sizeof(int16_t) > i * globalSizeBlocks; ++i) {
                std::vector<double> lpccoeffs(globalOrder + 1);
                for (size_t j = 0; j < globalOrder + 1; ++j) {
                    double coefficient;
                    std::memcpy(&coefficient, &data[i * (globalSizeBlocks + (globalOrder + 1) * 4) + 4 * j],
                                sizeof(double));
                    lpccoeffs[j] = coefficient;
                }

                LPC *lpc = new LPC(lpccoeffs);

                std::vector<int16_t> pcmData;
                int shift = i * globalSizeBlocks + (i + 1) * 4 * (globalOrder + 1);
                int pcmSize = data.size() - shift >= globalSizeBlocks ? globalSizeBlocks : data.size() - shift;
                pcmData.reserve(pcmSize);
                for (size_t j = 0; j < pcmSize; ++j) {
                    pcmData.push_back(lpc->predict(pcmData, j) + data[j + shift]);
                }
                delete &lpc;
                if (outputFile.is_open()) {
                    outputFile.write(reinterpret_cast<const char *>(pcmData.data()), pcmData.size() * sizeof(int16_t));
                } else {
                    sendErrorInformation("Failed to write file\n");
                    exit(-1);
                }
            }
            sendMessage("WAV data saved to: " + outputFilename.string() + '\n');
            outputFile.close();
        } else {
            sendErrorInformation("Failed to read FLAC file.\n");
            exit(-1);
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto info = CommonInformation(static_cast<int>(header.subchunk2Size / size),
                                      duration.count(), size, header.subchunk2Size);
        sendCommonInformation(info);
        sendGlobalParams();
    }


private:

    static void riceEncode(BitStream &stream, int num) {
        if (num < 0) {
            stream.addBit(true);
            num *= -1;
        } else {
            stream.addBit(false);
        }
        int q = num >> globalK;
        for (int i = 0; i < q; ++i) {
            stream.addBit(true);
        }
        stream.addBit(false);
        for (int i = 0; i < globalK; ++i) {
            stream.addBit((num >> (globalK - 1 - i)) & 1);
        }
    }

    static int riceDecode(BitStream &stream) {
        int sgn = 1;
        if (stream.getBit()) {
            sgn = -1;
        }
        int q = 0;
        while (stream.getBit() == 1) {
            q++;
        }
        int num = q << globalK;
        for (int j = 0; j < globalK; ++j) {
            if (stream.getBit()) {
                num |= (1 << (globalK - 1 - j));
            }
        }
        return sgn * num;
    }

    static std::vector<uint8_t> encodeVector(const std::vector<int16_t> &vec) {
        BitStream stream;
        for (int16_t num: vec) {
            riceEncode(stream, num);
        }
        return stream.data;
    }

    static std::vector<int16_t> decodeVector(std::vector<uint8_t> data) {
        BitStream stream(std::move(data));
        std::vector<int16_t> decoded;
        while (stream.bitIndex < stream.data.size() * 8 - 7) {
            decoded.push_back(riceDecode(stream));
        }
        return decoded;
    }

    bool readWAVHeader(const std::string &filename, WAVHeader &header) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            sendErrorInformation("Failed to open file: " + filename + '\n');
            return false;
        }

        file.read(reinterpret_cast<char *>(&header), sizeof(WAVHeader));

        if (std::string(header.chunkID, 4) != "RIFF" || std::string(header.format, 4) != "WAVE" ||
            header.audioFormat != 1 || header.numChannels != 1) {
            sendErrorInformation("Invalid WAV file format.\n");
            file.close();
            return false;
        }

        if (std::string(header.subchunk2ID) != "data") {
            file.seekg(header.subchunk2Size, std::ios::cur);
            file.read(reinterpret_cast<char *>(&(header.subchunk2ID)), sizeof(header.subchunk2ID));
            file.read(reinterpret_cast<char *>(&(header.subchunk2Size)), sizeof(int32_t));
        }
        file.close();
        return true;
    }

    std::vector<int16_t> readWAVData(const std::string &filename, const WAVHeader &header, int startIndex) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            sendErrorInformation("Failed to open file: " + filename + '\n');
            return {};
        }

        file.seekg(sizeof(WAVHeader), std::ios::beg);
        file.seekg(startIndex * sizeof(int16_t), std::ios::cur);


        if (header.subchunk2Size % sizeof(int16_t) != 0) {
            sendErrorInformation("Invalid data size in WAV file.\n");
            return {};
        }

        std::vector<int16_t> audioData;
        if (header.subchunk2Size / sizeof(int16_t) >= startIndex + globalSizeBlocks) {
            audioData.resize(globalSizeBlocks);
            file.read(reinterpret_cast<char *>(audioData.data()), globalSizeBlocks * sizeof(int16_t));
        } else {
            audioData.resize(header.subchunk2Size / sizeof(int16_t) - startIndex);
            file.read(reinterpret_cast<char *>(audioData.data()), header.subchunk2Size - sizeof(int16_t) * startIndex);
        }

        file.close();
        return audioData;
    }

};

#endif ARCHIVATOR_FLACALGO_H
