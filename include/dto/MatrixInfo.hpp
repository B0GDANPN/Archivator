//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_MATRIXINFO_HPP
#define ARCHIVATOR_MATRIXINFO_HPP

#include <vector>
#include <opencv2/opencv.hpp>
struct MatrixInfo {
    cv::Size size{};
    cv::Point point{};
    std::vector<unsigned char> data;
    size_t data_size{};
};
#endif
