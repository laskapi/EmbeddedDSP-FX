#ifndef EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
#define EMBEDDEDDSP_HOST_FFT_PROCESSOR_H

#include <vector>
#include <complex>
#include <cmath>
#include <cstdint>
#include <span>

namespace Host {

/// @brief Utility class for real-time audio spectrum analysis.
class FftProcessor {
public:
    explicit FftProcessor(size_t fftSize = 128) noexcept;

    /// @brief Converts raw PCM samples to magnitude spectrum (dB).
    const std::vector<float>& processFrame(std::span<const int16_t> pcmSamples) noexcept;

private:
    size_t m_fftSize;
    std::vector<float> m_hanningWindow;
    std::vector<std::complex<float>> m_fftBuffer;
    std::vector<float> m_magnitudeDb;

    void prepareWindow() noexcept;
    void computeCooleyTukey(std::span<std::complex<float>> buffer) noexcept;
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
