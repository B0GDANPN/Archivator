//
// Created by bogdan on 24.04.24.
//

#ifndef ARCHIVATOR_ICONTROLLER_HPP
#define ARCHIVATOR_ICONTROLLER_HPP

#include <dto/CommonInformation.hpp>
#include <string>
#include <fstream>
#include <utility>



/**
 * @brief Interface (base class) for controller components handling I/O and messages.
 *
 * Provides a common interface for classes that produce output messages (status, errors, metrics) and perform file size queries.
 * It holds configuration for output behavior (text vs file output) and provides utility methods for derived controllers and algorithms.
 */
class IController {
public:
    virtual ~IController() = default;

    /// If `true`, output messages are directed to an in-memory text stream (`oss`). If `false`, they are written to a file.
    bool is_text_output;
    /// Path to an output file where messages should be written if `is_text_output` is false.
    std::string output_file;
    /// Reference to a string stream for accumulating output messages (used when `is_text_output` is true).
    std::ostringstream &oss;

    /**
     * @brief Constructs the IController.
     * @param is_text_output If true, enable text mode output (write to `oss`); if false, write output to a file.
     * @param output_file Path to the output file (only used if `is_text_output` is false).
     * @param ref_oss Reference to a string stream for text output accumulation.
     */
    explicit IController(bool is_text_output, std::string output_file, std::ostringstream &ref_oss)
        : is_text_output(is_text_output),
          output_file(std::move(output_file)),
          oss(ref_oss) {}

    /**
     * @brief Send a generic message to the configured output.
     * @param message The string message to send.
     *
     * If in text mode, the message is appended to the internal string stream (`oss`). If in file mode, the message is appended to the output file.
     * If file output fails (e.g., cannot open file), the application will terminate.
     */
    void send_message(const std::string &message) const;

    /**
     * @brief Send common compression info to output.
     * @param common_information Struct containing compression ratio, time, input/output sizes.
     *
     * Formats and outputs a summary of the compression/decompression results. By default, it prints the compression ratio, time in milliseconds,
     * and input/output data sizes. Derived classes may override to prepend or format this message differently.
     */
    virtual void send_common_information(const CommonInformation &common_information);

    /**
     * @brief Send an error message to the output.
     * @param error The error message string.
     *
     * This default implementation simply forwards the error string to `send_message`. Derived classes can override to add context information.
     */
    virtual void send_error_information(const std::string &error);

    /**
     * @brief Get the size of a file in bytes.
     * @param filename Path to the file.
     * @return The file size in bytes, or -1 if the file cannot be opened.
     *
     * Opens the file in binary mode and seeks to the end to determine its length.
     */
    static std::streamsize get_filesize(const std::string &filename);
};

#endif
