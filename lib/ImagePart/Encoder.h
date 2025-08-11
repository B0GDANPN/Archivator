#ifndef ARCHIVATOR_ENCODER_H
#define ARCHIVATOR_ENCODER_H
#pragma once
#include "Image.h"
#include "IFSTransform.h"

class Encoder : public IController {
public:
    void sendCommonInformation(const CommonInformation &commonInformation) override {
        sendMessage("FlactalAlgo{ ");
        IController::sendCommonInformation(commonInformation);
        sendMessage("}\n");
    }

    void sendErrorInformation(const std::string &error) override {
        IController::sendErrorInformation("FractalAlgo{ " + error + "}\n");
    }

    Encoder(bool isTextOutput, const std::string &outputFile,std::ostringstream& ref_oss)
            : IController(isTextOutput, outputFile,ref_oss){
        img.isTextOutput=isTextOutput;
        img.outputFile=outputFile;
        //this->view = view;
    }

    ~Encoder() override = default;

    virtual Transforms *Encode(Image *source) = 0;

    // These functions are helpers
    int GetAveragePixel(const PixelValue *domainData, int domainWidth,
                        int domainX, int domainY, int size) {
        int top = 0;
        int bottom = (size * size);

        // Simple average of all pixels.
        for (int y = domainY; y < domainY + size; y++) {
            for (int x = domainX; x < domainX + size; x++) {
                top += domainData[y * domainWidth + x];
                if (top < 0) {
                    std::ostringstream oss;
                    oss << "Error: Accumulator rolled over averaging pixels." << '\n';
                    std::string str = oss.str();
                    sendErrorInformation(str);
                    delete domainData;
                    exit(-1);
                }
            }
        }

        return (top / bottom);
    };

    double GetScaleFactor(
            const PixelValue *domainData, int domainWidth, int domainX, int domainY, int domainAvg,
            const PixelValue *rangeData, int rangeWidth, int rangeX, int rangeY, int rangeAvg,
            int size) {
        int top = 0;
        int bottom = 0;

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int domain = (domainData[(domainY + y) * domainWidth + (domainX + x)] - domainAvg);
                int range = (rangeData[(rangeY + y) * rangeWidth + (rangeX + x)] - rangeAvg);

                // According to the formula we want (R*D) / (D*D)
                top += range * domain;
                bottom += domain * domain;

                if (bottom < 0) {
                    std::ostringstream oss;
                    oss << "Error: Overflow occurred during scaling"
                        << y << " " << domainWidth << " " << bottom << " " << top << '\n';
                    std::string str = oss.str();
                    sendErrorInformation(str);
                    delete rangeData;
                    delete domainData;
                    exit(-1);
                }
            }
        }

        if (bottom == 0) {
            top = 0;
            bottom = 1;
        }

        return static_cast<double>(top) / static_cast<double>(bottom);
    };

    double GetError(
            const PixelValue *domainData, int domainWidth, int domainX, int domainY, int domainAvg,
            const PixelValue *rangeData, int rangeWidth, int rangeX, int rangeY, int rangeAvg,
            int size, double scale) {
        double top = 0;
        auto bottom = static_cast<double>(size * size);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int domain = (domainData[(domainY + y) * domainWidth + (domainX + x)] - domainAvg);
                int range = (rangeData[(rangeY + y) * rangeWidth + (rangeX + x)] - rangeAvg);
                int diff = static_cast<int>(scale * static_cast<double>(domain)) - range;

                // According to the formula we want (DIFF*DIFF)/(SIZE*SIZE)
                top += (diff * diff);

                if (top < 0) {
                    std::ostringstream oss;
                    oss << "Error: Overflow occurred during error " << top << '\n';
                    std::string str = oss.str();
                    sendErrorInformation(str);
                    delete rangeData;
                    delete domainData;
                    exit(-1);
                }
            }
        }

        return (top / bottom);
    };

public:
    Image img{isTextOutput,outputFile,oss};
};

#endif ARCHIVATOR_ENCODER_H
