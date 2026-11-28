#ifndef EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H

#include "DSP/AudioEffectConcept.h"
#include "DSP/OnePoleLpf.h"
#include "Protocol/ProtocolCommon.h"
#include <algorithm>

namespace App::Dsp {

/// @brief Soft-clip overdrive with one-pole tone (LPF) section.
class OverdriveEffect {
public:
    static constexpr uint8_t Id = Protocol::EffectId::Overdrive;
    static constexpr const char* Name = "Overdrive";
    static constexpr uint8_t ParamCount = 3;
    static_assert(ParamCount <= Protocol::kMaxEffectParams);

    static constexpr EffectParam Params[ParamCount] = {
        {"Gain", 1.0f, 20.0f, 5.0f},
        {"Tone", 0.1f, 1.0f, 0.5f},
        {"Level", 0.0f, 1.0f, 0.7f}
    };

    explicit OverdriveEffect(float sampleRate) noexcept {
        updateSampleRate(sampleRate);
    }

    void updateSampleRate(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        updateTone();
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) return;

        auto processSample = [this](float input) {
            float x = input * m_gain;

            // Soft-clip: x - x^3/3, hard-limited outside [-1, 1]
            float saturated;
            if (x > 1.0f) saturated = 2.0f / 3.0f;
            else if (x < -1.0f) saturated = -2.0f / 3.0f;
            else saturated = x - (x * x * x) * 0.33333333f;

            return m_lpf.process(saturated) * m_level;
        };

        left = processSample(left);
        right = processSample(right);
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) {
            m_gain = std::clamp(val, Params[0].min, Params[0].max);
        } else if (id == 1) {
            m_tone = std::clamp(val, Params[1].min, Params[1].max);
            updateTone();
        } else if (id == 2) {
            m_level = std::clamp(val, Params[2].min, Params[2].max);
        }
    }

    [[nodiscard]] float paramValue(uint8_t id) const noexcept {
        if (id == 0) return m_gain;
        if (id == 1) return m_tone;
        if (id == 2) return m_level;
        return 0.0f;
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    void setBypassed(bool bypassed) noexcept { m_bypassed = bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    void updateTone() noexcept {
        m_lpf.setCutoffHz(toneCutoffHz(m_tone, Params[1].min, Params[1].max), m_sampleRate);
    }

    float m_sampleRate{};
    float m_gain{5.0f};
    float m_tone{0.5f};
    float m_level{0.7f};
    bool m_bypassed{false};
    OnePoleLpf m_lpf;
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
