#ifndef EMBEDDEDDSP_FIRMWARE_BITCRUSHER_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_BITCRUSHER_EFFECT_H

#include "DSP/AudioEffectConcept.h"
#include "DSP/OnePoleLpf.h"
#include "Protocol/ProtocolCommon.h"
#include <algorithm>
#include <cmath>

namespace App::Dsp {

/// @brief Lo-fi bit depth + fractional sample-hold downsample + 1-pole tone filter.
class BitcrusherEffect {
public:
    static constexpr uint8_t Id = Protocol::EffectId::Bitcrusher;
    static constexpr const char* Name = "Bitcrusher";
    static constexpr uint8_t ParamCount = 4;
    static_assert(ParamCount <= kMaxEffectParams);

    static constexpr EffectParam Params[ParamCount] = {
        {"Bits", 4.0f, 16.0f, 8.0f},
        {"Downsample", 1.0f, 16.0f, 4.0f},
        {"Tone", 0.1f, 1.0f, 1.0f},
        {"Mix", 0.0f, 1.0f, 1.0f}
    };

    explicit BitcrusherEffect(float sampleRate) noexcept
        : m_sampleRate(sampleRate > 0.0f ? sampleRate : 48000.0f) {
        updateQuantizeScale();
        updateFilter();
    }

    void updateSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            m_sampleRate = sampleRate;
            updateFilter();
        }
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) {
            return;
        }

        const float dryL = left;
        const float dryR = right;

        // Fractional downsample via phase accumulator
        m_phase += m_phaseStep;
        if (m_phase >= 1.0f) {
            m_phase -= 1.0f;
            m_heldL = quantize(left);
            m_heldR = quantize(right);
        }

        const float wetL = m_lpfL.process(m_heldL);
        const float wetR = m_lpfR.process(m_heldR);

        left = dryL * (1.0f - m_mix) + wetL * m_mix;
        right = dryR * (1.0f - m_mix) + wetR * m_mix;
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) {
            m_bits = std::clamp(val, Params[0].min, Params[0].max);
            updateQuantizeScale();
        } else if (id == 1) {
            m_downsample = std::clamp(val, Params[1].min, Params[1].max);
            m_phaseStep = 1.0f / m_downsample;
        } else if (id == 2) {
            m_tone = std::clamp(val, Params[2].min, Params[2].max);
            updateFilter();
        } else if (id == 3) {
            m_mix = std::clamp(val, Params[3].min, Params[3].max);
        }
    }

    [[nodiscard]] float paramValue(uint8_t id) const noexcept {
        if (id == 0) {
            return m_bits;
        }
        if (id == 1) {
            return m_downsample;
        }
        if (id == 2) {
            return m_tone;
        }
        if (id == 3) {
            return m_mix;
        }
        return 0.0f;
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    void setBypassed(bool bypassed) noexcept { m_bypassed = bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    void updateQuantizeScale() noexcept {
        m_quantizeScale = std::exp2f(m_bits - 1.0f);
        m_invQuantizeScale = 1.0f / m_quantizeScale;
    }

    void updateFilter() noexcept {
        const float cutoffHz = toneCutoffHz(m_tone, Params[2].min, Params[2].max);
        m_lpfL.setCutoffHz(cutoffHz, m_sampleRate);
        m_lpfR.setCutoffHz(cutoffHz, m_sampleRate);
    }

    [[nodiscard]] float quantize(float x) const noexcept {
        return std::roundf(x * m_quantizeScale) * m_invQuantizeScale;
    }

    float m_sampleRate{48000.0f};

    float m_bits{8.0f};
    float m_downsample{4.0f};
    float m_tone{1.0f};
    float m_mix{1.0f};

    float m_quantizeScale{128.0f};
    float m_invQuantizeScale{1.0f / 128.0f};
    float m_phaseStep{0.25f};
    float m_phase{0.0f};

    OnePoleLpf m_lpfL;
    OnePoleLpf m_lpfR;

    float m_heldL{0.0f};
    float m_heldR{0.0f};
    bool m_bypassed{false};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_BITCRUSHER_EFFECT_H
