#ifndef EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H

#include "DSP/AudioEffectConcept.h"

/// @brief Pass-through null effect (empty slot placeholder).
class EmptyEffect {
public:
    static constexpr uint8_t Id = 0x00;
    static constexpr const char* Name = "None";
    static constexpr uint8_t ParamCount = 0;
    static constexpr EffectParam Params[1] = {}; // empty-array workaround for ParamCount 0
    static_assert(ParamCount <= MAX_EFFECT_PARAMS);

    EmptyEffect() noexcept = default;

    /// @brief Same shape as other effects; sample rate is unused (pass-through placeholder).
    explicit EmptyEffect(float sampleRate) noexcept {
        updateSampleRate(sampleRate);
    }

    void updateSampleRate(float) noexcept {}

    void process(float&, float&) noexcept {}
    void setParamValue(uint8_t, float) noexcept {}
    [[nodiscard]] float paramValue(uint8_t) const noexcept { return 0.0f; }
    void toggleBypass() noexcept {}
    [[nodiscard]] bool isBypassed() const noexcept { return false; }
};

#endif // EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H
