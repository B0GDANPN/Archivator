#ifndef ARCHIVATOR_MATRIXINFO_HPP
#define ARCHIVATOR_MATRIXINFO_HPP

#include <vector>
#include <opencv2/opencv.hpp>

/**
 * @brief Metadata and payload for a submatrix (patch) used in video compression.
 *
 * Represents a rectangular region of a frame along with its pixel payload.
 * This structure is produced/consumed by Quantization algorithm when writing/
 * reading subframes (moving objects) and/or solid/background blocks.
 *
 * Unless otherwise specified by the encoder, pixel data is laid out in
 * row-major order and uses OpenCV's default 8-bit BGR channel order.
 */
struct MatrixInfo {
    /**
     * @brief Size of the submatrix (width x height) in pixels.
     */
    cv::Size size{};

    /**
     * @brief Top-left position (x, y) of the submatrix within the full frame.
     */
    cv::Point point{};

    /**
     * @brief Raw pixel payload of the submatrix.
     *
     * For non-solid matrices, this typically contains the decompressed BGR bytes
     * of length size.width * size.height * 3 in row-major order. For solid
     * matrices, this may be empty if the consumer reconstructs pixels from a
     * separate solid color field (depending on the on-disk format).
     */
    std::vector<unsigned char> data;

    /**
     * @brief Size of the compressed payload on disk (in bytes).
     *
     * Kept for bookkeeping when reading/writing binary streams; it may differ
     * from @ref data.size() because @ref data usually holds the *decompressed*
     * bytes ready to place into a frame, while @ref data_size stores the raw
     * compressed length read from file.
     */
    std::size_t data_size{};
};

#endif // ARCHIVATOR_MATRIXINFO_HPP
