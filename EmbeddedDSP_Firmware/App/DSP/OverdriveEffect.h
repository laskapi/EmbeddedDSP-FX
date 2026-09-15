#ifndef EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_OVERDRIVE_EFFECT_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

class OverdriveEffect {
public:
    static constexpr uint8_t Id = 2;
    static constexpr const char* Name = "Overdrive";
    
    struct ParamInfo { const char* name; float min; float max; float defaultValue; };
    static constexpr std::array Params = {
        ParamInfo{"Drive", 1.0f, 50.0f, 10.0f},
        ParamInfo{"Tone",  400.0f, 18000.0f, 6000.0f},
        ParamInfo{"Mix",   0.0f, 1.0f, 1.0f},
        ParamInfo{"Level", 0.0f, 2.0f, 1.0f}
    };
    static constexpr uint8_t ParamCount = (uint8_t)Params.size();

private:
    float m_sampleRate{48000.0f};
    float m_drive{10.0f};
    float m_toneHz{6000.0f};
    float m_wet{1.0f};
    float m_level{1.0f};
    bool m_bypassed{false};
    float m_filterState{0.0f};
    float m_b0{1.0f}, m_a1{0.0f};

    [[nodiscard]] static float softClip(float x) noexcept {
        x += 0.05f; 
        if (x > 1.0f) return 1.0f;
        if (x < -1.0f) return -1.0f;
        return x * (1.5f - 0.5f * x * x);
    }

    void updateFilter() noexcept {
        if (m_sampleRate <= 0.0f) return;
        float w0 = 6.2831853f * m_toneHz / m_sampleRate;
        m_a1 = std::exp(-w0);
        m_b0 = 1.0f - m_a1;
    }

public:
    constexpr OverdriveEffect() noexcept = default;

    void prepare(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        updateFilter();
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) return;
        left = processSample(left);
        right = processSample(right);
    }

    [[nodiscard]] float processSample(float input) noexcept {
        float x = input * m_drive;
        x = softClip(x);
        m_filterState = (m_b0 * x) + (m_a1 * m_filterState);
        x = m_filterState;
        float output = (x * m_wet) + (input * (1.0f - m_wet));
        return std::clamp(output * m_level, -1.0f, 1.0f);
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id >= ParamCount) return;
        val = std::clamp(val, Params[id].min, Params[id].max);
        switch (id) {
            case 0: m_drive = val; break;
            case 1: m_toneHz = val; updateFilter(); break;
            case 2: m_wet = val; break;
            case 3: m_level = val; break;
        }
    }

    [[nodiscard]] float getParamValue(uint8_t id) const noexcept {
        switch (id) {
            case 0: return m_drive;
            case 1: return m_toneHz;
            case 2: return m_wet;
            case 3: return m_level;
            default: return 0.0f;
        }
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }
};

#endif
