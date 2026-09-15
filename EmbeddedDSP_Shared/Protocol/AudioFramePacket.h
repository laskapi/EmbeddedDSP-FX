#ifndef EMBEDDEDDSP_AUDIO_FRAME_PACKET_H
#define EMBEDDEDDSP_AUDIO_FRAME_PACKET_H

#include <cstdint>
#include <array>
#include <cstddef>
#include "Checksums.h"
#include "ProtocolCommon.h"

namespace Protocol {

    inline constexpr std::size_t AUDIO_SAMPLES = 128;

#pragma pack(push, 1)
    struct AudioFramePacket {
        uint8_t  sof{SOF::Audio};
        uint8_t  sequenceNumber{0};
        uint16_t payloadLength{AUDIO_SAMPLES * sizeof(int16_t)};
        std::array<int16_t, AUDIO_SAMPLES> samples{};
        uint16_t crc16{0};

        [[nodiscard]] bool isValid() const noexcept {
            if (sof != SOF::Audio) return false;
            const uint8_t* rawData = reinterpret_cast<const uint8_t*>(this);
            return Checksums::crc16(rawData, sizeof(AudioFramePacket) - 2) == crc16;
        }

        void applyCRC() noexcept {
            const uint8_t* rawData = reinterpret_cast<const uint8_t*>(this);
            crc16 = Checksums::crc16(rawData, sizeof(AudioFramePacket) - 2);
        }
    };
#pragma pack(pop)

    static_assert(sizeof(AudioFramePacket) == (4 + AUDIO_SAMPLES * 2 + 2), "AudioFramePacket layout error");
}

#endif // EMBEDDEDDSP_AUDIO_FRAME_PACKET_H
