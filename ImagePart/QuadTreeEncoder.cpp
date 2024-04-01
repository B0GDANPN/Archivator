#include <vector>
#include <string>
#include "Image.h"
#include "IFSTransform.h"
#include "Encoder.h"
#include "QuadTreeEncoder.h"

extern bool useYCbCr;

#define BUFFER_SIZE        (32)

QuadTreeEncoder::QuadTreeEncoder(int threshold, bool symmetry) {
    this->threshold = threshold;
    this->symmetry = symmetry;
}

QuadTreeEncoder::~QuadTreeEncoder()
= default;

Transforms *QuadTreeEncoder::Encode(Image *source) {
    auto *transforms = new Transforms;

    img.width = source->GetWidth();
    img.height = source->GetHeight();
    img.channels = source->GetChannels();
    transforms->channels = img.channels;

    for (int channel = 1; channel <= img.channels; channel++) {
        // Load image into a local copy
        img.imagedata = new PixelValue[img.width * img.height];
        source->GetChannelData(channel, img.imagedata, img.width * img.height);


        // Make second channel the downsampled version of the image.
        img.imagedata2 = IFSTransform::DownSample(img.imagedata, img.width, 0, 0, img.width / 2);

        // When using YCbCr we can reduce the quality of colour, because the eye
        // is more sensitive to intensity which is channel 1.
        if (channel >= 2 && useYCbCr)
            threshold *= 2;

        // Go through all the range blocks
        for (int y = 0; y < img.height; y += BUFFER_SIZE) {
            for (int x = 0; x < img.width; x += BUFFER_SIZE) {
                findMatchesFor(transforms->ch[channel - 1], x, y, BUFFER_SIZE);
            }
        }

        // Bring the threshold back to original.
        if (channel >= 2 && useYCbCr)
            threshold /= 2;

        delete[]img.imagedata2;
        img.imagedata2 = nullptr;
        delete[]img.imagedata;
        img.imagedata = nullptr;
    }

    return transforms;
}

void QuadTreeEncoder::findMatchesFor(Transform &transforms, int toX, int toY, int blockSize) {
    int bestX = 0;
    int bestY = 0;
    int bestOffset = 0;
    IFSTransform::SYM bestSymmetry = IFSTransform::SYM_NONE;
    double bestScale = 0;
    double bestError = 1e9;

    auto *buffer = new PixelValue[blockSize * blockSize];

    // Get average pixel for the range block
    int rangeAvg = GetAveragePixel(img.imagedata, img.width, toX, toY, blockSize);

    // Go through all the downsampled domain blocks
    for (int y = 0; y < img.height; y += blockSize * 2) {
        for (int x = 0; x < img.width; x += blockSize * 2) {
            for (int symmetry = 0; symmetry < IFSTransform::SYM_MAX; symmetry++) {
                auto symmetryEnum = (IFSTransform::SYM) symmetry;
                auto *ifs = new IFSTransform(x, y, 0, 0, blockSize, symmetryEnum, 1.0, 0);
                ifs->Execute(img.imagedata2, img.width / 2, buffer, blockSize, true);

                // Get average pixel for the downsampled domain block
                int domainAvg = GetAveragePixel(buffer, blockSize, 0, 0, blockSize);

                // Get scale and offset
                double scale = GetScaleFactor(img.imagedata, img.width, toX, toY, domainAvg,
                                              buffer, blockSize, 0, 0, rangeAvg, blockSize);
                int offset = (int) (rangeAvg - scale * (double) domainAvg);

                // Get error and compare to best error so far
                double error = GetError(buffer, blockSize, 0, 0, domainAvg,
                                        img.imagedata, img.width, toX, toY, rangeAvg, blockSize, scale);

                if (error < bestError) {
                    bestError = error;
                    bestX = x;
                    bestY = y;
                    bestSymmetry = symmetryEnum;
                    bestScale = scale;
                    bestOffset = offset;
                }

                delete ifs;

                if (!symmetry)
                    break;
            }
        }
    }

    delete[]buffer;

    if (blockSize > 2 && bestError >= threshold) {
        // Recurse into the four corners of the current block.
        blockSize /= 2;
        findMatchesFor(transforms, toX, toY, blockSize);
        findMatchesFor(transforms, toX + blockSize, toY, blockSize);
        findMatchesFor(transforms, toX, toY + blockSize, blockSize);
        findMatchesFor(transforms, toX + blockSize, toY + blockSize, blockSize);
    } else {
        // Use this transformation
        transforms.push_back(
                new IFSTransform(
                        bestX, bestY,
                        toX, toY,
                        blockSize,
                        bestSymmetry,
                        bestScale,
                        bestOffset
                )
        );
    }
}
