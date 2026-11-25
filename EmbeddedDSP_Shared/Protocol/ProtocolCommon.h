#ifndef EMBEDDEDDSP_PROTOCOL_COMMON_H
#define EMBEDDEDDSP_PROTOCOL_COMMON_H

#include <stdint.h>
#include <stddef.h>

namespace Protocol {

/// @brief Samples per audio frame.
inline constexpr size_t AUDIO_SAMPLES = 128;

/// @brief System PCM sample rate (Hz); must match I2S AudioFreq on the MCU.
inline constexpr float SAMPLE_RATE_HZ = 48000.0f;

/// @brief User preset bank size (indices 0 .. kUserPresetCount-1).
inline constexpr uint8_t kUserPresetCount = 10;

/// @brief Wire effect type ids (ControlPacket::effectTypeId / manifest E:<id>).
namespace EffectId {
inline constexpr uint8_t Empty = 0x00;
inline constexpr uint8_t Delay = 0x01;
inline constexpr uint8_t Overdrive = 0x02;
inline constexpr uint8_t Tremolo = 0x03;
inline constexpr uint8_t Bitcrusher = 0x04;
} // namespace EffectId

} // namespace Protocol

/// @brief Start-of-frame byte values.
namespace Protocol::SOF {

inline constexpr uint8_t Control = 0xA5;
inline constexpr uint8_t Audio   = 0xA6;

} // namespace Protocol::SOF

/// @brief STM32 USB CDC discovery constants.
namespace Protocol::Hardware {

inline constexpr uint16_t USB_VID = 0x0483;
inline constexpr uint16_t USB_PID = 0x5740;
inline constexpr uint32_t DEFAULT_BAUD = 115200;

} // namespace Protocol::Hardware

#endif // EMBEDDEDDSP_PROTOCOL_COMMON_H
