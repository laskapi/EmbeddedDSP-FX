#ifndef EMBEDDEDDSP_FX_CRC16CALCULATOR_H
#define EMBEDDEDDSP_FX_CRC16CALCULATOR_H
#pragma once
#include <cstdint>
#include <cstddef>
#include <array>

namespace detail {
    // Helper function in detail namespace to guarantee true constant expression evaluation
    constexpr std::array<uint16_t, 256> generateCrc16Lut() noexcept {
        std::array<uint16_t, 256> table{};
        for (std::size_t i = 0; i < 256; ++i) {
            uint16_t curr = static_cast<uint16_t>(i << 8);
            for (std::size_t j = 0; j < 8; ++j) {
                if (curr & 0x8000) {
                    curr = static_cast<uint16_t>((curr << 1) ^ 0x1021);
                } else {
                    curr = static_cast<uint16_t>(curr << 1);
                }
            }
            table[i] = curr;
        }
        return table;
    }
} // namespace detail

class Crc16Calculator {
private:
    // Private static member placed at the top (Private First)
    static constexpr std::array<uint16_t, 256> m_crc16Table = detail::generateCrc16Lut();

public:
    [[nodiscard]] static uint16_t calculate(const uint8_t* data, std::size_t len) noexcept {
        uint16_t crc = 0xFFFF;
        for (std::size_t i = 0; i < len; ++i) {
            const uint8_t byte = data[i];
            const uint8_t lutIndex = static_cast<uint8_t>((crc >> 8) ^ byte);
            crc = static_cast<uint16_t>((crc << 8) ^ m_crc16Table[lutIndex]);
        }
        return crc;
    }
};
#endif //EMBEDDEDDSP_FX_CRC16CALCULATOR_H
