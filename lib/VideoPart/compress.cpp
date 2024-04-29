#include "codec.h"
#include "compress.h"
#include <fstream>
#include "logger.h"
#include <opencv2/core.hpp>

cv::Vec3b scalarToVec3b(const cv::Scalar& scalar) {
    return cv::Vec3b(static_cast<uchar>(scalar[0] + 0.5),
        static_cast<uchar>(scalar[1] + 0.5),
        static_cast<uchar>(scalar[2] + 0.5));
}

bool isSimilar(const cv::Vec3b& pixel1, const cv::Vec3b& pixel2, int threshold = CACHED_FRAME_DIFFERENCE) {
    int distance = 0;
    for (int i = 0; i < 3; ++i) {
        distance += static_cast<int>(pixel1[i]) - static_cast<int>(pixel2[i]);
    }
    return distance <= threshold;
}

std::pair<cv::Vec3b, bool> areSolid(const cv::Mat& matrix, double threshold = SOLID_DIFFERENCE) {
    cv::Scalar meanValue = cv::mean(matrix);

    for (int i = 0; i < matrix.rows; ++i) {
        for (int j = 0; j < matrix.cols; ++j) {
            cv::Vec3b pixel = matrix.at<cv::Vec3b>(i, j);
            double diffB = std::abs(pixel[0] - meanValue.val[0]);
            double diffG = std::abs(pixel[1] - meanValue.val[1]);
            double diffR = std::abs(pixel[2] - meanValue.val[2]);

            if (diffB > threshold || diffG > threshold || diffR > threshold) {
                return std::make_pair(scalarToVec3b(meanValue), false);
            }
        }
    }
    return std::make_pair(scalarToVec3b(meanValue), true);
}


std::vector<uchar> compressMat(const cv::Mat& image) {
    if (image.empty()) {
        logger << "Got empty image to compress!!!11!11!1" << std::endl;
        return {};
    }

    std::vector<uchar> compressedData;
    for (int row = 0; row < image.rows; ++row) {
        int count = 1;
        for (int col = 1; col < image.cols; ++col) {
            if (isSimilar(image.at<cv::Vec3b>(row, col), image.at<cv::Vec3b>(row, col - 1)) && count < 255) {
                count++;
            }
            else {
                compressedData.push_back(count);
                compressedData.push_back(image.at<cv::Vec3b>(row, col - 1)[0]);
                compressedData.push_back(image.at<cv::Vec3b>(row, col - 1)[1]);
                compressedData.push_back(image.at<cv::Vec3b>(row, col - 1)[2]);
                count = 1;
            }
        }
        compressedData.push_back(count);
        compressedData.push_back(image.at<cv::Vec3b>(row, image.cols - 1)[0]);
        compressedData.push_back(image.at<cv::Vec3b>(row, image.cols - 1)[1]);
        compressedData.push_back(image.at<cv::Vec3b>(row, image.cols - 1)[2]);
    }

    return compressedData;
}

void writeMatricesAndPoints(const std::vector<std::pair<cv::Point, cv::Mat>>& matrices, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary | std::ios::app);

    if (!ofs.is_open()) {
        logger << "Failed to open file for writing!11!111!" << std::endl;
        return;
    }

    for (const auto& matrix_info : matrices) {

        std::pair<cv::Vec3b, bool> isSolid = areSolid(matrix_info.second);
        cv::Size size = matrix_info.second.size();
        cv::Point point = matrix_info.first;

        ofs.write(reinterpret_cast<const char*>(&isSolid.first), sizeof(cv::Vec3b));
        ofs.write(reinterpret_cast<const char*>(&isSolid.second), sizeof(bool));
        ofs.write(reinterpret_cast<const char*>(&size), sizeof(cv::Size));
        ofs.write(reinterpret_cast<const char*>(&point), sizeof(cv::Point));

        if (!isSolid.second) {
            std::vector<uchar> vec = compressMat(matrix_info.second);
            size_t dataSize = vec.size();
            ofs.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
            ofs.write(reinterpret_cast<const char*>(vec.data()), dataSize);
        }
    }

    ofs.close();
}

std::vector<std::pair<cv::Point, cv::Mat>> splitMatrices(cv::Mat image, cv::Mat silents, int threshold) {
    std::vector<std::pair<cv::Point, cv::Mat>> result;
    std::queue<std::pair<cv::Point, cv::Mat>> to_process;
    to_process.push({ cv::Point(0, 0), silents });
    int counter = 0;

    while (!to_process.empty()) {

        auto top_left = to_process.front().first;
        auto matrix = to_process.front().second;
        to_process.pop();

        if (cv::sum(matrix)[0] > threshold) {
            int width = matrix.cols / 2;
            int height = matrix.rows / 2;

            to_process.push({ top_left, matrix(cv::Rect(0, 0, width, height)) });
            to_process.push({ top_left + cv::Point(width, 0), matrix(cv::Rect(width, 0, width, height)) });
            to_process.push({ top_left + cv::Point(0, height), matrix(cv::Rect(0, height, width, height)) });
            to_process.push({ top_left + cv::Point(width, height), matrix(cv::Rect(width, height, width, height)) });
        }
        else {
            result.push_back({ top_left, image(cv::Rect(top_left.x, top_left.y, matrix.cols, matrix.rows)) });
        }
        if (counter++ == SPLIT_DEPTH) return result;
    }

    return result;
}

void writeNumbersExcludingSubmatrices(const cv::Mat& big_matrix, const std::vector<std::pair<cv::Point,
    cv::Mat>>&submatrices, std::vector<cv::Vec3b>& result) {

    for (int i = 0; i < big_matrix.rows; ++i) {
        for (int j = 0; j < big_matrix.cols; ++j) {
            cv::Point current_point(j, i);

            bool is_inside_submatrix = false;
            for (const auto& submatrix : submatrices) {
                const cv::Point& submatrix_origin = submatrix.first;
                const cv::Mat& submatrix_data = submatrix.second;

                if (current_point.x >= submatrix_origin.x &&
                    current_point.x < submatrix_origin.x + submatrix_data.cols &&
                    current_point.y >= submatrix_origin.y &&
                    current_point.y < submatrix_origin.y + submatrix_data.rows) {
                    is_inside_submatrix = true;
                    break;
                }
            }

            if (!is_inside_submatrix) {
                result.push_back(big_matrix.at<cv::Vec3b>(current_point));
            }
        }
    }
}

void writeBufferToFile(const std::vector<cv::Vec3b>& buffer, const std::string& filename, int threshold = 10) {
    static int fileCounter = 0;

    std::stringstream ss;
    ss << filename << fileCounter++ << ".bin";
    std::string uniqueFilename = ss.str();

    std::ofstream outputFile(uniqueFilename, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Невозможно открыть файл: " << uniqueFilename << std::endl;
        return;
    }

    size_t i = 0;
    while (i < buffer.size()) {
        int count = 1;
        cv::Vec3b prev_pixel = buffer[i];
        size_t j = i + 1;
        while (j < buffer.size() && isSimilar(prev_pixel, buffer[j], threshold) && count < 255) {
            count++;
            j++;
        }

        outputFile.write(reinterpret_cast<const char*>(&count), sizeof(unsigned char));
        outputFile.write(reinterpret_cast<const char*>(&prev_pixel), sizeof(cv::Vec3b));

        i = j;
    }
}


void insertMatrix(cv::Mat& bigMatrix, const cv::Mat& smallMatrix, cv::Point position) {
    if (position.x < 0 || position.y < 0 ||
        position.x + smallMatrix.cols > bigMatrix.cols ||
        position.y + smallMatrix.rows > bigMatrix.rows) {
        logger << "Error: Invalid position or size for insertion!11!!11" << std::endl;
        return;
    }

    for (int y = 0; y < smallMatrix.rows; ++y) {
        for (int x = 0; x < smallMatrix.cols; ++x) {
            bigMatrix.at<cv::Vec3b>(position.y + y, position.x + x) = smallMatrix.at<cv::Vec3b>(y, x);
        }
    }
}