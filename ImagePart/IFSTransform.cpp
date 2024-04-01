#include <string>
#include <vector>
#include "Image.h"
#include "IFSTransform.h"

Transforms::Transforms() {
    channels = 0;
}

Transforms::~Transforms() {
    for (int i = 0; i < channels; i++) {
        for (auto &j: ch[i])
            delete j;
        ch[i].clear();
    }
}

PixelValue *IFSTransform::DownSample(const PixelValue *src, int srcWidth,
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
}

IFSTransform::IFSTransform(int fromX, int fromY, int toX, int toY, int size,
                           IFSTransform::SYM symmetry, double scale, int offset) : fromX(fromX), fromY(fromY),
                                                                                   toX(toX), toY(toY), size(size),
                                                                                   symmetry(symmetry), scale(scale),
                                                                                   offset(offset) {}

IFSTransform::~IFSTransform()
= default;

void IFSTransform::Execute(PixelValue *src, int srcWidth,
                           PixelValue *dest, int destWidth, bool downsampled) {
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

    for (int toY = this->toY; toY < (this->toY + size); toY++) {
        for (int toX = this->toX; toX < (this->toX + size); toX++) {


            int pixel = src[fromY * srcWidth + fromX];
            pixel = (int) (scale * pixel) + offset;

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
        nullptr;
    }
}

int IFSTransform::getSize() const {
    return size;
}

bool IFSTransform::isScanlineOrder() {
    return (
            symmetry == SYM_NONE ||
            symmetry == SYM_R180 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_VFLIP
    );
}

bool IFSTransform::isPositiveX() {
    return (
            symmetry == SYM_NONE ||
            symmetry == SYM_R90 ||
            symmetry == SYM_VFLIP ||
            symmetry == SYM_RDFLIP
    );
}

bool IFSTransform::isPositiveY() {
    return (
            symmetry == SYM_NONE ||
            symmetry == SYM_R270 ||
            symmetry == SYM_HFLIP ||
            symmetry == SYM_RDFLIP
    );
}

int IFSTransform::getFromX() const {
    return fromX;
}

int IFSTransform::getFromY() const {
    return fromY;
}

int IFSTransform::getToX() const {
    return toX;
}

int IFSTransform::getToY() const {
    return toY;
}

IFSTransform::SYM IFSTransform::getSymmetry() const {
    return symmetry;
}

int IFSTransform::getOffset() const {
    return offset;
}

double IFSTransform::getScale() const {
    return scale;
}

