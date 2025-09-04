//
// Created by bogdan on 10.04.24.
//

#ifndef ARCHIVATOR_FRACTAL_ALGO_HPP
#define ARCHIVATOR_FRACTAL_ALGO_HPP

#include <string>
#include <image/Image.hpp>
#include <controller/IController.hpp>

namespace fs = std::filesystem;
/**
 * @brief High-level fractal compression algorithm for images.
 *
 * Coordinates the fractal encoding of an image and subsequent Huffman compression of the encoded data. The fractal algorithm divides the image into range blocks and represents them via self-similar transforms (IFS), then uses multiple decoding iterations to construct an approximate image which is further compressed. This algorithm is typically used for image files (e.g., BMP, JPEG).
 *
 * @note This implementation uses an intermediate Huffman encoding stage: the fractal-approximated image is compressed using Huffman coding to produce the final output file (with extension ".hcf"). Direct fractal data output is not saved separately.
 */
class FractalAlgo final : public IController {
    /**
     * @brief Override: Tag and send compression summary for fractal algorithm.
     * @param common_information Compression metrics (ratio, time, sizes).
     *
     * Outputs the summary information wrapped in "FractalAlgo{ ... }" to distinguish fractal algorithm results. This typically shows the combined effect of fractal + Huffman compression.
     */
    void send_common_information(const CommonInformation &common_information) override;

    /**
     * @brief Override: Tag and send error message for fractal algorithm.
     * @param error Error description string.
     *
     * Prepends "FractalAlgo{ ... }" to the error message and outputs it, indicating the error occurred during fractal processing.
     */
    void send_error_information(const std::string &error) override;

    /**
     * @brief Send details about a completed encoding process.
     * @param width Width of the source image.
     * @param height Height of the source image.
     * @param num_transforms Total number of IFS transforms generated.
     *
     * Outputs a message summarizing the fractal encoding: it logs the image dimensions and the number of transforms produced. This information helps in understanding the complexity of the fractal representation.
     */
    void send_encoded_information(int width, int height, int num_transforms) const;

    /**
     * @brief Send details about a completed decoding process.
     * @param width Width of the reconstructed image.
     * @param height Height of the reconstructed image.
     * @param phases Number of decoding phases applied.
     *
     * Outputs a message summarizing the fractal decoding: logs the image dimensions and how many iterations (phases) were applied to reconstruct the image. This is useful for debugging or informational output in a standalone fractal decode (if implemented).
     */
    void send_decoded_information(int width, int height, int phases) const;

public:
    /**
     * @brief Constructs the Fractal algorithm handler.
     * @param is_text_output If true, direct output messages to text stream; if false, to a log file.
     * @param output_file Log file path (if not in text mode).
     * @param ref_oss Reference to output string stream (for text mode logging).
     */
    explicit FractalAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
        : IController(is_text_output, output_file, ref_oss) {}

    /**
     * @brief Compress an image using fractal compression.
     * @param input_filename Path to the input image file.
     * @param quality Quality parameter controlling compression accuracy (e.g., 100 is default; lower values produce higher quality at expense of compression).
     *
     * Performs fractal compression on the image file. It loads the image, optionally adjusts quality settings, and encodes the image via the QuadTreeEncoder to obtain IFS transforms. It then runs a few decoding iterations on those transforms to produce an approximate image. The resulting approximate image is saved to a temporary file, which is then compressed using Huffman coding (via HuffmanAlgo) into the final output (stored in "storageEncoded/" with extension ".hcf"). The temporary image file is removed after Huffman encoding.
     *
     * The method reports progress and info: starts with a message "Encoding..." and after completion, outputs compression ratio and time via `send_common_information`. It also logs intermediate details like the number of transforms and phases of decoding internally.
     *
     * @throws std::runtime_error If image loading fails or an unsupported image format is encountered (propagated from Image class).
     */
    void encode(const std::string &input_filename, int quality) const;
};


#endif