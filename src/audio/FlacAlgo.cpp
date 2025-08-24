#include <audio/FlacAlgo.hpp>
void ::FlacAlgo::Lpc::train(const std::vector<int16_t> &input) {
    int n = static_cast<int>(input.size());
    coeffs_.resize(kGlobalOrder + 1, 0);

    std::vector<double> r(kGlobalOrder + 1, 0); // Autocorrelation sequence

    for (int i = 0; i <= kGlobalOrder; ++i) {
        for (int j = 0; j < n - i; ++j) {
            r[i] += input[j] * input[j + i];
        }
    }

    // We perform the Levinson-Durbin method to find the LPC coefficients
    std::vector<double> alpha(kGlobalOrder + 1, 0);
    std::vector<double> kappa(kGlobalOrder + 1, 0);
    std::vector<double> am1(kGlobalOrder + 1, 0);


    am1[0] = 1;

    am1[0] = 1;
    alpha[0] = 1.0;
    coeffs_[0] = alpha[0];
    double em1 = r[0];

    for (int m = 1; m <= kGlobalOrder; ++m) {
        double sum = 0;
        for (int j = 1; j <= m - 1; ++j) {
            sum += am1[j] * r[m - j];
        }
        double km = (r[m] - sum) / em1;
        kappa[m - 1] = -static_cast<float>(km);
        alpha[m] = static_cast<float>(km);

        for (int j = 1; j <= m - 1; ++j) {
            alpha[j] = am1[j] - km * am1[m - j];
        }
        double em = (1 - km * km) * em1;
        for (int s = 0; s <= kGlobalOrder; ++s) {
            am1[s] = alpha[s];
        }
        coeffs_[m] = alpha[m];
        em1 = em;
    }

    for (int s = 0; s <= kGlobalOrder; ++s) {
        coeffs_[s] = alpha[s];
    }
}
int16_t FlacAlgo::Lpc::predict(const std::vector<int16_t> &input, size_t index) const {
    double prediction = coeffs_[0];
    for (size_t i = 1; i <= kGlobalOrder; ++i) {
        if (i <= index) {
            prediction += coeffs_[i] * input[index - i];
        }
    }
    return prediction;
}

// Linear predictive coding
void ::FlacAlgo::Lpc::clear() {
    coeffs_.clear();
}
void FlacAlgo::send_common_information(const CommonInformation &common_information)  {
    send_message("FlacAlgo{ ");
    IController::send_common_information(common_information);
    send_message("}\n");
}
void FlacAlgo::send_error_information(const std::string &error)  {
    IController::send_error_information("FlacAlgo{ " + error + "}\n");
}
void FlacAlgo::send_global_params() const  {
    std::ostringstream oss;
    oss << "FlacAlgo: kGlobalSizeBlocks: " <<kGlobalSizeBlocks  << ", kGlobalOrder: " << kGlobalOrder << ", k: "
            << kGlobalK << '\n';
    std::string str = oss.str();
    send_message(str);
}
void FlacAlgo::rice_encode(BitStream &stream, int num)  {
    if (num < 0) {
        stream.addBit(true);
        num *= -1;
    } else {
        stream.addBit(false);
    }
    int q = num >> kGlobalK;
    for (int i = 0; i < q; ++i) {
        stream.addBit(true);
    }
    stream.addBit(false);
    for (int i = 0; i < kGlobalK; ++i) {
        stream.addBit((num >> (kGlobalK - 1 - i)) & 1);
    }
}
int FlacAlgo::rice_decode(BitStream &stream)  {
    int sgn = 1;
    if (stream.getBit()) {
        sgn = -1;
    }
    int q = 0;
    while (stream.getBit() == 1) {
        q++;
    }
    int num = q << kGlobalK;
    for (int j = 0; j < kGlobalK; ++j) {
        if (stream.getBit()) {
            num |= (1 << (kGlobalK - 1 - j));
        }
    }
    return sgn * num;
}
std::vector<uint8_t> FlacAlgo::encode_vector(const std::vector<int16_t> &vec)  {
    BitStream stream;
    for (int16_t num: vec) {
        rice_encode(stream, num);
    }
    return stream.data;
}
std::vector<int16_t> FlacAlgo::decode_vector(std::vector<uint8_t> data)  {
    BitStream stream(std::move(data));
    std::vector<int16_t> decoded;
    while (stream.bitIndex < stream.data.size() * 8 - 7) {
        decoded.push_back(rice_decode(stream));
    }
    return decoded;
}
bool FlacAlgo::read_wav_header(const std::string &filename, WavHeader &header)  {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        send_error_information("Failed to open file: " + filename + '\n');
        return false;
    }

    file.read(reinterpret_cast<char *>(&header), sizeof(WavHeader));

    if (std::string(header.chunk_id, 4) != "RIFF" || std::string(header.format, 4) != "WAVE" ||
        header.audio_format != 1 || header.num_channels != 1) {
        send_error_information("Invalid WAV file format.\n");
        file.close();
        return false;
    }

    if (std::string(header.subchunk2_id) != "data") {
        file.seekg(header.subchunk2_size, std::ios::cur);
        file.read(reinterpret_cast<char *>(&(header.subchunk2_id)), sizeof(header.subchunk2_id));
        file.read(reinterpret_cast<char *>(&(header.subchunk2_size)), sizeof(int32_t));
    }
    file.close();
    return true;
}
std::vector<int16_t> FlacAlgo::read_wav_data(const std::string &filename, const WavHeader &header, size_t start_index)  {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        send_error_information("Failed to open file: " + filename + '\n');
        return {};
    }

    file.seekg(sizeof(WavHeader), std::ios::beg);
    file.seekg(start_index * sizeof(int16_t), std::ios::cur);


    if (header.subchunk2_size % sizeof(int16_t) != 0) {
        send_error_information("Invalid data size in WAV file.\n");
        return {};
    }

    std::vector<int16_t> audio_data;
    if (header.subchunk2_size / sizeof(int16_t) >= start_index + kGlobalSizeBlocks) {
        audio_data.resize(kGlobalSizeBlocks);
        file.read(reinterpret_cast<char *>(audio_data.data()), kGlobalSizeBlocks * sizeof(int16_t));
    } else {
        audio_data.resize(header.subchunk2_size / sizeof(int16_t) - start_index);
        file.read(reinterpret_cast<char *>(audio_data.data()), header.subchunk2_size - sizeof(int16_t) * start_index);
    }

    file.close();
    return audio_data;
}
void FlacAlgo::encode(const std::string &input_filename)  {
    auto start = std::chrono::high_resolution_clock::now();
    int size_input = static_cast<int>(get_filesize( input_filename));
    size_t last_slash_pos = input_filename.find_last_of('/');
    std::string tmp_input_filename =
            last_slash_pos != std::string::npos ? input_filename.substr(last_slash_pos + 1) : input_filename;
    size_t pos = tmp_input_filename.rfind('.');
    std::string output_filename ="storageEncoded/"+ tmp_input_filename.substr(0, pos) + ".flac";// путь сохранения
    WavHeader header{};
    if (!read_wav_header(input_filename, header)) {
        send_error_information("Failed to read WAV file.\n");
        exit(-1);
    }
    BitStream stream;
    size_t size = 0;
    for (size_t i = 0; header.subchunk2_size / sizeof(int16_t) > i * kGlobalSizeBlocks; ++i) {
        std::vector<int16_t> pcm_data = read_wav_data( input_filename, header, kGlobalSizeBlocks * i);
        lpc_.train(pcm_data);

        for (size_t j = 0; j < kGlobalOrder + 1; ++j) {
            int16_t arr[4] = {};
            std::memcpy(arr,&lpc_.coeffs_[j], sizeof(double));
            rice_encode(stream, arr[0]);
            rice_encode(stream, arr[1]);
            rice_encode(stream, arr[2]);
            rice_encode(stream, arr[3]);
        }

        for (size_t j = 0; j < pcm_data.size(); ++j) {
            rice_encode(stream, pcm_data[j] - lpc_.predict(pcm_data, j));
        }
        lpc_.clear();
    }
    std::ofstream output_file(output_filename, std::ios::binary);
    if (output_file.is_open()) {
        output_file.write(reinterpret_cast<const char *>(&header), sizeof(WavHeader));
        size = stream.data.size();
        output_file.write(reinterpret_cast<const char *>(&size), sizeof(size_t));
        output_file.write(reinterpret_cast<const char *>(stream.data.data()), size);
        output_file.close();
        send_message("FLAC data saved to: " + output_filename + '\n');
    } else {
        send_error_information("Failed to write FLAC file.\n");
        exit(-1);
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    int size_output = static_cast<int>(get_filesize(output_filename));
    double ratio = static_cast<double>(size_output) / size_input;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto info = CommonInformation(ratio,
                                  duration.count(), size_input, size_output);
    send_common_information(info);
    send_global_params();
}

void FlacAlgo::decode(const std::string &input_filename)  {
    auto start = std::chrono::high_resolution_clock::now();
    int size_input = static_cast<int>(get_filesize( input_filename));
    size_t last_slash_pos = input_filename.find_last_of('/');
    std::string tmp_input_filename =
            last_slash_pos != std::string::npos ? input_filename.substr(last_slash_pos + 1) : input_filename;
    size_t pos = tmp_input_filename.rfind('.');
    std::string output_filename ="storageEncoded/"+ tmp_input_filename.substr(0, pos) + ".flac";// путь сохранения
    WavHeader header{};
    if (!read_wav_header(input_filename, header)) {
        send_error_information("Failed to read WAV file.\n");
        exit(-1);
    }
    BitStream stream;
    size_t size = 0;
    for (size_t i = 0; header.subchunk2_size / sizeof(int16_t) > i * kGlobalSizeBlocks; ++i) {
        std::vector<int16_t> pcm_data = read_wav_data( input_filename, header, kGlobalSizeBlocks * i);
        lpc_.train(pcm_data);

        for (size_t j = 0; j < kGlobalOrder + 1; ++j) {
            int16_t arr[4] = {};
            std::memcpy(arr,&lpc_.coeffs_[j], sizeof(double));
            rice_encode(stream, arr[0]);
            rice_encode(stream, arr[1]);
            rice_encode(stream, arr[2]);
            rice_encode(stream, arr[3]);
        }

        for (size_t j = 0; j < pcm_data.size(); ++j) {
            rice_encode(stream, pcm_data[j] - lpc_.predict(pcm_data, j));
        }
        lpc_.clear();
    }
    std::ofstream output_file(output_filename, std::ios::binary);
    if (output_file.is_open()) {
        output_file.write(reinterpret_cast<const char *>(&header), sizeof(WavHeader));
        size = stream.data.size();
        output_file.write(reinterpret_cast<const char *>(&size), sizeof(size_t));
        output_file.write(reinterpret_cast<const char *>(stream.data.data()), size);
        output_file.close();
        send_message("FLAC data saved to: " + output_filename + '\n');
    } else {
        send_error_information("Failed to write FLAC file.\n");
        exit(-1);
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    int size_output = static_cast<int>(get_filesize(output_filename));
    double ratio = static_cast<double>(size_output) / size_input;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto info = CommonInformation(ratio,
                                  duration.count(), size_input, size_output);
    send_common_information(info);
    send_global_params();
}
