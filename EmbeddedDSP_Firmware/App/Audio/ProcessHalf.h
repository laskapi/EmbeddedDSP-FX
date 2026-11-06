#ifndef EMBEDDEDDSP_FIRMWARE_PROCESS_HALF_H
#define EMBEDDEDDSP_FIRMWARE_PROCESS_HALF_H

#include "Audio/I2sDma.h"
#include "Audio/Source.h"
#include "DSP/FxChain.h"
#include "Protocol/ProtocolCommon.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace App::Audio {

namespace detail {

constexpr float kAudioScaleFactor = 32767.0f;

[[nodiscard]] inline std::uint32_t floatToQ15SaturateStereo(float left, float right) noexcept
{
    const auto left32 = static_cast<std::int32_t>(std::lroundf(left * kAudioScaleFactor));
    const auto right32 = static_cast<std::int32_t>(std::lroundf(right * kAudioScaleFactor));

    std::uint32_t packedInput{0};
    asm volatile("pkhbt %0, %1, %2, lsl #16" : "=r"(packedInput) : "r"(left32), "r"(right32));

    std::uint32_t packedResult{0};
    asm volatile("ssat16 %0, #16, %1" : "=r"(packedResult) : "r"(packedInput));

    return packedResult;
}

[[nodiscard]] inline std::int16_t floatToQ15SaturateMono(float sample) noexcept
{
    const auto v = static_cast<std::int32_t>(std::lroundf(sample * kAudioScaleFactor));
    return static_cast<std::int16_t>(
        std::clamp(v, std::int32_t{-32768}, std::int32_t{32767}));
}

/// @brief Shared half loop; usbSamples used only when WithUsb is true.
template<bool WithUsb>
inline void processHalfBody(
    const std::int16_t* rx,
    Dsp::FxChain& chain,
    std::int16_t* tx,
    std::array<std::int16_t, Protocol::AUDIO_SAMPLES>* usbSamples) noexcept
{
    auto* txPtr32 = reinterpret_cast<std::uint32_t*>(tx);

    for (std::size_t i = 0; i < I2sDma::kStereoFramesPerHalf; ++i) {
        float left = 0.0f;
        float right = 0.0f;
        Source::readSample(rx, i, left, right);
        chain.process(left, right);
        txPtr32[i] = floatToQ15SaturateStereo(left, right);
        if constexpr (WithUsb) {
            (*usbSamples)[i] = floatToQ15SaturateMono(0.5f * (left + right));
        }
    }
}

} // namespace detail

/// @brief One DMA half: RX → FxChain → DAC TX.
inline void processHalf(const std::int16_t* rx,
                        Dsp::FxChain& chain,
                        std::int16_t* tx) noexcept
{
    detail::processHalfBody<false>(rx, chain, tx, nullptr);
}

/// @brief One DMA half: RX → FxChain → DAC TX + USB mono downmix.
inline void processHalf(const std::int16_t* rx,
                        Dsp::FxChain& chain,
                        std::int16_t* tx,
                        std::array<std::int16_t, Protocol::AUDIO_SAMPLES>& usbSamples) noexcept
{
    detail::processHalfBody<true>(rx, chain, tx, &usbSamples);
}

} // namespace App::Audio

#endif // EMBEDDEDDSP_FIRMWARE_PROCESS_HALF_H
