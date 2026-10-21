#include "DeviceSession.h"
#include "IDevice.h"
#include "IDeviceFactory.h"
#include "WorkerThread.h"

namespace Host::Device {

DeviceSession::DeviceSession(Host::Core::WorkerThread& worker,
                             std::unique_ptr<IDeviceFactory> factory,
                             QObject* parent)
    : QObject(parent)
    , m_worker(&worker)
    , m_factory(std::move(factory))
{
}

DeviceSession::~DeviceSession() {
    clearCurrentDevice();
}

void DeviceSession::clearCurrentDevice() {
    if (!m_device) {
        return;
    }

    Q_ASSERT(m_worker && m_worker->isRunning());
    Q_ASSERT(m_worker->hasAffinity(m_device));

    QMetaObject::invokeMethod(m_device, &IDevice::close, Qt::BlockingQueuedConnection);

    m_worker->release(m_device);
    m_device->disconnect();
    m_device->deleteLater();
    m_device = nullptr;
}

void DeviceSession::replace(std::unique_ptr<IDevice> device) {
    clearCurrentDevice();

    if (!device) {
        return;
    }

    m_device = device.release();
    m_worker->adopt(m_device);
    emit deviceReady(m_device);
}

void DeviceSession::open(const QString& connectionId) {
    if (connectionId.isEmpty() || !m_factory) {
        return;
    }

    auto device = m_factory->create(connectionId);
    replace(std::move(device));

    if (!m_device) {
        return;
    }

    QMetaObject::invokeMethod(m_device, &IDevice::open, Qt::QueuedConnection, connectionId);
}

} // namespace Host::Device
