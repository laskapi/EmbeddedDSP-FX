#ifndef EMBEDDEDDSP_EFFECT_PARAMS_H
#define EMBEDDEDDSP_EFFECT_PARAMS_H

#include <array>
#include <cstdint>

namespace EffectParams
{
    enum class EffectType : uint8_t
    {
        Empty = 0,
        Delay = 1,
        Overdrive = 2
    };

    enum class OverdriveParam : uint8_t
    {
        Drive = 0,
        Tone = 1,
        Wet = 2,
        Level = 3
    };

    enum class DelayParam : uint8_t
    {
        Time = 0,
        Feedback = 1,
        DryWet = 2
    };

    struct ParamDesc
    {
        uint8_t paramId;
        const char* name;
        float minValue;
        float maxValue;
        float defaultValue;
        const char* unit;
    };

    // Sync with DelayEffect::MAX_DELAY_SECONDS
    inline constexpr float kDelayMaxSeconds = (16000.0f - 1.0f) / 16000.0f;

    inline constexpr std::array<ParamDesc, 4> kOverdriveParams{
        {
            {0, "Drive", 1.0f, 20.0f, 5.0f, ""},
            {1, "Tone", 400.0f, 16000.0f, 3000.0f, "Hz"},
            {2, "Wet", 0.0f, 1.0f, 1.0f, ""},
            {3, "Level", 0.0f, 2.0f, 1.0f, ""},
        }
    };

    inline constexpr std::array<ParamDesc, 3> kDelayParams{
        {
            {0, "Time", 0.001f, kDelayMaxSeconds, 0.35f, "s"},
            {1, "Feedback", 0.0f, 0.95f, 0.4f, ""},
            {2, "Dry/Wet", 0.0f, 1.0f, 0.5f, ""},
        }
    };
} // namespace EffectParams

#endif // EMBEDDEDDSP_EFFECT_PARAMS_H
