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

class QuantizationAlgo : public IController {
    void send_common_information(const CommonInformation &common_information) override ;

    void send_error_information(const std::string &error) override;

    static void profile();

    void send_global_params() const ;

    std::vector<cv::Vec3b> decode_buffer_from_file(const std::string &filename);

    MatrixInfo read_next_matrix_and_point(const std::string &filename);

    static std::vector<unsigned char> decompress_mat(const std::vector<unsigned char> &compressed_data);

    std::vector<unsigned char> compress_mat(const cv::Mat &image);

    void write_matrices_and_points(const std::vector<std::pair<cv::Point, cv::Mat>> &matrices, const std::string &filename);

    static std::vector<std::pair<cv::Point, cv::Mat>>
    split_matrices(const cv::Mat &image, const cv::Mat &silents, int threshold);

    static void write_numbers_excluding_submatrices(const cv::Mat &big_matrix, const std::vector<std::pair<cv::Point,
            cv::Mat>> &submatrices, std::vector<cv::Vec3b> &result);

    void write_buffer_to_file(const std::vector<cv::Vec3b> &buffer, const std::string &filename, int threshold );

    static void fill(const cv::Vec3b &value, const cv::Size &size, std::vector<uchar> &data);

    static cv::Vec3b scalar_to_vec3_b(const cv::Scalar &scalar);

    static bool is_similar(const cv::Vec3b &pixel1, const cv::Vec3b &pixel2,  int threshold=kCachedFrameDifference);

    static std::pair<cv::Vec3b, bool> are_solid(const cv::Mat &matrix, double threshold=SOLID_DIFFERENCE);


public:
  explicit QuantizationAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &shared_oss)
          : IController(is_text_output, output_file, shared_oss) {}


    void encode(const std::string &input_filename);


    // need /../../..sample.mp4
    //storageEncoded/sample
    //const std::string& framedata = "sample/framedata.csv",
    //                       const std::string& matdata = "sample/matdata.bin",
    //                       const std::string& subframedata = "sample/subframe/"
    void decode(const std::string &dir_name);

};


#endif
