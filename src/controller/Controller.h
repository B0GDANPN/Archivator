//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_CONTROLLER_H
#define ARCHIVATOR_CONTROLLER_H


#include <string>
#pragma once
class Controller {
public:
    static void run(int argc, char *argv[]);
    static void sendMesssage(const std::string& message);
};


#endif //ARCHIVATOR_CONTROLLER_H
