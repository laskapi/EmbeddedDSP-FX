#ifndef EMBEDDEDDSP_CHECKSUMS_H
#define EMBEDDEDDSP_CHECKSUMS_H

#include <stdint.h>
#include <stddef.h>

/// @brief CRC helpers for the binary protocol.
namespace Protocol::Checksums {

/// @brief CRC-8 (poly 0x07).
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

/// @brief CRC-16-CCITT (poly 0x1021, init 0xFFFF).
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

} // namespace Protocol::Checksums

#endif // EMBEDDEDDSP_CHECKSUMS_H
