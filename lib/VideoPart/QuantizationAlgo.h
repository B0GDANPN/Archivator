#ifndef ARCHIVATOR_QUANTIZATIONALGO_H
#define ARCHIVATOR_QUANTIZATIONALGO_H

//
// Created by bogdan on 15.04.24.
//

#pragma once

#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <chrono>
#include "../../src/controller/Controller.h"
#include <sstream>
#include <opencv2/opencv.hpp>
#include <string>
#include "logger.h"

struct MatrixInfo {
    cv::Size size{};
    cv::Point point{};
    std::vector<unsigned char> data;
    size_t dataSize{};
};

const size_t SPLIT_DEPTH = 16;
const size_t NOIZES = 3000000;
const size_t NOIZES_PER_SUBFRAME = 500000;

struct Rect {
    size_t a;
    size_t b;
    size_t c;
    size_t d;

    explicit Rect(cv::Rect rect) {
        a = rect.x;
        b = rect.y;
        c = rect.x + rect.width;
        d = rect.y + rect.height;
    }
};

class QuantizationAlgo {
    static Logger logger;
    //logger.switchToConsole();
    //    logger.switchToFile();
public:
    static void encode(const std::string &inputFilename, const std::string &framedata, const std::string &matdata,
                       const std::string& subframedata, const std::string &outputFilename) {
        logger << "Encoding..." << std::endl;
        std::ofstream ofs(framedata);//std::string framedata="sampleZip/framedata.csv"
        cv::VideoCapture cap(inputFilename);//std::string &inputFilename="sample2.mp4"

        if (!cap.isOpened()) {
            logger << "Failed to open the video file." << std::endl;
            return -1;
        }

        cv::Mat frame, frame2, dst;
        size_t start_scene = 0;
        size_t end_scene = 0;

        cap.read(frame);
        cap.read(frame2);

        size_t sum;
        bool end_flag = false;

        logger << frame.size << std::endl;
        ofs << frame.rows << "," << frame.cols << std::endl;

        while (!frame.empty()) {

            while (true) {
                cap.read(frame2);
                if (frame2.empty()) {
                    break;
                    end_flag = true;
                }
                cv::subtract(frame, frame2, dst);
                sum = cv::sum(dst)[0];
                end_scene++;
                if (sum > NOIZES) break;
            }

            if (start_scene == cap.get(cv::CAP_PROP_FRAME_COUNT) - 1) break;

            auto matricies = splitMatrices(frame, dst, NOIZES_PER_SUBFRAME);

            logger << "Frames from " << start_scene
                   << " to " << end_scene
                   << " Count of mats " << matricies.size()
                   << std::endl;

            ofs << start_scene << "," << end_scene << "," << matricies.size() << std::endl;

            writeMatricesAndPoints(matricies, matdata);// std::string matdata="sampleZip/matdata.bin"

            cap.set(cv::CAP_PROP_POS_FRAMES, start_scene);

            std::vector<cv::Vec3b> subFrameBuffer;
            while (start_scene != end_scene) {
                cap.read(frame);
                writeNumbersExcludingSubmatrices(frame, matricies, subFrameBuffer);
                start_scene++;
            }
            cap.read(frame);
            std::string filename = subframedata;// std::string subframedata= "sampleZip/subframe/"
            logger << "Size of buffer: " << subFrameBuffer.size() * sizeof(cv::Vec3b) / 1024 << std::endl;
            writeBufferToFile(subFrameBuffer, filename);
        }
        logger << "Time for encoding: " << logger.elapsedTime() << " ms" << std::endl;
        logger << "Params: " << SPLIT_DEPTH << " "
               << NOIZES << " "
               << NOIZES_PER_SUBFRAME << std::endl;
    }

    //const std::string& framedata = "sampleZip/framedata.csv",
    //                       const std::string& matdata = "sampleZip/matdata.bin",
    //                       const std::string& subframedata = "sampleZip/subframe/"
    static void decode(const std::string &framedata,
                       const std::string &matdata,
                       const std::string &subframedata, const std::string &output) {
        logger << "Decoding" << std::endl;
        std::ifstream frame(framedata);

        int subFrameDataIndex = 0;
        int rows = -1;
        int cols = -1;
        char delimiter;
        std::string line;
        if (std::getline(frame, line)) {
            std::istringstream ss(line);
            if (ss >> rows >> delimiter >> cols && delimiter == ',') {
                logger << "Decoding: reading succes" << std::endl;
            } else {
                logger << "Decoding: Not enough data" << std::endl;
                exit(1);
            }
        } else {
            logger << "Decoding: reading failed" << std::endl;
            exit(-2);
        }

        logger << rows << " " << cols << std::endl;
        cv::Mat main(rows, cols, CV_8UC3, cv::Scalar(0, 0, 255));

        cv::VideoWriter videoWriter(output, cv::VideoWriter::fourcc('H', '2', '5', '6'), 30, cv::Size(cols, rows));
        if (!videoWriter.isOpened()) {
            logger << "Unable to open the VideoWriter" << std::endl;
            return;
        }

        int from, to, matrixCount;
        int scene = 0;
        while (std::getline(frame, line)) {
            std::istringstream ss(line);
            logger << "Decoding scene: " << ++scene << std::endl;
            if (ss >> from >> delimiter >> to >> delimiter >> matrixCount && delimiter == ',') {
                int frames = to - from;
                std::vector<cv::Rect> reserved;
                for (int i = 0; i < matrixCount; i++) {
                    MatrixInfo c = readNextMatrixAndPoint(matdata);
                    cv::Mat sub(c.size, CV_8UC3, c.data.data());
                    cv::Rect roi(c.point, sub.size());
                    reserved.push_back(roi);
                    sub.copyTo(main(roi));
                    if (c.data.empty()) {
                        logger << "Error: Not enough matrix data " << std::endl;
                        exit(3);
                    }
                }
                int pixelIndex = 0;
                std::vector<cv::Vec3b> pixels = decodeBufferFromFile(
                        subframedata + std::to_string(subFrameDataIndex) + ".bin");
                subFrameDataIndex++;
                for (int k = 0; k < frames; k++) {
                    for (int i = 0; i < main.rows; ++i) {
                        for (int j = 0; j < main.cols; ++j) {
                            bool isInDeprecated = false;
                            for (const auto &rect: reserved) {
                                if (rect.contains(cv::Point(j, i))) {
                                    isInDeprecated = true;
                                    break;
                                }
                            }
                            if (!isInDeprecated && pixelIndex < pixels.size()) {
                                main.at<cv::Vec3b>(i, j) = pixels[pixelIndex++];
                            }
                        }
                    }
                    videoWriter.write(main);
                }
            } else {
                logger << "Decoding: cann't read framedata.bin" << std::endl;
                exit(4);
            }
        }
        logger << "Time for decoding: " << logger.elapsedTime() << " ms" << std::endl;
        logger << "Params: " << SPLIT_DEPTH << " "
               << NOIZES << " "
               << NOIZES_PER_SUBFRAME << std::endl;
    }

private:

    static std::vector<cv::Vec3b> decodeBufferFromFile(const std::string &filename) {
        std::vector<cv::Vec3b> decodedBuffer;
        std::ifstream inputFile(filename, std::ios::binary);
        if (!inputFile.is_open()) {
            logger << "Failed to open the file while decoding from buffer: " << filename << std::endl;
            return decodedBuffer;
        }

        unsigned char count;
        cv::Vec3b pixel;
        while (inputFile.read(reinterpret_cast<char *>(&count), sizeof(unsigned char))) {
            if (count == '\0') {
                inputFile.seekg(-1, std::ios_base::cur);
                inputFile.putback(1);
                break;
            }

            inputFile.read(reinterpret_cast<char *>(&pixel), sizeof(cv::Vec3b));
            for (int i = 0; i < count; ++i) {
                decodedBuffer.push_back(pixel);
            }
        }

        std::ofstream outputFile(filename + ".temp", std::ios::binary);
        outputFile << inputFile.rdbuf();
        inputFile.close();
        outputFile.close();
        std::remove(filename.c_str());
        std::rename((filename + ".temp").c_str(), filename.c_str());

        return decodedBuffer;
    }

    static MatrixInfo readNextMatrixAndPoint(const std::string &filename) {
        std::ifstream ifs(filename, std::ios::binary);

        if (!ifs.is_open()) {
            logger << "Failed to open file " << filename << " for reading1!1!!11!1" << std::endl;
            exit(0);
        }

        cv::Size size;
        ifs.read(reinterpret_cast<char *>(&size), sizeof(cv::Size));
        if (ifs.eof()) {
            return {};
        }

        cv::Point point;
        ifs.read(reinterpret_cast<char *>(&point), sizeof(cv::Point));

        size_t dataSize;
        ifs.read(reinterpret_cast<char *>(&dataSize), sizeof(dataSize));
        std::vector<unsigned char> compressedData(dataSize);
        ifs.read(reinterpret_cast<char *>(compressedData.data()), dataSize);

        std::string tempFilename = filename + ".tmp";
        std::ofstream ofs(tempFilename, std::ios::binary);

        ofs << ifs.rdbuf();
        ifs.close();
        ofs.close();

        remove(filename.c_str());
        rename(tempFilename.c_str(), filename.c_str());
        std::vector<unsigned char> decompressedData = decompressMat(compressedData);
        cv::Mat e(size, CV_8UC3, decompressedData.data());
        MatrixInfo matrixInfo;
        matrixInfo.data = std::move(decompressedData);
        matrixInfo.size = size;
        matrixInfo.point = point;
        matrixInfo.dataSize = dataSize;
        return matrixInfo;
    }

    static std::vector<unsigned char> decompressMat(const std::vector<unsigned char> &compressedData) {
        std::vector<unsigned char> decompressedData;
        for (size_t i = 0; i < compressedData.size(); i += 4) {
            int count = compressedData[i];
            unsigned char blue = compressedData[i + 1];
            unsigned char green = compressedData[i + 2];
            unsigned char red = compressedData[i + 3];
            for (int j = 0; j < count; ++j) {
                decompressedData.push_back(blue);
                decompressedData.push_back(green);
                decompressedData.push_back(red);
            }
        }
        return decompressedData;
    }

    static std::vector<unsigned char> compressMat(const cv::Mat &image) {
        if (image.empty()) {
            logger << "Got empty image to compress!!!11!11!1" << std::endl;
            return {};
        }

        std::vector<unsigned char> compressedData;
        for (int row = 0; row < image.rows; ++row) {
            int count = 1;
            for (int col = 1; col < image.cols; ++col) {
                if (image.at<cv::Vec3b>(row, col) == image.at<cv::Vec3b>(row, col - 1)) {
                    count++;
                } else {
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

    static void
    writeMatricesAndPoints(const std::vector<std::pair<cv::Point, cv::Mat>> &matrices, const std::string &filename) {
        std::ofstream ofs(filename, std::ios::binary | std::ios::app);

        if (!ofs.is_open()) {
            logger << "Failed to open file for writing!11!111!" << std::endl;
            return;
        }

        for (const auto &matrix_info: matrices) {
            cv::Size size = matrix_info.second.size();
            ofs.write(reinterpret_cast<const char *>(&size), sizeof(cv::Size));

            cv::Point point = matrix_info.first;
            ofs.write(reinterpret_cast<const char *>(&point), sizeof(cv::Point));

            std::vector<unsigned char> vec = compressMat(matrix_info.second);
            size_t dataSize = vec.size();
            ofs.write(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
            ofs.write(reinterpret_cast<const char *>(vec.data()), dataSize);
        }

        ofs.close();
    }

    static std::vector<std::pair<cv::Point, cv::Mat>> splitMatrices(cv::Mat image, cv::Mat silents, int threshold) {
        std::vector<std::pair<cv::Point, cv::Mat>> result;
        std::queue<std::pair<cv::Point, cv::Mat>> to_process;
        to_process.push({cv::Point(0, 0), silents});
        int counter = 0;

        while (!to_process.empty()) {

            auto top_left = to_process.front().first;
            auto matrix = to_process.front().second;
            to_process.pop();

            if (cv::sum(matrix)[0] > threshold) {
                int width = matrix.cols / 2;
                int height = matrix.rows / 2;

                to_process.push({top_left, matrix(cv::Rect(0, 0, width, height))});
                to_process.push({top_left + cv::Point(width, 0), matrix(cv::Rect(width, 0, width, height))});
                to_process.push({top_left + cv::Point(0, height), matrix(cv::Rect(0, height, width, height))});
                to_process.push({top_left + cv::Point(width, height), matrix(cv::Rect(width, height, width, height))});
            } else {
                result.push_back({top_left, image(cv::Rect(top_left.x, top_left.y, matrix.cols, matrix.rows))});
            }
            if (counter++ == SPLIT_DEPTH) return result;
        }

        return result;
    }

    static void writeNumbersExcludingSubmatrices(const cv::Mat &big_matrix, const std::vector<std::pair<cv::Point,
            cv::Mat>> &submatrices, std::vector<cv::Vec3b> &result) {

        for (int i = 0; i < big_matrix.rows; ++i) {
            for (int j = 0; j < big_matrix.cols; ++j) {
                cv::Point current_point(j, i);

                bool is_inside_submatrix = false;
                for (const auto &submatrix: submatrices) {
                    const cv::Point &submatrix_origin = submatrix.first;
                    const cv::Mat &submatrix_data = submatrix.second;

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

    static void writeBufferToFile(const std::vector<cv::Vec3b> &buffer, const std::string &filename) {
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
                } else {
                    outputFile.write(reinterpret_cast<const char *>(&count), sizeof(unsigned char));
                    outputFile.write(reinterpret_cast<const char *>(&prev_pixel), sizeof(cv::Vec3b));
                    i = j - 1;
                    break;
                }
            }
            if (i == buffer.size() - 1) {
                count = 1;
                outputFile.write(reinterpret_cast<const char *>(&count), sizeof(unsigned char));
                outputFile.write(reinterpret_cast<const char *>(&prev_pixel), sizeof(cv::Vec3b));
            }
        }

        outputFile.close();
    }


    static void insertMatrix(cv::Mat &bigMatrix, const cv::Mat &smallMatrix, cv::Point position) {
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

};

#endif //ARCHIVATOR_QUANTIZATIONALGO_H
