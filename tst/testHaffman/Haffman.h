#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <cstdint>
#include <iomanip>
#include <filesystem>
#include "BitSteam.h"

class HuffmanAlgo {
private:
    struct HuffmanNode {
        int freq;
        unsigned char data;
        HuffmanNode *left;
        HuffmanNode *right;

        explicit HuffmanNode(unsigned char data, int frequency) : data(data), freq(frequency), left(nullptr),
                                                                  right(nullptr) {}

        explicit HuffmanNode(unsigned char data, int frequency, HuffmanNode *left, HuffmanNode *right) : data(data),
                                                                                                         freq(frequency),
                                                                                                         left(left),
                                                                                                         right(right) {}

        ~HuffmanNode() {
            delete left;
            delete right;
        }

        bool operator()(HuffmanNode *a, HuffmanNode *b) {
            return a->freq > b->freq;
        }

    };

    void writeTreeToStream(HuffmanNode *root, BitStream &stream) {
        if (root == nullptr)
            return;
        if (root->left == nullptr && root->right == nullptr) {
            stream.addBit(true);
            stream.addByte(root->data);
        } else {
            stream.addBit(false);
            writeTreeToStream(root->left, stream);
            writeTreeToStream(root->right, stream);
        }
    }

    HuffmanNode *readTreeFromStream(BitStream &stream) {
        bool isLeaf = stream.getBit();
        if (isLeaf) {
            unsigned char data = stream.getByte();
            return new HuffmanNode(data, 0);
        } else {
            HuffmanNode *left = readTreeFromStream(stream);
            HuffmanNode *right = readTreeFromStream(stream);
            return new HuffmanNode('\0', 0, left, right);
        }
    }

    void generateCodes(HuffmanNode *node, const std::string &code, std::map<unsigned char, std::string> &codes) {
        if (node == nullptr) return;
        if (node->left == nullptr && node->right == nullptr) {
            codes[node->data] = code;
        }
        generateCodes(node->left, code + "0", codes);
        generateCodes(node->right, code + "1", codes);
    }

public:
    explicit HuffmanAlgo(){}

    void encode(const std::string &inputFilename) {
        size_t lastSlashPos = inputFilename.find_last_of('/');
        std::string tmpInputFilename =
                lastSlashPos != std::string::npos ? inputFilename.substr(lastSlashPos + 1) : inputFilename;
        std::string outputFilename = tmpInputFilename + ".hcf";// путь сохранения

        std::ifstream inputFile(inputFilename, std::ios::binary);
        std::string text((std::istreambuf_iterator<char>(inputFile)), (std::istreambuf_iterator<char>()));
        inputFile.close();

        int frequencies[256] = {};
        for (unsigned char c: text) {
            frequencies[c]++;
        }
        auto comp = [](HuffmanNode *a, HuffmanNode *b) { return a->freq > b->freq; };
        std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, decltype(comp)> pq(comp);
        for (int i = 0; i < 256; i++) {
            if (frequencies[i] != 0) {
                pq.push(new HuffmanNode(static_cast<unsigned char>(i), frequencies[i]));
            }
        }

        while (pq.size() > 1) {
            HuffmanNode *left = pq.top();
            pq.pop();
            HuffmanNode *right = pq.top();
            pq.pop();
            auto *newNode = new HuffmanNode('\0', left->freq + right->freq, left, right);
            pq.push(newNode);
        }

        HuffmanNode *root = pq.top();
        std::map<unsigned char, std::string> codes;

        generateCodes(root, "", codes);

        BitStream bitStream{};
        writeTreeToStream(root, bitStream);
        delete root;

        for (unsigned char c: text) {
            std::string code = codes[c];
            for (unsigned char bit: code) {
                bitStream.addBit(bit - static_cast<unsigned char>('0'));
            }
        }

        std::ofstream outputFile(outputFilename, std::ios::binary);
        if (outputFile.is_open()) {
            size_t size = bitStream.data.size();
            outputFile.write(reinterpret_cast<const char *>(&size), sizeof(size_t));
            uint8_t a = size * 8 - bitStream.bitIndex;
            outputFile.write(reinterpret_cast<const char *>(&a), sizeof(uint8_t));
            outputFile.write(reinterpret_cast<const char *>(bitStream.data.data()), size);
            outputFile.close();
        } else {
            exit(-1);
        }
    }


    void decode(const std::string &inputFilename) {
        size_t lastSlashPos = inputFilename.find_last_of('/');
        std::string tmpInputFilename =
                lastSlashPos != std::string::npos ? inputFilename.substr(lastSlashPos + 1) : inputFilename;
        size_t pos = tmpInputFilename.rfind(".hcf");
        std::string outputFilename = tmpInputFilename.substr(0, pos-4) + "Decoded" + tmpInputFilename.substr(pos-4, pos);// путь сохранения

        std::ofstream outFile(outputFilename, std::ios::binary);

        std::ifstream inputFile(inputFilename, std::ios::binary);

        size_t size;
        inputFile.read(reinterpret_cast<char *>(&size), sizeof(size_t));

        uint8_t maxIdx = 0;
        inputFile.read(reinterpret_cast<char *>(&maxIdx), sizeof(uint8_t));

        std::vector<uint8_t> data(size);
        inputFile.read(reinterpret_cast<char *>(data.data()), size);

        inputFile.close();

        BitStream bitStream(data);

        HuffmanNode *root = readTreeFromStream(bitStream);
        HuffmanNode *current = root;

        while (bitStream.bitIndex < bitStream.data.size() * 8 - maxIdx) {
            if (bitStream.getBit()) {
                current = current->right;
            } else {
                current = current->left;
            }
            if (current->left == nullptr && current->right == nullptr) {
                outFile << current->data;
                current = root;
            }
        };
        delete root;
        outFile.close();
    }
};