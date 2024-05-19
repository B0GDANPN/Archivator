#include "../../src/controller/Controller.h"

int main() {
    fs::path currentPath = fs::current_path();
    fs::current_path("..");
    fs::path dir = "storage";
    if (!fs::exists(dir))
        fs::create_directory(dir);
    fs::current_path("storage");
    std::ostringstream oss;
    std::ostringstream &ref_oss = oss;
    FractalAlgo fractalAlgo{true, "", ref_oss};
    std::string sourceImage1 = "Lena.bmp";
    std::string sourceImage2 = "Pingvin.jpeg";
    std::string sourceImage3 = "sample.tga";
    currentPath = fs::current_path();
    fractalAlgo.encode(sourceImage1, 500);
    fractalAlgo.decode(sourceImage1, 5);
    fractalAlgo.encode(sourceImage2, 900);
    fractalAlgo.decode(sourceImage2, 5);
    fractalAlgo.encode(sourceImage3, 800);
    fractalAlgo.decode(sourceImage3, 5);
    return 0;
}