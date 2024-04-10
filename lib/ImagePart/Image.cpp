#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include <cmath>
#include "Image.h"

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
        std::ostringstream oss;
        oss << "Error: Could not load image" << '\n';
        std::string str = oss.str();
        Controller::sendMesssage(str);
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
        std::ostringstream oss;
        oss << "Modifying image to (width=" << new_width << " height=" << new_height << ")" << '\n';
        std::string str = oss.str();
        Controller::sendMesssage(str);
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
        imagedata[i] = R;
        imagedata2[i] = G;
        imagedata3[i] = B;
    }
    std::ostringstream oss;
    oss << "Readed image (width=" << width << " height=" << height << ")" << '\n';
    std::string str = oss.str();
    Controller::sendMesssage(str);
    delete[]original_data;
}

void Image::Save() const {
    std::ostringstream oss;
    oss << "Writing image (width=" << width << " height=" << height << " channels=" << channels << ")" << '\n';
    std::string str = oss.str();
    Controller::sendMesssage(str);

    auto *chunk = new unsigned char[originalSize];

    // Convert from image data type
    for (int i = 0; i < width * height; i++) {
        if (channels == 3) {
            PixelValue R = imagedata[i], G = imagedata2[i], B = imagedata3[i];
            chunk[i * 3 + 0] = static_cast<unsigned char> (R);
            chunk[i * 3 + 1] = static_cast<unsigned char> (G);
            chunk[i * 3 + 2] = static_cast<unsigned char> (B);
        } else if (channels == 1) {
            chunk[i] = imagedata[i];
        }
    }
    std::string fullName = (this->fileName + '.' + this->extension);
    if (this->extension == "bmp") {
        stbi_write_bmp(fullName.c_str(), width, height, channels, chunk);
    } else if (this->extension == "tga") {
        stbi_write_tga(fullName.c_str(), width, height, channels, chunk);
    } else if (this->extension == "jpg" || this->extension == "jpeg") {
        stbi_write_jpg(fullName.c_str(), width, height, channels, chunk, 100);
    } else {
        std::ostringstream oss;
        oss << "Error: Non supported extension " << this->extension << '\n';
        Controller::sendMesssage(oss.str());
        exit(-1);
    }
    delete[]chunk;
}

void Image::GetChannelData(int channel, PixelValue *buffer, int size) const {

    if ((channel == 1 && imagedata == nullptr) ||
        (channel == 2 && imagedata2 == nullptr) ||
        (channel == 3 && imagedata3 == nullptr)) {
        std::ostringstream oss;
        oss << "Error: Image data was not loaded yet." << '\n';
        Controller::sendMesssage(oss.str());
        delete[]buffer;
        exit(-1);
    }
    if (width * height != size) {
        std::ostringstream oss;
        oss << "Error: Image data size mismatch." << '\n';
        Controller::sendMesssage(oss.str());
        delete[]buffer;
        exit(-1);
    }
    if (channel > channels || channel <= 0) {
        std::ostringstream oss;
        oss << "Error: Image channel out of bounds." << '\n';
        Controller::sendMesssage(oss.str());
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
