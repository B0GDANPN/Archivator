//
// Created by bogdan on 11.03.24.
//

#ifndef ARCHIVATOR_CONTROLLER_HPP
#define ARCHIVATOR_CONTROLLER_HPP

#include <controller/IController.hpp>


/**
 * @brief Main controller that orchestrates compression and decompression tasks.
 *
 * The Controller parses user input commands, determines which compression algorithm to use,
 * and invokes the appropriate algorithm's encode/decode methods. It handles multiple commands in sequence and manages output logging.
 */
struct Controller : public IController {
  /**
   * @brief Constructs the Controller.
   * @param is_text_output If true, controller outputs messages to an internal text stream; if false, outputs to the given file.
   * @param output_file Path to output log file (used if not in text mode).
   * @param ref_oss Reference to a string stream for capturing text output.
   */
  explicit Controller(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
      : IController(is_text_output, output_file, ref_oss) {}

  /**
   * @brief Start processing input commands.
   * @param str Multi-line string containing one or more commands to execute.
   *
   * This method parses each line of the input string (each line should be a command specifying enc/dec, options, and files).
   * It creates necessary directories (for encoded/decoded data), uses Parser to interpret commands into Dto objects,
   * determines the appropriate AlgorithmEnum for each command via Selector, and then instantiates and executes the corresponding
   * algorithm class (Huffman, Flac, Fractal, Quantization) for either encoding or decoding.
   *
   * @note Exceptions thrown by algorithms (std::exception and derivatives) are caught for each command; any such error will result in an error message describing correct usage of options for that command.
   */
  void start(const std::string &str);
};
#endif