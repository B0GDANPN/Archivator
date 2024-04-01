#ifndef E_H
#define E_H
#pragma once

class Encoder {
public:

    virtual ~Encoder() = default;

    virtual Transforms *Encode(Image *source) = 0;

    // These functions are helpers
    static int GetAveragePixel(const PixelValue *domainData, int domainWidth,
                               int domainX, int domainY, int size);

    static double GetScaleFactor(
            const PixelValue *domainData, int domainWidth, int domainX, int domainY, int domainAvg,
            const PixelValue *rangeData, int rangeWidth, int rangeX, int rangeY, int rangeAvg,
            int size);

    static double GetError(
            const PixelValue *domainData, int domainWidth, int domainX, int domainY, int domainAvg,
            const PixelValue *rangeData, int rangeWidth, int rangeX, int rangeY, int rangeAvg,
            int size, double scale);

public:
    Image img;
};

#endif // E_H
