#ifndef EMBEDDEDDSP_FIRMWARE_EQ_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_EQ_EFFECT_H

#include "DSP/AudioEffectConcept.h"
#include "DSP/Biquad.h"
#include "Dsp/EqToneStack.h"
#include "Protocol/ProtocolCommon.h"
#include <algorithm>
#include <cmath>

namespace App::Dsp {

/// @brief 3-band tone stack: low shelf / parametric peaking mid / high shelf (RBJ biquads).
class EqEffect {
public:
    static constexpr uint8_t Id = Protocol::EffectId::Eq;
    static constexpr const char* Name = "EQ";
    static constexpr uint8_t ParamCount = 5;
    static_assert(ParamCount <= Protocol::kMaxEffectParams);

    static constexpr float kBassHz = Shared::Dsp::kEqBassHz;
    static constexpr float kTrebleHz = Shared::Dsp::kEqTrebleHz;

    static constexpr EffectParam Params[ParamCount] = {
        {"Bass", -12.0f, 12.0f, 0.0f, "dB"},
        {"Mid", -12.0f, 12.0f, 0.0f, "dB"},
        {"Treble", -12.0f, 12.0f, 0.0f, "dB"},
        {"Freq", 200.0f, 5000.0f, 1000.0f, "Hz"},
        {"Q", 0.3f, 8.0f, 0.7f}
    };

    explicit EqEffect(float sampleRate) noexcept {
        updateSampleRate(sampleRate);
    }

    void updateSampleRate(float sampleRate) noexcept {
        if (sampleRate > 0.0f) {
            m_sampleRate = sampleRate;
            updateLow();
            updateMid();
            updateHigh();
        }
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) {
            return;
        }

        left = m_highL.process(m_midL.process(m_lowL.process(left, m_low), m_mid), m_high);
        right = m_highR.process(m_midR.process(m_lowR.process(right, m_low), m_mid), m_high);
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) {
            m_bassDb = std::clamp(val, Params[0].min, Params[0].max);
            updateLow();
        } else if (id == 1) {
            m_midDb = std::clamp(val, Params[1].min, Params[1].max);
            updateMid();
        } else if (id == 2) {
            m_trebleDb = std::clamp(val, Params[2].min, Params[2].max);
            updateHigh();
        } else if (id == 3) {
            m_midHz = std::clamp(val, Params[3].min, Params[3].max);
            updateMid();
        } else if (id == 4) {
            m_midQ = std::clamp(val, Params[4].min, Params[4].max);
            updateMid();
        }
    }

    [[nodiscard]] float paramValue(uint8_t id) const noexcept {
        if (id == 0) {
            return m_bassDb;
        }
        if (id == 1) {
            return m_midDb;
        }
        if (id == 2) {
            return m_trebleDb;
        }
        if (id == 3) {
            return m_midHz;
        }
        if (id == 4) {
            return m_midQ;
        }
        return 0.0f;
    }

    void toggleBypass() noexcept { setBypassed(!m_bypassed); }

    void setBypassed(bool bypassed) noexcept {
        if (bypassed && !m_bypassed) {
            resetStates(); // drop frozen IIR tail before idle
        }
        m_bypassed = bypassed;
    }

    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    void updateLow() noexcept {
        m_low = (std::fabs(m_bassDb) < Shared::Dsp::kEqGainBypassDb)
            ? BiquadCoeffs::identity()
            : BiquadCoeffs::lowShelf(m_sampleRate, kBassHz, m_bassDb);
    }

    void updateMid() noexcept {
        m_mid = (std::fabs(m_midDb) < Shared::Dsp::kEqGainBypassDb)
            ? BiquadCoeffs::identity()
            : BiquadCoeffs::peaking(m_sampleRate, m_midHz, m_midDb, m_midQ);
    }

    void updateHigh() noexcept {
        m_high = (std::fabs(m_trebleDb) < Shared::Dsp::kEqGainBypassDb)
            ? BiquadCoeffs::identity()
            : BiquadCoeffs::highShelf(m_sampleRate, kTrebleHz, m_trebleDb);
    }

    void resetStates() noexcept {
        m_lowL.reset();
        m_midL.reset();
        m_highL.reset();
        m_lowR.reset();
        m_midR.reset();
        m_highR.reset();
    }

    float m_sampleRate{Protocol::SAMPLE_RATE_HZ};
    float m_bassDb{0.0f};
    float m_midDb{0.0f};
    float m_trebleDb{0.0f};
    float m_midHz{1000.0f};
    float m_midQ{0.7f};

    BiquadCoeffs m_low{};
    BiquadCoeffs m_mid{};
    BiquadCoeffs m_high{};

    BiquadState m_lowL{};
    BiquadState m_midL{};
    BiquadState m_highL{};
    BiquadState m_lowR{};
    BiquadState m_midR{};
    BiquadState m_highR{};

    bool m_bypassed{false};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_EQ_EFFECT_H
