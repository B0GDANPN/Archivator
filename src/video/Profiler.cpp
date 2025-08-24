#include "video/Profiler.hpp"

#include <numeric>

CompressParams Profiler::analyze(const std::string &path) {
    NoiseData nd = noise_analyzer(path);
    double solid = solid_analyzer(path);
    double dynamic = nd.duration / nd.scenes;

    // logger << "Duration: " << nd.duration << std::endl;
    // logger << "Scene motion: " << nd.sceneMotion << std::endl;
    // logger << "Scenes: " << nd.scenes << std::endl;
    // logger << "Solid: " << solid << std::endl;
    // logger << "Dynamic: " << dynamic << std::endl;
    CompressParams params = calculate(nd.scene_motion, nd.scenes, solid, dynamic);
    return params;
}

CompressParams Profiler::calculate(double scene_motion, int scenes, double solid, double dynamic) {

    size_t noizes = dynamic * 10e6;
    size_t noizes_per_subframe = noizes / 5;
    size_t solid_difference = solid * 100;

    return CompressParams(noizes, noizes_per_subframe, solid_difference);
}
std::vector<int> Profiler::get_frame_set(cv::VideoCapture& cap) {
    cv::Mat prev_frame, current_frame, frame_diff;
    bool first_frame = true;
    std::vector<int> frameset;
    while (true) {
        cap.read(current_frame);
        if (current_frame.empty()) {
            //std::cout << "End of video." << std::endl;
            break;
        }

        if (first_frame) {
            prev_frame = current_frame.clone();
            first_frame = false;
        }
        else {
            cv::absdiff(current_frame, prev_frame, frame_diff);
            cv::cvtColor(frame_diff, frame_diff, cv::COLOR_BGR2GRAY);
            cv::threshold(frame_diff, frame_diff, 25, 255, cv::THRESH_BINARY);
            frameset.push_back(cv::countNonZero(frame_diff));
            prev_frame = current_frame.clone();
        }
    }
    return frameset;
}

NoiseData Profiler::noise_analyzer(const std::string &path) {
    cv::VideoCapture cap(path);
    int frame_count = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    double duration = frame_count / fps;

    std::vector<int> frameset = get_frame_set(cap);
    auto [scenes, averageSceneMotion] = analyze_video_motion(frameset);

    // logger << "Duration: " << duration << std::endl;
    // logger << "Scene Changes: " << scenes << "\nAverage Scene Motion: " << averageSceneMotion << std::endl;
    cap.release();

    return NoiseData{ duration, averageSceneMotion, scenes };
}

bool Profiler::is_single_color(const cv::Mat& sub_mat, double threshold) {
    cv::Mat mean, stddev;
    meanStdDev(sub_mat, mean, stddev);

    return (stddev.at<double>(0) < threshold &&
        stddev.at<double>(1) < threshold &&
        stddev.at<double>(2) < threshold);
}

std::pair<int, int> Profiler::analyze_video_motion(const std::vector<int>& movements) {
    double mean = std::accumulate(movements.begin(), movements.end(), 0.0) / movements.size();

    double sq_sum = std::inner_product(movements.begin(), movements.end(), movements.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / movements.size() - mean * mean);
    double threshold = mean;
    int scene_changes = 0;
    std::vector<double> scene_movements;
    std::vector<int> current_scene_movement;

    for (int movement : movements) {
        if (movement > threshold) {
            scene_changes++;
            if (!current_scene_movement.empty()) {
                double avg_movement = std::accumulate(current_scene_movement.begin(), current_scene_movement.end(), 0.0) / current_scene_movement.size();
                scene_movements.push_back(avg_movement);
                current_scene_movement.clear();
            }
        }
        else {
            current_scene_movement.push_back(movement);
        }
    }

    if (!current_scene_movement.empty()) {
        const double avg_movement = std::accumulate(current_scene_movement.begin(), current_scene_movement.end(), 0.0) / current_scene_movement.size();
        scene_movements.push_back(avg_movement);
    }

    double average_scene_motion = 0.0;
    if (!scene_movements.empty()) {
        average_scene_motion = std::accumulate(scene_movements.begin(), scene_movements.end(), 0.0) / scene_movements.size();
    }

    return { scene_changes, average_scene_motion };
}

double Profiler::solid_analyzer(const std::string &path) {
    cv::VideoCapture cap(path);

    int total_sub_mats = 0;
    int single_color_sub_mats = 0;
    double threshold = 50.0;

    cv::Mat frame;
    cap.read(frame);

    const int sub_mat_size = frame.cols / 2;

    while (cap.read(frame)) {
        const int frame_rows = frame.rows;
        const int frame_cols = frame.cols;

        for (int i = 0; i < frame_rows; i += sub_mat_size) {
            for (int j = 0; j < frame_cols; j += sub_mat_size) {
                const int sub_mat_rows = std::min(sub_mat_size, frame_rows - i);
                const int sub_mat_cols = std::min(sub_mat_size, frame_cols - j);

                cv::Mat sub_mat = frame(cv::Rect(j, i, sub_mat_cols, sub_mat_rows));

                total_sub_mats++;
                if (is_single_color(sub_mat, threshold)) {
                    single_color_sub_mats++;
                }
            }
        }
    }

    double metric = static_cast<double>(single_color_sub_mats) / total_sub_mats;
    cap.release();

    return metric;
}
