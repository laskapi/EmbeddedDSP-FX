#ifndef EMBEDDEDDSP_FX_AUDIOFRAMEPACKET_H
#define EMBEDDEDDSP_FX_AUDIOFRAMEPACKET_H

#include <cstdint>
#include <array>

// Audio frame configuration
inline constexpr std::size_t AUDIO_PACKET_SAMPLES = 128; // Number of mono Q15 samples per packet

#pragma pack(push, 1)
struct AudioFramePacket {
    uint8_t  sof{0xA6};                                         // Start of Frame marker
    uint8_t  sequenceNumber{0};                                 // Frame counter (0-255) for loss detection
    uint16_t payloadLength{AUDIO_PACKET_SAMPLES * sizeof(int16_t)}; // Payload size in bytes
    std::array<int16_t, AUDIO_PACKET_SAMPLES> samples{};        // Raw Q15 PCM audio samples
    uint16_t crc16{0};                                          // CRC-16 checksum
};
#pragma pack(pop)

static_assert(sizeof(AudioFramePacket) == (4 + AUDIO_PACKET_SAMPLES * 2 + 2),
              "AudioFramePacket layout must be tightly packed without padding");
#endif //EMBEDDEDDSP_FX_AUDIOFRAMEPACKET_H
