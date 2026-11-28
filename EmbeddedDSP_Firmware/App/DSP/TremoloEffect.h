#ifndef EMBEDDEDDSP_FIRMWARE_TREMOLO_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_TREMOLO_EFFECT_H

#include "DSP/AudioEffectConcept.h"
#include "DSP/OnePoleLpf.h"
#include "Dsp/DspMath.h"
#include "Protocol/ProtocolCommon.h"
#include <algorithm>
#include <cmath>

namespace App::Dsp {

/// @brief Amplitude tremolo / auto-panner: quadrature LFO modulates output level.
class TremoloEffect {
public:
    static constexpr uint8_t Id = Protocol::EffectId::Tremolo;
    static constexpr const char* Name = "Tremolo";
    static constexpr uint8_t ParamCount = 3;
    static_assert(ParamCount <= Protocol::kMaxEffectParams);

    static constexpr EffectParam Params[ParamCount] = {
        {"Rate", 0.5f, 12.0f, 4.0f, "Hz"},
        {"Depth", 0.0f, 1.0f, 0.5f},
        {"Stereo", 0.0f, 1.0f, 0.0f}
    };

    explicit TremoloEffect(float sampleRate) noexcept {
        m_depthSmooth.reset(m_depth);
        m_stereoCosSmooth.reset(m_stereoCos);
        m_stereoSinSmooth.reset(m_stereoSin);
        updateSampleRate(sampleRate);
    }

    void updateSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            m_sampleRate = sampleRate;
            updatePhaseIncrement();
            m_depthSmooth.setParamSmoothSampleRate(m_sampleRate);
            m_stereoCosSmooth.setParamSmoothSampleRate(m_sampleRate);
            m_stereoSinSmooth.setParamSmoothSampleRate(m_sampleRate);
        }
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) {
            return;
        }

        const float depth = m_depthSmooth.process(m_depth);
        const float stereoCos = m_stereoCosSmooth.process(m_stereoCos);
        const float stereoSin = m_stereoSinSmooth.process(m_stereoSin);

        // Left: LFO from quadrature Y (m_sin)
        const float lfoL = 0.5f * (1.0f + m_sin);

        // Right: same LFO rotated by stereo angle (angle-addition, mul only)
        const float sinR = m_sin * stereoCos + m_cos * stereoSin;
        const float lfoR = 0.5f * (1.0f + sinR);

        left  *= 1.0f - depth * (1.0f - lfoL);
        right *= 1.0f - depth * (1.0f - lfoR);

        // Advance quadrature phasor + Taylor radius correction
        const float nextCos = m_cos * m_cosInc - m_sin * m_sinInc;
        const float nextSin = m_sin * m_cosInc + m_cos * m_sinInc;
        const float corr = 1.5f - 0.5f * (nextCos * nextCos + nextSin * nextSin);

        m_cos = nextCos * corr;
        m_sin = nextSin * corr;
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) {
            m_rate = std::clamp(val, Params[0].min, Params[0].max);
            updatePhaseIncrement();
        } else if (id == 1) {
            m_depth = std::clamp(val, Params[1].min, Params[1].max);
        } else if (id == 2) {
            m_stereo = std::clamp(val, Params[2].min, Params[2].max);
            const float shift = m_stereo * Shared::Dsp::kPi;
            m_stereoCos = std::cos(shift);
            m_stereoSin = std::sin(shift);
        }
    }

    [[nodiscard]] float paramValue(uint8_t id) const noexcept {
        if (id == 0) {
            return m_rate;
        }
        if (id == 1) {
            return m_depth;
        }
        if (id == 2) {
            return m_stereo;
        }
        return 0.0f;
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    void setBypassed(bool bypassed) noexcept { m_bypassed = bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    void updatePhaseIncrement() noexcept {
        const float phaseInc = Shared::Dsp::angularFreq(m_rate, m_sampleRate);
        m_cosInc = std::cos(phaseInc);
        m_sinInc = std::sin(phaseInc);
    }

    float m_sampleRate{Protocol::SAMPLE_RATE_HZ};
    float m_rate{4.0f};
    float m_depth{0.5f};
    float m_stereo{0.0f};

    // Target stereo-shift components (updated only in setParamValue)
    float m_stereoCos{1.0f}; // cos(0) = 1
    float m_stereoSin{0.0f}; // sin(0) = 0

    OnePoleLpf m_depthSmooth;
    OnePoleLpf m_stereoCosSmooth;
    OnePoleLpf m_stereoSinSmooth;

    // Quadrature oscillator state (angle 0: cos=1, sin=0)
    float m_cos{1.0f};
    float m_sin{0.0f};
    float m_cosInc{1.0f};
    float m_sinInc{0.0f};

    bool m_bypassed{false};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_TREMOLO_EFFECT_H
