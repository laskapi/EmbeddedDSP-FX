#ifndef EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_DELAY_EFFECT_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include "SharedBuffer.h"

class DelayEffect {
public:
    static constexpr uint8_t Id = 1;
    static constexpr const char* Name = "Delay";
    
    struct ParamInfo { const char* name; float min; float max; float defaultValue; };
    static constexpr std::array Params = {
        ParamInfo{"Time",     0.001f, 1.0f,  0.35f},
        ParamInfo{"Feedback", 0.0f,   0.95f, 0.4f},
        ParamInfo{"Mix",      0.0f,   1.0f,  0.5f}
    };
    static constexpr uint8_t ParamCount = (uint8_t)Params.size();

    static constexpr std::size_t MAX_LINE_SAMPLES = SharedBuffer::Capacity / sizeof(int16_t);
    static constexpr float LINE_SAMPLE_RATE{16000.0f};
    static constexpr std::size_t DECIMATION{3};
    static constexpr float INT16_SCALE{32767.0f};

private:
    static int16_t* delayLine() noexcept {
        return reinterpret_cast<int16_t*>(SharedBuffer::s_buffer);
    }

    float m_sampleRate{48000.0f};
    float m_targetDelayLineSamples{LINE_SAMPLE_RATE * 0.35f};
    float m_currentDelayLineSamples{LINE_SAMPLE_RATE * 0.35f};
    static constexpr float SMOOTHING_FACTOR{0.001f};
    
    float m_feedback{0.4f};
    float m_wet{0.5f};
    bool m_bypassed{false};

    std::size_t m_writeIndex{0};
    std::size_t m_decimCounter{0};
    float m_antiAliasState{0.0f};
    float m_antiAliasCoeff{0.0f};

    [[nodiscard]] float readLineInterpolated(float readPosition) const noexcept {
        while (readPosition < 0.0f) readPosition += (float)MAX_LINE_SAMPLES;
        while (readPosition >= (float)MAX_LINE_SAMPLES) readPosition -= (float)MAX_LINE_SAMPLES;
        
        const auto indexA = (std::size_t)readPosition;
        const std::size_t indexB = (indexA + 1) % MAX_LINE_SAMPLES;
        const float frac = readPosition - (float)indexA;
        
        auto* line = delayLine();
        return ((float)line[indexA] / INT16_SCALE) + 
               frac * (((float)line[indexB] / INT16_SCALE) - ((float)line[indexA] / INT16_SCALE));
    }

public:
    constexpr DelayEffect() noexcept = default;

    void prepare(float newSampleRate) noexcept {
        m_sampleRate = (newSampleRate > 0.0f) ? newSampleRate : 48000.0f;
        m_antiAliasCoeff = 1.0f - std::exp(-6.2831853f * (0.4f * (LINE_SAMPLE_RATE * 0.5f)) / m_sampleRate);
        reset();
    }

    void reset() noexcept {
        std::fill(delayLine(), delayLine() + MAX_LINE_SAMPLES, (int16_t)0);
        m_writeIndex = 0;
        m_decimCounter = 0;
        m_antiAliasState = 0.0f;
        m_currentDelayLineSamples = m_targetDelayLineSamples;
    }

    void process(float& left, float& right) noexcept {
        if (m_bypassed) return;
        m_currentDelayLineSamples += (m_targetDelayLineSamples - m_currentDelayLineSamples) * SMOOTHING_FACTOR;
        const float inputMono = (left + right) * 0.5f;
        m_antiAliasState += m_antiAliasCoeff * (inputMono - m_antiAliasState);
        const float readPosition = (float)m_writeIndex - m_currentDelayLineSamples;
        const float delayedSample = readLineInterpolated(readPosition);

        if (++m_decimCounter >= DECIMATION) {
            m_decimCounter = 0;
            const float newDelayValue = m_antiAliasState + (delayedSample * m_feedback);
            delayLine()[m_writeIndex] = (std::int16_t)std::clamp(newDelayValue * INT16_SCALE, -INT16_SCALE, INT16_SCALE);
            m_writeIndex = (m_writeIndex + 1) % MAX_LINE_SAMPLES;
        }
        left = (left * (1.0f - m_wet)) + (delayedSample * m_wet);
        right = (right * (1.0f - m_wet)) + (delayedSample * m_wet);
    }

    void setParamValue(uint8_t id, float val) noexcept {
        if (id >= ParamCount) return;
        val = std::clamp(val, Params[id].min, Params[id].max);
        switch (id) {
            case 0: m_targetDelayLineSamples = val * LINE_SAMPLE_RATE; break;
            case 1: m_feedback = val; break;
            case 2: m_wet = val; break;
        }
    }

    [[nodiscard]] float getParamValue(uint8_t id) const noexcept {
        switch (id) {
            case 0: return m_targetDelayLineSamples / LINE_SAMPLE_RATE;
            case 1: return m_feedback;
            case 2: return m_wet;
            default: return 0.0f;
        }
    }

    void toggleBypass() noexcept { m_bypassed = !m_bypassed; }
    [[nodiscard]] bool isBypassed() const noexcept { return m_bypassed; }
};

#endif
