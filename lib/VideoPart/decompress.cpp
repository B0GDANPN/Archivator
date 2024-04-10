#include "decompress.h"
#include <fstream>
#include "codec.h"
#include "logger.h"

std::vector<uchar> decompressMat(const std::vector<uchar>& compressedData) {
    std::vector<uchar> decompressedData;
    for (size_t i = 0; i < compressedData.size(); i += 4) {
        int count = compressedData[i];
        uchar blue = compressedData[i + 1];
        uchar green = compressedData[i + 2];
        uchar red = compressedData[i + 3];
        for (int j = 0; j < count; ++j) {
            decompressedData.push_back(blue);
            decompressedData.push_back(green);
            decompressedData.push_back(red);
        }
    }
    return decompressedData;
}

MatrixInfo readNextMatrixAndPoint(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs.is_open()) {
        logger << "Failed to open file " << filename << " for reading1!1!!11!1" << std::endl;
        exit(0);
    }

    cv::Size size;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(cv::Size));
    if (ifs.eof()) {
        return MatrixInfo();
    }

    cv::Point point;
    ifs.read(reinterpret_cast<char*>(&point), sizeof(cv::Point));

    size_t dataSize;
    ifs.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
    std::vector<uchar> compressedData(dataSize);
    ifs.read(reinterpret_cast<char*>(compressedData.data()), dataSize);

    std::string tempFilename = filename + ".tmp";
    std::ofstream ofs(tempFilename, std::ios::binary);

    ofs << ifs.rdbuf();
    ifs.close();
    ofs.close();

    remove(filename.c_str());
    rename(tempFilename.c_str(), filename.c_str());
    std::vector<uchar> decompressedData = decompressMat(compressedData);
    cv::Mat e(size, CV_8UC3, decompressedData.data());
    MatrixInfo matrixInfo;
    matrixInfo.data = std::move(decompressedData);
    matrixInfo.size = size;
    matrixInfo.point = point;
    matrixInfo.dataSize = dataSize;
    return matrixInfo;
}

std::vector<cv::Vec3b> decodeBufferFromFile(const std::string& filename) {
    std::vector<cv::Vec3b> decodedBuffer;
    std::ifstream inputFile(filename, std::ios::binary);
    if (!inputFile.is_open()) {
        logger << "Failed to open the file while decoding from buffer: " << filename << std::endl;
        return decodedBuffer;
    }

    unsigned char count;
    cv::Vec3b pixel;
    while (inputFile.read(reinterpret_cast<char*>(&count), sizeof(unsigned char))) {
        if (count == '\0') {
            inputFile.seekg(-1, std::ios_base::cur);
            inputFile.putback(1);
            break;
        }

        inputFile.read(reinterpret_cast<char*>(&pixel), sizeof(cv::Vec3b));
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


void decode(const std::string& framedata, const std::string& matdata,
    const std::string& subframedata, const std::string& output) {
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
        }
        else {
            logger << "Decoding: Not enough data" << std::endl;
            exit(1);
        }
    }
    else {
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
            std::vector<cv::Vec3b> pixels = decodeBufferFromFile(subframedata + std::to_string(subFrameDataIndex) + ".bin");
            subFrameDataIndex++;
            for (int k = 0; k < frames; k++) {
                for (int i = 0; i < main.rows; ++i) {
                    for (int j = 0; j < main.cols; ++j) {
                        bool isInDeprecated = false;
                        for (const auto& rect : reserved) {
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
        }
        else {
            logger << "Decoding: cann't read framedata.bin" << std::endl;
            exit(4);
        }
    }

}