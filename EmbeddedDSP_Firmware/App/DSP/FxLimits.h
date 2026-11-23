#ifndef EMBEDDEDDSP_FIRMWARE_FX_LIMITS_H
#define EMBEDDEDDSP_FIRMWARE_FX_LIMITS_H

#include <cstddef>
#include <cstdint>

namespace App::Dsp {

/// @brief Firmware rack capacity (emitted as manifest C:SLOTS).
inline constexpr std::size_t kMaxEffectSlots = 4;

/// @brief How many params one effect may have.
inline constexpr uint8_t kMaxEffectParams = 8;

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_FX_LIMITS_H
