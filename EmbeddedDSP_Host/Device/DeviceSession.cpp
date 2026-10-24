#include "DeviceSession.h"
#include "IDevice.h"
#include "IDeviceFactory.h"

namespace Host::Device {

DeviceSession::DeviceSession(std::unique_ptr<IDeviceFactory> factory, QObject* parent)
    : QObject(parent)
    , m_factory(std::move(factory))
{
}

DeviceSession::~DeviceSession() {
    clearCurrentDevice();
}

void DeviceSession::open(const QString& connectionId) {
    clearCurrentDevice();
    m_device = m_factory->create(connectionId);
    wireDevice(m_device.get());
    m_device->open(connectionId);
}

void DeviceSession::close() {
    if (!m_device) return;

    m_device->close();
}

bool DeviceSession::sendControlPacket(const Protocol::ControlPacket& pkt) {
    if (!m_device) return false;

    return m_device->sendControlPacket(pkt);
}

void DeviceSession::clearCurrentDevice() {
    if (!m_device) return;

    m_device->close();
    m_device->disconnect();
    m_device.reset();
}

void DeviceSession::wireDevice(IDevice* device) {
    connect(device, &IDevice::manifestReady, this, &DeviceSession::manifestReady);
    connect(device, &IDevice::audioFrameReady, this, &DeviceSession::audioFrameReady);
    connect(device, &IDevice::controlPacketReady, this, &DeviceSession::controlPacketReady);
    connect(device, &IDevice::connectionStatusChanged, this, &DeviceSession::connectionStatusChanged);
    connect(device, &IDevice::errorOccurred, this, &DeviceSession::errorOccurred);
}

} // namespace Host::Device
