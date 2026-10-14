#include "DeviceSession.h"
#include "IDevice.h"
#include <QThread>

namespace Host::Device {

DeviceSession::DeviceSession(QThread* workerThread, QObject* parent)
    : QObject(parent)
    , m_workerThread(workerThread)
{
}

DeviceSession::~DeviceSession() {
    clearCurrentDevice();
}

void DeviceSession::clearCurrentDevice() {
    if (!m_device) {
        return;
    }

    if (m_workerThread && m_workerThread->isRunning() && m_device->thread() == m_workerThread) {
        QMetaObject::invokeMethod(m_device, &IDevice::disconnectFromDevice, Qt::BlockingQueuedConnection);
    } else {
        m_device->disconnectFromDevice();
    }

    disconnect(m_workerThread, &QThread::finished, m_device, &QObject::deleteLater);
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
    m_device->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::finished, m_device, &QObject::deleteLater);
    emit deviceReady(m_device);
}

} // namespace Host::Device
