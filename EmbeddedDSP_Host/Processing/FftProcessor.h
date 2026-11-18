#ifndef EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
#define EMBEDDEDDSP_HOST_FFT_PROCESSOR_H

#include <complex>
#include <cstdint>
#include <span>
#include <vector>

namespace Host::Processing {

/// @brief Builds the spectrum FFT from incoming PCM chunks.
class FftProcessor {
public:
    explicit FftProcessor(std::size_t fftSize, std::size_t hopSize = 0) noexcept;

    /// @brief Run FFT on one full buffer (tests / non-streaming).
    const std::vector<float>& processFrame(std::span<const std::int16_t> pcmSamples) noexcept;

    /// @brief Feed samples; returns new spectrum when enough data arrived.
    [[nodiscard]] const std::vector<float>*
    pushSamples(std::span<const std::int16_t> pcmSamples) noexcept;

    [[nodiscard]] std::size_t fftSize() const noexcept { return m_fftSize; }
    [[nodiscard]] std::size_t hopSize() const noexcept { return m_hopSize; }

private:
    void prepareWindow() noexcept;
    void computeCooleyTukey(std::span<std::complex<float>> buffer) noexcept;
    void runFftOnRing() noexcept;

    std::size_t m_fftSize;
    std::size_t m_hopSize;
    std::vector<float> m_hanningWindow;
    std::vector<std::complex<float>> m_fftBuffer;
    std::vector<float> m_magnitudeDb;

    std::vector<std::int16_t> m_ring;
    std::size_t m_ringWrite{0};
    std::size_t m_ringFilled{0};
    std::size_t m_samplesSinceHop{0};
};

} // namespace Host::Processing

#endif // EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
