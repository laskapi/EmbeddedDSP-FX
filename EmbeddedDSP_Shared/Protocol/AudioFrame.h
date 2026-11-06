#ifndef EMBEDDEDDSP_AUDIO_FRAME_H
#define EMBEDDEDDSP_AUDIO_FRAME_H

#include "Protocol/Checksums.h"
#include "Protocol/ProtocolCommon.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace Protocol {

#pragma pack(push, 1)
/// @brief Packed PCM audio frame (SOF + sequence + samples + CRC16).
struct AudioFrame {
    uint8_t sof{SOF::Audio};
    uint8_t sequence{0};
    std::array<std::int16_t, AUDIO_SAMPLES> samples{};
    uint16_t crc{0};

    [[nodiscard]] bool isValid() const noexcept {
        if (sof != SOF::Audio) return false;
        return Checksums::crc16(reinterpret_cast<const uint8_t*>(samples.data()),
                                sizeof(samples)) == crc;
    }

    void applyCRC() noexcept {
        crc = Checksums::crc16(reinterpret_cast<const uint8_t*>(samples.data()), sizeof(samples));
    }
};
#pragma pack(pop)

static_assert(sizeof(AudioFrame) == (2 + AUDIO_SAMPLES * 2 + 2), "AudioFrame size mismatch");
}

#endif // EMBEDDEDDSP_AUDIO_FRAME_H
