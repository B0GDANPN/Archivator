#ifndef MY_IMAGE_PROCESSING_H
#define MY_IMAGE_PROCESSING_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct MatrixInfo {
    cv::Size size;
    cv::Point point;
    std::vector<uchar> data;
    size_t dataSize;
};

const size_t SPLIT_DEPTH = 64;
const size_t NOIZES = 2500000;
const size_t NOIZES_PER_SUBFRAME = 50000;
const size_t SOLID_DIFFERENCE = 50;
const size_t SUBFRAME_DIFFERENCE = 3;
const size_t CACHED_FRAME_DIFFERENCE = 3;
const size_t COLOR_CHANNELS = 3;


struct _Rect {
    size_t a;
    size_t b;
    size_t c;
    size_t d;
    _Rect(cv::Rect rect) {
        a = rect.x;
        b = rect.y;
        c = rect.x + rect.width;
        d = rect.y + rect.height;
    }
};


#endif MY_IMAGE_PROCESSING_H
