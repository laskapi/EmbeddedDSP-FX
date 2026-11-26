#ifndef EMBEDDEDDSP_FIRMWARE_BIQUAD_H
#define EMBEDDEDDSP_FIRMWARE_BIQUAD_H

#include "DSP/DspMath.h"
#include <cmath>

namespace App::Dsp {

/// @brief Direct-form I biquad coefficients (a0 normalized to 1).
struct BiquadCoeffs {
    float b0{1.0f};
    float b1{0.0f};
    float b2{0.0f};
    float a1{0.0f};
    float a2{0.0f};

    /// @brief Unity (bypass) section.
    [[nodiscard]] static BiquadCoeffs identity() noexcept { return {}; }

    /// @brief True when @p freqHz is safely below Nyquist (~0.49 * fs).
    [[nodiscard]] static bool isUsableFreq(float sampleRate, float freqHz) noexcept {
        return sampleRate > 0.0f && freqHz > 0.0f && freqHz < sampleRate * 0.49f;
    }

    /// @brief RBJ peaking EQ at @p freqHz with @p gainDb and @p q.
    [[nodiscard]] static BiquadCoeffs peaking(float sampleRate, float freqHz, float gainDb,
                                              float q) noexcept {
        if (!isUsableFreq(sampleRate, freqHz) || q <= 0.0f) {
            return identity();
        }
        const float A = Math::dbToEqA(gainDb);
        const float w0 = Math::angularFreq(freqHz, sampleRate);
        const float cosw = std::cosf(w0);
        const float sinw = std::sinf(w0);
        const float alpha = sinw / (2.0f * q);

        const float b0 = 1.0f + alpha * A;
        const float b1 = -2.0f * cosw;
        const float b2 = 1.0f - alpha * A;
        const float a0 = 1.0f + alpha / A;
        const float a1 = -2.0f * cosw;
        const float a2 = 1.0f - alpha / A;
        return normalized(b0, b1, b2, a0, a1, a2);
    }

    /// @brief RBJ low-shelf (S = 1) at @p freqHz with @p gainDb.
    [[nodiscard]] static BiquadCoeffs lowShelf(float sampleRate, float freqHz,
                                               float gainDb) noexcept {
        if (!isUsableFreq(sampleRate, freqHz)) {
            return identity();
        }
        const float A = Math::dbToEqA(gainDb);
        const float w0 = Math::angularFreq(freqHz, sampleRate);
        const float cosw = std::cosf(w0);
        const float sinw = std::sinf(w0);
        const float alpha = sinw * 0.5f * std::sqrtf(2.0f); // S=1

        const float twoSqrtAAlpha = 2.0f * std::sqrtf(A) * alpha;
        const float b0 = A * ((A + 1.0f) - (A - 1.0f) * cosw + twoSqrtAAlpha);
        const float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw);
        const float b2 = A * ((A + 1.0f) - (A - 1.0f) * cosw - twoSqrtAAlpha);
        const float a0 = (A + 1.0f) + (A - 1.0f) * cosw + twoSqrtAAlpha;
        const float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosw);
        const float a2 = (A + 1.0f) + (A - 1.0f) * cosw - twoSqrtAAlpha;
        return normalized(b0, b1, b2, a0, a1, a2);
    }

    /// @brief RBJ high-shelf (S = 1) at @p freqHz with @p gainDb.
    [[nodiscard]] static BiquadCoeffs highShelf(float sampleRate, float freqHz,
                                                float gainDb) noexcept {
        if (!isUsableFreq(sampleRate, freqHz)) {
            return identity();
        }
        const float A = Math::dbToEqA(gainDb);
        const float w0 = Math::angularFreq(freqHz, sampleRate);
        const float cosw = std::cosf(w0);
        const float sinw = std::sinf(w0);
        const float alpha = sinw * 0.5f * std::sqrtf(2.0f);

        const float twoSqrtAAlpha = 2.0f * std::sqrtf(A) * alpha;
        const float b0 = A * ((A + 1.0f) + (A - 1.0f) * cosw + twoSqrtAAlpha);
        const float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw);
        const float b2 = A * ((A + 1.0f) + (A - 1.0f) * cosw - twoSqrtAAlpha);
        const float a0 = (A + 1.0f) - (A - 1.0f) * cosw + twoSqrtAAlpha;
        const float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosw);
        const float a2 = (A + 1.0f) - (A - 1.0f) * cosw - twoSqrtAAlpha;
        return normalized(b0, b1, b2, a0, a1, a2);
    }

private:
    /// @brief Divide b/a coeffs by a0 so the section uses a0 = 1.
    [[nodiscard]] static BiquadCoeffs normalized(float b0, float b1, float b2, float a0, float a1,
                                                 float a2) noexcept {
        const float invA0 = 1.0f / a0;
        return {b0 * invA0, b1 * invA0, b2 * invA0, a1 * invA0, a2 * invA0};
    }
};

/// @brief Per-channel Direct Form I state for one biquad section.
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
