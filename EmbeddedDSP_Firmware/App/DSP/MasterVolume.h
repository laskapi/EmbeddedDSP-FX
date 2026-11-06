#ifndef EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H
#define EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H

#include <algorithm>

namespace App::Dsp {

/// @brief Global linear output gain after the effect slots.
class MasterVolume {
public:
    static constexpr float kDefaultGain = 0.8f;
    static constexpr float kMinGain = 0.0f;
    static constexpr float kMaxGain = 1.0f;

    [[nodiscard]] float gain() const noexcept { return m_gain; }

    void setGain(float gain) noexcept {
        m_gain = std::clamp(gain, kMinGain, kMaxGain);
    }

    /// @brief Applies master gain to one stereo sample.
    void apply(float& left, float& right) const noexcept {
        left *= m_gain;
        right *= m_gain;
    }

private:
    float m_gain{kDefaultGain};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_MASTER_VOLUME_H
