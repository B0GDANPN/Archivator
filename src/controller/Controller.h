//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_CONTROLLER_H
#define ARCHIVATOR_CONTROLLER_H

#include "../dto/CommonInformation.h"
#include <string>
#include <fstream>
#include <sstream>
#include <utility>
#include <iostream>
//#include "../view/View.h"
#pragma once
class IController {
public:
    bool isTextOutput;
    std::string outputFile;
    //View view;

    explicit IController(bool isTextOutput, std::string  outputFile/*,const View& view*/) : isTextOutput(isTextOutput), outputFile(std::move(outputFile))/*, view(view) */{}
    void sendMessage(const std::string& message) const{
        if (isTextOutput){
            std::cout<<message;
            //view.writeOutput(error);
        } else{
            std::ofstream file(outputFile, std::ios::app);

            if(file.is_open()) {
                file << message;
                file.close();
            } else {
                //view.writeOutput("Error: Unable to open Output file.\n";
                exit(-1);
            }
        }
    };
    virtual void sendCommonInformation(const CommonInformation& commonInformation) {
        std::stringstream oss;
        oss<<"Compression ratio: "<<commonInformation.compressionRatio<<"\n"<<
           "Time: "<<commonInformation.time<<'\n'<<
           "Size input data: "<<commonInformation.sizeInputData<<
           "Size output data: "<<commonInformation.sizeOutputData<<'\n';
        std::string tmp=oss.str();
        sendMessage(tmp);
    };
    virtual void sendErrorInformation(const std::string& error){
        if (isTextOutput){
            std::cout<<error;
            //view.writeOutput(error);
        } else{
            std::ofstream file(outputFile, std::ios::app);

            if(file.is_open()) {
                file << error;
                file.close();
            } else {
                //view.writeOutput("Error: Unable to open Output file.\n";
                exit(-1);
            }
        }
    };
};
class Controller : public IController {
public:
    void run(int argc, char *argv[]);

};


#endif //ARCHIVATOR_CONTROLLER_H
