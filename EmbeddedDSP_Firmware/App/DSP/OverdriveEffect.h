#ifndef EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H

#include <algorithm>
#include <cmath>
#include <cstdint>

/**
 * @brief Aggressive Overdrive with asymmetrical clipping and tone control.
 */
class OverdriveEffect {
private:
    float m_sampleRate{48000.0f};
    float m_drive{10.0f};      // Increased default drive
    float m_toneCutoffHz{6000.0f}; // Increased default cutoff for more "bite"
    float m_wet{1.0f};
    float m_level{1.0f};
    bool m_bypassed{false};

    float m_filterState{0.0f};
    float m_b0{1.0f};
    float m_a1{0.0f};

    /**
     * @brief Harder asymmetrical soft-clipping.
     * Combines exponential and cubic terms for a "nasty" analog feel.
     */
    [[nodiscard]] static float aggressiveSaturate(float x) noexcept {
        // Add a tiny DC offset for asymmetry (more even harmonics)
        x += 0.05f; 
        
        if (x > 1.0f) return 1.0f;
        if (x < -1.0f) return -1.0f;
        
        // Harder soft-clipping curve
        return x * (1.5f - 0.5f * x * x);
    }

    void updateToneCoefficients() noexcept {
        if (m_sampleRate <= 0.0f) return;
        const float w0 = 2.0f * 3.14159265f * m_toneCutoffHz / m_sampleRate;
        m_a1 = std::exp(-w0);
        m_b0 = 1.0f - m_a1;
    }

public:
    constexpr OverdriveEffect() noexcept = default;

    void prepare(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        updateToneCoefficients();
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) return; // Fixed bypass bug
        
        left = processSample(left);
        right = processSample(right);
    }

    [[nodiscard]] float processSample(float input) noexcept {
        // 1. Pre-gain (Drive)
        float x = input * m_drive;

        // 2. Aggressive Saturation
        x = aggressiveSaturate(x);

        // 3. Tone Filter (Low Pass)
        m_filterState = (m_b0 * x) + (m_a1 * m_filterState);
        x = m_filterState;

        // 4. Dry/Wet Mix
        float output = (x * m_wet) + (input * (1.0f - m_wet));

        return std::clamp(output * m_level, -1.0f, 1.0f);
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    void setBypass(bool bypassed) noexcept { m_bypassed = bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

    void setDrive(float drive) noexcept {
        m_drive = std::clamp(drive, 1.0f, 50.0f); // Increased range
    }

    void setTone(float toneHz) noexcept {
        m_toneCutoffHz = std::clamp(toneHz, 400.0f, 18000.0f);
        updateToneCoefficients();
    }

    void setWet(float wet) noexcept { m_wet = std::clamp(wet, 0.0f, 1.0f); }
    void setLevel(float level) noexcept { m_level = std::clamp(level, 0.0f, 2.0f); }
};

#endif