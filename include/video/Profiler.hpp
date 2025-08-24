#ifndef ARCHIVATOR_PROFILER_HPP
#define ARCHIVATOR_PROFILER_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include <vector>

struct NoiseData {
    double duration;
    int scene_motion;
    int scenes;
};

struct CompressParams {
    int noises = -1;
    int noises_per_subframe = -1;
    int solid_difference = -1;
};

class Profiler {
    static std::vector<int> get_frame_set(cv::VideoCapture& cap);
    static NoiseData noise_analyzer(const std::string &path);

    static bool is_single_color(const cv::Mat& sub_mat, double threshold);
    static double solid_analyzer(const std::string &path);

    static std::pair<int, int> analyze_video_motion(const std::vector<int>& movements);

    static CompressParams calculate(double scene_motion, int scenes, double solid, double dynamic);
public:
    static CompressParams analyze(const std::string &path);
};

#endif
