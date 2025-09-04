#ifndef ARCHIVATOR_IMAGE_HPP
#define ARCHIVATOR_IMAGE_HPP
#include <string>
#include <vector>
#include <controller/IController.hpp>

using pixel_value = unsigned char;
/**
 * @brief Image container and utility class for fractal compression.
 *
 * Wraps image data and operations like loading, saving, and channel manipulation. The Image class is used by the fractal algorithm to handle image input/output and channel data, including padding and splitting into channels.
 */
class Image final : public IController {
public:
    /// Width of the image in pixels (after any padding to fit algorithm requirements).
    int width = 0;
    /// Height of the image in pixels (after padding and possibly adjusted to square dimensions).
    int height = 0;
    /// Number of color channels (e.g., 1 for grayscale, 3 for RGB).
    int channels = 0;
    /// Pointer to raw data of channel 1 (nullptr if not loaded or channel empty).
    pixel_value *image_data1 = nullptr;
    /// Pointer to raw data of channel 2 (for second color channel, or nullptr if not used).
    pixel_value *image_data2 = nullptr;
    /// Pointer to raw data of channel 3 (for third color channel, or nullptr if not used).
    pixel_value *image_data3 = nullptr;
    /// Base name of the file (without extension) associated with this image.
    std::string file_name;
    /// File extension of the image (e.g., "bmp", "jpg").
    std::string extension;
    /// Original image size in bytes (width * height * channels before any padding).
    int original_size = 0;

    /**
     * @brief Constructs an Image object.
     * @param is_text_output If true, output messages (info/error) go to text stream; if false, to file.
     * @param output_file Output file path for logs (if not text mode).
     * @param ref_oss Reference to output string stream (for text mode logging).
     */
    Image(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
        : IController(is_text_output, output_file, ref_oss) {}

    /**
     * @brief Log information about modifying image dimensions.
     * @param new_width New width after padding or resizing.
     * @param new_height New height after padding or resizing.
     *
     * Outputs a message indicating that the image has been padded or resized to the new dimensions. Used during loading when the image is expanded to dimensions suitable for fractal encoding (e.g., multiples of 32 and squared).
     */
    void send_info_modified_image(int new_width, int new_height) const;

    /**
     * @brief Override: Send common info with "FractalAlgo" tag.
     * @param common_information Compression info (ratio, time, sizes).
     *
     * Used by fractal compression to output summary info prefixed with "FractalAlgo{ ... }". This helps identify the output as coming from the fractal algorithm stage (which eventually calls Huffman as well).
     */
    void send_common_information(const CommonInformation &common_information) override;

    /**
     * @brief Override: Send error info with "FractalAlgo" tag.
     * @param error Error message string.
     *
     * Formats error messages to include "FractalAlgo{ ... }" before delegating to base class. This tags any image/fractal-related errors.
     */
    void send_error_information(const std::string &error) override;

    ~Image() override = default;

    /**
     * @brief Compute the next multiple of a given number.
     * @param number The value to round up.
     * @param multiple The base multiple.
     * @return The smallest number that is a multiple of `multiple` and is >= `number`.
     *
     * This utility is used to calculate padded image dimensions (e.g., next multiple of 32 for width/height).
     */
    static int next_multiple_of(int number, int multiple);

    /**
     * @brief Load image data from file.
     *
     * Reads the image file specified by `file_name` and `extension` into memory. If the image dimensions are not multiples of 32 or if width != height, it pads the image with black pixels to reach the next multiple of 32 and makes it square (expanding the smaller dimension).
     * After loading (and padding if needed), it splits the image into separate channel buffers (`ch1_`, `ch2_`, `ch3_`) and updates `image_data1`, `image_data2`, `image_data3` pointers.
     *
     * @throws std::runtime_error if the image cannot be loaded or if an unsupported number of channels is encountered.
     */
    void load();

    /**
     * @brief Save image data to file.
     *
     * Writes the image data (from channel buffers) back to an image file on disk. The output file path is derived from `file_name` and `extension`.
     * Supports saving as BMP, TGA, JPG/JPEG, or PNG. If the image has 1 channel (grayscale) or 3 channels (RGB) it will save appropriately; other channel counts are not supported.
     *
     * @throws std::runtime_error if saving fails or if the image format (extension) is not supported.
     */
    void save();

    /**
     * @brief Get raw data of a specific channel.
     * @param channel Channel index (1, 2, or 3 corresponding to ch1, ch2, ch3).
     * @param buffer Pointer to a buffer where the channel data will be copied.
     * @param size Expected number of pixels in that channel (should equal width * height).
     *
     * Copies the internal data of the specified channel into the provided buffer. The buffer must be allocated with at least `size` elements.
     * This function is used by the fractal encoder to retrieve channel data for processing.
     *
     * @throws std::invalid_argument if `buffer` is null or if `width * height` does not equal `size`.
     * @throws std::out_of_range if the requested channel is not 1, 2, or 3 or exceeds the image's channel count.
     * @throws std::runtime_error if the image data has not been loaded yet (no data in the channel).
     */
    void get_channel_data(int channel, pixel_value *buffer, int size);

    /**
     * @brief Set raw data for a specific channel.
     * @param channel Channel index to set (1, 2, or 3).
     * @param buffer Pointer to the new channel data (must have `size_channel` elements).
     * @param size_channel Number of pixels in the provided data (should equal width * height).
     *
     * Replaces the internal data of the specified channel with the contents of `buffer`. The image's channel count will be updated if setting a channel higher than the current count (e.g., adding a third channel to a grayscale image).
     * Also updates `original_size` to reflect the new total data size and synchronizes raw pointers.
     *
     * @throws std::out_of_range if `channel` is not 1-3.
     * @throws std::invalid_argument if `buffer` is null or `size_channel` does not match `width * height`.
     */
    void set_channel_data(int channel, const pixel_value *buffer, int size_channel);

    /**
     * @brief Initialize file name and extension.
     * @param file_name_in Full file path or name (including extension) of an image.
     *
     * Splits the input string into `file_name` (path without extension) and `extension` (the part after the last dot). This should be called before `load()` to set up the Image object's target file.
     */
    void image_setup(const std::string &file_name_in);

private:
    // Internal channel data buffers (vectors manage memory automatically).
    std::vector<pixel_value> ch1_;
    std::vector<pixel_value> ch2_;
    std::vector<pixel_value> ch3_;

    // Synchronize public raw pointers with internal buffers.
    void sync_raw_ptrs() noexcept {
        image_data1 = ch1_.empty() ? nullptr : ch1_.data();
        image_data2 = ch2_.empty() ? nullptr : ch2_.data();
        image_data3 = ch3_.empty() ? nullptr : ch3_.data();
    }

    // Initialize all channels with mid-gray (127) values.
    void init_grey_planes() ;
};

#endif