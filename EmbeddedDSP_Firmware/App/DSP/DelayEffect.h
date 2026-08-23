#ifndef EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

/**
 * @brief Mono delay with shared int16 line @ 16 kHz (~1 s / 32 KB).
 *        Decimation x3 from ~48 kHz system rate. Max one instance in pipeline.
 */
class DelayEffect {
public:
    static constexpr std::size_t MAX_LINE_SAMPLES{16000};
    static constexpr float LINE_SAMPLE_RATE{16000.0f};
    static constexpr std::size_t DECIMATION{3};
    static constexpr float INT16_SCALE{32767.0f};
    static constexpr float MAX_DELAY_SECONDS{
        static_cast<float>(MAX_LINE_SAMPLES - 1) / LINE_SAMPLE_RATE};

private:
    float m_sampleRate{48000.0f};
    float m_targetDelayLineSamples{LINE_SAMPLE_RATE * 0.35f};
    float m_currentDelayLineSamples{LINE_SAMPLE_RATE * 0.35f};
    static constexpr float SMOOTHING_FACTOR{0.001f};

    float m_feedback{0.4f};
    float m_dryWet{0.5f};
    bool m_bypassed{false};

    std::size_t m_writeIndex{0};
    std::size_t m_decimCounter{0};
    float m_antiAliasState{0.0f};
    float m_antiAliasCoeff{0.0f};

    alignas(4) inline static std::array<std::int16_t, MAX_LINE_SAMPLES> s_delayLine{};

    [[nodiscard]] static float readLineInterpolated(float readPosition) noexcept
    {
        while (readPosition < 0.0f) {
            readPosition += static_cast<float>(MAX_LINE_SAMPLES);
        }
        while (readPosition >= static_cast<float>(MAX_LINE_SAMPLES)) {
            readPosition -= static_cast<float>(MAX_LINE_SAMPLES);
        }

        const auto indexA = static_cast<std::size_t>(readPosition);
        const std::size_t indexB = (indexA + 1) % MAX_LINE_SAMPLES;
        const float frac = readPosition - static_cast<float>(indexA);

        const float sampleA = static_cast<float>(s_delayLine[indexA]) / INT16_SCALE;
        const float sampleB = static_cast<float>(s_delayLine[indexB]) / INT16_SCALE;
        return sampleA + frac * (sampleB - sampleA);
    }

    static void clearSharedLine() noexcept
    {
        s_delayLine.fill(0);
    }

public:
    DelayEffect() noexcept = default;

    void prepare(float newSampleRate) noexcept
    {
        m_sampleRate = (newSampleRate > 0.0f) ? newSampleRate : 48000.0f;

        constexpr float twoPi = 6.28318530718f;
        const float cutoffHz = 0.4f * (LINE_SAMPLE_RATE * 0.5f);
        const float x = std::exp(-twoPi * cutoffHz / m_sampleRate);
        m_antiAliasCoeff = 1.0f - x;

        reset();
    }

    void reset() noexcept
    {
        clearSharedLine();
        m_writeIndex = 0;
        m_decimCounter = 0;
        m_antiAliasState = 0.0f;
        m_currentDelayLineSamples = m_targetDelayLineSamples;
    }

    void process(float& left, float& right) noexcept
    {
        if (m_bypassed) {
            return;
        }

        m_currentDelayLineSamples +=
            (m_targetDelayLineSamples - m_currentDelayLineSamples) * SMOOTHING_FACTOR;

        const float inputMono = (left + right) * 0.5f;
        m_antiAliasState += m_antiAliasCoeff * (inputMono - m_antiAliasState);

        const float readPosition =
            static_cast<float>(m_writeIndex) - m_currentDelayLineSamples;
        const float delayedSample = readLineInterpolated(readPosition);

        ++m_decimCounter;
        if (m_decimCounter >= DECIMATION) {
            m_decimCounter = 0;

            const float newDelayValue = m_antiAliasState + (delayedSample * m_feedback);
            const float clamped =
                std::clamp(newDelayValue * INT16_SCALE, -INT16_SCALE, INT16_SCALE);
            s_delayLine[m_writeIndex] = static_cast<std::int16_t>(clamped);

            m_writeIndex = (m_writeIndex + 1) % MAX_LINE_SAMPLES;
        }

        left = (left * (1.0f - m_dryWet)) + (delayedSample * m_dryWet);
        right = (right * (1.0f - m_dryWet)) + (delayedSample * m_dryWet);
    }

    void setDelayTime(float seconds) noexcept
    {
        seconds = std::clamp(seconds, 0.001f, MAX_DELAY_SECONDS);
        m_targetDelayLineSamples = seconds * LINE_SAMPLE_RATE;
    }

    void setFeedback(float fb) noexcept
    {
        m_feedback = std::clamp(fb, 0.0f, 0.95f);
    }

    void setDryWet(float dw) noexcept
    {
        m_dryWet = std::clamp(dw, 0.0f, 1.0f);
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    void setBypass(bool bypassed) noexcept { m_bypassed = bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

    [[nodiscard]] float getDelayTime() const noexcept
    {
        return m_targetDelayLineSamples / LINE_SAMPLE_RATE;
    }

    [[nodiscard]] float getFeedback() const noexcept { return m_feedback; }
    [[nodiscard]] float getDryWet() const noexcept { return m_dryWet; }
    [[nodiscard]] float getMaxDelayTime() const noexcept { return MAX_DELAY_SECONDS; }
};

#endif // EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H