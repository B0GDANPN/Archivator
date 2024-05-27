#ifndef ARCHIVATOR_QUANTIZATIONALGO_H
#define ARCHIVATOR_QUANTIZATIONALGO_H
#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <string>
#include "../../src/dto/MatrixInfo.h"
#include "../../src/dto/Rect.h"
#include "../../src/dto/CommonInformation.h"
#include "../../src/controller/Controller.h"
#include "Profiler.hpp"


size_t NOIZES = 2500000;
size_t NOIZES_PER_SUBFRAME = 500000;
size_t SOLID_DIFFERENCE = 100;

const size_t SPLIT_DEPTH = 16;
const size_t SUBFRAME_DIFFERENCE = 15;
const size_t CACHED_FRAME_DIFFERENCE = 10;
const size_t COLOR_CHANNELS = 3;

namespace fs = std::filesystem;

class QuantizationAlgo : public IController {
    void sendCommonInformation(const CommonInformation &commonInformation) override {
        sendMessage("QuantizationAlgo{ ");
        IController::sendCommonInformation(commonInformation);
        sendMessage("}\n");
    }

    void sendErrorInformation(const std::string &error) override {
        IController::sendErrorInformation("QuantizationAlgo{ " + error + "}\n");
    }

    static void profile() {
        Profiler profiler;
        CompressParams params = profiler.avalize("bob.mp4");

        NOIZES = params.NOIZES;
        NOIZES_PER_SUBFRAME = params.NOIZES_PER_SUBFRAME;
        SOLID_DIFFERENCE = params.SOLID_DIFFERENCE;

        //logger << "params: \nNoizes: " << NOIZES << "\n"
        //    << "Noizes per subframe: " << NOIZES_PER_SUBFRAME << "\n"
        //    << "Solid difference: " << SOLID_DIFFERENCE << "\n";
    }

    void sendGlobalParams() {
        std::stringstream oss;
        oss << "Params: " << SPLIT_DEPTH << " "
            << NOIZES << " "
            << NOIZES_PER_SUBFRAME << '\n';
        std::string str = oss.str();
        sendMessage(str);
    }

    std::vector<cv::Vec3b> decodeBufferFromFile(const std::string &filename) {
        std::vector<cv::Vec3b> decodedBuffer;
        std::ifstream inputFile(filename, std::ios::binary);
        if (!inputFile.is_open()) {
            sendErrorInformation("Failed to open the file while decoding from buffer: " + filename + "\n");
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

    MatrixInfo readNextMatrixAndPoint(const std::string &filename) {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs.is_open()) {
            sendErrorInformation("Failed to open file for reading1!1!!11!1\n");
            exit(0);
        }

        if (ifs.eof()) {
            return {};
        }

        cv::Vec3b scalar;
        bool isSolid;
        cv::Size size;
        cv::Point point;
        size_t dataSize;

        ifs.read(reinterpret_cast<char *>(&scalar), sizeof(cv::Vec3b));
        ifs.read(reinterpret_cast<char *>(&isSolid), sizeof(bool));
        ifs.read(reinterpret_cast<char *>(&size), sizeof(cv::Size));
        ifs.read(reinterpret_cast<char *>(&point), sizeof(cv::Point));


        // logger << "DATASIZE: " << dataSize << std::endl;
        // logger << "SIZE: " << size << std::endl;
        // logger << "POINT: " << point << std::endl;


        std::vector<uchar> decompressedData;

        if (!isSolid) {
            ifs.read(reinterpret_cast<char *>(&dataSize), sizeof(dataSize));
            std::vector<uchar> compressedData(dataSize);
            ifs.read(reinterpret_cast<char *>(compressedData.data()), dataSize);
            decompressedData = decompressMat(compressedData);
        } else {
            fill(scalar, size, decompressedData);
        }

        MatrixInfo matrixInfo;
        matrixInfo.data = std::move(decompressedData);
        matrixInfo.size = size;
        matrixInfo.point = point;
        matrixInfo.dataSize = size.height * size.width * COLOR_CHANNELS;

        std::string tempFilename = filename + ".tmp";
        std::ofstream ofs(tempFilename, std::ios::binary);

        ofs << ifs.rdbuf();
        ifs.close();
        ofs.close();

        remove(filename.c_str());
        rename(tempFilename.c_str(), filename.c_str());

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

    std::vector<unsigned char> compressMat(const cv::Mat &image) {
        if (image.empty()) {
            sendErrorInformation("Got empty image to compress!!!11!11!1\n");
            return {};
        }

        std::vector<uchar> compressedData;
        for (int row = 0; row < image.rows; ++row) {
            int count = 1;
            for (int col = 1; col < image.cols; ++col) {
                if (isSimilar(image.at<cv::Vec3b>(row, col), image.at<cv::Vec3b>(row, col - 1)) && count < 255) {
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

    void
    writeMatricesAndPoints(const std::vector<std::pair<cv::Point, cv::Mat>> &matrices, const std::string &filename) {
        std::ofstream ofs(filename, std::ios::binary | std::ios::app);

        if (!ofs.is_open()) {
            sendErrorInformation("Failed to open file for writing!11!111!\n");
            return;
        }

        for (const auto &matrix_info: matrices) {

            std::pair<cv::Vec3b, bool> isSolid = areSolid(matrix_info.second);
            cv::Size size = matrix_info.second.size();
            cv::Point point = matrix_info.first;

            ofs.write(reinterpret_cast<const char *>(&isSolid.first), sizeof(cv::Vec3b));
            ofs.write(reinterpret_cast<const char *>(&isSolid.second), sizeof(bool));
            ofs.write(reinterpret_cast<const char *>(&size), sizeof(cv::Size));
            ofs.write(reinterpret_cast<const char *>(&point), sizeof(cv::Point));

            if (!isSolid.second) {
                std::vector<uchar> vec = compressMat(matrix_info.second);
                size_t dataSize = vec.size();
                ofs.write(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
                ofs.write(reinterpret_cast<const char *>(vec.data()), dataSize);
            }
        }

        ofs.close();
    }

    static std::vector<std::pair<cv::Point, cv::Mat>>
    splitMatrices(const cv::Mat &image, const cv::Mat &silents, int threshold) {
        std::vector<std::pair<cv::Point, cv::Mat>> result;
        std::queue<std::pair<cv::Point, cv::Mat>> to_process;
        to_process.emplace(cv::Point(0, 0), silents);
        int counter = 0;

        while (!to_process.empty()) {

            auto top_left = to_process.front().first;
            auto matrix = to_process.front().second;
            to_process.pop();

            if (cv::sum(matrix)[0] > threshold) {
                int width = matrix.cols / 2;
                int height = matrix.rows / 2;

                to_process.emplace(top_left, matrix(cv::Rect(0, 0, width, height)));
                to_process.emplace(top_left + cv::Point(width, 0), matrix(cv::Rect(width, 0, width, height)));
                to_process.emplace(top_left + cv::Point(0, height), matrix(cv::Rect(0, height, width, height)));
                to_process.emplace(top_left + cv::Point(width, height), matrix(cv::Rect(width, height, width, height)));
            } else {
                result.emplace_back(top_left, image(cv::Rect(top_left.x, top_left.y, matrix.cols, matrix.rows)));
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

    void writeBufferToFile(const std::vector<cv::Vec3b> &buffer, const std::string &filename, int threshold = 10) {
        static int fileCounter = 0;

        std::stringstream ss;
        ss << filename << fileCounter++ << ".bin";
        std::string uniqueFilename = ss.str();
        std::ofstream outputFile(uniqueFilename, std::ios::binary);
        if (!outputFile.is_open()) {
            sendErrorInformation("Unable to open the file: \n");
            return;
        }
        for (size_t i = 0; i < buffer.size(); ++i) {
            int count = 1;
            cv::Vec3b prev_pixel = buffer[i];
            for (size_t j = i + 1; j < buffer.size(); ++j) {
                if (isSimilar(prev_pixel, buffer[j], threshold) && count < 255) {
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
    }

    static void fill(const cv::Vec3b &value, const cv::Size &size, std::vector<uchar> &data) {
        size_t numElements = size.width * size.height;  // 3 color channels

        for (size_t i = 0; i < numElements; i++) {
            data.push_back(value[0]);
            data.push_back(value[1]);
            data.push_back(value[2]);
        }

    }

    static cv::Vec3b scalarToVec3b(const cv::Scalar &scalar) {
        return {static_cast<uchar>(scalar[0] + 0.5),
                static_cast<uchar>(scalar[1] + 0.5),
                static_cast<uchar>(scalar[2] + 0.5)};
    }

    static bool isSimilar(const cv::Vec3b &pixel1, const cv::Vec3b &pixel2, int threshold = CACHED_FRAME_DIFFERENCE) {
        int distance = 0;
        for (int i = 0; i < 3; ++i) {
            distance += static_cast<int>(pixel1[i]) - static_cast<int>(pixel2[i]);
        }
        return distance <= threshold;
    }

    static std::pair<cv::Vec3b, bool> areSolid(const cv::Mat &matrix, double threshold = SOLID_DIFFERENCE) {
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


public:
    explicit QuantizationAlgo(bool isTextOutput, const std::string &outputFile, std::ostringstream &shared_oss)
            : IController(isTextOutput, outputFile, shared_oss) {}


    void encode(const std::string &inputFilename) {
        auto start = std::chrono::high_resolution_clock::now();
        int sizeInput = static_cast<int>(getFilesize(inputFilename));
        size_t lastSlashPos = inputFilename.find_last_of('/');
        std::string dirName =
                lastSlashPos != std::string::npos ? inputFilename.substr(lastSlashPos + 1) : inputFilename;
        size_t pos = dirName.rfind('.');
        dirName = dirName.substr(0, pos);// путь сохранения
        fs::create_directory("storageEncoded/" + dirName);

        std::string framedata = "storageEncoded/" + dirName + "/framedata.csv";
        std::string matdata = "storageEncoded/" + dirName + "/matdata.bin";
        std::string subframedata = "storageEncoded/" + dirName + "/subframe";
        fs::create_directory(subframedata);
        sendMessage("Encoding... video\n");
        std::ofstream ofs(framedata);
        cv::VideoCapture cap(inputFilename);
        double frameCount = cap.get(cv::CAP_PROP_FRAME_COUNT);
        double frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double channelCount = 3; // Цветное видео
        double pixelSize = 1; // 8-битное цветное видео
        if (!cap.isOpened()) {
            sendErrorInformation("Failed to open the video file.");
            return;
        }

        cv::Mat frame, frame2, dst;
        size_t start_scene = 0;
        size_t end_scene = 0;

        cap.read(frame);
        cap.read(frame2);

        size_t sum;
        bool end_flag = false;
        sendMessage("Size frame: " + std::to_string(frameHeight * frameWidth * channelCount * pixelSize) + "\n");
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
            sendMessage("Frames from" + std::to_string(start_scene) + " to " + std::to_string(end_scene)
                        + " Count of mats " + std::to_string(matricies.size()) + '\n');

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
            const std::string &filename = subframedata;// std::string subframedata= "sampleZip/subframe/"
            sendMessage("Size of buffer: " + std::to_string(subFrameBuffer.size() * sizeof(cv::Vec3b) / 1024) + '\n');
            writeBufferToFile(subFrameBuffer, filename);
        }
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count());
        uintmax_t sizeOutput1 = getFilesize(framedata);
        uintmax_t sizeOutput2 = getFilesize(subframedata);
        uintmax_t sizeOutput3 = getFilesize(matdata);

        int sizeOutput = static_cast<int>(sizeOutput1 + sizeOutput2 + sizeOutput3);
        double ratio = static_cast<double>(sizeOutput) / sizeInput;
        auto info = CommonInformation(ratio, duration, sizeInput, sizeOutput);
        sendGlobalParams();
        sendCommonInformation(info);
    }


    // need /../../..sample.mp4
    //storageEncoded/sample
    //const std::string& framedata = "sample/framedata.csv",
    //                       const std::string& matdata = "sample/matdata.bin",
    //                       const std::string& subframedata = "sample/subframe/"
    void decode(const std::string &dirName) {
        std::string framedata = "storageEncoded/" + dirName + "/framedata.csv";
        std::string matdata = "storageEncoded/" + dirName + "/matdata.bin";
        std::string subframedata = "storageEncoded/" + dirName + "/subframe";
        fs::path outputPath = "storageDecoded/" + dirName + ".mp4";// путь сохранения

        uintmax_t sizeInput1 = getFilesize(framedata);
        uintmax_t sizeInput2 = getFilesize(subframedata);
        uintmax_t sizeInput3 = getFilesize(matdata);

        int sizeInput = static_cast<int>(sizeInput1 + sizeInput2 + sizeInput3);
        auto start = std::chrono::high_resolution_clock::now();
        sendMessage("Decoding video\n");
        std::ifstream frame(framedata);

        int subFrameDataIndex = 0;
        int rows = -1;
        int cols = -1;
        char delimiter;
        std::string line;
        if (std::getline(frame, line)) {
            std::istringstream ss(line);
            if (ss >> rows >> delimiter >> cols && delimiter == ',') {
                sendMessage("Decoding: reading succes\n");
            } else {
                sendErrorInformation("Decoding: Not enough data\n");
                exit(1);
            }
        } else {
            sendErrorInformation("Decoding: reading failed\n");
            exit(-2);
        }
        sendMessage(std::to_string(rows) + ' ' + std::to_string(cols) + '\n');
        cv::Mat main(rows, cols, CV_8UC3, cv::Scalar(0, 0, 255));

        cv::VideoWriter videoWriter(outputPath.string(), cv::VideoWriter::fourcc('H', '2', '5', '6'), 30,
                                    cv::Size(cols, rows));
        if (!videoWriter.isOpened()) {
            sendErrorInformation("Unable to open the VideoWriter\n");
            return;
        }

        int from, to, matrixCount;
        int scene = 0;
        while (std::getline(frame, line)) {
            std::istringstream ss(line);
            sendMessage("Decoding scene: " + std::to_string(++scene) + '\n');
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
                        sendErrorInformation("Error: Not enough matrix data \n");
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
                sendErrorInformation("Decoding: can't read framedata.bin\n");
                exit(4);
            }
        }
        int sizeOutput = static_cast<int>(getFilesize(outputPath.string()));
        double ratio = static_cast<double>(sizeOutput) / sizeInput;
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count());
        auto info = CommonInformation(ratio, duration, sizeInput, sizeOutput);
        sendGlobalParams();
        sendCommonInformation(info);
    }

};


#endif ARCHIVATOR_QUANTIZATIONALGO_H
