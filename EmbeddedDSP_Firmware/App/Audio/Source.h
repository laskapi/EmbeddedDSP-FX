#ifndef EMBEDDEDDSP_FIRMWARE_AUDIO_SOURCE_H
#define EMBEDDEDDSP_FIRMWARE_AUDIO_SOURCE_H

#include <cstddef>
#include <cstdint>

#if USE_VIRTUAL_AUDIO
#include "Audio/VirtualSource.h"
#endif

namespace App::Audio::Source {


#if USE_VIRTUAL_AUDIO
inline VirtualSource g_virtualSource{
    Protocol::SAMPLE_RATE_HZ, 440.0f, VirtualSource::Waveform::Sine};
#else
inline constexpr float kInvAudioScale = 1.0f / 32768.0f;
#endif

/// @brief Fills one stereo frame from DMA buffer or the bench VirtualSource.
inline void readSample(const std::int16_t* rxPtr, std::size_t sampleIndex,
                       float& left, float& right) noexcept
{
#if USE_VIRTUAL_AUDIO
    (void)rxPtr;
    (void)sampleIndex;
    const StereoSample s = g_virtualSource.nextSample();
    left = s.left;
    right = s.right;
#else
    left = static_cast<float>(rxPtr[sampleIndex * 2]) * kInvAudioScale;
    right = static_cast<float>(rxPtr[sampleIndex * 2 + 1]) * kInvAudioScale;
#endif
}

} // namespace App::Audio::Source

#endif // EMBEDDEDDSP_FIRMWARE_AUDIO_SOURCE_H
