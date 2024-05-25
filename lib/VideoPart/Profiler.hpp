#ifndef PROFILER_H
#define PROFILER_H

#include <string>
#include <opencv2/opencv.hpp>
#include <vector>
#include <numeric>

struct NoizeData {
    double duration;
    int sceneMotion;
    int scenes;
};

struct CompressParams {
    int NOIZES = -1;
    int NOIZES_PER_SUBFRAME = -1;
    int SOLID_DIFFERENCE = -1;
};

class Profiler {
private:
    std::vector<int> getFrameSet(cv::VideoCapture& cap);
    NoizeData noize_analizer(std::string path);
    bool isSingleColor(const cv::Mat& subMat, double threshold);
    double solid_analizer(std::string path);
    std::pair<int, int> analyzeVideoMotion(const std::vector<int>& movements);
    CompressParams calculate(double sceneMotion, int scenes, double solid, double dynamic);
public:
    CompressParams avalize(std::string path);
};

#endif 
