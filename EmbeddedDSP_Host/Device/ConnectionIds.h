#ifndef EMBEDDEDDSP_HOST_CONNECTION_IDS_H
#define EMBEDDEDDSP_HOST_CONNECTION_IDS_H

#include <QString>

namespace Host::Device {

/// @brief Stable id for the built-in simulator entry in the connection list.
inline const QString kDemoConnectionId = QStringLiteral("DEMO_MODE");
inline const QString kDemoConnectionDescription = QStringLiteral("Built-in Simulator");

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_CONNECTION_IDS_H
