#ifndef EMBEDDEDDSP_CONTROL_PACKET_H
#define EMBEDDEDDSP_CONTROL_PACKET_H

#include <stdint.h>
#include <string.h>
#include "Protocol/Checksums.h"
#include "Protocol/ProtocolCommon.h"

namespace Protocol {

/// @brief Control opcodes. High nibble = direction for easy hex debugging:
///        0x0* device→host (report/notify), 0x1* host→device (set/request).
///        0x00 and 0x10 are intentionally empty.
enum class Command : uint8_t {
    // --- 0x00–0x0F: device → host ---
    // 0x00 reserved empty
    ReportParam         = 0x01,
    ReportEffectType    = 0x02,
    ReportBypass        = 0x03,
    ManifestFollows     = 0x04, ///< Next bytes are null-terminated manifest text.
    ReportMasterVolume  = 0x05,

    // --- 0x10–0x1F: host → device ---
    // 0x10 reserved empty
    SetParam            = 0x11,
    SetEffectType       = 0x12,
    BypassToggle        = 0x13,
    ClearSlot           = 0x14,
    SwapSlots           = 0x15,
    SetActiveSlots      = 0x16, ///< Reserved in v1 (fixed rack; unused slots = Empty).
    GetState            = 0x17,
    SetMasterVolume     = 0x18  ///< Global output gain in value(); range 0..1.
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
    [[nodiscard]] float value() const noexcept {
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
