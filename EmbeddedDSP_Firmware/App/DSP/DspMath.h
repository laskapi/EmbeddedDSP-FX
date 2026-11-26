#ifndef EMBEDDEDDSP_FIRMWARE_DSP_MATH_H
#define EMBEDDEDDSP_FIRMWARE_DSP_MATH_H

#include <cmath>

namespace App::Dsp::Math {

inline constexpr float kPi = 3.14159265359f;
inline constexpr float kTwoPi = 6.28318530718f;

/// @brief Digitized angular frequency ω = 2π f / fs (filters, LFO phase increment).
[[nodiscard]] inline float angularFreq(float freqHz, float sampleRate) noexcept {
    return kTwoPi * freqHz / sampleRate;
}

/// @brief Amplitude from dB: 10^(dB/20). Use for gains / compressor makeup.
[[nodiscard]] inline float dbToAmplitude(float db) noexcept {
    return std::powf(10.0f, db * 0.05f);
}

/// @brief RBJ cookbook A = 10^(dB/40) for peaking / shelf filters (not the same as amplitude).
[[nodiscard]] inline float dbToEqA(float db) noexcept {
    return std::powf(10.0f, db * 0.025f);
}

} // namespace App::Dsp::Math

#endif // EMBEDDEDDSP_FIRMWARE_DSP_MATH_H
