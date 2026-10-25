#ifndef EMBEDDEDDSP_CONTROL_PACKET_H
#define EMBEDDEDDSP_CONTROL_PACKET_H

#include <stdint.h>
#include <string.h>
#include "Checksums.h"
#include "ProtocolCommon.h"

namespace Protocol {

/// @brief Host <-> device control commands.
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
/// @brief Packed 9-byte control message (union maps command-specific fields).
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

    [[nodiscard]] bool isValid() const noexcept {
        if (sof != SOF::Control) return false;
        return Checksums::crc8(reinterpret_cast<const uint8_t*>(this), sizeof(ControlPacket) - 1) == crc;
    }

    void applyCRC() noexcept {
        crc = Checksums::crc8(reinterpret_cast<const uint8_t*>(this), sizeof(ControlPacket) - 1);
    }

    /// @brief FPU-safe float read (memcpy, avoids unaligned load).
    [[nodiscard]] float getValue() const noexcept {
        float temp;
        memcpy(&temp, &m_rawValue, sizeof(float));
        return temp;
    }

    /// @brief FPU-safe float write (memcpy, avoids unaligned store).
    void setValue(float val) noexcept {
        memcpy(&m_rawValue, &val, sizeof(float));
    }
};
#pragma pack(pop)

static_assert(sizeof(ControlPacket) == 9, "ControlPacket layout error");
}

#endif // EMBEDDEDDSP_CONTROL_PACKET_H
