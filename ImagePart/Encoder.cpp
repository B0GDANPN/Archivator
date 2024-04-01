#include <cstdlib>
#include <vector>
#include <iostream>
#include "Image.h"
#include "IFSTransform.h"
#include "Encoder.h"

double Encoder::GetScaleFactor(
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
                std::cout << "Error: Overflow occurred during scaling "
                          << y << " " << domainWidth << " " << bottom << " " << top << std::endl;
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

    return ((double) top) / ((double) bottom);
}

double Encoder::GetError(
        const PixelValue *domainData, int domainWidth, int domainX, int domainY, int domainAvg,
        const PixelValue *rangeData, int rangeWidth, int rangeX, int rangeY, int rangeAvg,
        int size, double scale) {
    double top = 0;
    auto bottom = (double) (size * size);

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int domain = (domainData[(domainY + y) * domainWidth + (domainX + x)] - domainAvg);
            int range = (rangeData[(rangeY + y) * rangeWidth + (rangeX + x)] - rangeAvg);
            int diff = (int) (scale * (double) domain) - range;

            // According to the formula we want (DIFF*DIFF)/(SIZE*SIZE)
            top += (diff * diff);

            if (top < 0) {
                std::cout << "Error: Overflow occurred during error " << top << std::endl;
                delete rangeData;
                delete domainData;
                exit(-1);
            }
        }
    }

    return (top / bottom);
}

int Encoder::GetAveragePixel(const PixelValue *domainData, int domainWidth,
                             int domainX, int domainY, int size) {
    int top = 0;
    int bottom = (size * size);

    // Simple average of all pixels.
    for (int y = domainY; y < domainY + size; y++) {
        for (int x = domainX; x < domainX + size; x++) {
            top += domainData[y * domainWidth + x];

            if (top < 0) {
                std::cout << "Error: Accumulator rolled over averaging pixels." << std::endl;
                delete domainData;
                exit(-1);
            }
        }
    }

    return (top / bottom);
}

