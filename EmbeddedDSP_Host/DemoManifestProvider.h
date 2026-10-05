#ifndef EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
#define EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H

#include "EffectSpec.h"

namespace Host {

/**
 * @brief Helper class to provide a rich DeviceManifest for testing without hardware.
 */
class DemoManifestProvider {
public:
    /// @return A pre-filled DeviceManifest with common guitar effects.
    static DeviceManifest get();
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_DEMOMANIFESTPROVIDER_H
