#ifndef EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H
#define EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H

#include "DSP/FxLimits.h"

namespace App::Dsp {

/// @brief Name, range, default and unit for one knob (goes into the manifest).
struct EffectParam {
    const char* name;
    float min;
    float max;
    float defaultValue;
    const char* unit = "";
};

/// @brief Duck-typed effect checklist (no vtable): Id, Name, Params, process, bypass.

} // namespace App::Dsp

#endif // EMBEDDEDDSP_DYNAMIC_AUDIO_EFFECT_CONCEPT_H
