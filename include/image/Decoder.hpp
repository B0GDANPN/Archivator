#ifndef ARCHIVATOR_DECODER_HPP
#define ARCHIVATOR_DECODER_HPP



#include <memory>
#include <string>
#include <image/Image.hpp>
#include <image/IFSTransform.hpp>
/**
 * @brief Fractal image decoder that applies IFS transforms to reconstruct an image.
 *
 * Given a set of IFS transforms (from fractal encoding), this class iteratively applies them to an initially blank image to approximate the original image. Typically, multiple decode phases are run to refine the image quality.
 */
class Decoder {
public:
    /**
     * @brief Constructs a Decoder for a given image size and channel count.
     * @param width Width of the image to decode.
     * @param height Height of the image to decode.
     * @param channels Number of color channels.
     * @param is_text_output If true, output messages from any internal image operations go to text stream; if false, to file.
     * @param output_file Output file path for logs (if not text mode).
     * @param ref_oss Reference to text output string stream for logs.
     *
     * Initializes an internal Image (`img_`) of the specified dimensions and channels. The image data is filled with a neutral gray value (127) for all pixels as a starting point for decoding.
     * @throws std::invalid_argument if width or height are <= 0, or if channels is not 1-3.
     */
    Decoder(int width, int height, int channels, bool is_text_output, std::string output_file,
            std::ostringstream &ref_oss);

    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;
    Decoder(Decoder&&) noexcept = default;
    Decoder& operator=(Decoder&&) noexcept = delete;
    ~Decoder() = default;

    /**
     * @brief Apply one iteration of fractal decoding using a set of transforms.
     * @param transforms The Transforms (set of IFS transforms for each channel) to apply.
     *
     * This method goes through each stored transform in `transforms` for each channel and applies it to the internal image (`img_`). If `transforms.channels` is non-zero and greater than the current image channels, the decoder will add and initialize additional channels to accommodate the data.
     * Each transform’s execute() function is invoked to modify the image. After calling this once (one "phase"), the image stored in the decoder is an approximation of the original. Repeating the decode with the same transforms will refine the image.
     *
     * @throws std::invalid_argument if the number of channels in transforms is out of range (0 or >3).
     * @throws std::runtime_error if an expected image channel buffer is uninitialized (this would indicate an internal error).
     */
    void decode(const Transforms &transforms);

    /**
     * @brief Create an Image object from the decoder's current image state.
     * @param file_name Base file name to assign to the new Image (extension can be added later).
     * @param channel If 0 (default), include all channels; if 1,2,3, extract only that channel as a grayscale image.
     * @return A unique pointer to a new Image containing the pixel data from the decoder.
     *
     * This function packages the internally decoded image data into a new Image object that can be saved or further processed. If `channel` is specified (1-3), the output image will contain only that single channel’s data (useful for debugging or viewing one channel). Otherwise, the output image will have the same number of channels as the decoder’s internal image.
     *
     * @throws std::out_of_range if a specific channel is requested that is not available.
     * @throws std::runtime_error if the requested channel data is not present (null pointer).
     */
    std::unique_ptr<Image> make_image(const std::string &file_name, int channel = 0) const;

    /**
     * @brief Get a new Image of the current state (alias of make_image).
     * @param file_name Base file name for the output image.
     * @param channel Channel selection (0 for all channels, or 1-3 for a specific channel).
     * @return Unique pointer to Image representing the decoder's image.
     *
     * This is provided for convenience and simply calls `make_image`.
     */
    std::unique_ptr<Image> get_new_image(const std::string &file_name, int channel = 0) const {
        return make_image(file_name, channel);
    }

private:
    bool is_text_output_;           ///< Copy of output mode flag for internal Image creation.
    std::string output_file_;       ///< Copy of output file path for internal Image creation.
    std::ostringstream &ref_oss_;   ///< Reference to output string stream for internal Image logging.
    Image img_;                     ///< Internal image buffer that is progressively updated by decode().

    /**
     * @brief Initialize all channels of the internal image with gray (127).
     *
     * Fills each pixel of each channel with the value 127 (midpoint of 0-255) to start decoding from a neutral image.
     */
    void init_grey_channels();

    /**
     * @brief Ensure the internal image has a certain number of channels.
     * @param required_channels The number of channels that must be present.
     *
     * If `img_` currently has fewer than `required_channels` channels, this will add channels up to that number, initializing the new channel data to gray. It adjusts `img_.channels` and `img_.original_size` accordingly.
     *
     * @throws std::invalid_argument if `required_channels` is out of the supported range (1-3).
     */
    void ensure_channels(int required_channels);
};

#endif


