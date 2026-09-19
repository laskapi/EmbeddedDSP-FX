#ifndef EMBEDDEDDSP_CONTROL_PACKET_H
#define EMBEDDEDDSP_CONTROL_PACKET_H

#include <cstddef>
#include <stdint.h>
#include <string.h>
#include "Checksums.h"
#include "ProtocolCommon.h"

namespace Protocol {

    /** @brief Command identifiers for binary protocol. */
    enum class Command : uint8_t {
        SetParam       = 0x01,
        SetEffectType  = 0x02,
        BypassToggle   = 0x03,
        ClearSlot      = 0x04,
        SwapSlots      = 0x05,
        SetActiveSlots = 0x06,
        GetState       = 0x07,
        ReportState    = 0x08
    };

#pragma pack(push, 1)
    /**
     * @brief 9-byte binary control packet.
     * 
     * Uses anonymous union for context-dependent fields.
     */
    struct ControlPacket {
        uint8_t  sof{SOF::Control};
        Command  command{Command::SetParam};
        uint8_t  slotId{0};
        
        union {
            uint8_t paramId{0};
            uint8_t effectTypeId;
            uint8_t targetSlotId;
            uint8_t signalId;
        };

    private:
        float    m_rawValue{0.0f};
    public:
        uint8_t  crc{0};

        /** @return True if SOF is correct and CRC matches data. */
        [[nodiscard]] bool isValid() const noexcept {
            if (sof != SOF::Control) return false;
            return Checksums::crc8(reinterpret_cast<const uint8_t*>(this), sizeof(ControlPacket) - 1) == crc;
        }

        /** @brief Calculates and applies CRC to the packet. */
        void applyCRC() noexcept {
            crc = Checksums::crc8(reinterpret_cast<const uint8_t*>(this), sizeof(ControlPacket) - 1);
        }

        /** @return Float value handled safely for ARM alignment. */
        [[nodiscard]] float getValue() const noexcept {
            float temp;
            memcpy(&temp, &m_rawValue, sizeof(float));
            return temp;
        }

        /** @brief Sets float value using safe memcpy. */
        void setValue(float val) noexcept {
            memcpy(&m_rawValue, &val, sizeof(float));
        }
    };
#pragma pack(pop)

    static_assert(sizeof(ControlPacket) == 9, "ControlPacket layout error");
}

#endif // EMBEDDEDDSP_CONTROL_PACKET_H
