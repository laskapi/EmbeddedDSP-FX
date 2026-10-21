#ifndef EMBEDDEDDSP_HOST_DEMO_CONNECTION_CONSTANTS_H
#define EMBEDDEDDSP_HOST_DEMO_CONNECTION_CONSTANTS_H

#include <QString>

namespace Host::Device {

/// @brief Stable id for the built-in simulator entry in the connection list.
inline const QString kDemoConnectionId = QStringLiteral("DEMO_MODE");
/// @brief User-visible label for the simulator entry in the connection combo.
inline const QString kDemoConnectionDisplayLabel = QStringLiteral("Built-in Simulator");

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEMO_CONNECTION_CONSTANTS_H
