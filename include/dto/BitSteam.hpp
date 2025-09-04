#ifndef ARCHIVATOR_BITSTREAM_HPP
#define ARCHIVATOR_BITSTREAM_HPP

#include <vector>
#include <cstdint>
#include <cstddef>

/**
 * @brief Simple MSB-first bitstream for sequential write/read of bits and bytes.
 *
 * The stream stores bits in a byte vector with **MSB-first** order inside each byte:
 * bit index 0 refers to the most significant bit of data[0] (mask 0x80),
 * then 1 -> 0x40, ..., 7 -> 0x01; index 8 continues in data[1], etc.
 *
 * Typical usage:
 * - Writing: call add_bit()/add_byte() in sequence; the stream grows as needed.
 * - Reading: reset bit_index to 0 (default) and call get_bit()/get_byte().
 *
 * @note Reading and writing share the same @ref bit_index cursor; treat read and
 *       write phases separately (or manage the cursor explicitly).
 */
struct BitStream {
    /**
     * @brief Underlying byte buffer storing the bitstream.
     *
     * Bits are packed MSB-first within each byte. Partially filled trailing byte
     * may exist during/after writing; readers should consume exactly the number
     * of valid bits known by the producer (e.g., via an external "padding" value).
     */
    std::vector<std::uint8_t> data;

    /**
     * @brief Current bit cursor (0-based) for read/write operations.
     *
     * For writing, it points to the next bit position to fill (may extend @ref data).
     * For reading, it points to the next bit to read (0..total_bits-1).
     */
    std::size_t bit_index;

    /**
     * @brief Construct an empty bitstream (cursor at 0).
     */
    BitStream() : bit_index(0) {}

    /**
     * @brief Construct a bitstream from an existing byte buffer (cursor at 0).
     * @param data Byte vector to take ownership of (moved).
     *
     * The provided bytes are interpreted MSB-first for subsequent bit reads.
     */
    explicit BitStream(std::vector<std::uint8_t> data) : data(std::move(data)), bit_index(0) {}

    /**
     * @brief Append a single bit to the stream at the current cursor.
     * @param bit Logical value to append: false -> 0, true -> 1.
     *
     * If the cursor is at a byte boundary, a new byte is appended to @ref data.
     * The bit is placed at position (7 - (bit_index % 8)) of the target byte.
     * The cursor advances by 1.
     */
    void add_bit(bool bit);

    /**
     * @brief Append a full byte (8 bits) to the stream at the current cursor.
     * @param byte The byte value to append.
     *
     * If the cursor is byte-aligned, this is equivalent to pushing @p byte into
     * @ref data. If not aligned, the 8 bits of @p byte are written MSB-first,
     * possibly spanning two adjacent bytes in @ref data. The cursor advances by 8.
     */
    void add_byte(unsigned char byte);

    /**
     * @brief Read the next bit at the current cursor and advance.
     * @return The bit value (false for 0, true for 1).
     *
     * The bit is read MSB-first within its byte. The cursor advances by 1.
     * @pre There must be at least one unread bit remaining; caller is responsible
     *      for ensuring bounds (e.g., via a known total bit count or padding info).
     */
    bool get_bit();

    /**
     * @brief Read the next 8 bits (one byte) at the current cursor and advance.
     * @return Byte value composed from the next eight bits (MSB-first).
     *
     * If the cursor is byte-aligned, this returns the current byte and advances
     * by 8. If not aligned, it assembles a byte from bits that may span two
     * consecutive bytes of @ref data. The cursor advances by 8.
     *
     * @pre There must be at least eight unread bits remaining.
     */
    unsigned char get_byte();
};

#endif // ARCHIVATOR_BITSTREAM_HPP
