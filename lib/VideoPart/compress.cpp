#include "codec.h"
#include "compress.h"
#include <fstream>
#include "logger.h"

std::vector<uchar> compressMat(const cv::Mat& image) {
    if (image.empty()) {
        logger << "Got empty image to compress!!!11!11!1" << std::endl;
        return {};
    }

    std::vector<uchar> compressedData;
    for (int row = 0; row < image.rows; ++row) {
        int count = 1;
        for (int col = 1; col < image.cols; ++col) {
            if (image.at<cv::Vec3b>(row, col) == image.at<cv::Vec3b>(row, col - 1)) {
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
        cv::Size size = matrix_info.second.size();
        ofs.write(reinterpret_cast<const char*>(&size), sizeof(cv::Size));

        cv::Point point = matrix_info.first;
        ofs.write(reinterpret_cast<const char*>(&point), sizeof(cv::Point));

        std::vector<uchar> vec = compressMat(matrix_info.second);
        size_t dataSize = vec.size();
        ofs.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        ofs.write(reinterpret_cast<const char*>(vec.data()), dataSize);
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

void writeBufferToFile(const std::vector<cv::Vec3b>& buffer, const std::string& filename) {
    static int fileCounter = 0;

    std::stringstream ss;
    ss << filename << fileCounter++ << ".bin";
    std::string uniqueFilename = ss.str();

    std::ofstream outputFile(uniqueFilename, std::ios::binary);
    if (!outputFile.is_open()) {
        logger << "Unable to open the file: " << uniqueFilename << std::endl;
        return;
    }

    for (size_t i = 0; i < buffer.size(); ++i) {
        int count = 1;
        cv::Vec3b prev_pixel = buffer[i];
        for (size_t j = i + 1; j < buffer.size(); ++j) {
            if (buffer[j] == prev_pixel && count < 255) {
                count++;
            }
            else {
                outputFile.write(reinterpret_cast<const char*>(&count), sizeof(unsigned char));
                outputFile.write(reinterpret_cast<const char*>(&prev_pixel), sizeof(cv::Vec3b));
                i = j - 1;
                break;
            }
        }
        if (i == buffer.size() - 1) {
            count = 1;
            outputFile.write(reinterpret_cast<const char*>(&count), sizeof(unsigned char));
            outputFile.write(reinterpret_cast<const char*>(&prev_pixel), sizeof(cv::Vec3b));
        }
    }

    outputFile.close();
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