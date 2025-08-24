#include <video/QuantizationAlgo.hpp>


#include <fstream>
#include <filesystem>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <string>
#include <dto/MatrixInfo.hpp>
#include <dto/CommonInformation.hpp>
#include <video/Profiler.hpp>
namespace fs = std::filesystem;

void QuantizationAlgo::send_common_information(const CommonInformation& common_information) {
        send_message("QuantizationAlgo{ ");
        IController::send_common_information(common_information);
        send_message("}\n");
}
void QuantizationAlgo::send_error_information(const std::string& error){
  IController::send_error_information("QuantizationAlgo{ " + error + "}\n");
}
void QuantizationAlgo::profile(){
  Profiler profiler;
  CompressParams params = profiler.analyze("bob.mp4");

  NOIZES = params.noises;
  NOIZES_PER_SUBFRAME = params.noises_per_subframe;
  SOLID_DIFFERENCE = params.solid_difference;
}
void QuantizationAlgo::send_global_params() const{
  std::stringstream oss;
  oss << "Params: " << kSplitDepth << " "
      << NOIZES << " "
      << NOIZES_PER_SUBFRAME << '\n';
  std::string str = oss.str();
  send_message(str);
}
std::vector<cv::Vec3b> QuantizationAlgo::decode_buffer_from_file(const std::string& filename){
  std::vector<cv::Vec3b> decoded_buffer;
  std::ifstream input_file(filename, std::ios::binary);
  if (!input_file.is_open()) {
    send_error_information("Failed to open the file while decoding from buffer: " + filename + "\n");
    return decoded_buffer;
  }

  unsigned char count;
  cv::Vec3b pixel;
  while (input_file.read(reinterpret_cast<char *>(&count), sizeof(unsigned char))) {
    if (count == '\0') {
      input_file.seekg(-1, std::ios_base::cur);
      input_file.putback(1);
      break;
    }

    input_file.read(reinterpret_cast<char *>(&pixel), sizeof(cv::Vec3b));
    for (int i = 0; i < count; ++i) {
      decoded_buffer.push_back(pixel);
    }
  }

  std::ofstream output_file(filename + ".temp", std::ios::binary);
  output_file << input_file.rdbuf();
  input_file.close();
  output_file.close();
  std::remove(filename.c_str());
  std::rename((filename + ".temp").c_str(), filename.c_str());

  return decoded_buffer;
}
MatrixInfo QuantizationAlgo::read_next_matrix_and_point(const std::string &filename) {
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs.is_open()) {
            send_error_information("Failed to open file for reading1!1!!11!1\n");
            exit(0);
        }

        if (ifs.eof()) {
            return {};
        }

        cv::Vec3b scalar;
        bool is_solid;
        cv::Size size;
        cv::Point point;
        size_t data_size;

        ifs.read(reinterpret_cast<char *>(&scalar), sizeof(cv::Vec3b));
        ifs.read(reinterpret_cast<char *>(&is_solid), sizeof(bool));
        ifs.read(reinterpret_cast<char *>(&size), sizeof(cv::Size));
        ifs.read(reinterpret_cast<char *>(&point), sizeof(cv::Point));


        std::vector<uchar> decompressed_data;

        if (!is_solid) {
            ifs.read(reinterpret_cast<char *>(&data_size), sizeof(data_size));
            std::vector<uchar> compressed_data(data_size);
            ifs.read(reinterpret_cast<char *>(compressed_data.data()), data_size);
            decompressed_data = decompress_mat(compressed_data);
        } else {
            fill(scalar, size, decompressed_data);
        }

        MatrixInfo matrix_info;
        matrix_info.data = std::move(decompressed_data);
        matrix_info.size = size;
        matrix_info.point = point;
        matrix_info.data_size = size.height * size.width * kColorChannels;

        std::string temp_filename = filename + ".tmp";
        std::ofstream ofs(temp_filename, std::ios::binary);

        ofs << ifs.rdbuf();
        ifs.close();
        ofs.close();

        remove(filename.c_str());
        rename(temp_filename.c_str(), filename.c_str());

        return matrix_info;
    }
std::vector<unsigned char> QuantizationAlgo::decompress_mat(const std::vector<unsigned char> &compressed_data) {
        std::vector<unsigned char> decompressed_data;
        for (size_t i = 0; i < compressed_data.size(); i += 4) {
            int count = compressed_data[i];
            unsigned char blue = compressed_data[i + 1];
            unsigned char green = compressed_data[i + 2];
            unsigned char red = compressed_data[i + 3];
            for (int j = 0; j < count; ++j) {
                decompressed_data.push_back(blue);
                decompressed_data.push_back(green);
                decompressed_data.push_back(red);
            }
        }
        return decompressed_data;
    }
std::vector<unsigned char> QuantizationAlgo::compress_mat(const cv::Mat &image) {
        if (image.empty()) {
            send_error_information("Got empty image to compress!!!11!11!1\n");
            return {};
        }

        std::vector<uchar> compressed_data;
        for (int row = 0; row < image.rows; ++row) {
            int count = 1;
            for (int col = 1; col < image.cols; ++col) {
                if (is_similar(image.at<cv::Vec3b>(row, col), image.at<cv::Vec3b>(row, col - 1)) && count < 255) {
                    count++;
                } else {
                    compressed_data.push_back(count);
                    compressed_data.push_back(image.at<cv::Vec3b>(row, col - 1)[0]);
                    compressed_data.push_back(image.at<cv::Vec3b>(row, col - 1)[1]);
                    compressed_data.push_back(image.at<cv::Vec3b>(row, col - 1)[2]);
                    count = 1;
                }
            }
            compressed_data.push_back(count);
            compressed_data.push_back(image.at<cv::Vec3b>(row, image.cols - 1)[0]);
            compressed_data.push_back(image.at<cv::Vec3b>(row, image.cols - 1)[1]);
            compressed_data.push_back(image.at<cv::Vec3b>(row, image.cols - 1)[2]);
        }

        return compressed_data;
    }


void QuantizationAlgo::write_matrices_and_points(const std::vector<std::pair<cv::Point, cv::Mat>> &matrices, const std::string &filename) {
        std::ofstream ofs(filename, std::ios::binary | std::ios::app);

        if (!ofs.is_open()) {
            send_error_information("Failed to open file for writing!11!111!\n");
            return;
        }

        for (const auto & [fst, snd]: matrices) {

            std::pair<cv::Vec3b, bool> is_solid = are_solid(snd);
            cv::Size size = snd.size();
            cv::Point point = fst;

            ofs.write(reinterpret_cast<const char *>(&fst), sizeof(cv::Vec3b));
            ofs.write(reinterpret_cast<const char *>(&is_solid.second), sizeof(bool));
            ofs.write(reinterpret_cast<const char *>(&size), sizeof(cv::Size));
            ofs.write(reinterpret_cast<const char *>(&point), sizeof(cv::Point));

            if (!is_solid.second) {
                std::vector<uchar> vec = compress_mat(snd);
                size_t data_size = vec.size();
                ofs.write(reinterpret_cast<const char *>(&data_size), sizeof(data_size));
                ofs.write(reinterpret_cast<const char *>(vec.data()), data_size);
            }
        }

        ofs.close();
    }

std::vector<std::pair<cv::Point, cv::Mat>> QuantizationAlgo::split_matrices(const cv::Mat &image, const cv::Mat &silents, int threshold) {
        std::vector<std::pair<cv::Point, cv::Mat>> result;
        std::queue<std::pair<cv::Point, cv::Mat>> to_process;
        to_process.emplace(cv::Point(0, 0), silents);
        int counter = 0;

        while (!to_process.empty()) {

            auto top_left = to_process.front().first;
            auto matrix = to_process.front().second;
            to_process.pop();

            if (cv::sum(matrix)[0] > threshold) {
                int width = matrix.cols / 2;
                int height = matrix.rows / 2;

                to_process.emplace(top_left, matrix(cv::Rect(0, 0, width, height)));
                to_process.emplace(top_left + cv::Point(width, 0), matrix(cv::Rect(width, 0, width, height)));
                to_process.emplace(top_left + cv::Point(0, height), matrix(cv::Rect(0, height, width, height)));
                to_process.emplace(top_left + cv::Point(width, height), matrix(cv::Rect(width, height, width, height)));
            } else {
                result.emplace_back(top_left, image(cv::Rect(top_left.x, top_left.y, matrix.cols, matrix.rows)));
            }
            if (counter++ == kSplitDepth) return result;
        }

        return result;
    }

void QuantizationAlgo::write_numbers_excluding_submatrices(const cv::Mat &big_matrix, const std::vector<std::pair<cv::Point,
            cv::Mat>> &submatrices, std::vector<cv::Vec3b> &result) {

        for (int i = 0; i < big_matrix.rows; ++i) {
            for (int j = 0; j < big_matrix.cols; ++j) {
                cv::Point current_point(j, i);

                bool is_inside_submatrix = false;
                for (const auto &submatrix: submatrices) {
                    const cv::Point &submatrix_origin = submatrix.first;
                    const cv::Mat &submatrix_data = submatrix.second;

                    if (current_point.x >= submatrix_origin.x &&
                        current_point.x < submatrix_origin.x + submatrix_data.cols &&
                        current_point.y >= submatrix_origin.y &&
                        current_point.y < submatrix_origin.y + submatrix_data.rows) {
                        is_inside_submatrix = true;
                        break;
                    }
                }

                if (!is_inside_submatrix) {
                    result.push_back(big_matrix.at<cv::Vec3b>(current_point));
                }
            }
        }
    }

void QuantizationAlgo::write_buffer_to_file(const std::vector<cv::Vec3b> &buffer, const std::string &filename, int threshold=10 ) {
        static int file_counter = 0;

        std::stringstream ss;
        ss << filename << file_counter++ << ".bin";
        std::string unique_filename = ss.str();
        std::ofstream output_file(unique_filename, std::ios::binary);
        if (!output_file.is_open()) {
            send_error_information("Unable to open the file: \n");
            return;
        }
        for (size_t i = 0; i < buffer.size(); ++i) {
            int count = 1;
            cv::Vec3b prev_pixel = buffer[i];
            for (size_t j = i + 1; j < buffer.size(); ++j) {
                if (is_similar(prev_pixel, buffer[j], threshold) && count < 255) {
                    count++;
                } else {
                    output_file.write(reinterpret_cast<const char *>(&count), sizeof(unsigned char));
                    output_file.write(reinterpret_cast<const char *>(&prev_pixel), sizeof(cv::Vec3b));
                    i = j - 1;
                    break;
                }
            }
            if (i == buffer.size() - 1) {
                count = 1;
                output_file.write(reinterpret_cast<const char *>(&count), sizeof(unsigned char));
                output_file.write(reinterpret_cast<const char *>(&prev_pixel), sizeof(cv::Vec3b));
            }
        }
    }

void QuantizationAlgo::fill(const cv::Vec3b &value, const cv::Size &size, std::vector<uchar> &data) {
        size_t num_elements = size.width * size.height;  // 3 color channels

        for (size_t i = 0; i < num_elements; i++) {
            data.push_back(value[0]);
            data.push_back(value[1]);
            data.push_back(value[2]);
        }

    }

cv::Vec3b QuantizationAlgo::scalar_to_vec3_b(const cv::Scalar &scalar) {
        return {static_cast<uchar>(scalar[0] + 0.5),
                static_cast<uchar>(scalar[1] + 0.5),
                static_cast<uchar>(scalar[2] + 0.5)};
    }

    bool QuantizationAlgo::is_similar(const cv::Vec3b &pixel1, const cv::Vec3b &pixel2, int threshold) {
        int distance = 0;
        for (int i = 0; i < 3; ++i) {
            distance += static_cast<int>(pixel1[i]) - static_cast<int>(pixel2[i]);
        }
        return distance <= threshold;
    }

std::pair<cv::Vec3b, bool> QuantizationAlgo::are_solid(const cv::Mat& matrix, double threshold) {
  cv::Scalar mean_value = cv::mean(matrix);

  for (int i = 0; i < matrix.rows; ++i) {
    for (int j = 0; j < matrix.cols; ++j) {
      cv::Vec3b pixel = matrix.at<cv::Vec3b>(i, j);
      double diff_b = std::abs(pixel[0] - mean_value.val[0]);
      double diff_g = std::abs(pixel[1] - mean_value.val[1]);
      double diff_r = std::abs(pixel[2] - mean_value.val[2]);

      if (diff_b > threshold || diff_g > threshold || diff_r > threshold) {
        return std::make_pair(scalar_to_vec3_b(mean_value), false);
      }
    }
  }
  return std::make_pair(scalar_to_vec3_b(mean_value), true);
}

void QuantizationAlgo::encode(const std::string& input_filename) {
        auto start = std::chrono::high_resolution_clock::now();
        int size_input = static_cast<int>(get_filesize(input_filename));
        size_t last_slash_pos = input_filename.find_last_of('/');
        std::string dir_name =
                last_slash_pos != std::string::npos ? input_filename.substr(last_slash_pos + 1) : input_filename;
        size_t pos = dir_name.rfind('.');
        dir_name = dir_name.substr(0, pos);// путь сохранения
        fs::create_directory("storageEncoded/" + dir_name);

        std::string framedata = "storageEncoded/" + dir_name + "/framedata.csv";
        std::string matdata = "storageEncoded/" + dir_name + "/matdata.bin";
        std::string subframedata = "storageEncoded/" + dir_name + "/subframe";
        //fs::create_directory(subframedata);
        send_message("Encoding... video\n");
        std::ofstream ofs(framedata);
        cv::VideoCapture cap(input_filename);
        double frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double channel_count = 3; // Цветное видео
        double pixel_size = 1; // 8-битное цветное видео
        if (!cap.isOpened()) {
            send_error_information("Failed to open the video file.");
            return;
        }

        cv::Mat frame, frame2, dst;
        size_t start_scene = 0;
        size_t end_scene = 0;

        cap.read(frame);
        cap.read(frame2);

        size_t sum;
        send_message("Size frame: " + std::to_string(frame_height * frame_width * channel_count * pixel_size) + "\n");
        ofs << frame.rows << "," << frame.cols << std::endl;

        while (!frame.empty()) {

            while (true) {
                cap.read(frame2);
                if (frame2.empty()) {
                    break;
                }
                cv::subtract(frame, frame2, dst);
                sum = cv::sum(dst)[0];
                end_scene++;
                if (sum > NOIZES) break;
            }

            if (start_scene == cap.get(cv::CAP_PROP_FRAME_COUNT) - 1) break;

            auto matricies = split_matrices(frame, dst, NOIZES_PER_SUBFRAME);
            send_message("Frames from" + std::to_string(start_scene) + " to " + std::to_string(end_scene)
                        + " Count of mats " + std::to_string(matricies.size()) + '\n');

            ofs << start_scene << "," << end_scene << "," << matricies.size() << std::endl;

            write_matrices_and_points(matricies, matdata);// std::string matdata="sampleZip/matdata.bin"

            cap.set(cv::CAP_PROP_POS_FRAMES, start_scene);

            std::vector<cv::Vec3b> sub_frame_buffer;
            while (start_scene != end_scene) {
                cap.read(frame);
                write_numbers_excluding_submatrices(frame, matricies, sub_frame_buffer);
                start_scene++;
            }
            cap.read(frame);
            const std::string &filename = subframedata;// std::string subframedata= "sampleZip/subframe/"
            send_message("Size of buffer: " + std::to_string(sub_frame_buffer.size() * sizeof(cv::Vec3b) / 1024) + '\n');
            write_buffer_to_file(sub_frame_buffer, filename);
        }
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count());
        uintmax_t size_output1 = get_filesize(framedata);
        uintmax_t size_output2 = get_filesize(subframedata);
        uintmax_t size_output3 = get_filesize(matdata);

        int size_output = static_cast<int>(size_output1 + size_output2 + size_output3);
        double ratio = static_cast<double>(size_output) / size_input;
        auto info = CommonInformation(ratio, duration, size_input, size_output);
        send_global_params();
        send_common_information(info);
    }

void QuantizationAlgo::decode(const std::string& dir_name)
 {
        std::string framedata = "storageEncoded/" + dir_name + "/framedata.csv";
        std::string matdata = "storageEncoded/" + dir_name + "/matdata.bin";
        std::string subframedata = "storageEncoded/" + dir_name + "/subframe";
        fs::path output_path = "storageDecoded/" + dir_name + ".mp4";// путь сохранения

        uintmax_t size_input1 = get_filesize(framedata);
        uintmax_t size_input2 = get_filesize(subframedata);
        uintmax_t size_input3 = get_filesize(matdata);

        int size_input = static_cast<int>(size_input1 + size_input2 + size_input3);
        auto start = std::chrono::high_resolution_clock::now();
        send_message("Decoding video\n");
        std::ifstream frame(framedata);

        int sub_frame_data_index = 0;
        int rows = -1;
        int cols = -1;
        char delimiter;
        std::string line;
        if (std::getline(frame, line)) {
            std::istringstream ss(line);
            if (ss >> rows >> delimiter >> cols && delimiter == ',') {
                send_message("Decoding: reading succes\n");
            } else {
                send_error_information("Decoding: Not enough data\n");
                exit(1);
            }
        } else {
            send_error_information("Decoding: reading failed\n");
            exit(-2);
        }
        send_message(std::to_string(rows) + ' ' + std::to_string(cols) + '\n');
        cv::Mat main(rows, cols, CV_8UC3, cv::Scalar(0, 0, 255));

        cv::VideoWriter video_writer(output_path.string(), cv::VideoWriter::fourcc('H', '2', '5', '6'), 30,
                                    cv::Size(cols, rows));
        if (!video_writer.isOpened()) {
            send_error_information("Unable to open the VideoWriter\n");
            return;
        }

        int from, to, matrix_count;
        int scene = 0;
        while (std::getline(frame, line)) {
            std::istringstream ss(line);
            send_message("Decoding scene: " + std::to_string(++scene) + '\n');
            if (ss >> from >> delimiter >> to >> delimiter >> matrix_count && delimiter == ',') {
                int frames = to - from;
                std::vector<cv::Rect> reserved;
                for (int i = 0; i < matrix_count; i++) {
                    MatrixInfo c = read_next_matrix_and_point(matdata);
                    cv::Mat sub(c.size, CV_8UC3, c.data.data());
                    cv::Rect roi(c.point, sub.size());
                    reserved.push_back(roi);
                    sub.copyTo(main(roi));
                    if (c.data.empty()) {
                        send_error_information("Error: Not enough matrix data \n");
                        exit(3);
                    }
                }
                size_t pixel_index = 0;
                std::vector<cv::Vec3b> pixels = decode_buffer_from_file(
                        subframedata + std::to_string(sub_frame_data_index) + ".bin");
                sub_frame_data_index++;
                for (int k = 0; k < frames; k++) {
                    for (int i = 0; i < main.rows; ++i) {
                        for (int j = 0; j < main.cols; ++j) {
                            bool is_in_deprecated = false;
                            for (const auto &rect: reserved) {
                                if (rect.contains(cv::Point(j, i))) {
                                    is_in_deprecated = true;
                                    break;
                                }
                            }
                            if (!is_in_deprecated && pixel_index < pixels.size()) {
                                main.at<cv::Vec3b>(i, j) = pixels[pixel_index++];
                            }
                        }
                    }
                    video_writer.write(main);
                }
            } else {
                send_error_information("Decoding: can't read framedata.bin\n");
                exit(4);
            }
        }
        int size_output = static_cast<int>(get_filesize(output_path.string()));
        double ratio = static_cast<double>(size_output) / size_input;
        auto finish = std::chrono::high_resolution_clock::now();
        auto duration = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count());
        auto info = CommonInformation(ratio, duration, size_input, size_output);
        send_global_params();
        send_common_information(info);
    }



