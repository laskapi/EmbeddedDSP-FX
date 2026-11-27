#ifndef EMBEDDEDDSP_FIRMWARE_BIQUAD_H
#define EMBEDDEDDSP_FIRMWARE_BIQUAD_H

#include "Dsp/BiquadCoeffs.h"

namespace App::Dsp {

using Shared::Dsp::BiquadCoeffs;

/// @brief Per-channel Direct Form I state for one biquad section (MCU realtime path).
struct BiquadState {
    float x1{0.0f};
    float x2{0.0f};
    float y1{0.0f};
    float y2{0.0f};

    [[nodiscard]] float process(float x, const BiquadCoeffs& c) noexcept {
        const float y = c.b0 * x + c.b1 * x1 + c.b2 * x2 - c.a1 * y1 - c.a2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }

    void reset() noexcept {
        x1 = x2 = y1 = y2 = 0.0f;
    }
};

} // namespace App::Dsp

#endif // EMBEDDEDDSP_FIRMWARE_BIQUAD_H
