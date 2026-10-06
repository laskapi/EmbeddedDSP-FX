#ifndef EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H

#include "AudioEffectConcept.h"
#include <algorithm>
#include <cmath>

/// @brief Soft-clipping saturation effect with integrated Tone filter.
class OverdriveEffect {
public:
    static constexpr uint8_t Id = 0x02;
    static constexpr const char* Name = "Overdrive";
    static constexpr uint8_t ParamCount = 2;

    static constexpr EffectParam Params[ParamCount] = {
        {"Gain", 1.0f, 20.0f, 5.0f},
        {"Tone", 0.1f, 1.0f, 0.5f}
    };

    void prepare(float sampleRate) noexcept {
        m_sampleRate = (sampleRate > 0.0f) ? sampleRate : 48000.0f;
        updateToneCoefficients();
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) return;

        auto processSample = [this](float input) {
            // 1. Apply Input Gain
            float x = input * m_gain;

            // 2. Polynomial Soft-Clipping (x - x^3/3)
            float saturated;
            if (x > 1.0f) saturated = 2.0f/3.0f;
            else if (x < -1.0f) saturated = -2.0f/3.0f;
            else saturated = x - (x * x * x) * 0.33333333f;

            // 3. Tone section: Single-pole IIR Low-Pass Filter
            m_filterState = (m_b0 * saturated) + (m_a1 * m_filterState);
            
            return m_filterState;
        };

        left = processSample(left);
        right = processSample(right);
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) {
            m_gain = std::clamp(val, Params[0].min, Params[0].max);
        } else if (id == 1) {
            m_tone = std::clamp(val, Params[1].min, Params[1].max);
            updateToneCoefficients();
        }
    }

    [[nodiscard]] float getParamValue(uint8_t id) const noexcept {
        if (id == 0) return m_gain;
        if (id == 1) return m_tone;
        return 0.0f;
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    void updateToneCoefficients() noexcept {
        // Map 0.1 - 1.0 tone param to 400Hz - 10kHz
        float cutoffHz = 400.0f + (m_tone - 0.1f) * (9600.0f / 0.9f);
        float w0 = 2.0f * 3.14159265f * cutoffHz / m_sampleRate;
        m_a1 = std::exp(-w0);
        m_b0 = 1.0f - m_a1;
    }

    float m_sampleRate{48000.0f};
    float m_gain{5.0f};
    float m_tone{0.5f};
    bool m_bypassed{false};

    // Filter state
    float m_filterState{0.0f};
    float m_b0{1.0f};
    float m_a1{0.0f};
};

#endif // EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
