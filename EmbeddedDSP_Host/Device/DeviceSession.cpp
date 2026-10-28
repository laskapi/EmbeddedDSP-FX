#include "Device/DeviceSession.h"
#include "Device/IDevice.h"
#include "Device/IDeviceFactory.h"

namespace Host::Device {

namespace {

[[nodiscard]] uint16_t paramKey(uint8_t slotId, uint8_t paramId) {
    return static_cast<uint16_t>((static_cast<uint16_t>(slotId) << 8) | paramId);
}

} // namespace

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
    clearPendingTx();
    if (!m_device) return;

    m_device->close();
}

bool DeviceSession::sendControlPacket(const Protocol::ControlPacket& pkt) {
    if (!m_device) return false;

    if (pkt.command == Protocol::Command::SetParam) {
        m_pendingSetParams.insert(paramKey(pkt.slotId, pkt.paramId), pkt);
    } else {
        m_otherCommands.enqueue(pkt);
    }

    trySend();
    return true;
}

void DeviceSession::trySend() {
    if (m_txBusy || !m_device) {
        return;
    }

    Protocol::ControlPacket pkt;
    if (!m_otherCommands.isEmpty()) {
        pkt = m_otherCommands.dequeue();
    } else if (!m_pendingSetParams.isEmpty()) {
        const uint16_t key = m_pendingSetParams.constBegin().key();
        pkt = m_pendingSetParams.take(key);
    } else {
        return;
    }

    m_txBusy = true;
    if (!m_device->sendControlPacket(pkt)) {
        m_txBusy = false;
        return;
    }

    QMetaObject::invokeMethod(this, [this] {
        m_txBusy = false;
        trySend();
    }, Qt::QueuedConnection);
}

void DeviceSession::clearPendingTx() {
    m_pendingSetParams.clear();
    m_otherCommands.clear();
    m_txBusy = false;
}

void DeviceSession::clearCurrentDevice() {
    clearPendingTx();
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
