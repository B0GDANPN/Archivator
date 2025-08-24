#ifndef ARCHIVATOR_HUFFMAN_HPP
#define ARCHIVATOR_HUFFMAN_HPP

#include <map>
#include <string>
#include <controller/Controller.hpp>
#include <dto/BitSteam.hpp>

class HuffmanAlgo final : public IController {
    struct HuffmanNode {
        int freq;
        unsigned char data;
        HuffmanNode *left;
        HuffmanNode *right;

        explicit HuffmanNode(unsigned char data, int frequency, HuffmanNode *left=nullptr, HuffmanNode *right=nullptr) : freq(frequency),
                                                                                                         data(data),
                                                                                                         left(left),
                                                                                                         right(right) {}

        ~HuffmanNode() {
            delete left;
            delete right;
        }

        bool operator()(const HuffmanNode *a, const HuffmanNode *b) const {
            return a->freq > b->freq;
        }

    };

    void send_common_information(const CommonInformation &common_information) override;

    void send_error_information(const std::string &error) override ;
    static void clear_huffman_root(const HuffmanNode * root);
    static void write_tree_to_stream(const HuffmanNode *root, BitStream &stream) ;

    static HuffmanNode *read_tree_from_stream(BitStream &stream);

    static void generate_codes(const HuffmanNode *node, const std::string &code, std::map<unsigned char, std::string> &codes);

public:
    explicit HuffmanAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss)
            : IController(is_text_output, output_file, ref_oss) {
    }

    void encode(const std::string &input_filename, const std::string &mod = "Huffman",int size_input1=-1,long duration1=0);



    void decode(const std::string &input_filename);
};

#endif