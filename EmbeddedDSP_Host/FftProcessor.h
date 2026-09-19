#ifndef EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
#define EMBEDDEDDSP_HOST_FFT_PROCESSOR_H

#include <vector>
#include <complex>
#include <cmath>
#include <cstdint>
#include <span>

/**
 * @brief Utility for real-time FFT processing of audio frames.
 */
class FftProcessor {
private:
    size_t m_fftSize;
    std::vector<float> m_hanningWindow;
    std::vector<std::complex<float>> m_fftBuffer;
    std::vector<float> m_magnitudeDb;

    void prepareWindow() noexcept;
    void computeCooleyTukey(std::span<std::complex<float>> buffer) noexcept;

public:
    /**
     * @brief Constructor for FftProcessor.
     * @param fftSize Number of samples per frame (must be power of 2).
     */
    explicit FftProcessor(size_t fftSize = 128) noexcept;

    /**
     * @brief Processes a frame of PCM samples into a dB magnitude spectrum.
     * @param pcmSamples Span of input PCM data.
     * @return Reference to the vector containing magnitude values in dB.
     */
    const std::vector<float>& processFrame(std::span<const int16_t> pcmSamples) noexcept;
};

#endif // EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
