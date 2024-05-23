#include "controller/Controller.h"
#include "view/View.h"
#include <vector>
#pragma once

int main() {
    std::vector<std::string> argv = {"enc -o 600 -f Lena.bmp",
                                     "dec -o 5 -f storageEncoded/Lena.bmp",
                                     "enc -f sss.pdf",
                                     "dec -f sss.pdf.hcf"
                                     };
    std::string str = "enc -o 500 -f ../Lena.bmp\ndec -o 5 -f Lena.bmp";
    View::start();
    //Controller controller{true, ""};
    //std::string res = controller.start(str);
    //delete &controller;
    return 0;
}