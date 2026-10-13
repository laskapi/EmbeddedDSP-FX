#ifndef EMBEDDEDDSP_HOST_DEVICE_MANIFEST_PARSER_H
#define EMBEDDEDDSP_HOST_DEVICE_MANIFEST_PARSER_H

#include "DeviceModel.h"

namespace Host::Models {

/// @brief Parses MCU text manifest (C:/E:/P: lines) into a DeviceManifest.
[[nodiscard]] DeviceManifest parseDeviceManifest(const QString& manifestText);

} // namespace Host::Models

#endif // EMBEDDEDDSP_HOST_DEVICE_MANIFEST_PARSER_H
