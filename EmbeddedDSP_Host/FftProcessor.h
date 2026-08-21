#ifndef EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
#define EMBEDDEDDSP_HOST_FFT_PROCESSOR_H

#include <vector>
#include <complex>
#include <cmath>
#include <cstdint>
#include <span>

class FftProcessor {
private:
    size_t m_fftSize;
    std::vector<float> m_hanningWindow;
    std::vector<std::complex<float>> m_fftBuffer;
    std::vector<float> m_magnitudeDb;

    void prepareWindow() noexcept;
    void computeCooleyTukey(std::span<std::complex<float>> buffer) noexcept;

public:
    explicit FftProcessor(size_t fftSize = 128) noexcept;

    // Process raw int16_t PCM buffer into dB magnitude spectrum
    const std::vector<float>& processFrame(std::span<const int16_t> pcmSamples) noexcept;
};

#endif // EMBEDDEDDSP_HOST_FFT_PROCESSOR_H
