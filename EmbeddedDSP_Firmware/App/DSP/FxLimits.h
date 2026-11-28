#ifndef EMBEDDEDDSP_FIRMWARE_FX_LIMITS_H
#define EMBEDDEDDSP_FIRMWARE_FX_LIMITS_H

#include <cstddef>

namespace App::Dsp {

/// @brief Firmware rack capacity (emitted as manifest C:SLOTS).
inline constexpr std::size_t kMaxEffectSlots = 6;

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_FX_LIMITS_H
