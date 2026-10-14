#ifndef EMBEDDEDDSP_HOST_IDEVICE_FACTORY_H
#define EMBEDDEDDSP_HOST_IDEVICE_FACTORY_H

#include <QString>
#include <memory>

namespace Host::Device {

class IDevice;

/// @brief Creates an IDevice implementation for a selected connection id.
class IDeviceFactory {
public:
    virtual ~IDeviceFactory() = default;
    [[nodiscard]] virtual std::unique_ptr<IDevice> create(const QString& connectionId) = 0;
};

} // namespace Host::Device

#endif // EMBEDDEDDSP_HOST_IDEVICE_FACTORY_H
