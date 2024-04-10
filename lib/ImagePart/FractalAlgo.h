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

using json = nlohmann::json;

class FractalAlgo {
public:
    static void encode(const std::string& fileName,int quality) {
        /*std::string fileName;
        int quality = 100;
        bool usage = true;
        for (int i = 1; i < argc && usage; i++) {
            std::string param(argv[i]);

            if (param == "-t" && i + 1 < argc)
                quality = std::stoi(argv[i + 1]);
            if (param.at(0) == '-') {
                i++;
            } else {
                fileName = param;
                usage = false;
            }
        }

        if (usage) {
            std::ostringstream oss;
            oss << "Usage: %s [-t #] filename for encoding\n"
                   "\t-t 100  quality (i.e. quality) for encoding\n"<<argv[0]<<'\n';
            std::string str = oss.str();
            Controller::sendMesssage(str);
            exit(-1);
        }*/
        auto *source = new Image();
        source->ImageSetup(fileName);
        auto *enc = new QuadTreeEncoder(quality);
        source->Load();

        int width = source->width;
        int height = source->height;
        auto start = std::chrono::high_resolution_clock::now();
        Transforms *transforms = enc->Encode(source);
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

        std::string encodedName = source->fileName + "_encoded.json";
        SaveTransformsToJson(transforms, encodedName, source->extension, width, height);
        size_t numTransforms = transforms->ch[0].size() +
                            transforms->ch[1].size() + transforms->ch[2].size();
        size_t transformSize = numTransforms * sizeof(IFSTransform);
        size_t ratio = width * height * 3 / (transformSize / sizeof(int));
        std::ostringstream oss;
        oss << "Reading image (width=" << width << " height=" << height << ")\n" <<
            "Number of transforms: " << numTransforms << "\n" <<
            "Compression Ratio: " << ratio << ":1\n" <<
            "Encoding time: " << duration.count() << " ms" << '\n';
        std::string str = oss.str();
        Controller::sendMesssage(str);
        delete transforms;
        delete enc;
        delete source;
    };

    static void decode(const std::string& fileName,int phases) {
        /*std::string fileName;
        int phases = 5;
        bool usage = true;
        for (int i = 1; i < argc && usage; i++) {
            std::string param(argv[i]);
            if (param == "-p" && i + 1 < argc)
                phases = std::stoi(argv[i + 1]);
            if (param.at(0) == '-') {
                i++;
            } else {
                fileName = param;
                usage = false;
            }
        }
        if (usage) {
            std::ostringstream oss;
            oss << "Usage: %s [-p #] filename for decoding\n"<<
                   "\t-p 5    Number of decoding phases\n"<<argv[0]<<'\n';
            std::string str = oss.str();
            Controller::sendMesssage(str);
            exit(-1);
        }*/
        int width, height;
        std::string extension;
        Transforms *transforms2;
        std::tie(transforms2, extension) = LoadTransformsFromJson(fileName, &width, &height);
        auto start = std::chrono::high_resolution_clock::now();
        auto *dec = new Decoder(width, height, transforms2->channels);
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

        for (int phase = 1; phase <= phases; phase++) {
            dec->Decode(transforms2);
        }
        std::string newFileName = fileName.substr(0, fileName.rfind('.'));
        std::string decodedName = newFileName + extension;
        Image *producer = dec->GetNewImage(decodedName, 0);
        producer->Save();
        std::ostringstream oss;
        oss <<"Reading image (width=" << width << " height=" << height << ")\n" <<
            "Decoding time: " << duration.count() << " ms" << '\n';
        std::string str = oss.str();
        Controller::sendMesssage(str);
        delete producer;
        delete dec;
    };
private:
    static void SaveTransformsToJson(const Transforms *transforms, const std::string &filePath, const std::string &extension,
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

    static std::pair<Transforms *, std::string> LoadTransformsFromJson(const std::string &filePath, int *width, int *height) {
        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            std::ostringstream oss;
            oss <<"Error: Unable to open file for reading: " << filePath  << '\n';
            std::string str = oss.str();
            Controller::sendMesssage(str);
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


#endif //TESTFRACTAL_FRACTALALGO_H
