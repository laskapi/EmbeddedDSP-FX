#ifndef EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H

#include "AudioEffectConcept.h"
#include <algorithm>
#include <cmath>

/// @brief Soft-clipping saturation effect.
class OverdriveEffect {
public:
    static constexpr uint8_t Id = 0x02;
    static constexpr const char* Name = "Overdrive";
    static constexpr uint8_t ParamCount = 2;

    static constexpr EffectParam Params[ParamCount] = {
        {"Gain", 1.0f, 20.0f, 5.0f},
        {"Tone", 0.1f, 1.0f, 0.5f}
    };

    void prepare(float /*sampleRate*/) noexcept {}
    void process(float& left, float& right) noexcept;

    void setParamValue(uint8_t id, float val) noexcept;
    [[nodiscard]] float getParamValue(uint8_t id) const noexcept;

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    float m_gain{5.0f};
    float m_tone{0.5f};
    bool m_bypassed{false};
};

#endif // EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
