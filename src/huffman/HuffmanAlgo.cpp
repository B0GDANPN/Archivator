#include <chrono>
#include <queue>
#include <huffman/HuffmanAlgo.hpp>

void HuffmanAlgo::send_common_information(const CommonInformation& common_information)
{
  send_message("HuffmanAlgo{ ");
  IController::send_common_information(common_information);
  send_message("}\n");
}
void HuffmanAlgo::send_error_information(const std::string& error)
{
    IController::send_error_information("HuffmanAlgo{ " + error + "}\n");
}
void HuffmanAlgo::clear_huffman_root(const HuffmanNode* root)
{
  if (root->left!=nullptr){ clear_huffman_root(root->left); }
  if (root->right!=nullptr){ clear_huffman_root(root->right); }
  delete root;
}
void HuffmanAlgo::write_tree_to_stream(const HuffmanNode* root, BitStream& stream)
{
  if (root == nullptr)
    return;
  if (root->left == nullptr && root->right == nullptr) {
    stream.addBit(true);
    stream.addByte(root->data);
  } else {
    stream.addBit(false);
    write_tree_to_stream(root->left, stream);
    write_tree_to_stream(root->right, stream);
  }
}
HuffmanAlgo::HuffmanNode* HuffmanAlgo::read_tree_from_stream(BitStream& stream)
{
  if (stream.getBit()) {
    unsigned char data = stream.getByte();
    return new HuffmanNode(data, 0);
  }
  HuffmanNode *left = read_tree_from_stream(stream);
    HuffmanNode *right = read_tree_from_stream(stream);
    return new HuffmanNode('\0', 0, left, right);
}
void HuffmanAlgo::generate_codes(const HuffmanNode* node, const std::string& code, std::map<unsigned char, std::string>& codes)
{
  if (node == nullptr) return;
  if (node->left == nullptr && node->right == nullptr) {
    codes[node->data] = code;
  }
  generate_codes(node->left, code + "0", codes);
  generate_codes(node->right, code + "1", codes);
}
void HuffmanAlgo::encode(const std::string& input_filename, const std::string& mod, int size_input1, long duration1)
{
        auto start = std::chrono::high_resolution_clock::now();
        int size_input;
        if(size_input1==-1){
            size_input = static_cast<int>(get_filesize(input_filename));
        } else{
            size_input=size_input1;
        }
        size_t last_slash_pos = input_filename.find_last_of('/');
        std::string tmpinput_filename =
                last_slash_pos != std::string::npos ? input_filename.substr(last_slash_pos + 1) : input_filename;
        std::string output_filename = "storageEncoded/" + tmpinput_filename + ".hcf";// путь сохранения

        std::ifstream input_file(input_filename, std::ios::binary);
        std::string text((std::istreambuf_iterator<char>(input_file)), (std::istreambuf_iterator<char>()));
        input_file.close();

        int frequencies[256] = {};
        for (unsigned char c: text) {
            frequencies[c]++;
        }
        auto comp = [](const HuffmanNode *a, const HuffmanNode *b) { return a->freq > b->freq; };
        std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, decltype(comp)> pq(comp);
        for (int i = 0; i < 256; i++) {
            if (frequencies[i] != 0) {
                pq.push(new HuffmanNode(static_cast<unsigned char>(i), frequencies[i]));
            }
        }

        while (pq.size() > 1) {
            HuffmanNode *left = pq.top();
            pq.pop();
            HuffmanNode *right = pq.top();
            pq.pop();
            auto new_node = new HuffmanNode('\0', left->freq + right->freq, left, right);
            pq.push(new_node);
        }

        HuffmanNode *root = pq.top();
        std::map<unsigned char, std::string> codes;

        generate_codes(root, "", codes);

        BitStream bit_stream{};
        write_tree_to_stream(root, bit_stream);
        clear_huffman_root(root);

        for (unsigned char c: text) {
            std::string code = codes[c];
            for (unsigned char bit: code) {
                bit_stream.addBit(bit - static_cast<unsigned char>('0'));
            }
        }

        if (std::ofstream output_file(output_filename, std::ios::binary);output_file.is_open()) {
            size_t size = bit_stream.data.size();
            output_file.write(reinterpret_cast<const char *>(&size), sizeof(size_t));
            uint8_t a = size * 8 - bit_stream.bitIndex;
            output_file.write(reinterpret_cast<const char *>(&a), sizeof(uint8_t));
            output_file.write(reinterpret_cast<const char *>(bit_stream.data.data()), size);
            output_file.close();
            send_message(mod + " data saved to: " + output_filename + '\n');
        } else {
            send_error_information("Failed to write Huffman file.\n");
            exit(-1);
        }
        int size_output = static_cast<int>(get_filesize(output_filename));
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double ratio = static_cast<double>(size_output) / size_input;
        auto info = CommonInformation(ratio,
                                      duration.count()+duration1, size_input, size_output);
        send_message(mod + "Algo{ ");
        IController::send_common_information(info);
        send_message("}\n");

    }
void HuffmanAlgo::decode(const std::string& input_filename)
{
        auto start = std::chrono::high_resolution_clock::now();
        int size_input = static_cast<int>(get_filesize(input_filename));
        size_t last_slash_pos = input_filename.find_last_of('/');
        std::string tmp_input_filename =
                last_slash_pos != std::string::npos ? input_filename.substr(last_slash_pos + 1) : input_filename;
        size_t pos = tmp_input_filename.rfind(".hcf");
        std::string output_filename = "storageDecoded/" + tmp_input_filename.substr(0, pos);// путь сохранения

        std::ofstream out_file(output_filename, std::ios::binary);

        std::ifstream input_file(input_filename, std::ios::binary);

        size_t size;
        input_file.read(reinterpret_cast<char *>(&size), sizeof(size_t));

        uint8_t max_idx = 0;
        input_file.read(reinterpret_cast<char *>(&max_idx), sizeof(uint8_t));

        std::vector<uint8_t> data(size);
        input_file.read(reinterpret_cast<char *>(data.data()), size);

        input_file.close();

        BitStream bit_stream(data);

        HuffmanNode *root = read_tree_from_stream(bit_stream);
        HuffmanNode *current = root;

        while (bit_stream.bitIndex < bit_stream.data.size() * 8 - max_idx) {
            if (bit_stream.getBit()) {
                current = current->right;
            } else {
                current = current->left;
            }
            if (current->left == nullptr && current->right == nullptr) {
                out_file << current->data;
                current = root;
            }
        };
        clear_huffman_root(root);
        out_file.close();

        int size_output = static_cast<int>(get_filesize(output_filename));
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double ratio = static_cast<double>(size_output) / size_input;
        auto info = CommonInformation(ratio,
                                      duration.count(), size_input, size_output);
        send_common_information(info);
    }
