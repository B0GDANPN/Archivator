#ifndef ARCHIVATOR_PARSER_HPP
#define ARCHIVATOR_PARSER_HPP

#include <vector>
#include <string>
#include <dto/Dto.hpp>

/**
 * @brief Command-line parser for user input.
 *
 * Provides utilities to parse text-based input (from a string or file) into structured Dto objects that represent actions, options, and files.
 * It supports commands prefixed by "enc" or "dec", followed by option flags and file flags.
 */
class Parser {
public:
  /**
   * @brief Parse lines of text into Dto commands.
   * @param lines A vector of strings, each string being one command line input.
   * @return A vector of Dto objects representing the parsed commands.
   *
   * Each line should start with "enc" (for compression) or "dec" (for decompression).
   * Options after "-o" and files after "-f" are collected into the Dto. Lines that do not start with "enc"/"dec` are ignored.
   */
  static std::vector<Dto> parse(const std::vector<std::string> &lines);

  /**
   * @brief Read commands from a text file.
   * @param name_of_file Path to a text file containing commands (one per line).
   * @return A vector of strings, where each element is a line from the file.
   *
   * This function opens the given file and reads it line by line. It is used to support passing a text file of commands to the controller.
   * Returns an empty vector if the file cannot be opened.
   */
  static std::vector<std::string> get_arg_from_txt(const std::string &name_of_file);
};
#endif