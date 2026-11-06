#ifndef EMBEDDEDDSP_FIRMWARE_VIRTUAL_SOURCE_H
#define EMBEDDEDDSP_FIRMWARE_VIRTUAL_SOURCE_H

#include "Protocol/ProtocolCommon.h"
#include <cmath>
#include <stdint.h>

namespace App::Audio {

/// @brief Stereo sample pair (linear float PCM).
struct StereoSample {
    float left;
    float right;
};

/// @brief Multi-waveform test oscillator for bench audio (no codec / HAL dependency).
class VirtualSource {
public:
    enum class Waveform { Sine, Square, Triangle, Saw };

    static constexpr int kLutSize = 1024;

    /// @brief Builds sine LUT and sets frequency and waveform.
    explicit VirtualSource(float sampleRate = Protocol::SAMPLE_RATE_HZ,
                           float frequencyHz = 440.0f,
                           Waveform waveform = Waveform::Saw) noexcept
        : m_step((frequencyHz * static_cast<float>(kLutSize)) / sampleRate)
        , m_type(waveform)
    {
        for (int i = 0; i < kLutSize; ++i) {
            m_sinLut[i] = sinf(2.0f * 3.1415926535f * static_cast<float>(i) /
                               static_cast<float>(kLutSize));
        }
    }

    /// @brief Next stereo sample from current phase and waveform.
    [[nodiscard]] StereoSample nextSample() noexcept
    {
        float val = 0.0f;
        const float normalizedPhase = m_phase / static_cast<float>(kLutSize);

        switch (m_type) {
        case Waveform::Sine: {
            const int i1 = static_cast<int>(m_phase);
            const int i2 = (i1 + 1) & (kLutSize - 1);
            const float frac = m_phase - static_cast<float>(i1);
            val = m_sinLut[i1] + frac * (m_sinLut[i2] - m_sinLut[i1]);
            break;
        }
        case Waveform::Square:
            val = (normalizedPhase < 0.5f) ? 1.0f : -1.0f;
            break;
        case Waveform::Triangle:
            val = 2.0f * fabsf(2.0f * (normalizedPhase - floorf(normalizedPhase + 0.5f))) - 1.0f;
            break;
        case Waveform::Saw:
            val = 2.0f * (normalizedPhase - floorf(normalizedPhase + 0.5f));
            break;
        }

        m_phase += m_step;
        if (m_phase >= static_cast<float>(kLutSize)) {
            m_phase -= static_cast<float>(kLutSize);
        }

        return {val, val};
    }

private:
    float m_sinLut[kLutSize];
    float m_phase{0.0f};
    float m_step;
    Waveform m_type;
};

} // namespace App::Audio

#endif // EMBEDDEDDSP_FIRMWARE_VIRTUAL_SOURCE_H
