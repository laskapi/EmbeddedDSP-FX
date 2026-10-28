#ifndef EMBEDDEDDSP_FIRMWARE_VIRTUAL_AUDIO_SOURCE_H
#define EMBEDDEDDSP_FIRMWARE_VIRTUAL_AUDIO_SOURCE_H

#include "Protocol/ProtocolCommon.h"
#include <cmath>
#include <stdint.h>

/// @brief Stereo sample pair (linear float PCM).
struct StereoSample
{
    float left;
    float right;
};

/// @brief Multi-waveform test oscillator for pipeline debugging.
class VirtualAudioSource
{
public:
    enum class Waveform { Sine, Square, Triangle, Saw };

    static constexpr int LUT_SIZE = 1024;

    explicit VirtualAudioSource(float sampleRate) noexcept
        : m_sampleRate(sampleRate > 0 ? sampleRate : Protocol::SAMPLE_RATE_HZ)
    {
        for (int i = 0; i < LUT_SIZE; ++i)
        {
            m_sinLut[i] = sinf(2.0f * 3.1415926535f * (float)i / (float)LUT_SIZE);
        }
        setFrequency(440.0f);
    }

    void setFrequency(float freq) noexcept
    {
        m_step = (freq * (float)LUT_SIZE) / m_sampleRate;
    }

    void setWaveform(Waveform type) noexcept { m_type = type; }

    /// @brief Next stereo sample from current phase and waveform.
    [[nodiscard]] StereoSample nextSample() noexcept
    {
        float val = 0.0f;
        float normalizedPhase = m_phase / (float)LUT_SIZE;

        switch (m_type)
        {
        case Waveform::Sine:
            {
                int i1 = static_cast<int>(m_phase);
                int i2 = (i1 + 1) & (LUT_SIZE - 1);
                float frac = m_phase - (float)i1;
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
        if (m_phase >= (float)LUT_SIZE) m_phase -= (float)LUT_SIZE;

        return {val, val};
    }

private:
    float m_sampleRate;
    float m_sinLut[LUT_SIZE];
    float m_phase{0.0f};
    float m_step{0.0f};
    Waveform m_type{Waveform::Sine};
};

#endif // EMBEDDEDDSP_FIRMWARE_VIRTUAL_AUDIO_SOURCE_H
