#ifndef EMBEDDEDDSP_PROTOCOL_H
#define EMBEDDEDDSP_PROTOCOL_H

#include <cstdint>
#include <cstring> // dla std::memcpy

namespace Protocol {

    // Identyfikatory komend
    enum Command : uint8_t {
        SetParam       = 0x01,
        SetEffectType  = 0x02,
        BypassToggle   = 0x03
    };

#pragma pack(push, 1)
    struct ControlPacket {
        uint8_t sof{0xA5};
        uint8_t command{0};
        uint8_t slotId{0};
        uint8_t paramId{0};
        float rawValue{0.0f}; // Pamiętaj: nie odczytujemy FPU bezpośrednio z nieupakowanego RAM!
        uint8_t crc{0};

        [[nodiscard]] static uint8_t calculateCRC(const uint8_t* data, size_t len) noexcept {
            uint8_t crc = 0xFF;
            for (size_t i = 0; i < len; ++i) {
                crc ^= data[i];
            }
            return crc;
        }

        [[nodiscard]] bool isValid() const noexcept {
            if (sof != 0xA5) return false;
            const auto* bytes = reinterpret_cast<const uint8_t*>(this);
            return calculateCRC(bytes, sizeof(ControlPacket) - 1) == crc;
        }

        // Bezpieczny odczyt wartości float bez ryzyka HardFault na ARM Cortex-M4 FPU
        [[nodiscard]] float getValue() const noexcept {
            float temp{0.0f};
            std::memcpy(&temp, &rawValue, sizeof(float));
            return temp;
        }

        // Bezpieczny zapis wartości float
        void setValue(float val) noexcept {
            std::memcpy(&rawValue, &val, sizeof(float));
        }
    };
#pragma pack(pop)

    static_assert(sizeof(ControlPacket) == 9, "ControlPacket must be exactly 9 bytes");

} // namespace Protocol

#endif // EMBEDDEDDSP_PROTOCOL_H