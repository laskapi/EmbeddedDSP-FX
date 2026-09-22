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

    void prepare(float sampleRate) noexcept;
    void process(float& left, float& right) noexcept;

    void setParamValue(uint8_t id, float val) noexcept;
    [[nodiscard]] float getParamValue(uint8_t id) const noexcept;

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
