//
// Created by bogdan on 15.04.24.

#ifndef ARCHIVATOR_FLACALGO_HPP
#define ARCHIVATOR_FLACALGO_HPP

#include <fstream>
#include <vector>
#include <filesystem>
#include <dto/BitSteam.hpp>
#include <dto/WAWHeader.hpp>
#include <dto/CommonInformation.hpp>

#include <controller/IController.hpp>

/**
 *
 */
static constexpr  int kGlobalSizeBlocks = 16384 * 8; // Size of blocks (INT32_MAX for 1 block)
static constexpr int kGlobalOrder = 25;            // LPC model GLOBAL_ORDER
static constexpr int kGlobalK = 8;                // Rice code parameter

// Linear predictive coding
class FlacAlgo final : public IController {
    class    Lpc {
        std::vector<double> coeffs_; // Coefficients LPC
    public:
        Lpc() =default;
        explicit Lpc(std::vector<double> coeffs) : coeffs_(std::move(coeffs)) {}
        void train(const std::vector<int16_t> &input);
        int16_t predict(const std::vector<int16_t> &input, size_t index) const;
        void clear();
        friend class FlacAlgo;
    };
    Lpc lpc_;
    virtual void send_common_information(const CommonInformation &common_information) override;

    virtual void send_error_information(const std::string &error) override;

    void send_global_params() const;

    static void rice_encode(BitStream &stream, int num);

    static int rice_decode(BitStream &stream);

    static std::vector<uint8_t> encode_vector(const std::vector<int16_t> &vec) ;

    static std::vector<int16_t> decode_vector(std::vector<uint8_t> data);

    bool read_wav_header(const std::string &filename, WavHeader &header);

    std::vector<int16_t> read_wav_data(const std::string &filename, const WavHeader &header, size_t start_index);

public:
    explicit FlacAlgo(bool is_text_output, const std::string &output_file, std::ostringstream &ref_oss) : IController(
            is_text_output, output_file, ref_oss) {
    };

    void encode(const std::string &input_filename);

    void decode(const std::string &input_filename);

};

#endif
