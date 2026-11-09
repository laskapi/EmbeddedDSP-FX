#ifndef EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H
#define EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H

#include "Protocol/ProtocolCommon.h"
#include <algorithm>

namespace App::Dsp {

/// @brief Global linear output gain after the effect slots (per-sample smoothed).
class MasterVolume {
public:
    static constexpr float kDefaultGain = 0.8f;
    static constexpr float kMinGain = 0.0f;
    static constexpr float kMaxGain = 1.0f;

    /// @brief One-pole toward target; ~10 ms (alpha ≈ 1 - exp(-1/(tau*fs))).
    static constexpr float kSmoothingTimeSec = 0.010f;
    static constexpr float kSmoothingAlpha =
        1.0f / (kSmoothingTimeSec * Protocol::SAMPLE_RATE_HZ);

    /// @brief Commanded gain (wire / UI); not the mid-ramp value.
    [[nodiscard]] float gain() const noexcept { return m_target; }

    void setGain(float gain) noexcept {
        m_target = std::clamp(gain, kMinGain, kMaxGain);
    }

    /// @brief Applies smoothed master gain to one stereo sample.
    void apply(float& left, float& right) noexcept {
        m_current += kSmoothingAlpha * (m_target - m_current);
        left *= m_current;
        right *= m_current;
    }

private:
    float m_target{kDefaultGain};
    float m_current{kDefaultGain};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H
