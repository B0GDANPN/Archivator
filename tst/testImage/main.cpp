#include <csignal>
#include "../../lib/ImagePart/FractalAlgo.h"
int main(){
    chdir("..");
    auto fractalAlgo=new FractalAlgo(true,"");
    //std::string sourceImage1="Lena.bmp";
    //std::string encodedImage1="Lena_encoded.json";
    //std::string decodedImage1="Lena_decoded.bmp";
    //fractalAlgo->encode(sourceImage1,encodedImage1,500);
    //fractalAlgo->decode(encodedImage1,decodedImage1,5);
    //std::string sourceImage2="Pingvin.jpeg";
    //fractalAlgo->encode(sourceImage2,500);
    //fractalAlgo->decode("Pingvin_encoded.json",5);
    //std::string sourceImage3="sample.tga";
    //fractalAlgo->encode(sourceImage3,500);
    //fractalAlgo->decode("sample_encoded.json",5);
    delete fractalAlgo;
    return 0;
}