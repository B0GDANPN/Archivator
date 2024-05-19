//
// Created by bogdan on 10.04.24.
//

#ifndef TESTFRACTAL_FRACTALALGO_H
#define TESTFRACTAL_FRACTALALGO_H

#include <vector>
#include <string>
#include "json.hpp"
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

using json = nlohmann::json;
namespace fs = std::filesystem;

class FractalAlgo : public IController {
public:
    explicit FractalAlgo(bool isTextOutput, const std::string &outputFile, std::ostringstream &ref_oss)
            : IController(isTextOutput, outputFile, ref_oss) {}

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

    void encode(const std::string &fileName, int quality) {
        sendMessage("\nEncoding:\n");
        std::string debug_str=oss.str();
        size_t lastSlashPos = fileName.find_last_of('/');
        std::string tmp = lastSlashPos != std::string::npos ? fileName.substr(lastSlashPos + 1) : fileName;


        size_t pos = tmp.rfind('.');
        std::string outputFileName = tmp.substr(0, pos) + ".json";
        auto *source = new Image(isTextOutput, outputFile, oss);
        source->ImageSetup(fileName);
        auto *enc = new QuadTreeEncoder(isTextOutput, outputFile, oss, quality);
        source->Load();

        int width = source->width;
        int height = source->height;
        auto start = std::chrono::high_resolution_clock::now();
        Transforms *transforms = enc->Encode(source);
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

        SaveTransformsToJson(transforms, outputFileName, source->extension, width, height);
        size_t numTransforms = transforms->ch[0].size() +
                               transforms->ch[1].size() + transforms->ch[2].size();
        int ratio = width * height * 3 / transforms->getSize();
        CommonInformation information = {static_cast<int>(ratio),
                                         static_cast<int>(duration.count()),
                                         width * height * 3, transforms->getSize()};
        debug_str=oss.str();
        sendCommonInformation(information);
        debug_str=oss.str();
        sendEncodedInformation(width, height, numTransforms);
        debug_str=oss.str();
        delete transforms;
        delete enc;
        delete source;
    };

    void decode(const std::string &imageName, int phases) {
        sendMessage("\nDecoding:\n");
        size_t pos = imageName.rfind('.');
        std::string fileName = imageName.substr(0, pos) + ".json";
        int width, height;
        std::string extension;
        Transforms *transforms2;
        std::tie(transforms2, extension) = LoadTransformsFromJson(fileName, &width, &height);
        auto start = std::chrono::high_resolution_clock::now();
        auto *dec = new Decoder(width, height, transforms2->channels, isTextOutput, outputFile, oss);
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

        for (int phase = 1; phase <= phases; phase++) {
            dec->Decode(transforms2);
        }
        //fs::path outputImagePath = "../storageDecoded/" + imageName;
        fs::path outputImagePath = /*"../storageDecoded/" +*/ imageName;// путь сохранения для теста картинок
        Image *producer = dec->GetNewImage(outputImagePath, 0);
        producer->Save();
        int ratio = width * height * producer->channels / transforms2->getSize();
        CommonInformation information = {ratio, static_cast<int>(duration.count()), transforms2->getSize(),
                                         width * height * producer->channels};
        sendCommonInformation(information);
        sendDecodedInformation(width, height, phases);
        delete producer;
        delete dec;
    };
private:
    static void
    SaveTransformsToJson(const Transforms *transforms, const std::string &filePath, const std::string &extension,
                         int width, int height) {
        json jsonTransforms;
        jsonTransforms["extension"] = extension;
        jsonTransforms["width"] = width;
        jsonTransforms["height"] = height;
        jsonTransforms["channels"] = transforms->channels;
        // Serialize the transforms for each channel.
        for (int i = 0; i < transforms->channels; ++i) {
            json jsonChannel;
            for (const IFSTransform *transform: transforms->ch[i]) {
                json jsonTransform;
                jsonTransform["fromX"] = transform->getFromX();
                jsonTransform["fromY"] = transform->getFromY();
                jsonTransform["toX"] = transform->getToX();
                jsonTransform["toY"] = transform->getToY();
                jsonTransform["size"] = transform->getSize();
                jsonTransform["symmetry"] = transform->getSymmetry();
                jsonTransform["scale"] = transform->getScale();
                jsonTransform["offset"] = transform->getOffset();
                jsonChannel.push_back(jsonTransform);
            }
            jsonTransforms["channel" + std::to_string(i)] = jsonChannel;
        }
        std::ofstream outFile(filePath);
        outFile << jsonTransforms.dump(4);
        outFile.close();
    }

    std::pair<Transforms *, std::string> LoadTransformsFromJson(const std::string &filePath, int *width, int *height) {
        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            sendErrorInformation("Error: Unable to open file for reading: " + filePath + '\n');
            exit(-1);
        }
        json jsonTransforms;
        inFile >> jsonTransforms;
        inFile.close();
        *width = jsonTransforms["width"];
        *height = jsonTransforms["height"];
        // Create the Transforms object from the parsed JSON.
        auto *transforms = new Transforms();
        transforms->channels = jsonTransforms["channels"];

        for (int i = 0; i < transforms->channels; ++i) {
            json jsonChannel = jsonTransforms["channel" + std::to_string(i)];
            for (const auto &jsonTransform: jsonChannel) {
                int fromX = jsonTransform["fromX"];
                int fromY = jsonTransform["fromY"];
                int toX = jsonTransform["toX"];
                int toY = jsonTransform["toY"];
                int size = jsonTransform["size"];
                IFSTransform::SYM symmetry = jsonTransform["symmetry"];
                double scale = jsonTransform["scale"];
                int offset = jsonTransform["offset"];
                auto *transform = new IFSTransform(
                        fromX, fromY, toX, toY,
                        size, symmetry, scale,
                        offset
                );
                transforms->ch[i].push_back(transform);
            }
        }
        return std::make_pair(transforms, jsonTransforms["extension"]);
    }
};


#endif TESTFRACTAL_FRACTALALGO_H