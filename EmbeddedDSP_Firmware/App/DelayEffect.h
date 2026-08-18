#ifndef EMBEDDEDDSP_FIRMWARE_DELAYEFFECT_H
#define EMBEDDEDDSP_FIRMWARE_DELAYEFFECT_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

/**
 * @brief Memory-optimized stereo delay effect using int16_t mono ring buffer.
 *
 * Fits within tight micro-controller SRAM limits by downmixing signal to 16-bit mono
 * and using linear fractional interpolation for delay time parameter changes.
 */
class DelayEffect {
public:
    // 24000 samples * 2 bytes (int16_t) = 48 KB SRAM (500 ms delay line at 48 kHz)
    static constexpr std::size_t MAX_DELAY_SAMPLES{24000};
    static constexpr float INT16_SCALE{32767.0f};

    DelayEffect() = default;

    /**
     * @brief Inicjalizuje efekt pod konkretną częstotliwość próbkowania i czyści bufor.
     * @param newSampleRate Częstotliwość próbkowania systemu (np. 48000.0f)
     */
    void prepare(float newSampleRate) noexcept {
        sampleRate = newSampleRate;
        reset();
    }

    /**
     * @brief Czyści bufor opóźnienia i resetuje wskaźniki (przydatne np. przy przełączaniu presetów)
     */
    void reset() noexcept {
        delayBufferMono.fill(0);
        writeIndex = 0;
        currentDelaySamples = targetDelaySamples;
    }

    /**
     * @brief Processes one stereo sample pair in-place.
     * @param left Reference to left channel float audio sample [-1.0f, 1.0f].
     * @param right Reference to right channel float audio sample [-1.0f, 1.0f].
     */
    void process(float& left, float& right) noexcept {
        if (bypassed) {
            return;
        }

        // Low-pass parameter smoothing to prevent zipper noise on delay time changes
        currentDelaySamples += (targetDelaySamples - currentDelaySamples) * SMOOTHING_FACTOR;

        float readPosition = static_cast<float>(writeIndex) - currentDelaySamples;
        if (readPosition < 0.0f) {
            readPosition += static_cast<float>(MAX_DELAY_SAMPLES);
        }

        std::size_t indexA = static_cast<std::size_t>(readPosition);
        std::size_t indexB = (indexA + 1) % MAX_DELAY_SAMPLES;
        float frac = readPosition - static_cast<float>(indexA);

        float sampleA = static_cast<float>(delayBufferMono[indexA]) / INT16_SCALE;
        float sampleB = static_cast<float>(delayBufferMono[indexB]) / INT16_SCALE;

        // Linear interpolation
        float delayedSample = sampleA + frac * (sampleB - sampleA);

        // Mono downmix for feedback loop storage
        float inputMono = (left + right) * 0.5f;
        float newDelayValue = inputMono + (delayedSample * feedback);

        float clampedValue = std::clamp(newDelayValue * INT16_SCALE, -INT16_SCALE, INT16_SCALE);
        delayBufferMono[writeIndex] = static_cast<std::int16_t>(clampedValue);

        // Dry/Wet mixing back to stereo output channels
        left = (left * (1.0f - dryWet)) + (delayedSample * dryWet);
        right = (right * (1.0f - dryWet)) + (delayedSample * dryWet);

        writeIndex = (writeIndex + 1) % MAX_DELAY_SAMPLES;
    }

    /**
     * @brief Sets the delay time in seconds.
     * @param seconds Target delay duration (clamped according to MAX_DELAY_SAMPLES and sampleRate).
     */
    void setDelayTime(float seconds) noexcept {
        float maxDelaySeconds = static_cast<float>(MAX_DELAY_SAMPLES - 1) / sampleRate;
        seconds = std::clamp(seconds, 0.001f, maxDelaySeconds);
        targetDelaySamples = seconds * sampleRate;
    }

    /**
     * @brief Sets the delay feedback coefficient.
     * @param fb Feedback factor (clamped to [0.0, 0.95] to prevent unstable oscillation).
     */
    void setFeedback(float fb) noexcept {
        feedback = std::clamp(fb, 0.0f, 0.95f);
    }

    /**
     * @brief Sets the Dry/Wet mix ratio.
     * @param dw Balance value (0.0 = completely dry, 1.0 = completely wet).
     */
    void setDryWet(float dw) noexcept {
        dryWet = std::clamp(dw, 0.0f, 1.0f);
    }

    void toggleBypass() noexcept {
        bypassed = !bypassed;
    }

    [[nodiscard]] bool isBypassed() const noexcept {
        return bypassed;
    }

private:
    std::array<std::int16_t, MAX_DELAY_SAMPLES> delayBufferMono{};
    std::size_t writeIndex{0};

    float sampleRate{48000.0f}; // Elastyczna częstotliwość ustawiana przez prepare()
    float targetDelaySamples{12000.0f}; // Default ~250 ms
    float currentDelaySamples{12000.0f};
    static constexpr float SMOOTHING_FACTOR{0.001f};

    float feedback{0.4f};
    float dryWet{0.5f};
    bool bypassed{false};
};

#endif // EMBEDDEDDSP_FIRMWARE_DELAYEFFECT_H