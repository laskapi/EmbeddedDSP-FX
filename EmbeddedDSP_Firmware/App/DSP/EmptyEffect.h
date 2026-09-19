#ifndef EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H

#include "AudioEffectConcept.h"

/**
 * @brief Null object effect that passes audio through untouched.
 */
class EmptyEffect {
public:
    static constexpr uint8_t Id = 0x00;
    static constexpr const char* Name = "None";
    static constexpr uint8_t ParamCount = 0;
    static constexpr EffectParam Params[1] = {}; // Empty array workaround

    void prepare(float) noexcept {}
    void process(float&, float&) noexcept {}
    void setParamValue(uint8_t, float) noexcept {}
    [[nodiscard]] float getParamValue(uint8_t) const noexcept { return 0.0f; }
    void toggleBypass() noexcept {}
    [[nodiscard]] bool isBypassed() const noexcept { return false; }
};

#endif // EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H
