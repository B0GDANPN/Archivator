//
// Created by bogdan on 10.04.24.
//

#ifndef ARCHIVATOR_FRACTALALGO_H
#define ARCHIVATOR_FRACTALALGO_H

#include <vector>
#include <string>
#include <fstream>
#include "Image.h"
#include "IFSTransform.h"
#include "Encoder.h"
#include "QuadTreeEncoder.h"
#include "Decoder.h"
#include <chrono>
#include <utility>
#include <filesystem>
#include "../../src/controller/IController.h"
#include "../Huffman/HuffmanAlgo.h"

namespace fs = std::filesystem;

class FractalAlgo : public IController {
    void sendCommonInformation(const CommonInformation &commonInformation) override {
        sendMessage("FractalAlgo{ ");
        IController::sendCommonInformation(commonInformation);
        sendMessage("}\n");
    }

    void sendErrorInformation(const std::string &error) override {
        IController::sendErrorInformation("FractalAlgo{ " + error + "}\n");
    }

    void sendEncodedInformation(int width, int height, int numTransforms) {
        std::stringstream oss;
        oss << "Reading image (width=" << width << " height=" << height << ")\n" <<
            "Number of transforms: " << numTransforms << "\n";
        std::string tmp = oss.str();
        sendMessage(tmp);
    };

    void sendDecodedInformation(int width, int height, int phases) {
        std::stringstream oss;
        oss << "Created image (width=" << width << " height=" << height << ")\n" <<
            "Number of phases: " << phases << "\n";
        std::string tmp = oss.str();
        sendMessage(tmp);
    }

public:
    explicit FractalAlgo(bool isTextOutput, const std::string &outputFile, std::ostringstream &ref_oss)
            : IController(isTextOutput, outputFile, ref_oss) {}


    void encode(const std::string &inputFilename, int quality) {
        auto start = std::chrono::high_resolution_clock::now();
        int sizeInput = static_cast<int>(getFilesize(inputFilename));
        sendMessage("\nEncoding:\n");
        size_t lastSlashPos = inputFilename.find_last_of('/');
        std::string tmpInputFilename =
                lastSlashPos != std::string::npos ? inputFilename.substr(lastSlashPos + 1) : inputFilename;


        size_t pos = tmpInputFilename.rfind('.');
        auto *source = new Image(isTextOutput, outputFile, oss);
        source->ImageSetup(inputFilename);
        auto *enc = new QuadTreeEncoder(isTextOutput, outputFile, oss, quality);
        source->Load();

        int width = source->width;
        int height = source->height;
        Transforms *transforms = enc->Encode(source);
        size_t numTransforms = transforms->ch[0].size() +
                               transforms->ch[1].size() + transforms->ch[2].size();
        sendEncodedInformation(width, height, static_cast<int>(numTransforms));
        auto *dec = new Decoder(width, height, transforms->channels, isTextOutput, outputFile, oss);
        for (int phase = 1; phase <= 5; phase++) {
            dec->Decode(transforms);
        }
        std::string outputFilename = "storageEncoded/" + tmpInputFilename.substr(0, pos) +'.' +source->extension;// путь сохранения
        Image *producer = dec->GetNewImage(outputFilename, 0);
        producer->Save();
        HuffmanAlgo huffmanAlgo{isTextOutput, outputFile, oss};
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        huffmanAlgo.encode(outputFilename,"Fractal",sizeInput,duration);
        remove(outputFilename.c_str());
        delete transforms;
        delete enc;
        delete source;
    };
};


#endif ARCHIVATOR_FRACTALALGO_H