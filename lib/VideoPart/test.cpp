#include <opencv2/opencv.hpp>
#include <vector>
#include "test.h"
#include "logger.h"
#include "compress.h"
#include "decompress.h"
#include "codec.h"

void testDecompressMat(bool needShow = false) {
    std::string filename = "testData/chad.jpg";
    cv::Mat image = cv::imread(filename);

    logger << "Start decompress..." << std::endl;
    if (image.empty()) {
        logger << "Failed to load image: " << filename << std::endl;
        return;
    }

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

    cv::Point point1(0, 0);
    cv::Point point2(30, 30);
    cv::Point point3(100, 90);
    cv::Mat submatrix1 = image(cv::Rect(0, 0, 10, 15));
    cv::Mat submatrix2 = image(cv::Rect(30, 30, 20, 45));
    cv::Mat submatrix3 = image(cv::Rect(100, 90, 50, 5));

    pointsAndSubmatrices.push_back(std::make_pair(point1, submatrix1));
    pointsAndSubmatrices.push_back(std::make_pair(point2, submatrix2));
    pointsAndSubmatrices.push_back(std::make_pair(point3, submatrix3));

    std::vector<cv::Vec3b> subFrameBuffer;
    writeNumbersExcludingSubmatrices(image, pointsAndSubmatrices, subFrameBuffer);
    writeBufferToFile(subFrameBuffer, "testData/", SUBFRAME_DIFFERENCE);
    logger << "Succesfully written buffer to file" << std::endl;
}

void testReadBufferFromFile() {
    logger << "Start decode buffer from file..." << std::endl;
    decodeBufferFromFile("testData/0.bin");
    logger << "Succesfully decode file from buffer" << std::endl;
}

void diagnostic() {
    testCompressMat();
    testDecompressMat();
    testWriteMats();
    testReadingMats();
    testWriteBuffer();
    testReadBufferFromFile();

}