//
// Created by bogdan on 24.04.24.
//

#ifndef ARCHIVATOR_ICONTROLLER_H
#define ARCHIVATOR_ICONTROLLER_H

#include "../dto/CommonInformation.h"
#include <string>
#include <fstream>
#include <sstream>
#include <utility>

#pragma once

class IController {
public:
    virtual ~IController() = default;

    bool isTextOutput;
    std::string outputFile;
    std::ostringstream &oss;

    explicit IController(bool isTextOutput, std::string outputFile, std::ostringstream &ref_oss) : isTextOutput(
            isTextOutput),
                                                                                                   outputFile(std::move(
                                                                                                           outputFile)),
                                                                                                   oss(ref_oss) {}

    void sendMessage(const std::string &message) const {
        if (isTextOutput) {
            oss << message;
        } else {
            std::ofstream file(outputFile, std::ios::app);

            if (file.is_open()) {
                file << message;
                file.close();
            } else {
                exit(-1);
            }
        }
    };

    virtual void sendCommonInformation(const CommonInformation &commonInformation) {
        oss << "Compression ratio: 1:" << commonInformation.compressionRatio << '\n' <<
            "Time: " << commonInformation.time << "ms \n" <<
            "Size input data: " << commonInformation.sizeInputData << " bytes\n" <<
            "Size output data: " << commonInformation.sizeOutputData << " bytes\n";
        //std::string tmp = oss.str();
        //sendMessage(tmp);
    };

    virtual void sendErrorInformation(const std::string &error) {
        sendMessage(error);
    };

    static std::streamsize getFilesize(const std::string &filename) {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        if (in) {
            std::streamsize size = in.tellg();
            in.close();
            return size;
        }
        return -1;
    }

};

#endif ARCHIVATOR_ICONTROLLER_H
