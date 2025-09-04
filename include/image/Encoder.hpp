#ifndef ARCHIVATOR_ENCODER_HPP
#define ARCHIVATOR_ENCODER_HPP

#include <string>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
#include "controller/IController.hpp"
/**
 * @brief Base class for fractal encoders (image compression encoders).
 *
 * Inherits from IController to leverage output messaging. The Encoder class defines the interface for fractal encoding (the `encode()` method) and provides helper functions to evaluate block statistics (average, scale, error).
 * It also contains an Image `img` which holds working image data and metadata for the encoding process.
 */
class Encoder : public IController {
public:
    /**
     * @brief Override: Send summary info with "FractalAlgo" tag.
     * @param common_information Compression results (ratio, time, sizes).
     *
     * Outputs the compression summary prefixed by "FractalAlgo{ ... }". This is used to clearly identify output messages from fractal encoding procedures.
     */
    void send_common_information(const CommonInformation &common_information) override;

    /**
     * @brief Override: Send error info with "FractalAlgo" tag.
     * @param error Error message string.
     *
     * Formats the error with a "FractalAlgo{ ... }" prefix and passes it to the base implementation to handle output.
     */
    void send_error_information(const std::string &error) override;

    /**
     * @brief Constructs the Encoder base.
     * @param is_text_output Text output mode flag (passed to IController).
     * @param output_file Log file path (passed to IController).
     * @param ref_oss Output string stream reference (passed to IController).
     */
    Encoder(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss);

    ~Encoder() override = default;

    /**
     * @brief Perform fractal encoding on an image.
     * @param source Reference to an Image object containing the source image data.
     * @return A unique pointer to a Transforms structure representing the fractal compression result.
     *
     * This pure virtual function must be implemented by concrete encoders (e.g., QuadTreeEncoder). It takes an image and produces a set of IFS transforms (for each channel) that approximate the image. The transforms can then be used by a Decoder to reconstruct the image.
     */
    virtual std::unique_ptr<Transforms> encode(const Image &source) = 0;

    /**
     * @brief Compute average pixel value of a square block.
     * @param domain_data Pointer to the domain image data (one channel).
     * @param domain_width Width of the domain image.
     * @param domain_x X-coordinate of top-left of the block in the domain image.
     * @param domain_y Y-coordinate of top-left of the block in the domain image.
     * @param size Side length of the square block.
     * @return The average pixel intensity of the block (rounded down to nearest integer).
     *
     * Iterates through all pixels in the specified block and computes their average. Uses 64-bit accumulation to avoid overflow. This is used to calculate mean values for domain and range blocks in fractal encoding.
     * @throws std::invalid_argument if `domain_data` is null or if `domain_width` or `size` are <= 0.
     * @throws std::out_of_range if the block specified goes out of the domain image bounds.
     */
    int get_average_pixel(const pixel_value* domain_data, int domain_width,
                          int domain_x, int domain_y, int size);

    /**
     * @brief Compute the optimal scale factor for matching a domain block to a range block.
     * @param domain_data Pointer to domain image data (one channel).
     * @param domain_width Width of the domain image.
     * @param domain_x X-coordinate of domain block top-left.
     * @param domain_y Y-coordinate of domain block top-left.
     * @param domain_avg Precomputed average of the domain block.
     * @param range_data Pointer to range image data (one channel).
     * @param range_width Width of the range image.
     * @param range_x X-coordinate of range block top-left.
     * @param range_y Y-coordinate of range block top-left.
     * @param range_avg Precomputed average of the range block.
     * @param size Side length of the blocks (domain and range blocks are the same size).
     * @return A double representing the best-fit linear scale factor.
     *
     * Calculates the linear least-squares scale factor `s` that, when multiplying the domain block (after subtracting domain_avg) best matches the range block (after subtracting range_avg). This uses the formula `s = Σ((R_i - range_avg)*(D_i - domain_avg)) / Σ((D_i - domain_avg)^2)`.
     * If the domain block has zero variance (sum of squares = 0), the function returns 0.0.
     *
     * @throws std::invalid_argument if any data pointer is null or if widths/size are <= 0.
     * @throws std::out_of_range if the specified blocks extend out of the image bounds.
     */
    double get_scale_factor(const pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
                            const pixel_value* range_data,  int range_width,  int range_x,  int range_y,  int range_avg,
                            int size);

    /**
     * @brief Compute the mean squared error between a scaled domain block and a range block.
     * @param domain_data Pointer to domain image data (one channel).
     * @param domain_width Width of the domain image.
     * @param domain_x X-coordinate of domain block top-left.
     * @param domain_y Y-coordinate of domain block top-left.
     * @param domain_avg Average of domain block pixels.
     * @param range_data Pointer to range image data (one channel).
     * @param range_width Width of the range image.
     * @param range_x X-coordinate of range block top-left.
     * @param range_y Y-coordinate of range block top-left.
     * @param range_avg Average of range block pixels.
     * @param size Side length of the blocks.
     * @param scale Scale factor applied to domain block.
     * @return Mean squared error between `scale * (domain block - domain_avg)` and `(range block - range_avg)`.
     *
     * Computes the sum of squared differences between each pixel in the range block and the scaled corresponding pixel in the domain block (with their means removed), then divides by the number of pixels (size*size). The result is the mean squared error (MSE), which is used to judge the quality of a match.
     *
     * @throws std::invalid_argument if any data pointers are null or if widths/size are <= 0.
     * @throws std::out_of_range if the block coordinates extend outside the image data bounds.
     */
    double get_error(const pixel_value* domain_data, int domain_width, int domain_x, int domain_y, int domain_avg,
                     const pixel_value* range_data,  int range_width,  int range_x,  int range_y,  int range_avg,
                     int size, double scale);

    /// Working image data and metadata used during encoding (copied from the source image for processing).
    Image img;
};

#endif
