#ifndef LOGGER_H
#define LOGGER_H
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include "../../src/controller/Controller.h"
class Logger {
private:
    std::ostream* output;
    std::ofstream fileStream;
    std::ostream& consoleStream;
    std::chrono::steady_clock::time_point startTime;
    std::ostringstream oss;
    bool toFile;
    bool needTime;

    void printCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm localTime{};
        localtime_s(&localTime, &time_t_now);
        (*output) << "[" << std::put_time(&localTime, "%T") << "] ";
    }

public:
    Logger() : consoleStream(std::cout), needTime(false) {
        startTime = std::chrono::steady_clock::now();
        toFile = false;
        output = &consoleStream;
    }

    void switchToFile() {
        if (toFile) {
            consoleStream<<"Already logging to file\n";
            return;
        }
        fileStream.open("log.txt");
        output = &fileStream;
        toFile = true;
    }

    void switchToConsole() {
        if (!toFile) {
            consoleStream<<"Already logging to console\n";
            return;
        }
        output = &consoleStream;
        fileStream.close();
        toFile = false;
    }

    float elapsedTime() const {
        auto currentTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
        return static_cast<float>(duration.count()) / 1000.0f;
    }

    ~Logger() {
        if (toFile) {
            fileStream.close();
        } else{
            Controller::sendMesssage(oss.str());
        }
    }

    template<typename T>
    Logger& operator<<(const T& value) {
        (*output) << value;
        return *this;
    }

    Logger& operator<<(std::ostream& (*func)(std::ostream&)) {

        func(*output);
        return *this;
    }
};

#endif
