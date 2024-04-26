//
// Created by bogdan on 24.04.24.
//
#include <atomic>
#include "../../src/controller/Controller.h"

int main() {
    chdir("../testController");
    auto controller = new Controller(true, "");
    std::vector<std::string> argv = {"enc -o 500 -f Lena.bmp Lena_encoded.json",
                                     "enc -o 500 -f Lena.bmp",
                                     "dec -o 5 -f Lena_encoded.json",
                                     "dec -o 5 -f Lena_encoded.json Lena.bmp",
                                     "testTxt1.txt",
                                     "testTxt2.txt"};
    controller->run(argv);
    delete controller;
}