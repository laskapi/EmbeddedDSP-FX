#ifndef EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H
#define EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H

#include <stdint.h>

/// @brief Max parameters for effect (Settings blob capacity per slot).
inline constexpr uint8_t MAX_EFFECT_PARAMS = 8;

/// @brief Static metadata for one effect parameter (name + range + default).
struct EffectParam {
    const char* name;
    float min;
    float max;
    float defaultValue;
};

/// @brief Effect contract: Id/Name/Params + ctor(SR)/updateSampleRate/process/paramValue/bypass.
/// ParamCount must be <= MAX_EFFECT_PARAMS.

#endif // EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H
