#ifndef EMBEDDEDDSP_FIRMWARE_CHAIN_SETTINGS_H
#define EMBEDDEDDSP_FIRMWARE_CHAIN_SETTINGS_H

#include "DSP/FxLimits.h"
#include <cstdint>

namespace App::Dsp {

/// @brief Flash-friendly FX chain snapshot (packed POD).
struct ChainSettings {
    static constexpr uint32_t kMagic = 0x44535046; // "DSPF"
    static constexpr uint16_t kVersion = 1;
    /// @brief Matches MasterVolume::kDefaultGain (keep POD header free of that include).
    static constexpr float kDefaultMasterGain = 0.8f;

    uint32_t magic{0};
    uint16_t version{0};
    uint8_t slotCount{0};
    uint8_t reserved{0};
    float masterGain{0.0f};
    struct SlotData {
        uint8_t effectId{0};
        uint8_t bypassed{0};
        uint8_t pad[2]{};
        float params[kMaxEffectParams]{};
    } slots[kMaxEffectSlots]{};

    /// @brief Empty rack: all slots None, valid magic/version for applySettings.
    [[nodiscard]] static ChainSettings makeEmpty(
        float masterGain = kDefaultMasterGain) noexcept {
        ChainSettings s{};
        s.magic = kMagic;
        s.version = kVersion;
        s.slotCount = static_cast<uint8_t>(kMaxEffectSlots);
        s.masterGain = masterGain;
        return s;
    }
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_CHAIN_SETTINGS_H
