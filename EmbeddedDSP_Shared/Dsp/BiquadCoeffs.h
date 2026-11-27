#ifndef EMBEDDEDDSP_SHARED_BIQUAD_COEFFS_H
#define EMBEDDEDDSP_SHARED_BIQUAD_COEFFS_H

#include "Dsp/DspMath.h"
#include <cmath>

namespace Shared::Dsp {

/// @brief Direct-form I biquad coefficients (a0 normalized to 1). Design + analysis only.
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
        const float A = dbToEqA(gainDb);
        const float w0 = angularFreq(freqHz, sampleRate);
        const float cosw = std::cos(w0);
        const float sinw = std::sin(w0);
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
        const float A = dbToEqA(gainDb);
        const float w0 = angularFreq(freqHz, sampleRate);
        const float cosw = std::cos(w0);
        const float sinw = std::sin(w0);
        const float alpha = sinw * 0.5f * std::sqrt(2.0f); // S=1

        const float twoSqrtAAlpha = 2.0f * std::sqrt(A) * alpha;
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
        const float A = dbToEqA(gainDb);
        const float w0 = angularFreq(freqHz, sampleRate);
        const float cosw = std::cos(w0);
        const float sinw = std::sin(w0);
        const float alpha = sinw * 0.5f * std::sqrt(2.0f);

        const float twoSqrtAAlpha = 2.0f * std::sqrt(A) * alpha;
        const float b0 = A * ((A + 1.0f) + (A - 1.0f) * cosw + twoSqrtAAlpha);
        const float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw);
        const float b2 = A * ((A + 1.0f) + (A - 1.0f) * cosw - twoSqrtAAlpha);
        const float a0 = (A + 1.0f) - (A - 1.0f) * cosw + twoSqrtAAlpha;
        const float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosw);
        const float a2 = (A + 1.0f) - (A - 1.0f) * cosw - twoSqrtAAlpha;
        return normalized(b0, b1, b2, a0, a1, a2);
    }

    /// @brief Complex frequency response magnitude in dB at @p freqHz (a0 = 1).
    [[nodiscard]] float magnitudeDb(float freqHz, float sampleRate) const noexcept {
        if (sampleRate <= 0.0f || freqHz < 0.0f) {
            return 0.0f;
        }
        const float w = angularFreq(freqHz, sampleRate);
        const float c1 = std::cos(w);
        const float s1 = std::sin(w);
        const float c2 = std::cos(2.0f * w);
        const float s2 = std::sin(2.0f * w);

        // H(e^jw) = (b0 + b1 z^-1 + b2 z^-2) / (1 + a1 z^-1 + a2 z^-2), z^-k = cos(kw)-j sin(kw)
        const float numRe = b0 + b1 * c1 + b2 * c2;
        const float numIm = -(b1 * s1 + b2 * s2);
        const float denRe = 1.0f + a1 * c1 + a2 * c2;
        const float denIm = -(a1 * s1 + a2 * s2);

        const float numMag2 = numRe * numRe + numIm * numIm;
        const float denMag2 = denRe * denRe + denIm * denIm;
        if (denMag2 <= 0.0f) {
            return 0.0f;
        }
        return amplitudeToDb(std::sqrt(numMag2 / denMag2));
    }

private:
    /// @brief Divide b/a coeffs by a0 so the section uses a0 = 1.
    [[nodiscard]] static BiquadCoeffs normalized(float b0, float b1, float b2, float a0, float a1,
                                                 float a2) noexcept {
        const float invA0 = 1.0f / a0;
        return {b0 * invA0, b1 * invA0, b2 * invA0, a1 * invA0, a2 * invA0};
    }
};

} // namespace Shared::Dsp

#endif // EMBEDDEDDSP_SHARED_BIQUAD_COEFFS_H
