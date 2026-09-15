#ifndef EMBEDDEDDSP_PROTOCOL_CHECKSUMS_H
#define EMBEDDEDDSP_PROTOCOL_CHECKSUMS_H

#include <cstdint>
#include <cstddef>
#include <array>

namespace Protocol {

    class Checksums {
    public:
        static uint8_t crc8(const uint8_t* data, std::size_t len) noexcept {
            uint8_t crc = 0xFF;
            for (std::size_t i = 0; i < len; ++i) crc ^= data[i];
            return crc;
        }

        static uint16_t crc16(const uint8_t* data, std::size_t len) noexcept {
            uint16_t crc = 0xFFFF;
            for (std::size_t i = 0; i < len; ++i) {
                const uint8_t lutIndex = static_cast<uint8_t>((crc >> 8) ^ data[i]);
                crc = static_cast<uint16_t>((crc << 8) ^ m_crc16Table[lutIndex]);
            }
            return crc;
        }

    private:
        static constexpr std::array<uint16_t, 256> m_crc16Table = []() {
            std::array<uint16_t, 256> table{};
            for (int i = 0; i < 256; ++i) {
                uint16_t curr = static_cast<uint16_t>(i << 8);
                for (int j = 0; j < 8; ++j) {
                    curr = (curr & 0x8000) ? (curr << 1) ^ 0x1021 : (curr << 1);
                }
                table[i] = curr;
            }
            return table;
        }();
    };
}

#endif // EMBEDDEDDSP_PROTOCOL_CHECKSUMS_H
