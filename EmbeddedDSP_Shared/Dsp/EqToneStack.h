#ifndef EMBEDDEDDSP_SHARED_EQ_TONE_STACK_H
#define EMBEDDEDDSP_SHARED_EQ_TONE_STACK_H

#include "Dsp/BiquadCoeffs.h"
#include <cmath>

namespace Shared::Dsp {

/// @brief Fixed shelf corners for the 3-band tone stack (matches firmware EqEffect).
inline constexpr float kEqBassHz = 250.0f;
inline constexpr float kEqTrebleHz = 4000.0f;

/// @brief Treat near-zero gain as bypassed section (same threshold as firmware).
inline constexpr float kEqGainBypassDb = 0.01f;

/// @brief Cascade |H| of low-shelf + peaking mid + high-shelf, in dB at @p freqHz.
[[nodiscard]] inline float eqCascadeMagnitudeDb(float sampleRate, float bassDb, float midDb,
                                                float trebleDb, float midHz, float midQ,
                                                float freqHz) noexcept {
    float totalDb = 0.0f;

    if (std::fabs(bassDb) >= kEqGainBypassDb) {
        totalDb += BiquadCoeffs::lowShelf(sampleRate, kEqBassHz, bassDb).magnitudeDb(freqHz, sampleRate);
    }
    if (std::fabs(midDb) >= kEqGainBypassDb) {
        totalDb +=
            BiquadCoeffs::peaking(sampleRate, midHz, midDb, midQ).magnitudeDb(freqHz, sampleRate);
    }
    if (std::fabs(trebleDb) >= kEqGainBypassDb) {
        totalDb +=
            BiquadCoeffs::highShelf(sampleRate, kEqTrebleHz, trebleDb).magnitudeDb(freqHz, sampleRate);
    }
    return totalDb;
}

} // namespace Shared::Dsp

#endif // EMBEDDEDDSP_SHARED_EQ_TONE_STACK_H
