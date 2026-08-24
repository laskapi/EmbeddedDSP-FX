#ifndef EMBEDDEDDSP_FIRMWARE_EFFECT_PARAMS_H
#define EMBEDDEDDSP_FIRMWARE_EFFECT_PARAMS_H

#include <array>
#include <cstdint>
#include <cstddef>

namespace EffectParams
{
    inline constexpr size_t kMaxSlots = 4;

    enum class EffectType : uint8_t {
        Empty = 0,
        Delay = 1,
        Overdrive = 2
    };

    struct ParamDesc {
        uint8_t paramId;
        const char* name;
        float minValue;
        float maxValue;
        float defaultValue;
        const char* unit;
    };

    inline constexpr float kDelayMaxSeconds = (16000.0f - 1.0f) / 16000.0f;

    inline constexpr std::array<ParamDesc, 4> kOverdriveParams{{
        {0, "Drive", 1.0f, 20.0f, 5.0f, ""},
        {1, "Tone", 400.0f, 16000.0f, 3000.0f, "Hz"},
        {2, "Wet", 0.0f, 1.0f, 1.0f, ""},
        {3, "Level", 0.0f, 2.0f, 1.0f, ""}
    }};

    inline constexpr std::array<ParamDesc, 3> kDelayParams{{
        {0, "Time", 0.001f, kDelayMaxSeconds, 0.35f, "s"},
        {1, "Feedback", 0.0f, 0.95f, 0.4f, ""},
        {2, "Dry/Wet", 0.0f, 1.0f, 0.5f, ""}
    }};

    struct EffectMetadata {
        EffectType type;
        const char* name;
        const ParamDesc* params;
        uint8_t paramCount;
    };

    inline constexpr std::array<EffectMetadata, 3> kMetadata{{
        { EffectType::Empty,     "Empty",     nullptr,               0 },
        { EffectType::Delay,     "Delay",     kDelayParams.data(),     (uint8_t)kDelayParams.size() },
        { EffectType::Overdrive, "Overdrive", kOverdriveParams.data(), (uint8_t)kOverdriveParams.size() }
    }};
}

#endif // EMBEDDEDDSP_FIRMWARE_EFFECT_PARAMS_H