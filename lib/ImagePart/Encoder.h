#ifndef E_H
#define E_H
#pragma once
#include "../../src/controller/Controller.h"
class Encoder: public IController{
public:
    void sendCommonInformation(const CommonInformation &commonInformation) override {
        sendMessage("FlactalAlgo{ ");
        IController::sendCommonInformation(commonInformation);
        sendMessage("}\n");
    }

    void sendErrorInformation(const std::string &error) override {
        IController::sendErrorInformation("FractalAlgo{ "+error+"}\n");
    }
    Encoder(bool isTextOutput, const std::string &outputFile)
            : IController(isTextOutput, outputFile) {
        //this->view = view;
    }
    virtual ~Encoder() = default;

    virtual Transforms *Encode(Image *source) = 0;

    // These functions are helpers
    int GetAveragePixel(const PixelValue *domainData, int domainWidth,
                               int domainX, int domainY, int size);

    double GetScaleFactor(
            const PixelValue *domainData, int domainWidth, int domainX, int domainY, int domainAvg,
            const PixelValue *rangeData, int rangeWidth, int rangeX, int rangeY, int rangeAvg,
            int size);

    double GetError(
            const PixelValue *domainData, int domainWidth, int domainX, int domainY, int domainAvg,
            const PixelValue *rangeData, int rangeWidth, int rangeX, int rangeY, int rangeAvg,
            int size, double scale);

public:
    Image img;
};

#endif // E_H
