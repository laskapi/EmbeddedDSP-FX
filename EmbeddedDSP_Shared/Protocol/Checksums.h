#ifndef EMBEDDEDDSP_CHECKSUMS_H
#define EMBEDDEDDSP_CHECKSUMS_H

#include <stdint.h>
#include <stddef.h>

namespace Protocol {

/**
 * @brief Utilities for CRC calculation used by the binary protocol.
 */
namespace Checksums {

    /**
     * @brief Standard CRC-8 calculation.
     * @param data Pointer to the buffer.
     * @param len Number of bytes.
     * @return 8-bit checksum.
     */
    inline uint8_t crc8(const uint8_t* data, size_t len) {
        uint8_t crc = 0;
        for (size_t i = 0; i < len; ++i) {
            crc ^= data[i];
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x80) crc = (crc << 1) ^ 0x07;
                else crc <<= 1;
            }
        }
        return crc;
    }

    /**
     * @brief Standard CRC-16 (CCITT) calculation.
     * @param data Pointer to the buffer.
     * @param len Number of bytes.
     * @return 16-bit checksum.
     */
    inline uint16_t crc16(const uint8_t* data, size_t len) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < len; ++i) {
            crc ^= (uint16_t)data[i] << 8;
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
                else crc <<= 1;
            }
        }
        return crc;
    }

} // namespace Checksums
} // namespace Protocol

#endif // EMBEDDEDDSP_CHECKSUMS_H
