#ifndef EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
#define EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H

#include "EffectSpec.h"

namespace Host {

/// @brief Helper providing a pre-filled DeviceManifest for offline testing.
class DemoManifestProvider {
public:
    static DeviceManifest get();
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
