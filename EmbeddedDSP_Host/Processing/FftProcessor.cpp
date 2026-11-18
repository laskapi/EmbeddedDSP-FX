#include "Processing/FftProcessor.h"

#include <algorithm>
#include <numbers>

namespace Host::Processing {

FftProcessor::FftProcessor(std::size_t fftSize, std::size_t hopSize) noexcept
    : m_fftSize(fftSize)
    , m_hopSize(hopSize == 0 ? fftSize : hopSize)
    , m_hanningWindow(fftSize)
    , m_fftBuffer(fftSize)
    , m_magnitudeDb(fftSize / 2)
    , m_ring(fftSize)
{
    prepareWindow();
}

void FftProcessor::prepareWindow() noexcept {
    for (std::size_t i = 0; i < m_fftSize; ++i) {
        m_hanningWindow[i] = 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> * static_cast<float>(i)
                                                     / static_cast<float>(m_fftSize - 1)));
    }
}

void FftProcessor::computeCooleyTukey(std::span<std::complex<float>> buffer) noexcept {
    const std::size_t n = buffer.size();

    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(buffer[i], buffer[j]);
        }
    }

    for (std::size_t len = 2; len <= n; len <<= 1) {
        const float angle = -2.0f * std::numbers::pi_v<float> / static_cast<float>(len);
        const std::complex<float> wlen(std::cos(angle), std::sin(angle));

        for (std::size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (std::size_t k = 0; k < len / 2; ++k) {
                const std::complex<float> u = buffer[i + k];
                const std::complex<float> v = buffer[i + k + len / 2] * w;
                buffer[i + k] = u + v;
                buffer[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

void FftProcessor::runFftOnRing() noexcept {
    for (std::size_t i = 0; i < m_fftSize; ++i) {
        const std::size_t idx = (m_ringWrite + i) % m_fftSize;
        const float normalized = static_cast<float>(m_ring[idx]) / 32768.0f;
        m_fftBuffer[i] = std::complex<float>(normalized * m_hanningWindow[i], 0.0f);
    }

    computeCooleyTukey(m_fftBuffer);

    constexpr float minDb = -100.0f;
    for (std::size_t i = 0; i < m_fftSize / 2; ++i) {
        const float magnitude = std::abs(m_fftBuffer[i]) / static_cast<float>(m_fftSize);
        const float db = 20.0f * std::log10(magnitude + 1e-5f);
        m_magnitudeDb[i] = std::max(db, minDb);
    }
}

const std::vector<float>& FftProcessor::processFrame(std::span<const std::int16_t> pcmSamples) noexcept {
    const std::size_t count = std::min(pcmSamples.size(), m_fftSize);

    for (std::size_t i = 0; i < count; ++i) {
        m_ring[i] = pcmSamples[i];
    }
    for (std::size_t i = count; i < m_fftSize; ++i) {
        m_ring[i] = 0;
    }
    m_ringWrite = 0;
    m_ringFilled = m_fftSize;
    runFftOnRing();
    return m_magnitudeDb;
}

const std::vector<float>* FftProcessor::pushSamples(std::span<const std::int16_t> pcmSamples) noexcept {
    const std::vector<float>* ready = nullptr;

    for (const std::int16_t sample : pcmSamples) {
        m_ring[m_ringWrite] = sample;
        m_ringWrite = (m_ringWrite + 1) % m_fftSize;
        if (m_ringFilled < m_fftSize) {
            ++m_ringFilled;
        }
        ++m_samplesSinceHop;

        if (m_ringFilled == m_fftSize && m_samplesSinceHop >= m_hopSize) {
            runFftOnRing();
            m_samplesSinceHop = 0;
            ready = &m_magnitudeDb;
        }
    }

    return ready;
}

} // namespace Host::Processing
