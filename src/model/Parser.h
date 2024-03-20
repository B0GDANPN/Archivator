#ifndef ARCHIVATOR_PARSER_H
#define ARCHIVATOR_PARSER_H
#pragma once

#include "../dto/Dto.h"
#include <vector>
#include <string>

class Parser {
public:
    //"{nameOfFileOrDirectory} -{encodeOrDecode} -{option1} -{option2} .... -{optionN}"
    static std::vector<Dto> parse(const std::vector<std::string> &lines);

    static std::vector<std::string> getVectorArgvFromTxt(const std::vector<std::string> &namesOfFiels);

private:
    static bool isDirectory(const std::string &line);

    static bool isExist(const std::string &line);
};
/*
 *  строки ->Parser
 *  для каждой строки выбирает алгоритм парсинга и флаги
 *  возвращает в main массив структур {имя, алгоритм, флаги}
 *
 *
 *
 *
 *
 * */

#endif //ARCHIVATOR_PARSER_H
