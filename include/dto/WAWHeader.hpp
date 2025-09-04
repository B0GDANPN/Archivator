//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_WAW_HEADER_HPP
#define ARCHIVATOR_WAW_HEADER_HPP

#include <opencv2/opencv.hpp>
/**
 * @brief Header of a WAV audio file (PCM format).
 *
 * This struct maps to the 44-byte WAV file header for PCM audio. It is used to read and validate WAV files
 * for the FLAC algorithm. **Note:** Only 16-bit PCM, mono audio is supported by this application.
 */
struct WavHeader {
    char     chunk_id[4];     ///< File chunk ID (should be "RIFF").
    uint32_t chunk_size;      ///< Size of the WAV file minus 8 bytes.
    char     format[4];       ///< Format field (should be "WAVE").
    char     subchunk1_id[4]; ///< Subchunk1 ID (should be "fmt ").
    uint32_t subchunk1_size;  ///< Size of the fmt chunk (16 for PCM).
    uint16_t audio_format;    ///< Audio format code (1 for PCM).
    uint16_t num_channels;    ///< Number of audio channels (1 for mono, 2 for stereo, etc.).
    uint32_t sample_rate;     ///< Sample rate (samples per second).
    uint32_t byte_rate;       ///< Byte rate (`sample_rate * num_channels * bits_per_sample/8`).
    uint16_t block_align;     ///< Block alignment (`num_channels * bits_per_sample/8`).
    uint16_t bits_per_sample; ///< Bits per sample (e.g., 16 for PCM16).
    char     subchunk2_id[4]; ///< Subchunk2 ID (should be "data").
    uint32_t subchunk2_size;  ///< Size of the audio data in bytes.
};
#endif