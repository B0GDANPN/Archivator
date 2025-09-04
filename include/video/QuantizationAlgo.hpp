#ifndef ARCHIVATOR_QUANTIZATIONALGO_HPP
#define ARCHIVATOR_QUANTIZATIONALGO_HPP

#include <utility>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>
#include <dto/MatrixInfo.hpp>
#include <dto/CommonInformation.hpp>

#include <controller/IController.hpp>


inline size_t NOIZES = 2500000;
inline size_t NOIZES_PER_SUBFRAME = 500000;
inline size_t SOLID_DIFFERENCE = 100;

constexpr size_t kSplitDepth = 16;
constexpr size_t kSubframeDifference = 15;
constexpr size_t kCachedFrameDifference = 10;
constexpr size_t kColorChannels = 3;

/**
 * @brief Video frame differencing and quantization compression algorithm.
 *
 * Compresses a video by identifying scenes and differences between consecutive frames. Each scene is processed by extracting moving objects (subframes) and compressing static background separately. The output is stored in a directory containing metadata and binary data for subframes and background.
 *
 * This algorithm is lossy, focusing on reducing temporal and spatial redundancy in video frames (good for videos with static backgrounds and moving objects).
 */
class QuantizationAlgo : public IController {
    /**
     * @brief Override: Send summary info with "QuantizationAlgo" tag.
     * @param common_information Compression metrics (ratio, time, sizes).
     *
     * Outputs the compression results prefixed by "QuantizationAlgo{ ... }" for clarity.
     */
    void send_common_information(const CommonInformation &common_information) override;

    /**
     * @brief Override: Send error info with "QuantizationAlgo" tag.
     * @param error Error message string.
     *
     * Formats error messages with "QuantizationAlgo{ ... }" to indicate they originate from the video compression algorithm.
     */
    void send_error_information(const std::string &error) override;

    /**
     * @brief Analyze a sample video to adjust parameters (profiling).
     *
     * Uses an external Profiler (if available) to analyze a reference video (e.g., "bob.mp4") and tune global thresholds (NOIZES, NOIZES_PER_SUBFRAME, SOLID_DIFFERENCE) for noise and solid region detection.
     * The adjusted values influence how scene breaks and subframes are identified.
     */
    static void profile();

    /**
     * @brief Log current global parameters for video compression.
     *
     * Outputs the values of key parameters such as kSplitDepth, NOIZES (noise threshold), and NOIZES_PER_SUBFRAME. This helps in verifying or debugging the chosen thresholds.
     */
    void send_global_params() const;

    /**
     * @brief Decode a run-length encoded pixel buffer from file.
     * @param filename Path to the binary file containing the encoded pixel buffer.
     * @return A vector of pixels (cv::Vec3b) representing the decoded pixel sequence.
     *
     * Reads a file of RLE-encoded pixels (each run stored as [count, B, G, R]). It reconstructs the original sequence of pixels by expanding each run. After reading one sequence (until a terminator or EOF), it rewrites the file excluding the bytes that were decoded (effectively consuming that part of the file). This allows progressive reading of a large buffer stored on disk.
     * In the context of decoding, each such buffer corresponds to the background pixel data for a range of frames.
     */
    std::vector<cv::Vec3b> decode_buffer_from_file(const std::string &filename);

    /**
     * @brief Read the next compressed matrix (subframe) from file.
     * @param filename Path to the binary file containing compressed subframe matrices.
     * @return A MatrixInfo struct with the decompressed matrix data and its position.
     *
     * This function reads one record from the `matdata.bin` file, which contains:
     * - A representative color (scalar) for the matrix.
     * - A flag indicating if the matrix is a solid color.
     * - The size (width, height) of the matrix.
     * - The top-left position (point) of the matrix in the frame.
     * - If not solid: the size of compressed data and the compressed pixel data for that matrix.
     * It then either decompresses the data (if not solid, using `decompress_mat`) or fills a buffer with the solid color (if solid, using `fill`). The file is truncated to remove the read bytes (so that subsequent calls read the next matrix).
     */
    MatrixInfo read_next_matrix_and_point(const std::string &filename);

    /**
     * @brief Decompress run-length encoded matrix data.
     * @param compressed_data Vector of bytes representing compressed matrix (each run stored as [count, B, G, R]).
     * @return A vector of bytes (unsigned char) with the decompressed full matrix pixel data (concatenated B,G,R values).
     *
     * This expands the run-length encoding by iterating through the input: each 4-byte sequence (count and 3 color bytes) is expanded into `count` repetitions of that color in the output vector.
     */
    static std::vector<unsigned char> decompress_mat(const std::vector<unsigned char> &compressed_data);

    /**
     * @brief Compress a single image matrix (frame or subframe) using run-length encoding.
     * @param image OpenCV matrix (cv::Mat) of type CV_8UC3 to compress.
     * @return A vector of bytes representing the run-length encoded data.
     *
     * This compresses the matrix row by row. For each row, it scans across pixels and counts consecutive pixels that are "similar" (see `is_similar`). Each run is output as: one byte count (number of pixels, max 255) and three bytes of the pixel color (BGR). The threshold for "similar" is defined by `is_similar` (using kCachedFrameDifference by default).
     * If the image is empty, an error is logged and an empty vector is returned.
     */
    std::vector<unsigned char> compress_mat(const cv::Mat &image);

    /**
     * @brief Write compressed matrices and their metadata to file.
     * @param matrices Vector of pairs of (Point, Mat), where Point is the top-left position of the submatrix in the frame, and Mat is the submatrix image.
     * @param filename Path to the output binary file (e.g., "matdata.bin") where matrices will be appended.
     *
     * For each submatrix, this writes:
     * - The mean color of the submatrix (Vec3b).
     * - A boolean flag indicating if the submatrix is a solid color (all pixels within a threshold of the mean).
     * - The size (cv::Size) of the submatrix.
     * - The top-left position (cv::Point) of the submatrix in the original frame.
     * - If not solid: the length of compressed data (size_t) followed by the compressed byte data (obtained via `compress_mat`).
     * The file is opened in append mode, since multiple scenes' submatrices will be written sequentially.
     *
     * @note If the file cannot be opened for writing, it logs an error and returns without writing.
     */
    void write_matrices_and_points(const std::vector<std::pair<cv::Point, cv::Mat>> &matrices, const std::string &filename);

    /**
     * @brief Split a difference matrix into significant submatrices.
     * @param image The original frame (cv::Mat) corresponding to the difference matrix.
     * @param silents The difference image (cv::Mat) indicating pixel changes (e.g., result of frame subtraction).
     * @param threshold A pixel sum threshold to decide if a region contains significant changes.
     * @return Vector of (Point, Mat) pairs, each representing the top-left coordinate and pixel matrix of a region with changes.
     *
     * This function uses a quadtree-like approach: it starts with the whole difference matrix and recursively subdivides it into four quadrants whenever the sum of absolute differences in a region exceeds `threshold`. Regions below the threshold are considered static and are not subdivided further. For each final region (leaf), it takes the corresponding patch from the original frame (`image`) as a submatrix to be encoded separately.
     * The recursion depth is limited by `kSplitDepth` to avoid infinite splitting on noise.
     */
    static std::vector<std::pair<cv::Point, cv::Mat>> split_matrices(const cv::Mat &image, const cv::Mat &silents, int threshold);

    /**
     * @brief Extract all pixels outside submatrices for a frame.
     * @param big_matrix The full frame image (cv::Mat).
     * @param submatrices List of submatrix regions (Point and Mat) that represent moving objects.
     * @param result Vector to append the remaining pixels to (in row-major order).
     *
     * Scans through each pixel of `big_matrix`. If the pixel lies within any of the submatrix regions (moving objects), it is skipped. Otherwise, the pixel is part of the static background and is appended to `result`. This effectively collects the background pixels for the frame, which will later be run-length encoded and stored in a buffer.
     */
    static void write_numbers_excluding_submatrices(const cv::Mat &big_matrix,
                                                   const std::vector<std::pair<cv::Point, cv::Mat>> &submatrices,
                                                   std::vector<cv::Vec3b> &result);

    /**
     * @brief Write a buffer of run-length encoded pixels to a new file.
     * @param buffer Vector of pixels (Vec3b) representing a sequence of background pixels across frames.
     * @param filename Base filename (without index or extension) for the output.
     * @param threshold Similarity threshold for RLE (default is 10, e.g., kCachedFrameDifference).
     *
     * This function takes the accumulated background pixel buffer for a set of frames (usually a scene), and writes it out in run-length encoded form to a file. It generates a unique file name by appending an index and ".bin" extension to the given base.
     * It uses an RLE scheme: as it iterates through `buffer`, it counts consecutive pixels that are similar (difference within `threshold`) and writes runs (count + pixel color).
     * If the output file cannot be created, it logs an error and returns.
     */
    void write_buffer_to_file(const std::vector<cv::Vec3b> &buffer, const std::string &filename, int threshold);

    /**
     * @brief Fill a buffer with a solid color.
     * @param value A pixel color (Vec3b) to fill with.
     * @param size Dimensions (width x height) of the area to fill.
     * @param data Vector where the raw pixel data will be appended.
     *
     * Writes `size.width * size.height` pixels of the given color into `data`. This is used when a submatrix is determined to be solid (uniform color) – rather than store each pixel, we just store one value and then fill during decoding.
     */
    static void fill(const cv::Vec3b &value, const cv::Size &size, std::vector<uchar> &data);

    /**
     * @brief Convert an OpenCV Scalar color to Vec3b.
     * @param scalar An OpenCV Scalar, typically representing a mean BGR color.
     * @return A Vec3b with the BGR values (each component truncated to 0-255 range).
     *
     * This utility takes the average color (which may be a Scalar of doubles from cv::mean) and converts it to a byte triple suitable for image use.
     */
    static cv::Vec3b scalar_to_vec3_b(const cv::Scalar &scalar);

    /**
     * @brief Determine if two pixels are similar in color.
     * @param pixel1 First pixel (BGR color).
     * @param pixel2 Second pixel (BGR color).
     * @param threshold Maximum total difference allowed (default is kCachedFrameDifference, e.g., 10).
     * @return `true` if the sum of absolute differences of the B, G, and R components is <= threshold, otherwise `false`.
     *
     * This is used during run-length encoding to decide if two adjacent pixels can be considered "the same" for compression purposes.
     */
    static bool is_similar(const cv::Vec3b &pixel1, const cv::Vec3b &pixel2, int threshold = kCachedFrameDifference);

    /**
     * @brief Check if a matrix is nearly a solid color.
     * @param matrix An image patch (cv::Mat of type CV_8UC3).
     * @param threshold Allowed deviation from the mean (default is SOLID_DIFFERENCE, e.g., 100).
     * @return A pair containing the average color (Vec3b) and a boolean which is true if the matrix is solid (all pixels within `threshold` of the mean).
     *
     * Computes the mean color of the matrix and then verifies that each pixel does not deviate from this mean by more than the threshold in any channel. If this holds true for all pixels, the matrix is considered "solid" (uniform color).
     */
    static std::pair<cv::Vec3b, bool> are_solid(const cv::Mat &matrix, double threshold = SOLID_DIFFERENCE);

public:
    /**
     * @brief Constructs the video quantization algorithm handler.
     * @param is_text_output If true, send log output to text stream; if false, send to file.
     * @param output_file Log file path (used if not in text mode).
     * @param shared_oss Reference to output string stream for logging (text mode).
     */
    explicit QuantizationAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &shared_oss)
        : IController(is_text_output, output_file, shared_oss) {}

    /**
     * @brief Compress a video file using frame differencing and quantization.
     * @param input_filename Path to the input video file (e.g., .mp4).
     *
     * The video is processed frame by frame. The algorithm:
     * - Opens the video and reads frames.
     * - Determines scene boundaries by comparing consecutive frames (if the difference sum exceeds NOIZES, a new scene starts).
     * - For each scene, the first frame is taken as reference and differences `dst` to subsequent frames are computed.
     * - The function `split_matrices` is used on the first frame's difference image to find moving objects (submatrices).
     * - These submatrices are compressed and stored via `write_matrices_and_points` (to matdata.bin), and their count and frame range are recorded in a CSV (framedata.csv).
     * - The background pixels for the frames in the scene (excluding moving object areas) are collected with `write_numbers_excluding_submatrices` across frames and then compressed into a separate binary buffer via `write_buffer_to_file` (subframe*.bin).
     * - After processing all scenes, it calculates the total compressed size and time taken, and outputs compression ratio and info via `send_common_information` and `send_global_params()`.
     *
     * @note The output is stored in a directory under "storageEncoded/" named after the input video (without extension). This directory contains:
     *  - framedata.csv (dimensions and scene index data),
     *  - matdata.bin (subframe matrices data),
     *  - subframe0.bin, subframe1.bin, ... (background data for each scene).
     * @throws (Implicitly) If the video file can't be opened or an output file fails to write, error messages are logged. The function returns early on such failures.
     */
    void encode(const std::string &input_filename);

    /**
     * @brief Decompress a previously compressed video.
     * @param dir_name Name of the directory (within storageEncoded) that contains the compressed video data.
     *
     * Reconstructs the video by reading the compressed data:
     * - Opens framedata.csv to get the frame dimensions and scene breakdown.
     * - Initializes a VideoWriter to write the output video file in "storageDecoded/" (with .mp4 extension, using H256 codec).
     * - For each scene, for each frame:
     *   - Restores any saved submatrices by reading from matdata.bin (using `read_next_matrix_and_point`) and placing them into a frame buffer.
     *   - Fills the remaining background pixels by reading the corresponding subframeX.bin buffer (with `decode_buffer_from_file`) and inserting pixels in all positions not covered by submatrices.
     *   - Writes the reconstructed frame via VideoWriter.
     * - After processing all scenes, it closes the video file and outputs the overall ratio and time via `send_common_information` and `send_global_params()`.
     *
     * @note The decoding process assumes the same parameters and structure as encoding. It will terminate with errors if expected data is missing or if output video cannot be created.
     */
    void decode(const std::string &dir_name);
};

#endif
