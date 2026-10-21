#include "DeviceFactory.h"
#include "DemoConnectionConstants.h"
#include "DemoDevice.h"
#include "SerialDevice.h"

namespace Host::Device {

std::unique_ptr<IDevice> DeviceFactory::create(const QString& connectionId) {
    if (connectionId == kDemoConnectionId) {
        return std::make_unique<DemoDevice>();
    }
    return std::make_unique<SerialDevice>();
}

} // namespace Host::Device
