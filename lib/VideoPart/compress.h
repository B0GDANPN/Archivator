#ifndef COMPRESS_H
#define COMPRESS_H

#include <opencv2/opencv.hpp>
#include <vector>

void writeMatricesAndPoints(const std::vector<std::pair<cv::Point, cv::Mat>>& matrices, const std::string& filename);

std::vector<std::pair<cv::Point, cv::Mat>> splitMatrices(cv::Mat image, cv::Mat silents, int threshold);

void writeNumbersExcludingSubmatrices(const cv::Mat& big_matrix, const std::vector<std::pair<cv::Point, cv::Mat>>& submatrices, std::vector<cv::Vec3b>& result);

void insertMatrix(cv::Mat& bigMatrix, const cv::Mat& smallMatrix, cv::Point position);

std::vector<uchar> compressMat(const cv::Mat& image);

void writeBufferToFile(const std::vector<cv::Vec3b>& buffer, const std::string& filename, int theshold);

std::pair<cv::Vec3b, bool> areSolid(const cv::Mat& matrix, double threshold);

#endif COMPRESS_H