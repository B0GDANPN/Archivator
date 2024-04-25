//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_MATRIXINFO_H
#define ARCHIVATOR_MATRIXINFO_H
#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
struct MatrixInfo {
    cv::Size size{};
    cv::Point point{};
    std::vector<unsigned char> data;
    size_t dataSize{};
};
#endif ARCHIVATOR_MATRIXINFO_H
