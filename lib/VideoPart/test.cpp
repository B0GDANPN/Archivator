#include <opencv2/opencv.hpp>
#include <vector>
#include "test.h"
#include "logger.h"
#include "compress.h"
#include "decompress.h"
#include "codec.h"

int width = -1;
int height = -1;
size_t byte_size = -1;

void testDecompressMat(bool needShow = true) {
    std::string filename = "testData/chad.jpg";
    cv::Mat image = cv::imread(filename);

    logger << "Start decompress..." << std::endl;
    if (image.empty()) {
        logger << "Failed to load image: " << filename << std::endl;
        return;
    }

    width = image.rows;
    height = image.cols;
    std::vector<uchar> encoded = compressMat(image);
    std::vector<uchar> decoded = decompressMat(encoded);

    cv::Mat decompressedImage;

    decompressedImage = cv::Mat(image.size(), CV_8UC3, decoded.data());

    if (needShow) {
        cv::imshow("Decompressed Image", decompressedImage);
        cv::waitKey(0);
    }
    logger << "Decompress succesfully passed!" << std::endl;
}

void testCompressMat() {
	std::string filename = "testData/chad.jpg";
    cv::Mat image = cv::imread(filename);
    
    logger << "Start compress..." << std::endl;

    if (image.empty()) {
        logger << "\tFailed to load image: " << filename << std::endl;
        return;
    }
    
    logger << "\tImage size: " << image.total() * image.elemSize() << " bytes" << std::endl;
    std::vector<uchar> encoded = compressMat(image);
    logger << "\tCompressed data size: " << sizeof(encoded[0]) * encoded.size() << " bytes" << std::endl;
    logger << "\tRation: " << (1.0f - static_cast<float>(sizeof(encoded[0]) * encoded.size()) / (image.total() * image.elemSize())) * 100 
        << "%"
        << std::endl;
    logger << "Compress succefully passed!" << std::endl;
}

void testWriteMats() {

    std::string filename = "testData/chad.jpg";
    cv::Mat image = cv::imread(filename);

    logger << "Start writing cached mats..." << std::endl;

    if (image.empty()) {
        logger << "\tFailed to load image: " << filename << std::endl;
        return;
    }

    std::vector<std::pair<cv::Point, cv::Mat>> matrix;
    std::string matfilename = "testData/cachedChad.bin";

    matrix.push_back(std::make_pair(cv::Point(0, 0), image));
    writeMatricesAndPoints(matrix, matfilename);
    logger << "Matrix succesfully written!" << std::endl;
}

void testReadingMats() {
    logger << "Start reading matdata from file...";
    std::string filename = "testData/cachedChad.bin";
    MatrixInfo c = readNextMatrixAndPoint(filename);
    logger << "\n\tMatrix info: " << std::endl
        << "\t\tResolution: " << c.size << std::endl
        << "\t\tPlace point: " << c.point << std::endl
        << "\t\tByte size:" << c.dataSize << std::endl
        << "Reading matrix data successuly passed" << std::endl;
}

void testWriteBuffer() {
    logger << "Start writting buffer" << std::endl;
    std::string filename = "testData/chad.jpg";
    cv::Mat image = cv::imread(filename);

    if (image.empty()) {
        logger << "\tFailed to load image: " << filename << std::endl;
        return;
    }

    std::vector<std::pair<cv::Point, cv::Mat>> pointsAndSubmatrices;

    std::vector<cv::Vec3b> subFrameBuffer;
    writeNumbersExcludingSubmatrices(image, pointsAndSubmatrices, subFrameBuffer);
    byte_size =  subFrameBuffer.size() * sizeof(subFrameBuffer[0]);
    writeBufferToFile(subFrameBuffer, "testData/", SUBFRAME_DIFFERENCE);
    logger << "Succesfully written buffer to file" << std::endl;
}

void testReadBufferFromFile() {
    logger << "Start decode buffer from file..." << std::endl;
    std::vector<cv::Vec3b> decodedBuffer = decodeBufferFromFile("testData/0.bin");
    if (decodedBuffer.size() * sizeof(decodedBuffer[0])) {
        std::cout << "Buffer succesfully decoded!" << std::endl;
    }
    else {
        std::cout << "Something went wrong during decoding buffer from file :c" << std::endl;
    }
}

void testSimilar() {
    std::string filename = "testData/white.jpg";
    cv::Mat image = cv::imread(filename);

    if (image.empty()) {
        logger << "\tFailed to load image: " << filename << std::endl;
        return;
    }

    std::pair<cv::Vec3b, bool> data = areSolid(image, 10);
    if (data.second) {
        std::cout << "Solid test succesfully passed!" << std::endl;
    }
    else {
        std::cout << "Solid NE passed :C" << std::endl;
    }
}

void diagnostic() {
    testCompressMat();
    testDecompressMat();
    testWriteMats();
    testReadingMats();
    testWriteBuffer();
    testReadBufferFromFile();
    testSimilar();
}