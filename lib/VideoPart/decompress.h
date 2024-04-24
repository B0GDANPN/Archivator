#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "codec.h"

std::vector<cv::Vec3b> decodeBufferFromFile(const std::string& filename);

std::vector<uchar> decompressMat(const std::vector<uchar>& compressedData);

MatrixInfo readNextMatrixAndPoint(const std::string& filename);

void decode(const std::string& framedata, const std::string& matdata, const std::string& subframedata, const std::string& output);

#endif DECOMPRESS_H