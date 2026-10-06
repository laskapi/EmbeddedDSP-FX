#ifndef EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H

#include "AudioEffectConcept.h"
#include "SharedBuffer.h"
#include <algorithm>
#include <Protocol/ProtocolCommon.h>

/// @brief Standard digital delay with feedback and mixing.
class DelayEffect {
public:
    static constexpr uint8_t Id = 0x01;
    static constexpr const char* Name = "Delay";
    static constexpr uint8_t ParamCount = 3;

    static constexpr EffectParam Params[ParamCount] = {
        {"Time", 0.01f, 1.0f, 0.5f},
        {"Feedback", 0.0f, 0.95f, 0.3f},
        {"Mix", 0.0f, 1.0f, 0.4f}
    };

    void prepare(float sampleRate) noexcept {
        m_sampleRate = (sampleRate > 0.0f) ? sampleRate : 48000.0f;
        m_writePos = 0;
        // SharedBuffer handles memory, no need to clear it here
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) return;

        float delaySamples = m_time * m_sampleRate;
        float readPos = static_cast<float>(m_writePos) - delaySamples;
        
        constexpr int capacity = SharedBuffer::Capacity / sizeof(float);
        while (readPos < 0.0f) readPos += static_cast<float>(capacity);

        int i0 = static_cast<int>(readPos);
        int i1 = (i0 + 1) % capacity;
        float frac = readPos - static_cast<float>(i0);
        
        float delayed = m_buffer[i0] * (1.0f - frac) + m_buffer[i1] * frac;

        // Feedback loop
        m_buffer[m_writePos] = left + (delayed * m_feedback);
        m_writePos = (m_writePos + 1) % capacity;

        // Dry/Wet mix
        left = (left * (1.0f - m_mix)) + (delayed * m_mix);
        right = left; // Mono delay
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) m_time = std::clamp(val, Params[0].min, Params[0].max);
        else if (id == 1) m_feedback = std::clamp(val, Params[1].min, Params[1].max);
        else if (id == 2) m_mix = std::clamp(val, Params[2].min, Params[2].max);
    }

    [[nodiscard]] float getParamValue(uint8_t id) const noexcept {
        if (id == 0) return m_time;
        if (id == 1) return m_feedback;
        if (id == 2) return m_mix;
        return 0.0f;
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    float* m_buffer = reinterpret_cast<float*>(SharedBuffer::s_buffer);
    int m_writePos{0};
    float m_sampleRate{48000.0f};
    float m_time{0.5f};
    float m_feedback{0.3f};
    float m_mix{0.4f};
    bool m_bypassed{false};
};

#endif // EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H
