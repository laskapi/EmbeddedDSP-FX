#ifndef EMBEDDEDDSP_AUDIO_FRAME_PACKET_H
#define EMBEDDEDDSP_AUDIO_FRAME_PACKET_H

#include <stdint.h>
#include <stddef.h>
#include "Checksums.h"
#include "ProtocolCommon.h"

namespace Protocol {

#pragma pack(push, 1)
    /**
     * @brief Binary packet containing a frame of PCM audio samples.
     */
    struct AudioFramePacket {
        uint8_t  sof{SOF::Audio};
        uint8_t  sequence{0};
        int16_t  samples[AUDIO_SAMPLES];
        uint16_t crc{0};

        /** @return True if SOF is correct and CRC matches samples. */
        [[nodiscard]] bool isValid() const noexcept {
            if (sof != SOF::Audio) return false;
            return Checksums::crc16(reinterpret_cast<const uint8_t*>(samples), sizeof(samples)) == crc;
        }

        /** @brief Calculates and applies CRC to the packet. */
        void applyCRC() noexcept {
            crc = Checksums::crc16(reinterpret_cast<const uint8_t*>(samples), sizeof(samples));
        }
    };
#pragma pack(pop)

    static_assert(sizeof(AudioFramePacket) == (2 + AUDIO_SAMPLES * 2 + 2), "AudioFramePacket size mismatch");
}

#endif // EMBEDDEDDSP_AUDIO_FRAME_PACKET_H
