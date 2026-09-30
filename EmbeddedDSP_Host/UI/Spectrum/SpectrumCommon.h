#ifndef EMBEDDEDDSP_HOST_SPECTRUM_COMMON_H
#define EMBEDDEDDSP_HOST_SPECTRUM_COMMON_H

#include <cmath>
#include <algorithm>

namespace Host::UI::Spectrum {

    struct TickLabel {
        float value;
        const char* label;
    };

    static constexpr TickLabel X_TICK_LABELS[] = {
        {20.0f, "20"}, {100.0f, "100"}, {1000.0f, "1k"}, {10000.0f, "10k"}, {20000.0f, "20k"}
    };

    /// @brief Margin to prevent labels from being clipped at the edges.
    constexpr int AXIS_SIDE_MARGIN = 12;
    constexpr int AXIS_TOP_BOTTOM_MARGIN = 10;

    /// @brief Frequency range for the X-axis.
    constexpr float MIN_FREQ = 20.0f;
    constexpr float MAX_FREQ = 20000.0f;
    
    inline const float LOG_MIN = std::log10(MIN_FREQ);
    inline const float LOG_MAX = std::log10(MAX_FREQ);

    /// @brief Maps frequency to a normalized 0..1 position on a logarithmic scale.
    inline float freqToPos(float freq) {
        float f = std::clamp(freq, MIN_FREQ, MAX_FREQ);
        return (std::log10(f) - LOG_MIN) / (LOG_MAX - LOG_MIN);
    }

    /// @brief Maps dB value to a normalized 0..1 position on a linear scale.
    inline float dbToPos(float db, float minDb, float maxDb) {
        float val = std::clamp(db, minDb, maxDb);
        return (val - minDb) / (maxDb - minDb);
    }

} // namespace Host::UI::Spectrum

#endif // EMBEDDEDDSP_HOST_SPECTRUM_COMMON_H
