#ifndef EMBEDDEDDSP_HOST_DEVICE_FACTORY_H
#define EMBEDDEDDSP_HOST_DEVICE_FACTORY_H

#include "IDeviceFactory.h"

namespace Host::Device {

/// @brief Production factory: DEMO_MODE -> DemoDevice, otherwise SerialDevice.
class DeviceFactory : public IDeviceFactory {
public:
    [[nodiscard]] std::unique_ptr<IDevice> create(const QString& connectionId) override;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_DEVICE_FACTORY_H
