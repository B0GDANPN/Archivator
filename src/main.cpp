//
// Created by bogdan on 11.03.24.
//
#include "controller/Controller.h"
#include "view/View.h"
#include <vector>
#include <SFML/Graphics.hpp>

#pragma once

int main() {
    std::vector<std::string> argv = {"enc -o 500 -f Lena.bmp",
                                     "dec -o 5 -f Lena.bmp"};
    std::string str = "enc -o 500 -f ../Lena.bmp\ndec -o 5 -f Lena.bmp";
    View::start();
    //Controller controller{true, ""};
    //std::string res = controller.start(str);
    //delete &controller;
    return 0;
}