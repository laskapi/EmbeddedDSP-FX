#ifndef EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H
#define EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H

#include "DSP/AudioEffectConcept.h"
#include "Protocol/ProtocolCommon.h"

namespace App::Dsp {

/// @brief Empty slot: does nothing to the audio.
class EmptyEffect {
public:
    static constexpr uint8_t Id = Protocol::EffectId::Empty;
    static constexpr const char* Name = "None";
    static constexpr uint8_t ParamCount = 0;
    static constexpr EffectParam Params[1] = {}; // need a non-empty array when ParamCount is 0
    static_assert(ParamCount <= kMaxEffectParams);

    EmptyEffect() noexcept = default;

    explicit EmptyEffect(float sampleRate) noexcept {
        updateSampleRate(sampleRate);
    }

    void updateSampleRate(float) noexcept {}

    void process(float&, float&) noexcept {}
    void setParamValue(uint8_t, float) noexcept {}
    [[nodiscard]] float paramValue(uint8_t) const noexcept { return 0.0f; }
    void toggleBypass() noexcept {}
    void setBypassed(bool) noexcept {}
    [[nodiscard]] bool isBypassed() const noexcept { return false; }
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_EMPTY_EFFECT_H
