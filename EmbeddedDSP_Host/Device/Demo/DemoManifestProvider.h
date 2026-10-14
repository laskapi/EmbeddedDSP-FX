#ifndef EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
#define EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H

#include "DeviceModel.h"

namespace Host::Device::Demo {

/// @brief Helper providing a pre-filled DeviceManifest for offline testing.
Models::DeviceManifest getDemoManifest();

} // namespace Host::Device::Demo

#endif // EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
