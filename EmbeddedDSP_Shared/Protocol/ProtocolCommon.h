#ifndef EMBEDDEDDSP_PROTOCOL_COMMON_H
#define EMBEDDEDDSP_PROTOCOL_COMMON_H

#include <cstdint>

namespace Protocol {

    namespace SOF {
        inline constexpr uint8_t Control = 0xA5;
        inline constexpr uint8_t Audio   = 0xA6;
    }

    namespace ReservedParam {
        inline constexpr uint8_t ManifestSignal = 0xFE;
    }

}

#endif // EMBEDDEDDSP_PROTOCOL_COMMON_H
