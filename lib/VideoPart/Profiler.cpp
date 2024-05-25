#include "profiler.h"

CompressParams Profiler::avalize(std::string path) {
    NoizeData nd = noize_analizer(path);
    double solid = solid_analizer(path);
    double dynamic = nd.duration / nd.scenes;

    // logger << "Duration: " << nd.duration << std::endl;
    // logger << "Scene motion: " << nd.sceneMotion << std::endl;
    // logger << "Scenes: " << nd.scenes << std::endl;
    // logger << "Solid: " << solid << std::endl;
    // logger << "Dynamic: " << dynamic << std::endl;
    CompressParams params = calculate(nd.sceneMotion, nd.scenes, solid, dynamic);
    return params;
}

CompressParams Profiler::calculate(double sceneMotion, int scenes, double solid, double dynamic) {

    size_t noizes = dynamic * 10e6;
    size_t noizes_per_subframe = noizes / 5;
    size_t solid_difference = solid * 100;

    //logger << "evaluated consts: \n\tnoizes: " << noizes
    //    << "\n\tnoizes per subframe: " << noizes_per_subframe
    //    << "\n\tsolid difference: " << solid_difference << "\n";

    return CompressParams(noizes, noizes_per_subframe, solid_difference);
}
std::vector<int> Profiler::getFrameSet(cv::VideoCapture& cap) {
    cv::Mat prevFrame, currentFrame, frameDiff;
    bool firstFrame = true;
    std::vector<int> frameset;
    while (true) {
        cap.read(currentFrame);
        if (currentFrame.empty()) {
            //std::cout << "End of video." << std::endl;
            break;
        }

        if (firstFrame) {
            prevFrame = currentFrame.clone();
            firstFrame = false;
        }
        else {
            cv::absdiff(currentFrame, prevFrame, frameDiff);
            cv::cvtColor(frameDiff, frameDiff, cv::COLOR_BGR2GRAY);
            cv::threshold(frameDiff, frameDiff, 25, 255, cv::THRESH_BINARY);
            frameset.push_back(cv::countNonZero(frameDiff));
            prevFrame = currentFrame.clone();
        }
    }
    return frameset;
}

NoizeData Profiler::noize_analizer(std::string path) {
    cv::VideoCapture cap(path);

    if (!cap.isOpened()) {
        logger << "Error: Could not open video file." << std::endl;
        return NoizeData();
    }

    int frame_count = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    double fps = cap.get(cv::CAP_PROP_FPS);
    double duration = frame_count / fps;

    std::vector<int> frameset = getFrameSet(cap);
    auto [scenes, averageSceneMotion] = analyzeVideoMotion(frameset);

    // logger << "Duration: " << duration << std::endl;
    // logger << "Scene Changes: " << scenes << "\nAverage Scene Motion: " << averageSceneMotion << std::endl;
    cap.release();

    return NoizeData{ duration, averageSceneMotion, scenes };
}

bool Profiler::isSingleColor(const cv::Mat& subMat, double threshold) {
    cv::Mat mean, stddev;
    meanStdDev(subMat, mean, stddev);

    return (stddev.at<double>(0) < threshold &&
        stddev.at<double>(1) < threshold &&
        stddev.at<double>(2) < threshold);
}

std::pair<int, int> Profiler::analyzeVideoMotion(const std::vector<int>& movements) {
    double mean = std::accumulate(movements.begin(), movements.end(), 0.0) / movements.size();

    double sq_sum = std::inner_product(movements.begin(), movements.end(), movements.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / movements.size() - mean * mean);
    double threshold = mean;
    int sceneChanges = 0;
    std::vector<double> sceneMovements;
    std::vector<int> currentSceneMovement;

    for (int movement : movements) {
        if (movement > threshold) {
            sceneChanges++;
            if (!currentSceneMovement.empty()) {
                double avgMovement = std::accumulate(currentSceneMovement.begin(), currentSceneMovement.end(), 0.0) / currentSceneMovement.size();
                sceneMovements.push_back(avgMovement);
                currentSceneMovement.clear();
            }
        }
        else {
            currentSceneMovement.push_back(movement);
        }
    }

    if (!currentSceneMovement.empty()) {
        double avgMovement = std::accumulate(currentSceneMovement.begin(), currentSceneMovement.end(), 0.0) / currentSceneMovement.size();
        sceneMovements.push_back(avgMovement);
    }

    double averageSceneMotion = 0.0;
    if (!sceneMovements.empty()) {
        averageSceneMotion = std::accumulate(sceneMovements.begin(), sceneMovements.end(), 0.0) / sceneMovements.size();
    }

    return { sceneChanges, averageSceneMotion };
}

double Profiler::solid_analizer(std::string path) {
    cv::VideoCapture cap(path);
    if (!cap.isOpened()) {
        logger << "Error opening video stream or file" << std::endl;
        return -1;
    }

    int totalSubMats = 0;
    int singleColorSubMats = 0;
    double threshold = 50.0;

    cv::Mat frame;
    cap.read(frame);

    int subMatSize = frame.cols / 2;

    while (cap.read(frame)) {
        int frameRows = frame.rows;
        int frameCols = frame.cols;

        for (int i = 0; i < frameRows; i += subMatSize) {
            for (int j = 0; j < frameCols; j += subMatSize) {
                int subMatRows = std::min(subMatSize, frameRows - i);
                int subMatCols = std::min(subMatSize, frameCols - j);

                cv::Mat subMat = frame(cv::Rect(j, i, subMatCols, subMatRows));

                totalSubMats++;
                if (isSingleColor(subMat, threshold)) {
                    singleColorSubMats++;
                }
            }
        }
    }

    double metric = static_cast<double>(singleColorSubMats) / totalSubMats;
    cap.release();

    return metric;
}
