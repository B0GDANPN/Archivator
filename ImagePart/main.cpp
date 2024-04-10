#include <vector>
#include <string>
#include <json.hpp>
#include <fstream>
#include <iostream>
#include "Image.h"
#include "IFSTransform.h"
#include "Encoder.h"
#include "QuadTreeEncoder.h"
#include "Decoder.h"

using json = nlohmann::json;
bool useYCbCr = true;

void PrintUsage(char *exe) {
    std::cout << "Usage: %s [-v #] [-t #] [-p #] [-o #] [-f] [-r] filename\n"
                 "\t-t 100  Threshold (i.e. quality)\n"
                 "\t-p 5    Number of decoding phases\n"
                 "\t-f      Force symmetry operations during encoding\n"
                 "\t-r      Enable RGB instead of YCbCr\n"
              << exe;
}

void SaveTransformsToJson(const Transforms *transforms, const std::string &filePath, int width, int height) {
    json jsonTransforms;
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

Transforms *LoadTransformsFromJson(const std::string &filePath, int *width, int *height) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        std::cout << "Error: Unable to open file for reading: " << filePath << std::endl;
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
    return transforms;
}

void Convert(Encoder *enc, Image *source, int maxphases) {
    source->Load();

    int width = source->GetWidth();
    int height = source->GetHeight();
    Transforms *transforms = enc->Encode(source);


    std::string encodedName = source->GetFileName() + "_encoded." + source->GetExtension();
    SaveTransformsToJson(transforms, encodedName, width, height);
    Transforms *transforms2 = LoadTransformsFromJson(encodedName, &width, &height);
    auto *dec = new Decoder(width, height, transforms2->channels);

    for (int phase = 1; phase <= maxphases; phase++) {
        dec->Decode(transforms2);
    }

    // Save the final image.
    std::string decodedName = source->GetFileName() + "_decoded." + source->GetExtension();
    Image *producer = dec->GetNewImage(decodedName, 0);
    producer->Save();
    delete producer;
    delete dec;
    delete transforms;
}


int main(int argc, char **argv) {
    std::string fileName;
    int quality = 100;
    bool symmetry = false;
    int phases = 5;
    bool usage = true;

    // Load parameters
    for (int i = 1; i < argc && usage; i++) {
        std::string param(argv[i]);

        if (param == "-t" && i + 1 < argc)
            quality = std::stoi(argv[i + 1]);
        else if (param == "-p" && i + 1 < argc)
            phases = std::stoi(argv[i + 1]);
        else if (param == "-f" && --i >= 0)
            symmetry = true;
        else if (param == "-r" && --i >= 0)
            useYCbCr = false;

        if (param.at(0) == '-') {
            i++;
        } else {
            fileName = param;
            usage = false;
        }
    }

    if (usage) {
        PrintUsage(argv[0]);
        return -1;
    }

    auto *source = new Image();
    source->ImageSetup(fileName);
    auto *enc = new QuadTreeEncoder(quality, symmetry);
    Convert(enc, source, phases);
    delete enc;
    delete source;
}