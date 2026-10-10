#ifndef EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
#define EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H

#include "EffectSpec.h"

namespace Host::Backend::Demo {

/// @brief Helper providing a pre-filled DeviceManifest for offline testing.
Models::DeviceManifest getDemoManifest();

} // namespace Host::Backend::Demo

#endif // EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
