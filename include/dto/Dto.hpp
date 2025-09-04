//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_DTO_H
#define ARCHIVATOR_DTO_H
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief Data transfer object for user commands.
 *
 * Holds information about a single compress or decompress command, including the action type,
 * any options provided, and the list of target files. Used to pass parsed user input to the controller.
 */
struct Dto {
    /// Indicates the action type: `true` for encode (compression), `false` for decode (decompression).
    bool action_;
    /// List of option strings (e.g., quality levels or other parameters) provided with the command.
    std::vector<std::string> options_;
    /// List of file paths relevant to the command (one or more input files or targets).
    std::vector<std::string> files_;

    /**
     * @brief Equality comparison operator.
     * @param other Another Dto to compare with.
     * @return `true` if all fields (action, options, files) match exactly, otherwise `false`.
     */
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
    /**
     * @brief Constructs a Dto with given action, options, and files.
     * @param action Boolean indicating encode (`true`) or decode (`false`).
     * @param options Vector of option strings.
     * @param files Vector of file path strings.
     */
    explicit Dto(bool action, std::vector<std::string> options, std::vector<std::string> files) : action_(action),
                                                                                                  options_(std::move(
                                                                                                          options)),
                                                                                                  files_(std::move(
                                                                                                          files)) {}
    /**
     * @brief Formats the Dto into a readable string.
     * @param dto The Dto instance to format.
     * @return A string listing the action, options, and files contained in the Dto.
     *
     * This is used for debugging or error messages to display the contents of a Dto.
     */
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
