//
// Created by bogdan on 21.04.24.
//

#ifndef ARCHIVATOR_COMMONINFORMATION_HPP
#define ARCHIVATOR_COMMONINFORMATION_HPP
#include <cstddef>
/**
 * @brief Aggregates compression results for reporting.
 *
 * Stores the outcome metrics of a compression or decompression operation, including compression ratio,
 * execution time, and input/output sizes. This struct is used to pass summary information to the user interface or logs.
 */
struct CommonInformation {
    /// Compression ratio expressed as `original_size / compressed_size` (e.g., 1:compressionRatio).
    double compression_ratio;
    /// Time taken for the operation in milliseconds.
    size_t time;
    /// Size of input data in bytes.
    size_t size_input_data;
    /// Size of output data in bytes.
    size_t size_output_data;
};
#endif