//
// Created by bogdan on 24.04.24.
//
#include <atomic>
#include "../../src/controller/Controller.h"

int main() {
    fs::path currentPath = fs::current_path();
    fs::current_path("..");
    std::ostringstream oss;
    std::ostringstream &ref_oss = oss;
    Controller controller{true, "", ref_oss};

    std::vector<std::string> argv = {"enc -o 500 -f Lena.bmp Lena_encoded.json",
                                     "enc -o 500 -f Lena.bmp",
                                     "end",
                                     "gfdgffgf",
                                     "", "-o",
                                     "dec -o 5 -f Lena_encoded.json",
                                     "dec -o 5 -f Lena_encoded.json Lena.bmp",
                                     "testTxt1.txt",
                                     "testTxt2.txt"};
    std::stringstream ss;
    for (size_t i = 0; i < argv.size(); ++i) {
        if (i != 0)
            ss << '\n';
        ss << argv[i];
    }
    std::string str = ss.str();
    controller.start(str);
}