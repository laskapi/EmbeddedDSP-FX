#ifndef EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H
#define EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H

#include <cstdint>
#include <cstddef>

namespace SharedBuffer {

    inline constexpr std::size_t Capacity = 32768;

    alignas(4) inline uint8_t s_buffer[Capacity];

}

#endif // EMBEDDEDDSP_FIRMWARE_SHARED_BUFFER_H
