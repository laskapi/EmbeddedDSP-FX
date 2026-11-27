#ifndef EMBEDDEDDSP_HOST_EQ_CURVE_STATE_H
#define EMBEDDEDDSP_HOST_EQ_CURVE_STATE_H

#include <QColor>

namespace Host::Models {

/// @brief One visible EQ response curve for spectrum overlay (Host UI DTO).
struct EqCurveState {
    QColor color;
    float bassDb{0.0f};
    float midDb{0.0f};
    float trebleDb{0.0f};
    float midHz{1000.0f};
    float midQ{0.7f};
};

} // namespace Host::Models

#endif // EMBEDDEDDSP_HOST_EQ_CURVE_STATE_H
