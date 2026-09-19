#ifndef EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H
#define EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H

#include <stdint.h>

/**
 * @brief Static metadata for an effect parameter.
 */
struct EffectParam {
    const char* name;
    float min;
    float max;
    float defaultValue;
};

/**
 * @brief Interface documentation for static polymorphism effects.
 * 
 * Each effect must implement:
 * - static constexpr uint8_t Id
 * - static constexpr const char* Name
 * - static constexpr uint8_t ParamCount
 * - static constexpr EffectParam Params[]
 * - void prepare(float sampleRate)
 * - void process(float& left, float& right)
 * - void setParamValue(uint8_t id, float val)
 * - float getParamValue(uint8_t id)
 * - void toggleBypass()
 * - bool isBypassed()
 */

#endif // EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H
