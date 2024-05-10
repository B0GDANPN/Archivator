#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <cstdint>
#include <iomanip>

struct HuffmanNode {
    unsigned char data;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(unsigned char data, int frequency) : data(data), freq(frequency), left(nullptr), right(nullptr) {}
    HuffmanNode(unsigned char data, int frequency, HuffmanNode* left, HuffmanNode* right) : data(data), freq(frequency), left(left), right(right) {}
};

struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

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

void writeTreeToStream(HuffmanNode* root, BitStream& stream) {
    if (root == nullptr)
        return;
    if (root->left == nullptr && root->right == nullptr) {
        stream.addBit(1);
        stream.addByte(root->data);
    } else {
        stream.addBit(0);
        writeTreeToStream(root->left, stream);
        writeTreeToStream(root->right, stream);
    }
}


HuffmanNode* readTreeFromStream(BitStream& stream) {
    bool isLeaf = stream.getBit();
    if (isLeaf) {
        unsigned char data = stream.getByte();
        return new HuffmanNode(data, 0);
    } else {
        HuffmanNode* left = readTreeFromStream(stream);
        HuffmanNode* right = readTreeFromStream(stream);
        return new HuffmanNode('\0', 0, left, right);
    }
}

void generateCodes(HuffmanNode* node, std::string code, std::map<unsigned char, std::string>& codes) {
    if (node == nullptr) return;
    if (node->left == nullptr && node->right == nullptr) {
        codes[node->data] = code;
    }
    generateCodes(node->left, code + "0", codes);
    generateCodes(node->right, code + "1", codes);
};

void encode(const std::string& inputFilename) {
    std::string outputFilename = inputFilename + ".hcf"; //TODO: путь сохранения

    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::string text((std::istreambuf_iterator<char>(inputFile)), (std::istreambuf_iterator<char>()));
    inputFile.close();

    int frequencies[256] = {};
    for (unsigned char c : text) {
        frequencies[c]++;
    }

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] != 0) {
            pq.push(new HuffmanNode(static_cast<unsigned char>(i), frequencies[i]));
        }
    }

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top();
        pq.pop();
        HuffmanNode* right = pq.top();
        pq.pop();
        HuffmanNode* newNode = new HuffmanNode('\0', left->freq + right->freq, left, right);
        pq.push(newNode);
    }

    HuffmanNode* root = pq.top();
    std::map<unsigned char, std::string> codes;

    generateCodes(root, "", codes);

    BitStream bitStream{};
    writeTreeToStream(root, bitStream);

    for (unsigned char c : text) {
        std::string code = codes[c];
        for (unsigned char bit : code) {
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
        //sendMessage("FLAC data saved to: " + outputFilename + '\n');
    } else {
        //sendErrorInformation("Failed to write FLAC file.\n");
        exit(-1);
    }
}


void decode(const std::string& inputFilename) {
    size_t pos = inputFilename.rfind(".hcf");
    std::string outputFilename = inputFilename.substr(0, pos) + ".dec"; //TODO: путь сохранения
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

    HuffmanNode* root = readTreeFromStream(bitStream);
    HuffmanNode* current = root;

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
    }
    outFile.close();
}

int main() {
    std::string inputFilename = "input.bmp";

    encode(inputFilename);

    decode(inputFilename + ".hcf");

    return 0;
}
