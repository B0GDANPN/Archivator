#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include <cmath>
#include <iostream>
#include "Image.h"

extern bool useYCbCr;

Image::Image() {
    imagedata = nullptr;
    imagedata2 = nullptr;
    imagedata3 = nullptr;
    width = height = 0;
    channels = 0;
    originalSize = 0;
}

Image::~Image() {
    if (imagedata != nullptr) {
        delete[]imagedata;
        imagedata = nullptr;
    }
    if (imagedata2 != nullptr) {
        delete[]imagedata2;
        imagedata2 = nullptr;
    }
    if (imagedata3 != nullptr) {
        delete[]imagedata3;
        imagedata3 = nullptr;
    }
}

void Image::ImageSetup(const std::string &fileName) {
    size_t lastDotIndex = fileName.rfind('.');
    this->fileName = fileName.substr(0, lastDotIndex);
    this->extension = fileName.substr(lastDotIndex + 1);
}

int NextMultipleOf(int number, int multiple) {
    return ((number + multiple - 1) / multiple) * multiple;
}

void Image::Load() {
    std::string fullName = (this->fileName + '.' + this->extension);
    unsigned char *original_data = stbi_load(fullName.c_str(), &width, &height, &channels, 0);
    if (!original_data) {
        std::cout << "Error: Could not load image" << std::endl;
        exit(-1);
    }
    originalSize = width * height * channels;
    int new_width = NextMultipleOf(width, 32);
    int new_height = NextMultipleOf(height, 32);
    if (new_height > new_width)
        new_width = new_height;
    if (new_width > new_height)
        new_height = new_width;
    if (new_width != width || new_height != height) {
        std::cout << "Modifying image (width=" << width << " height=" << height << ")" << std::endl;
        originalSize = new_width * new_height * channels;
        auto *new_data = new unsigned char[originalSize];
        memset(new_data, 0, originalSize);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                for (int c = 0; c < channels; ++c) {
                    new_data[(y * new_width + x) * channels + c] = original_data[(y * width + x) * channels + c];
                }
            }
        }
        width = new_width;
        height = new_height;
        stbi_image_free(original_data);
        original_data = new_data;
    }



    // Convert to image data type
    if (imagedata != nullptr) {
        delete[]imagedata;
        delete[]imagedata2;
        delete[]imagedata3;
    }
    int sizeChannel = originalSize / 3;
    imagedata = new PixelValue[sizeChannel];
    imagedata2 = new PixelValue[sizeChannel];
    imagedata3 = new PixelValue[sizeChannel];
    for (int i = 0; i < sizeChannel; i++) {
        PixelValue R = original_data[i * 3];
        PixelValue G = original_data[i * 3 + 1];
        PixelValue B = original_data[i * 3 + 2];

        ConvertToYCbCr(
                imagedata[i], imagedata2[i], imagedata3[i],
                R, G, B
        );
    }
    std::cout << "Readed image (width=" << width << " height=" << height << ")" << std::endl;
    delete[]original_data;
}

void Image::Save() const {
    std::cout << "Writing image (width=" << width << " height=" << height << " channels=" << channels << ")"
              << std::endl;

    auto *chunk = new unsigned char[originalSize];

    // Convert from image data type
    for (int i = 0; i < width * height; i++) {
        if (channels == 3) {
            PixelValue R, G, B;
            ConvertFromYCbCr(
                    R, G, B,
                    imagedata[i], imagedata2[i], imagedata3[i]
            );
            chunk[i * 3 + 0] = static_cast<unsigned char> (R);
            chunk[i * 3 + 1] = static_cast<unsigned char> (G);
            chunk[i * 3 + 2] = static_cast<unsigned char> (B);
        } else if (channels == 1) {
            chunk[i] = imagedata[i];
        }
    }
    std::string fullName = (this->fileName + '.' + this->extension);
    std::cout << "Writing " << fileName << " (size=" << originalSize << ")" << std::endl;
    if (this->extension == "bmp") {
        stbi_write_bmp(fullName.c_str(), width, height, channels, chunk);
    } else if (this->extension == "tga") {
        stbi_write_tga(fullName.c_str(), width, height, channels, chunk);
    } else if (this->extension == "jpg" || this->extension == "jpeg") {
        stbi_write_jpg(fullName.c_str(), width, height, channels, chunk, 100);
    } else {
        std::cout << "Error: Non supported extension " << this->extension << std::endl;
        exit(-1);
    }
    delete[]chunk;
}

void Image::GetChannelData(int channel, PixelValue *buffer, int size) const {
    if ((channel == 1 && imagedata == nullptr) ||
        (channel == 2 && imagedata2 == nullptr) ||
        (channel == 3 && imagedata3 == nullptr)) {
        std::cout << "Error: Image data was not loaded yet." << std::endl;
        delete[]buffer;
        exit(-1);
    }
    if (width * height != size) {
        std::cout << "Error: Image data size mismatch." << std::endl;
        delete[]buffer;
        exit(-1);
    }
    if (channel > channels || channel <= 0) {
        std::cout << "Error: Image channel out of bounds." << std::endl;
        delete[]buffer;
        exit(-1);
    }

    if (channel == 1)
        memcpy(buffer, imagedata, size);
    else if (channel == 2)
        memcpy(buffer, imagedata2, size);
    else if (channel == 3)
        memcpy(buffer, imagedata3, size);
}

void Image::SetChannelData(int channel, PixelValue *buffer, int sizeChannel) {
    PixelValue *imagedataTemp = nullptr;


    if (channel > channels)
        channels = channel;

    if (channel == 1) {
        imagedataTemp = imagedata;
        imagedata = new PixelValue[sizeChannel];
        memcpy(imagedata, buffer, sizeChannel);
    } else if (channel == 2) {
        imagedataTemp = imagedata2;
        imagedata2 = new PixelValue[sizeChannel];
        memcpy(imagedata2, buffer, sizeChannel);
    } else if (channel == 3) {
        imagedataTemp = imagedata3;
        imagedata3 = new PixelValue[sizeChannel];
        memcpy(imagedata3, buffer, sizeChannel);
    }
    delete[]imagedataTemp;
}

int Image::GetWidth() const {
    return width;
}

int Image::GetHeight() const {
    return height;
}

int Image::GetChannels() const {
    return channels;
}

int Image::GetOriginalSize() const {
    return originalSize;
}

void Image::ConvertToYCbCr(PixelValue &Y, PixelValue &Cb, PixelValue &Cr,
                           PixelValue R, PixelValue G, PixelValue B) {
    if (useYCbCr) {
        Y = (PixelValue) (0.299 * R + 0.587 * G + 0.114 * B);
        Cb = (PixelValue) (-0.1687 * R - 0.3313 * G + 0.5 * B + 128);
        Cr = (PixelValue) (0.5 * R - 0.4187 * G - 0.0813 * B + 128);
    } else {
        Y = R;
        Cb = G;
        Cr = B;
    }
}

void Image::ConvertFromYCbCr(PixelValue &R, PixelValue &G, PixelValue &B,
                             PixelValue Y, PixelValue Cb, PixelValue Cr) {
    if (useYCbCr) {
        R = (PixelValue) (Y + 1.402 * (Cr - 128));
        G = (PixelValue) (Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128));
        B = (PixelValue) (Y + 1.772 * (Cb - 128));
    } else {
        R = Y;
        G = Cb;
        B = Cr;
    }
}

std::string Image::GetFileName() const {
    return this->fileName;
}

std::string Image::GetExtension() const {
    return this->extension;
}