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

} // namespace Protocol

#endif // EMBEDDEDDSP_PROTOCOL_COMMON_H
