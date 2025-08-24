//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_WAW_HEADER_HPP
#define ARCHIVATOR_WAW_HEADER_HPP

#include <opencv2/opencv.hpp>

struct WavHeader {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char subchunk2_id[4];
    uint32_t subchunk2_size;
};
#endif