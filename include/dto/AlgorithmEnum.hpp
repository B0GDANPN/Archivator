//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_ALGORITHMENUM_HPP
#define ARCHIVATOR_ALGORITHMENUM_HPP
/**
 * @brief Enum identifying the compression algorithm type.
 *
 * Used to select the appropriate algorithm based on file type or extension.
 */
enum class AlgorithmEnum {
    QUANTIZATION,  ///< Video compression via frame quantization (e.g., for .mp4 files).
    FRACTAL,       ///< Image compression via fractal algorithm (e.g., for .bmp/.jpg images).
    FLAC,          ///< Audio compression using FLAC-like algorithm (e.g., for .wav files).
    HUFFMAN,       ///< Generic data compression using Huffman coding (default or text files).
    ERROR          ///< Indicates an unsupported or unrecognized algorithm type.
};
#endif
