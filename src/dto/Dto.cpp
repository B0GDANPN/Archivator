//
// Created by bogdan on 11.03.24.
//

#include "Dto.h"

#include <utility>

Dto::Dto(std::string name, bool action, std::vector<std::string> options) : name_(std::move(name)), action_(action),
                                                                            options_(std::move(options)) {}
