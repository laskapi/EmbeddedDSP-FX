#ifndef EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H

#include <algorithm>
#include <cmath>
#include <cstdint>

/**
 * @brief Overdrive effect featuring fast polynomial soft-clipping
 *        and a single-pole IIR Low-Pass filter for Tone control.
 *
 * Fully compliant with C++20 AudioEffect concept. Zero dynamic allocation.
 */
class OverdriveEffect {
private:
    // --- PRIVATE MEMBERS (DSP STATE & PARAMETERS) ---
    float m_sampleRate{48000.0f};
    float m_drive{5.0f};
    float m_toneCutoffHz{3000.0f};
    float m_wet{1.0f};
    float m_level{1.0f};
    bool m_bypassed{false};

    // IIR Tone Filter State
    float m_filterState{0.0f};
    float m_b0{1.0f};
    float m_a1{0.0f};

    /**
     * @brief Fast polynomial approximation of soft-clipping curve.
     * Computes f(x) = x - (x^3 / 6.75) for x in [-1.5, 1.5].
     * Translates directly to ARM Cortex-M4 FMA instructions.
     */
    [[nodiscard]] static constexpr float fastSaturate(float x) noexcept {
        if (x <= -1.5f) {
            return -1.0f;
        }
        if (x >= 1.5f) {
            return 1.0f;
        }
        // 1/6.75 approx 0.148148148f
        return x * (1.0f - (x * x) * 0.148148148f);
    }

    /**
     * @brief Recalculates single-pole IIR filter coefficients.
     */
    void updateToneCoefficients() noexcept {
        if (m_sampleRate <= 0.0f) return;

        const float w0 = 2.0f * 3.14159265358979323846f * m_toneCutoffHz / m_sampleRate;
        m_a1 = std::exp(-w0);
        m_b0 = 1.0f - m_a1;
    }

public:
    constexpr OverdriveEffect() noexcept = default;

    /**
     * @brief Prepares the effect processor with system sample rate.
     * @param sampleRate System sampling frequency in Hz (e.g., 48000.0f)
     */
    void prepare(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        updateToneCoefficients();
    }

    /**
     * @brief Stereo in-place processing interface for DynamicAudioPipeline.
     * @param left Reference to left channel sample [-1.0f, 1.0f]
     * @param right Reference to right channel sample [-1.0f, 1.0f]
     */
    void process(float& left, float& right) noexcept {
        left = processSample(left);
        right = processSample(right);
    }

    /**
     * @brief Processes a single audio sample in real-time.
     * @param input Input sample in range [-1.0f, 1.0f]
     * @return Processed audio sample
     */
    [[nodiscard]] float processSample(float input) noexcept {
        // 1. Apply Input Gain (Drive)
        const float drivenSample = input * m_drive;

        // 2. Polynomial Soft-Clipping (Saturator)
        const float saturatedSample = fastSaturate(drivenSample);

        // 3. Tone section: Single-pole IIR Low-Pass Filter (y[n] = b0*x[n] + a1*y[n-1])
        m_filterState = (m_b0 * saturatedSample) + (m_a1 * m_filterState);

        // 4. Dry/Wet Mix + Master Output Level
        const float mixed = (m_filterState * m_wet) + (input * (1.0f - m_wet));

        return std::clamp(mixed * m_level, -1.0f, 1.0f);
    }

    // --- BYPASS & CONTROL INTERFACE ---

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    void setBypass(bool bypassed) noexcept { m_bypassed = bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

    // --- PARAMETER SETTERS ---

    void setDrive(float drive) noexcept {
        m_drive = std::clamp(drive, 1.0f, 20.0f);
    }

    void setTone(float toneHz) noexcept {
        m_toneCutoffHz = std::clamp(toneHz, 400.0f, 16000.0f);
        updateToneCoefficients();
    }

    void setWet(float wet) noexcept {
        m_wet = std::clamp(wet, 0.0f, 1.0f);
    }

    void setLevel(float level) noexcept {
        m_level = std::clamp(level, 0.0f, 2.0f);
    }

    // --- GETTERS (FOR UNIT TESTING & GUI SYNC) ---

    [[nodiscard]] float getDrive() const noexcept { return m_drive; }
    [[nodiscard]] float getTone() const noexcept { return m_toneCutoffHz; }
    [[nodiscard]] float getWet() const noexcept { return m_wet; }
    [[nodiscard]] float getLevel() const noexcept { return m_level; }
};

#endif // EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H