#ifndef EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H
#define EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H

#include "DSP/OnePoleLpf.h"
#include "Protocol/ProtocolCommon.h"
#include <algorithm>

namespace App::Dsp {

/// @brief Global linear output gain after the effect slots (per-sample smoothed).
class MasterVolume {
public:
    static constexpr float kDefaultGain = 0.8f;
    static constexpr float kMinGain = 0.0f;
    static constexpr float kMaxGain = 1.0f;

    MasterVolume() noexcept {
        updateSampleRate(Protocol::SAMPLE_RATE_HZ);
        m_smooth.reset(kDefaultGain);
    }

    void updateSampleRate(float sampleRate) noexcept {
        m_smooth.setParamSmoothSampleRate(sampleRate);
    }

    /// @brief Commanded gain (wire / UI); not the mid-ramp value.
    [[nodiscard]] float gain() const noexcept { return m_target; }

    void setGain(float gain) noexcept {
        m_target = std::clamp(gain, kMinGain, kMaxGain);
    }

    /// @brief Applies smoothed master gain to one stereo sample.
    void apply(float& left, float& right) noexcept {
        const float g = m_smooth.process(m_target);
        left *= g;
        right *= g;
    }

private:
    float m_target{kDefaultGain};
    OnePoleLpf m_smooth;
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H
