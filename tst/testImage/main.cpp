#include "../../lib/ImagePart/FractalAlgo.h"

int main() {
    fs::path currentPath = fs::current_path();
    fs::current_path("..");
    std::ostringstream oss;
    std::ostringstream &ref_oss = oss;
    FractalAlgo fractalAlgo{true, "", ref_oss};
    std::string sourceImage1 = "Lena.bmp";
    std::string sourceImage2 = "Pingvin.jpeg";
    std::string sourceImage3 = "sample.tga";

    fractalAlgo.encode(sourceImage1, 1000);
    fractalAlgo.encode(sourceImage2, 750);
    fractalAlgo.encode(sourceImage3, 500);

    fractalAlgo.decode(sourceImage1, 2);
    fractalAlgo.decode(sourceImage1, 3);
    fractalAlgo.decode(sourceImage3, 4);
    return 0;
}