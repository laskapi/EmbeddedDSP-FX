#ifndef EMBEDDEDDSP_FIRMWARE_ONE_POLE_LPF_H
#define EMBEDDEDDSP_FIRMWARE_ONE_POLE_LPF_H

#include <cmath>

namespace App::Dsp {

/// @brief param / gain smoother time constant (~10 ms).
inline constexpr float kParamSmoothTimeSec = 0.010f;

/// @brief Log-map Tone knob (default 0.1..1.0) to cutoff Hz (~400 Hz .. 10 kHz).
[[nodiscard]] inline float toneCutoffHz(float tone,
                                        float knobMin = 0.1f,
                                        float knobMax = 1.0f) noexcept {
    const float span = knobMax - knobMin;
    const float t = (span > 0.0f) ? (tone - knobMin) / span : 0.0f;
    return 400.0f * std::powf(25.0f, t); // 400 * 25^1 = 10 kHz
}

/// @brief Cheap 1-pole low-pass: y += a * (x - y).
class OnePoleLpf {
public:
    /// @brief Audio Tone path: coeff from cutoff Hz and sample rate.
    void setCutoffHz(float cutoffHz, float sampleRate) noexcept {
        if (sampleRate <= 0.0f) {
            return;
        }
        const float w0 = 2.0f * 3.14159265f * cutoffHz / sampleRate;
        m_coeff = 1.0f - std::expf(-w0);
    }

    /// @brief Param/gain smoother: recalc coeff for @ref kParamSmoothTimeSec at @p sampleRate.
    void setParamSmoothSampleRate(float sampleRate) noexcept {
        if (sampleRate <= 0.0f) {
            return;
        }
        m_coeff = 1.0f - std::expf(-1.0f / (kParamSmoothTimeSec * sampleRate));
    }

    [[nodiscard]] float process(float x) noexcept {
        m_state += m_coeff * (x - m_state);
        return m_state;
    }

    void reset(float value = 0.0f) noexcept { m_state = value; }

private:
    float m_coeff{1.0f};
    float m_state{0.0f};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_ONE_POLE_LPF_H
