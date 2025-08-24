//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_DTO_H
#define ARCHIVATOR_DTO_H
#include <sstream>
#include <string>
#include <vector>


struct Dto {
    bool action_;
    std::vector<std::string> options_;
    std::vector<std::string> files_;

    bool operator==(const Dto &other) const {
        if (action_ != other.action_) return false;
        if (options_.size() != other.options_.size()) return false;
        for (size_t i = 0; i < options_.size(); ++i) {
            if (options_[i] != other.options_[i]) return false;
        }
        if (files_.size() != other.files_.size()) return false;
        for (size_t i = 0; i < files_.size(); ++i) {
            if (files_[i] != other.files_[i]) return false;
        }
        return true;
    }

    explicit Dto(bool action, std::vector<std::string> options, std::vector<std::string> files) : action_(action),
                                                                                                  options_(std::move(
                                                                                                          options)),
                                                                                                  files_(std::move(
                                                                                                          files)) {}

    static std::string to_string(const Dto &dto) {
        std::ostringstream oss;
        oss << "action: " << dto.action_ << '\n';
        oss << "options: ";
        for (const auto &option: dto.options_)
            oss << option << ' ';
        oss << "\nfiles: ";
        for (const auto &option: dto.files_)
            oss << option << ' ';
        return oss.str();
    }
};

#endif
