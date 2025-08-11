#ifndef ARCHIVATOR_IFST_H
#define ARCHIVATOR_IFST_H
#pragma once

#include <vector>
#include "Image.h"

class IFSTransform {
public:

    enum SYM {
        SYM_NONE = 0,
        SYM_R90,
        SYM_R180,
        SYM_R270,
        SYM_HFLIP,
        SYM_VFLIP,
        SYM_RDFLIP,
    };

public:

    static PixelValue *DownSample(const PixelValue *src, int srcWidth,
                                  int startX, int startY, int targetSize) {
        auto *dest = new PixelValue[targetSize * targetSize];
        int destX = 0;
        int destY = 0;

        for (int y = startY; y < startY + targetSize * 2; y += 2) {
            for (int x = startX; x < startX + targetSize * 2; x += 2) {
                // Perform simple 2x2 average
                int pixel = 0;
                pixel += src[y * srcWidth + x];
                pixel += src[y * srcWidth + (x + 1)];
                pixel += src[(y + 1) * srcWidth + x];
                pixel += src[(y + 1) * srcWidth + (x + 1)];
                pixel /= 4;

                dest[destY * targetSize + destX] = pixel;
                destX++;
            }
            destY++;
            destX = 0;
        }

        return dest;
    };


    IFSTransform(int fromX, int fromY, int toX, int toY, int size,
                 SYM symmetry, double scale, int offset) : fromX(fromX), fromY(fromY),
                                                           toX(toX), toY(toY), size(size),
                                                           symmetry(symmetry), scale(scale),
                                                           offset(offset) {};

    ~IFSTransform() = default;

    void Execute(const PixelValue *src, int srcWidth,
                 PixelValue *dest, int destWidth, bool downsampled) const;

    int getSize() const {
        return size;
    };

    int getFromX() const {
        return fromX;
    };

    int getFromY() const {
        return fromY;
    };

    int getToX() const {
        return toX;
    };

    int getToY() const {
        return toY;
    };

    SYM getSymmetry() const {
        return symmetry;
    };

    double getScale() const {
        return scale;
    };

    int getOffset() const {
        return offset;
    };

private:

    bool isScanlineOrder() const {
        return (
                symmetry == SYM_NONE ||
                symmetry == SYM_R180 ||
                symmetry == SYM_HFLIP ||
                symmetry == SYM_VFLIP
        );
    };

    bool isPositiveX() const {
        return (
                symmetry == SYM_NONE ||
                symmetry == SYM_R90 ||
                symmetry == SYM_VFLIP ||
                symmetry == SYM_RDFLIP
        );
    };

    bool isPositiveY() const {
        return (
                symmetry == SYM_NONE ||
                symmetry == SYM_R270 ||
                symmetry == SYM_HFLIP ||
                symmetry == SYM_RDFLIP
        );
    };

private:

    // Spatial transformation
    size_t fromX;
    size_t fromY;
    size_t toX;
    size_t toY;
    size_t size;

    // Symmetry operation
    SYM symmetry;

    // Pixel intensity
    double scale;
    int offset;

};

inline void IFSTransform::Execute(const PixelValue *src, int srcWidth, PixelValue *dest, int destWidth,
    bool downsampled) const {
    int fromX = this->fromX / 2;
    int fromY = this->fromY / 2;
    int dX = 1;
    int dY = 1;
    bool inOrder = isScanlineOrder();

    if (!downsampled) {
        PixelValue *newSrc = DownSample(src, srcWidth, this->fromX, this->fromY, size);
        src = newSrc;
        srcWidth = size;
        fromX = fromY = 0;
    }

    if (!isPositiveX()) {
        fromX += size - 1;
        dX = -1;
    }

    if (!isPositiveY()) {
        fromY += size - 1;
        dY = -1;
    }

    int startX = fromX;
    int startY = fromY;

    for (size_t toY = this->toY; toY < (this->toY + size); toY++) {
        for (size_t toX = this->toX; toX < (this->toX + size); toX++) {


            int pixel = src[fromY * srcWidth + fromX];
            pixel = static_cast<int>(scale * pixel) + offset;

            if (pixel < 0)
                pixel = 0;
            if (pixel > 255)
                pixel = 255;

            dest[toY * destWidth + toX] = pixel;

            if (inOrder)
                fromX += dX;
            else
                fromY += dY;
        }

        if (inOrder) {
            fromX = startX;
            fromY += dY;
        } else {
            fromY = startY;
            fromX += dX;
        }
    }

    if (!downsampled) {
        delete[]src;
        src = nullptr;
    }
}

typedef std::vector<class IFSTransform *> Transform;

class Transforms {
public:
    Transforms() {
        channels = 0;
    }

    ~Transforms() {
        for (int i = 0; i < channels; i++) {
            for (auto &j: ch[i])
                delete j;

            ch[i].clear();
        }
    }
    int getSize() const{
        int res=sizeof(channels)+(ch[0].size()+ch[1].size()+ch[2].size())*sizeof(IFSTransform);
        return res;
    }
public:
    Transform ch[3];
    int channels;
};



#endif ARCHIVATOR_IFST_H
