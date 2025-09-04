#ifndef ARCHIVATOR_IFS_TRANSFORM_HPP
#define ARCHIVATOR_IFS_TRANSFORM_HPP

#include <vector>
#include <memory>
#include <cstddef>
#include <image/Image.hpp> // pixel_value


/**
 * @brief Iterated Function System (IFS) transform representing a fractal mapping from one image block to another.
 *
 * An IFSTransform maps a small block (domain) of the image to another block (range) using rotation/reflection (symmetry), scaling of intensity, and offset.
 * It is the core element of fractal compression, encapsulating how one part of the image approximates another.
 */
class IFSTransform {
public:
    /**
     * @brief Symmetry operations for IFS transform.
     *
     * These enumeration values represent the geometric transformation applied to the domain block to best match the range block.
     */
    enum Sym {
        SYM_NONE = 0,  ///< No rotation or flip.
        SYM_R90,       ///< Rotate 90 degrees clockwise.
        SYM_R180,      ///< Rotate 180 degrees.
        SYM_R270,      ///< Rotate 270 degrees.
        SYM_HFLIP,     ///< Horizontal flip (mirror left-right).
        SYM_VFLIP,     ///< Vertical flip (mirror top-bottom).
        SYM_RDFLIP     ///< Flip across the main diagonal (transpose).
    };

    // Spatial parameters defining the mapping.
    std::size_t from_x;  ///< X-coordinate of the top-left corner of the source (domain) block.
    std::size_t from_y;  ///< Y-coordinate of the top-left corner of the source (domain) block.
    std::size_t to_x;    ///< X-coordinate of the top-left corner of the target (range) block.
    std::size_t to_y;    ///< Y-coordinate of the top-left corner of the target (range) block.
    std::size_t size;    ///< Width/height of the square block (block is size x size).

    // Transform parameters for intensity adjustment.
    Sym    symmetry; ///< Symmetry transformation to apply to the domain block.
    double scale;    ///< Scale factor for pixel intensities (brightness scaling).
    int    offset;   ///< Additive offset for pixel intensities after scaling.

    /**
     * @brief Down-sample a block of pixels by 2x.
     * @param src Pointer to the source image data (assumed at least `src_width * src_width` in size).
     * @param src_width Width of the source image (in pixels).
     * @param start_x X-coordinate of the top-left of the block to down-sample.
     * @param start_y Y-coordinate of the top-left of the block to down-sample.
     * @param target_size Desired size of the output down-sampled block (in pixels per side).
     * @return A vector of pixels (length `target_size * target_size`) containing the down-sampled block.
     *
     * This static function reduces a block of the source image (from [start_x, start_y] with size 2*target_size) to a smaller block (target_size),
     * averaging 2x2 pixel regions to produce one pixel. It is used to create half-scale versions of image regions for domain pool in fractal compression.
     */
    static std::vector<pixel_value> down_sample(const pixel_value* src, int src_width,
                                                int start_x, int start_y, int target_size);

    /**
     * @brief Construct an IFS transform with specified parameters.
     * @param from_x Source block top-left X.
     * @param from_y Source block top-left Y.
     * @param to_x Target block top-left X.
     * @param to_y Target block top-left Y.
     * @param size Block size (width and height).
     * @param symmetry Symmetry operation to apply.
     * @param scale Intensity scale factor.
     * @param offset Intensity offset.
     */
    IFSTransform(int from_x, int from_y, int to_x, int to_y, int size, Sym symmetry, double scale, int offset) noexcept
        : from_x(static_cast<std::size_t>(from_x))
        , from_y(static_cast<std::size_t>(from_y))
        , to_x(static_cast<std::size_t>(to_x))
        , to_y(static_cast<std::size_t>(to_y))
        , size(static_cast<std::size_t>(size))
        , symmetry(symmetry)
        , scale(scale)
        , offset(offset) {}

    ~IFSTransform() = default;

    /**
     * @brief Execute this transform on a source image plane to produce/update a destination image plane.
     * @param src Pointer to the source image plane data (e.g., down-sampled domain image).
     * @param src_width Width of the source image plane.
     * @param dest Pointer to the destination image plane data (e.g., range image being reconstructed).
     * @param dest_width Width of the destination image plane.
     * @param downsampled If false, the function will internally down-sample the source block before applying transformations. If true, assumes `src` is already a down-sampled domain block.
     *
     * This applies the stored symmetry (rotation/flip) to the appropriate block in `src`, then scales and offsets the pixels, writing them into the corresponding block in `dest`.
     * If `downsampled` is false, the function first creates a temporary down-sampled version of the `src` block.
     */
    void execute(const pixel_value* src, int src_width,
                 pixel_value* dest, int dest_width,
                 bool downsampled) const;

private:
    /// @brief Determine if current symmetry implies a scanline-order traversal (helper for execute).
    bool is_scanline_order() const noexcept ;
    /// @brief Check if symmetry keeps positive X direction (helper for execute).
    bool is_positive_x() const noexcept ;
    /// @brief Check if symmetry keeps positive Y direction (helper for execute).
    bool is_positive_y() const noexcept ;
};

using transform = std::vector<std::unique_ptr<IFSTransform>>;
/**
 * @brief Container for all IFS transforms of an image.
 *
 * Holds vectors of transforms for each color channel of the image. After fractal encoding, this structure contains the entire set of transformations representing the image.
 */
struct Transforms {
    int channels{0};             ///< Number of image channels represented in this transform set.
    transform ch[3];             ///< Array of transform lists per channel (index 0 for channel 1, etc.).

    Transforms() = default;
    ~Transforms() = default;     // unique_ptr in vectors will clean up IFSTransform objects.

    /**
     * @brief Get the total count of transforms across all channels.
     * @return Number of IFS transforms stored (sum of lengths of ch[0], ch[1], ch[2]).
     */
    int get_size() const noexcept {
        return static_cast<int>(ch[0].size() + ch[1].size() + ch[2].size());
    }
};


#endif
