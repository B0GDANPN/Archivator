//
// Created by bogdan on 11.03.24.
//
#include "controller/Controller.h"
//#include "view/View.h"
#include <vector>
int main() {
    std::vector<std::string> argv = {"enc -o 500 -f Lena.bmp Lena_encoded.json",
                                     "enc -o 500 -f Lena.bmp",
                                     "dec -o 5 -f Lena_encoded.json",
                                     "dec -o 5 -f Lena_encoded.json Lena.bmp"};
    //View view* =new View();
    //std::vector<std::string> argv =view.getArgv();
    Controller controller =*new Controller(true,"");
    controller.run(argv);
    return 0;
}