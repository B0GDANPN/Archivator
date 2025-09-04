#ifndef ARCHIVATOR_SELECTOR_HPP
#define ARCHIVATOR_SELECTOR_HPP


#include <string>
#include <filesystem>
#include <dto/Dto.hpp>
#include <dto/AlgorithmEnum.hpp>

/**
 * @brief Helper to select the appropriate algorithm based on file type and action.
 *
 * The Selector contains logic to map file extensions and action types (encode/decode) to a corresponding AlgorithmEnum.
 * It determines which compression algorithm should handle a given input file.
 */
class Selector {
public:
    /**
     * @brief Determine algorithm from a Dto.
     * @param dto The Dto containing action and file information.
     * @return An AlgorithmEnum indicating which algorithm to use.
     *
     * This examines the first file in the Dto and uses its extension along with the action (encode or decode) to choose an algorithm.
     * For example, if encoding a ".mp4" file, returns QUANTIZATION; if encoding an image file (".bmp", ".jpg", etc.), returns FRACTAL;
     * if encoding a ".wav", returns FLAC; otherwise defaults to HUFFMAN for other file types.
     * For decoding, it checks the file extension of the compressed file (e.g., ".hcf" for Huffman, ".flac" for Flac).
     */
    static AlgorithmEnum get_algorithm_from_dto(const Dto &dto);

    /**
     * @brief Determine algorithm from a file name and action.
     * @param name File name or path.
     * @param action Boolean indicating encode (`true`) or decode (`false`).
     * @return AlgorithmEnum corresponding to the file type and action.
     *
     * This function inspects the file extension in `name`. If `action` is true (encoding), it selects:
     * - QUANTIZATION for ".mp4" videos
     * - FRACTAL for image formats (".tga", ".jpg", ".jpeg", ".bmp")
     * - FLAC for ".wav" audio
     * - HUFFMAN for any other file type (default text/general compression).
     * If `action` is false (decoding), it expects `name` to be a compressed file or directory:
     * - QUANTIZATION if extension is empty (assumes a directory name for video)
     * - FLAC for ".flac" files
     * - HUFFMAN for ".hcf" files
     * - ERROR if none of the above match.
     */
    static AlgorithmEnum get_algorithm_from_name(const std::string &name, bool action);
};

#endif