#ifndef EMBEDDEDDSP_SHARED_DSP_MATH_H
#define EMBEDDEDDSP_SHARED_DSP_MATH_H

#include <cmath>

namespace Shared::Dsp {

inline constexpr float kPi = 3.14159265359f;
inline constexpr float kTwoPi = 6.28318530718f;

/// @brief Digitized angular frequency ω = 2π f / fs (filters, LFO phase increment).
[[nodiscard]] inline float angularFreq(float freqHz, float sampleRate) noexcept {
    return kTwoPi * freqHz / sampleRate;
}

/// @brief Amplitude from dB: 10^(dB/20). Use for gains / compressor makeup.
[[nodiscard]] inline float dbToAmplitude(float db) noexcept {
    return std::pow(10.0f, db * 0.05f);
}

/// @brief Amplitude to dB: 20*log10(a). @p eps avoids log(0).
[[nodiscard]] inline float amplitudeToDb(float amplitude, float eps = 1e-20f) noexcept {
    return 20.0f * std::log10(amplitude + eps);
}

/// @brief RBJ cookbook A = 10^(dB/40) for peaking / shelf filters (not the same as amplitude).
[[nodiscard]] inline float dbToEqA(float db) noexcept {
    return std::pow(10.0f, db * 0.025f);
}

} // namespace Shared::Dsp

#endif // EMBEDDEDDSP_SHARED_DSP_MATH_H
