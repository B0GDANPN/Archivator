#ifndef IMAGE_H
#define IMAGE_H
#pragma once
#include <sstream>
#include "../../src/controller/Controller.h"
typedef unsigned char PixelValue;


class Image {
public:
    int width;
    int height;
    int channels;
    PixelValue *imagedata;
    PixelValue *imagedata2;
    PixelValue *imagedata3;
    std::string fileName;
    std::string extension;
    int originalSize;
public:

    explicit Image();

    ~Image();

    void Load();

    void Save() const;

    void GetChannelData(int channel, PixelValue *buffer, int size) const;

    void SetChannelData(int channel, PixelValue *buffer, int size);
    void ImageSetup(const std::string &fileName);
};

#endif // IMAGE_H
