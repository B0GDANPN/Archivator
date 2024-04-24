//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_WAWHEADER_H
#define ARCHIVATOR_WAWHEADER_H
#pragma once
#include <opencv2/opencv.hpp>
#include <cstdint>

struct WAVHeader {
    char chunkID[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char subchunk2ID[4];
    uint32_t subchunk2Size;
};
#endif //ARCHIVATOR_WAWHEADER_H
