#ifndef EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H

#include "AudioEffectConcept.h"
#include <algorithm>
#include <cmath>

/**
 * @brief Soft-clipping overdrive effect.
 */
class OverdriveEffect {
public:
    static constexpr uint8_t Id = 0x02;
    static constexpr const char* Name = "Overdrive";
    static constexpr uint8_t ParamCount = 2;

    /** @brief Parameter definitions for the manifest. */
    static constexpr EffectParam Params[ParamCount] = {
        {"Gain", 1.0f, 20.0f, 5.0f},
        {"Tone", 0.1f, 1.0f, 0.5f}
    };

    void prepare(float /*sampleRate*/) noexcept {}

    /** @brief Applies soft-clipping to the input samples. */
    void process(float& left, float& right) noexcept {
        auto clip = [gain = m_gain](float in) {
            float x = in * gain;
            if (x > 1.0f) return 2.0f/3.0f;
            if (x < -1.0f) return -2.0f/3.0f;
            return x - (x*x*x)/3.0f;
        };
        left = clip(left);
        right = clip(right);
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) m_gain = std::clamp(val, Params[0].min, Params[0].max);
        else if (id == 1) m_tone = std::clamp(val, Params[1].min, Params[1].max);
    }

    [[nodiscard]] float getParamValue(uint8_t id) const noexcept {
        if (id == 0) return m_gain;
        if (id == 1) return m_tone;
        return 0.0f;
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    float m_gain{5.0f};
    float m_tone{0.5f};
    bool m_bypassed{false};
};

#endif // EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
