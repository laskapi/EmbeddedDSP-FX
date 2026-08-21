#include "FftProcessor.h"
#include <numbers>
#include <algorithm>

FftProcessor::FftProcessor(size_t fftSize) noexcept
    : m_fftSize(fftSize),
    m_hanningWindow(fftSize),
    m_fftBuffer(fftSize),
    m_magnitudeDb(fftSize / 2) {
    prepareWindow();
}

void FftProcessor::prepareWindow() noexcept {
    for (size_t i = 0; i < m_fftSize; ++i) {
        // Hanning Window formula: 0.5 * (1 - cos(2 * pi * n / (N - 1)))
        m_hanningWindow[i] = 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> * static_cast<float>(i)
                                                     / static_cast<float>(m_fftSize - 1)));
    }
}

// In-place Radix-2 Decimation-in-Time FFT
void FftProcessor::computeCooleyTukey(std::span<std::complex<float>> buffer) noexcept {
    const size_t n = buffer.size();

    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(buffer[i], buffer[j]);
        }
    }

    // Cooley-Tukey Radix-2
    for (size_t len = 2; len <= n; len <<= 1) {
        float angle = -2.0f * std::numbers::pi_v<float> / static_cast<float>(len);
        std::complex<float> wlen(std::cos(angle), std::sin(angle));

        for (size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (size_t k = 0; k < len / 2; ++k) {
                std::complex<float> u = buffer[i + k];
                std::complex<float> v = buffer[i + k + len / 2] * w;
                buffer[i + k] = u + v;
                buffer[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

const std::vector<float>& FftProcessor::processFrame(std::span<const int16_t> pcmSamples) noexcept {
    const size_t count = std::min(pcmSamples.size(), m_fftSize);

    // 1. Convert Q15 int16_t -> Normalized Float [-1.0, 1.0] + Windowing
    for (size_t i = 0; i < count; ++i) {
        float normalized = static_cast<float>(pcmSamples[i]) / 32768.0f;
        m_fftBuffer[i] = std::complex<float>(normalized * m_hanningWindow[i], 0.0f);
    }

    // Zero-padding if input is shorter than FFT size
    for (size_t i = count; i < m_fftSize; ++i) {
        m_fftBuffer[i] = std::complex<float>(0.0f, 0.0f);
    }

    // 2. Compute FFT
    computeCooleyTukey(m_fftBuffer);

    // 3. Calculate Magnitude Spectrum in dB (only for positive frequencies: N / 2)
    const float minDb = -100.0f;
    for (size_t i = 0; i < m_fftSize / 2; ++i) {
        float magnitude = std::abs(m_fftBuffer[i]) / static_cast<float>(m_fftSize);
        float db = 20.0f * std::log10(magnitude + 1e-5f); // 1e-5 guard against log(0)

        m_magnitudeDb[i] = std::max(db, minDb);
    }

    return m_magnitudeDb;
}