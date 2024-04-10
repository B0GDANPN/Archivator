#ifndef IFST_H
#define IFST_H
#pragma once
typedef std::vector<class IFSTransform *> Transform;

class Transforms {
public:
    Transforms();

    ~Transforms();

public:
    Transform ch[3];
    int channels;
};


class IFSTransform {
public:

    enum SYM {
        SYM_NONE = 0,
        SYM_R90,
        SYM_R180,
        SYM_R270,
        SYM_HFLIP,
        SYM_VFLIP,
        SYM_FDFLIP,
        SYM_RDFLIP,
        SYM_MAX
    };

public:

    static PixelValue *DownSample(const PixelValue *src, int srcWidth,
                                  int startX, int startY, int targetSize);


    IFSTransform(int fromX, int fromY, int toX, int toY, int size,
                 SYM symmetry, double scale, int offset);

    ~IFSTransform();

    void Execute(PixelValue *src, int srcWidth,
                 PixelValue *dest, int destWidth, bool downsampled);

    int getSize() const;

    int getFromX() const;

    int getFromY() const;

    int getToX() const;

    int getToY() const;

    SYM getSymmetry() const;

    double getScale() const;

    int getOffset() const;

private:

    bool isScanlineOrder();

    bool isPositiveX();

    bool isPositiveY();

private:

    // Spatial transformation
    int fromX;
    int fromY;
    int toX;
    int toY;
    int size;

    // Symmetry operation
    SYM symmetry;

    // Pixel intensity
    double scale;
    int offset;

};

#endif // IFST_H
