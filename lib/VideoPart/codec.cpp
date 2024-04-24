#include <opencv2/opencv.hpp>
#include <fstream>

#include "codec.h"
#include "compress.h"
#include "decompress.h"
#include "logger.h"
#include "test.h"

//int main(int argc, char* argv[]) {
int main() {
    std::string framedata = "sampleZip/framedata.csv";
    std::string matdata = "sampleZip/matdata.bin";
    std::string subframedata = "sampleZip/subframe/";
    
    std::string command = "test";

    logger.switchToConsole();
    //logger.switchToFile();

    if (command == "encode") {
        logger << "Encoding..." << std::endl;
        std::ofstream ofs("sampleZip/framedata.csv");
        cv::VideoCapture cap("sample2.mp4");

        if (!cap.isOpened()) {
            logger << "Failed to open the video file." << std::endl;
            return -1;
        }

        cv::Mat frame, frame2, dst;
        size_t start_scene = 0;
        size_t end_scene = 0;

        cap.read(frame);
        cap.read(frame2);

        size_t sum;
        bool end_flag = false;

        logger << frame.size << std::endl;
        ofs << frame.rows << "," << frame.cols << std::endl;

        while (!frame.empty()) {

            while (true) {
                cap.read(frame2);
                if (frame2.empty()) {
                    break;
                    end_flag = true;
                }
                cv::subtract(frame, frame2, dst);
                sum = cv::sum(dst)[0];
                end_scene++;
                if (sum > NOIZES) break;
            }

            if (start_scene == cap.get(cv::CAP_PROP_FRAME_COUNT) - 1) break;

            auto matricies = splitMatrices(frame, dst, NOIZES_PER_SUBFRAME);

            logger << "Frames from " << start_scene 
                << " to " << end_scene 
                << " Count of mats " << matricies.size() 
                << std::endl;
            
            ofs << start_scene << "," << end_scene << "," << matricies.size() << std::endl;

            writeMatricesAndPoints(matricies, "sampleZip/matdata.bin");

            cap.set(cv::CAP_PROP_POS_FRAMES, start_scene);

            std::vector<cv::Vec3b> subFrameBuffer;
            while (start_scene != end_scene) {
                cap.read(frame);
                writeNumbersExcludingSubmatrices(frame, matricies, subFrameBuffer);
                start_scene++;
            }
            cap.read(frame);
            std::string filename = "sampleZip/subframe/";
            logger << "Size of buffer: " << subFrameBuffer.size() * sizeof(cv::Vec3b) / 1024 << std::endl;
            writeBufferToFile(subFrameBuffer, filename, SUBFRAME_DIFFERENCE);
        }
        logger << "Time for encoding: " << logger.elapsedTime() << " ms" << std::endl;
        logger << "Params: " << SPLIT_DEPTH << " " 
                             << NOIZES << " " 
                             << NOIZES_PER_SUBFRAME << std::endl;
    }
    else if (command == "decode") {
        logger << "Decoding" << std::endl;
        decode(framedata, matdata, subframedata, "output1.mp4");
        logger << "Time for decoding: " << logger.elapsedTime() << " ms" << std::endl;
        logger << "Params: " << SPLIT_DEPTH << " "
            << NOIZES << " "
            << NOIZES_PER_SUBFRAME << std::endl;
        return 0;
    }
    else if (command == "test") {
        diagnostic();
    }
    else {
        logger << "Unknown command" << std::endl;
    }

    return 0;
}
