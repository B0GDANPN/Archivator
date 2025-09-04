//
// Created by bogdan on 15.04.24.

#ifndef ARCHIVATOR_FLACALGO_HPP
#define ARCHIVATOR_FLACALGO_HPP

#include <fstream>
#include <vector>
#include <filesystem>
#include <dto/BitSteam.hpp>
#include <dto/WAWHeader.hpp>
#include <dto/CommonInformation.hpp>

#include <controller/IController.hpp>

/**
 *
 */
static constexpr  int kGlobalSizeBlocks = 16384 * 8; // Size of blocks (INT32_MAX for 1 block)
static constexpr int kGlobalOrder = 25;            // LPC model GLOBAL_ORDER
static constexpr int kGlobalK = 8;                // Rice code parameter

// Linear predictive coding
/**
 * @brief Audio compression algorithm inspired by FLAC (lossless compression for WAV audio).
 *
 * Implements a simplified FLAC-like compression for 16-bit PCM mono WAV files using linear predictive coding (LPC) and Rice coding. The algorithm reads a WAV file, computes LPC coefficients for blocks of audio samples, encodes the coefficients and residuals using Rice codes, and writes a compressed file with ".flac" extension (not standard FLAC format, but a custom binary format).
 */
class FlacAlgo final : public IController {
    /**
     * @brief Internal class implementing Linear Predictive Coding (LPC).
     *
     * The LPC class is used to compute prediction coefficients from audio samples and to predict sample values. It uses the Levinson-Durbin algorithm to determine LPC coefficients for a given block of audio.
     */
    class Lpc {
        std::vector<double> coeffs_; ///< LPC coefficients for the current model (size = kGlobalOrder+1).
    public:
        Lpc() = default;
        /**
         * @brief Initialize LPC with given coefficients.
         * @param coeffs Precomputed coefficient vector.
         */
        explicit Lpc(std::vector<double> coeffs) : coeffs_(std::move(coeffs)) {}

        /**
         * @brief Train (compute) LPC coefficients on a block of audio samples.
         * @param input Vector of audio samples (int16_t) to analyze.
         *
         * Uses the Levinson-Durbin recursion on the autocorrelation of the input signal to calculate `kGlobalOrder` LPC coefficients. The computed coefficients are stored internally and used for prediction.
         */
        void train(const std::vector<int16_t> &input);

        /**
         * @brief Predict the next sample using the LPC model.
         * @param input Vector of audio samples.
         * @param index The index of the sample to predict (function will predict sample at `index` based on previous samples).
         * @return The predicted sample value (int16_t).
         *
         * Calculates a predicted value using the formula: `prediction = coeffs_[0] + Σ_{i=1..order}(coeffs_[i] * input[index - i])`. If `index` is smaller than `i`, those terms are omitted.
         */
        int16_t predict(const std::vector<int16_t> &input, size_t index) const;

        /**
         * @brief Reset the LPC model.
         *
         * Clears the stored coefficients. Should be called after finishing with one block of data, before training on a new block.
         */
        void clear();
        friend  class FlacAlgo;
    };

    Lpc lpc_;  ///< LPC model instance used for encoding consecutive blocks.

    /**
     * @brief Override: Send summary info with "FlacAlgo" tag.
     * @param common_information Compression metrics (ratio, time, sizes).
     *
     * Outputs the summary of the FLAC compression results prefixed with "FlacAlgo{ ... }" to distinguish it from other algorithms.
     */
    void send_common_information(const CommonInformation &common_information) override;

    /**
     * @brief Override: Send error info with "FlacAlgo" tag.
     * @param error Error message string.
     *
     * Formats and forwards error messages with a "FlacAlgo{ ... }" tag for clarity.
     */
    void send_error_information(const std::string &error) override;

    /**
     * @brief Log global compression parameters.
     *
     * Outputs a line listing the global constants used by the FLAC algorithm (such as block size, LPC order, Rice parameter). This helps in debugging or confirming the settings used for compression.
     */
    void send_global_params() const;

    /**
     * @brief Encode a single integer using Rice coding.
     * @param stream BitStream to write bits into.
     * @param num Integer number to encode (can be negative, uses sign bit).
     *
     * Rice coding encodes the number in two parts: a unary representation of `num >> kGlobalK` followed by a fixed `kGlobalK`-bit remainder. A leading sign bit is also added (1 for negative, 0 for non-negative).
     */
    static void rice_encode(BitStream &stream, int num);

    /**
     * @brief Decode a single integer from Rice coding.
     * @param stream BitStream to read bits from.
     * @return The decoded integer.
     *
     * This performs the inverse of `rice_encode`, reading a sign bit, then reading a unary count of bits until a zero, and then `kGlobalK` bits for the remainder, to reconstruct the original integer.
     */
    static int rice_decode(BitStream &stream);

    /**
     * @brief Encode a sequence of 16-bit values using Rice coding.
     * @param vec Vector of int16_t values to encode.
     * @return A vector of bytes containing the Rice-coded bits.
     *
     * Iterates through the input vector and encodes each value using `rice_encode` into a BitStream, then returns the underlying byte buffer from that stream.
     */
    static std::vector<uint8_t> encode_vector(const std::vector<int16_t> &vec);

    /**
     * @brief Decode a sequence of bytes into 16-bit values using Rice coding.
     * @param data Vector of bytes containing Rice-coded data (as produced by encode_vector).
     * @return A vector of int16_t values decoded from the input data.
     *
     * Reads bits from the provided data using a BitStream and repeatedly applies `rice_decode` to obtain all original values until no more complete values can be read.
     */
    static std::vector<int16_t> decode_vector(std::vector<uint8_t> data);

    /**
     * @brief Read the WAV file header.
     * @param filename Path to the WAV file.
     * @param header Reference to a WavHeader struct to populate.
     * @return `true` if a valid WAV header was read, `false` if the file format is invalid or file couldn't be opened.
     *
     * Opens the file and reads the header bytes into `header`. It validates basic WAV format conditions (RIFF/WAVE markers, PCM format, mono channel). If the data chunk ID is not immediately after the fmt chunk, it seeks to find it.
     */
    bool read_wav_header(const std::string &filename, WavHeader &header);

    /**
     * @brief Read PCM audio data from a WAV file.
     * @param filename Path to the WAV file.
     * @param header WavHeader previously read from the file (used to know data size).
     * @param start_index Starting sample index (0-based) from which to read data.
     * @return A vector of int16_t audio samples read from the file, or empty if an error occurred.
     *
     * Seeks to the data section of the WAV file (skipping the header) and then further offsets by `start_index` samples. Reads either a full block of `kGlobalSizeBlocks` samples or the remaining samples if fewer are left. This is used to process large audio in chunks.
     */
    std::vector<int16_t> read_wav_data(const std::string &filename, const WavHeader &header, size_t start_index);

public:
    /**
     * @brief Constructs the Flac algorithm handler.
     * @param is_text_output If true, use text stream for output messages; if false, output to a file.
     * @param output_file Path to output log file (if not in text mode).
     * @param ref_oss Reference to output string stream for logging (text mode).
     */
    explicit FlacAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
        : IController(is_text_output, output_file, ref_oss) {}

    /**
     * @brief Compress a WAV audio file.
     * @param input_filename Path to the input .wav file.
     *
     * Reads the WAV header and validates it. Then processes the audio data in blocks of `kGlobalSizeBlocks` samples:
     * For each block, it trains the LPC model to get prediction coefficients, then Rice-encodes these coefficients and the residuals (difference between actual samples and predicted samples).
     * All encoded data (including a copy of the WAV header and the size of encoded data) is written to a file in "storageEncoded/" with extension ".flac".
     * Finally, it logs the compression ratio, time, and global parameters used via `send_common_information` and `send_global_params()`.
     *
     * @throws If the WAV file cannot be read or if writing the output fails, an error is logged and the function may terminate the program.
     */
    void encode(const std::string &input_filename);

    /**
     * @brief Decompress a FLAC-compressed file.
     * @param input_filename Path to the .flac file to decode.
     *
     * **Current Implementation Note:** The decode function currently mirrors the encode logic (it was intended to perform decompression but as implemented it may simply recompute the same encoded stream). In a complete implementation, this would:
     *  - Read the WAV header and encoded data size.
     *  - For each encoded block, decode LPC coefficients and residuals using `rice_decode`, reconstruct audio samples using the LPC model, and accumulate them.
     *  - Write the reconstructed audio samples to a WAV file in "storageDecoded/" with the original .wav extension.
     *  - Log the time taken and compression ratio (which should match the original).
     *
     * @note This function should handle the reverse of encode, but it is not fully implemented in the current version.
     */
    void decode(const std::string &input_filename);
};

#endif
