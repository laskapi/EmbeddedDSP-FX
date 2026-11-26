#ifndef EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H

#include "DSP/AudioEffectConcept.h"
#include "Protocol/ProtocolCommon.h"
#include "SharedBuffer.h"
#include <algorithm>
#include <cstdint>
#include <cstring>

namespace App::Dsp {

/// @brief Mono digital delay (time / feedback / mix) using int16 SharedBuffer line.
class DelayEffect {
public:
    static constexpr uint8_t Id = Protocol::EffectId::Delay;
    static constexpr const char* Name = "Delay";
    static constexpr uint8_t ParamCount = 3;
    static_assert(ParamCount <= kMaxEffectParams);

    /// @brief Packed delay-line sample type (2x length vs float in the same arena).
    using Sample = std::int16_t;

    /// @brief Usable delay-line length (leave 2 samples for linear interpolation).
    static constexpr int kMaxDelaySamples =
        static_cast<int>(App::SharedBuffer::kCapacity / sizeof(Sample)) - 2;
    static constexpr float kMaxTimeSec =
        static_cast<float>(kMaxDelaySamples) / Protocol::SAMPLE_RATE_HZ;

    static constexpr EffectParam Params[ParamCount] = {
        {"Time", 0.01f, kMaxTimeSec, 0.25f, "s"},
        {"Feedback", 0.0f, 0.95f, 0.3f},
        {"Mix", 0.0f, 1.0f, 0.4f}
    };

    explicit DelayEffect(float sampleRate) noexcept {
        updateSampleRate(sampleRate);
    }

    ~DelayEffect() {
        App::g_sharedBuffer.release(App::SharedBuffer::Owner::Delay);
    }

    DelayEffect(const DelayEffect&) = delete;
    DelayEffect& operator=(const DelayEffect&) = delete;
    DelayEffect(DelayEffect&&) noexcept = default;
    DelayEffect& operator=(DelayEffect&&) noexcept = default;

    void updateSampleRate(float sampleRate) noexcept {
        m_sampleRate = sampleRate;
        m_writePos = 0;
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) return;

        const auto buffer = App::g_sharedBuffer.acquire<Sample>(App::SharedBuffer::Owner::Delay);
        if (!buffer) {
            m_needsClear = true;
            return; // dry while Manifest owns the arena
        }

        const int capacity = static_cast<int>(buffer.count);

        if (m_needsClear) {
            std::memset(buffer.data, 0, buffer.count * sizeof(Sample));
            m_writePos = 0;
            m_needsClear = false;
        }

        const float delaySamples = m_time * m_sampleRate;
        float readPos = static_cast<float>(m_writePos) - delaySamples;
        if (readPos < 0.0f) {
            readPos += static_cast<float>(capacity);
        }

        const int i0 = static_cast<int>(readPos);
        const int i1 = (i0 + 1) % capacity;
        const float frac = readPos - static_cast<float>(i0);

        const float delayed =
            decode(buffer.data[i0]) * (1.0f - frac) + decode(buffer.data[i1]) * frac;
        const float input = 0.5f * (left + right);

        buffer.data[m_writePos] = encode(input + delayed * m_feedback);
        m_writePos = (m_writePos + 1) % capacity;

        const float out = (input * (1.0f - m_mix)) + (delayed * m_mix);
        left = out;
        right = out;
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id == 0) m_time = std::clamp(val, Params[0].min, Params[0].max);
        else if (id == 1) m_feedback = std::clamp(val, Params[1].min, Params[1].max);
        else if (id == 2) m_mix = std::clamp(val, Params[2].min, Params[2].max);
    }

    [[nodiscard]] float paramValue(uint8_t id) const noexcept {
        if (id == 0) return m_time;
        if (id == 1) return m_feedback;
        if (id == 2) return m_mix;
        return 0.0f;
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    void setBypassed(bool bypassed) noexcept { m_bypassed = bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }

private:
    static constexpr float kScale = 32767.0f;

    [[nodiscard]] static float decode(Sample s) noexcept {
        return static_cast<float>(s) / kScale;
    }

    [[nodiscard]] static Sample encode(float x) noexcept {
        const float clamped = std::clamp(x, -1.0f, 1.0f);
        return static_cast<Sample>(clamped * kScale);
    }

    int m_writePos{0};
    float m_sampleRate{};
    float m_time{0.25f};
    float m_feedback{0.3f};
    float m_mix{0.4f};
    bool m_bypassed{false};
    bool m_needsClear{true};
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H
