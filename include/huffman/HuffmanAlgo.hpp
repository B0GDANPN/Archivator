#ifndef ARCHIVATOR_HUFFMAN_HPP
#define ARCHIVATOR_HUFFMAN_HPP

#include <map>
#include <string>
#include <controller/Controller.hpp>
#include <dto/BitSteam.hpp>

/**
 * @brief Huffman coding algorithm for general file compression.
 *
 * Provides methods to encode a file using Huffman coding and decode a Huffman-compressed file. This algorithm is suited for lossless compression of text or binary data.
 * The output format is a custom ".hcf" (Huffman Compressed File) containing the Huffman tree and encoded data.
 */
class HuffmanAlgo final : public IController {
    /**
         * @brief Internal node of the Huffman tree.
         *
         * Represents a node in the Huffman binary tree, used during tree construction and traversal. Each node may have a character (for leaves), a frequency count, and pointers to left/right children.
         */
    struct HuffmanNode {
        /// Frequency of the character or sum of frequencies for an internal node.
        uint32_t freq;
        /// Data byte (valid only for leaf nodes; undefined for internal nodes where this may be 0).
        unsigned char data;
        /// Pointer to left child node (nullptr if leaf).
        std::shared_ptr<HuffmanNode> left;
        /// Pointer to right child node (nullptr if leaf).
        std::shared_ptr<HuffmanNode> right;

        /**
         * @brief Construct a HuffmanNode (leaf or internal).
         * @param data Byte value (character) for a leaf node, or 0 for an internal node.
         * @param frequency Frequency of occurrence (for leaves) or combined frequency (for internal nodes).
         * @param left Pointer to left child (optional, for internal nodes).
         * @param right Pointer to right child (optional, for internal nodes).
         */
        explicit HuffmanNode(unsigned char data, uint32_t frequency, HuffmanNode *left=nullptr, HuffmanNode *right=nullptr) : freq(frequency),
                                                                                                         data(data),
                                                                                                         left(left),
                                                                                                         right(right) {}
        /**
                 * @brief Comparator for prioritizing nodes in a min-heap.
                 *
                 * Enables using `std::shared_ptr<HuffmanNode>` in a priority queue sorted by frequency (smallest frequency has highest priority).
                 */
       struct Cmp {
          bool operator()(const std::shared_ptr<HuffmanNode>& a,
                          const std::shared_ptr<HuffmanNode>& b) const noexcept {
            return a->freq > b->freq; // min-heap via greater
          }};

    };
    /**
     * @brief Build a Huffman tree from frequency data.
     * @param freq A vector of size 256 containing frequency of each byte value (0-255).
     * @return Root node of the constructed Huffman tree (as a shared_ptr), or nullptr if input is empty.
     *
     * This static helper uses a min-heap to construct the optimal Huffman binary tree for the given frequency distribution.
     */
   static std::shared_ptr<HuffmanNode> build_huffman_tree(const std::vector<uint32_t>& freq);

    /**
 * @brief Override: Send common info with "HuffmanAlgo" tag.
 * @param common_information Compression info (ratio, time, sizes).
 *
 * Outputs the summary information enclosed in a "HuffmanAlgo{ ... }" tag to distinguish messages from this algorithm.
 */
    void send_common_information(const CommonInformation &common_information) override;
  /**
       * @brief Override: Send error info with "HuffmanAlgo" tag.
       * @param error Error message string.
       *
       * Formats error messages to include "HuffmanAlgo{ ... }" for clarity, then delegates to the base error handling.
       */
    void send_error_information(const std::string &error) override ;
  /**
       * @brief Write a Huffman tree structure to a bit stream.
       * @param root Root node of the Huffman tree.
       * @param stream BitStream to write into.
       *
       * Performs a pre-order traversal of the Huffman tree and writes it into the bit stream in a serialized form.
       * Leaf nodes are marked and followed by the byte data; internal nodes are marked and followed recursively by their children.
       */
  static void write_tree_to_stream(const std::shared_ptr<HuffmanNode>& root, BitStream &stream) ;
  /**
       * @brief Read a Huffman tree structure from a bit stream.
       * @param stream BitStream to read from.
       * @return Root node of the reconstructed Huffman tree.
       *
       * Reads a serialized Huffman tree (written by write_tree_to_stream) from the stream and reconstructs the tree structure.
       * This is used during decoding to retrieve the original tree for bit interpretation.
       */
    static std::shared_ptr<HuffmanNode> read_tree_from_stream(BitStream &stream);
  /**
       * @brief Generate Huffman code strings for each byte.
       * @param node Current node in the Huffman tree.
       * @param code The prefix code string accumulated so far (use "" for initial call).
       * @param codes Map from byte value to its Huffman code string, which will be populated by this function.
       *
       * Traverses the Huffman tree and assigns a code to each leaf (byte) found. Left edges add "0" and right edges add "1" to the code prefix.
       * On completion, `codes` will contain an entry for each byte present in the tree, mapping to its binary string code.
       */
    static void generate_codes(const  std::shared_ptr<HuffmanNode>& node, const std::string &code, std::map<unsigned char, std::string> &codes);

public:
  /**
     * @brief Constructs the Huffman algorithm handler.
     * @param is_text_output If true, send output messages to text stream; if false, to log file.
     * @param output_file Output log file path (only if not text output).
     * @param ref_oss Reference to output string stream (for text mode).
     */
    explicit HuffmanAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
            : IController(is_text_output, output_file, ref_oss) {
    }
  /**
       * @brief Compress a file using Huffman coding.
       * @param input_filename Path to the input file to compress.
       * @param mod Optional label to identify this operation in messages (default "Huffman").
       * @param size_input1 Optional precomputed input size (if -1, the size will be determined automatically).
       * @param duration1 Optional accumulated duration in milliseconds to add (use 0 for none).
       *
       * Reads the entire input file, computes frequency of each byte, builds the Huffman tree, and writes out a compressed file in "storageEncoded" with extension ".hcf".
       * The compressed file begins with the size of the encoded data, a padding byte count, followed by the serialized Huffman tree and the bit-coded content.
       * On success, writes a message with the output file path; on failure to write output, sends an error and terminates the program.
       * Also calculates compression ratio and time, and outputs them via `send_common_information`.
       */
    void encode(const std::string &input_filename, const std::string &mod = "Huffman",int size_input1=-1,long duration1=0);


  /**
       * @brief Decompress a Huffman-compressed file.
       * @param input_filename Path to the ".hcf" file to decompress.
       *
       * Reads the Huffman tree and encoded data from the given file, reconstructs the original content, and writes it to "storageDecoded/" with the original filename (extension removed).
       * The output file is the exact original data prior to compression.
       * This method measures the time and size information and outputs the compression ratio (which should match original compression) and other metrics.
       * If any file operation fails (e.g., cannot open output), an error is logged and the program may terminate.
       */
    void decode(const std::string &input_filename);
};

#endif