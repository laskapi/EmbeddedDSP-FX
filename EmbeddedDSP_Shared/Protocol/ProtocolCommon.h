#ifndef EMBEDDEDDSP_PROTOCOL_COMMON_H
#define EMBEDDEDDSP_PROTOCOL_COMMON_H

#include <stdint.h>
#include <stddef.h>

namespace Protocol {

    /** @brief Number of audio samples per frame packet. */
    inline constexpr size_t AUDIO_SAMPLES = 64;

    /** @brief Start of Frame identifiers for protocol synchronization. */
    namespace SOF {
        inline constexpr uint8_t Control = 0xA5;
        inline constexpr uint8_t Audio   = 0xA6;
    }

    /** @brief Reserved parameter identifiers for internal signaling. */
    namespace ReservedParam {
        inline constexpr uint8_t ManifestSignal = 0xFE;
    }

    /** @brief Hardware identifiers for device discovery (STM32 USB CDC). */
    namespace Hardware {
        inline constexpr uint16_t USB_VID = 0x0483;
        inline constexpr uint16_t USB_PID = 0x5740;
        inline constexpr uint32_t DEFAULT_BAUD = 115200;
    }

} // namespace Protocol

#endif // EMBEDDEDDSP_PROTOCOL_COMMON_H
