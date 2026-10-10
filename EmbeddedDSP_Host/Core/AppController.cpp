#include "AppController.h"
#include "AudioFrameWorker.h"
#include "IDeviceBackend.h"

#include <Protocol/ControlPacket.h>
#include <QThread>
#include <QTimer>

using namespace Host::Processing;
using namespace Host::Backend;
using namespace Host::Models;

namespace Host::Core {

AppController::AppController(std::unique_ptr<IDeviceBackend> backend, QObject* parent)
    : QObject(parent)
{
    m_workerThread = new QThread(this);
    m_audioWorker = new AudioFrameWorker();
    m_audioWorker->moveToThread(m_workerThread);
    m_backend = std::move(backend);
    m_backend->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_audioWorker, &QObject::deleteLater);
    connect(m_audioWorker, &AudioFrameWorker::spectrumReady, this, &AppController::spectrumReady);
   
    setupBackend();

    m_scanTimer = new QTimer(this);
    connect(m_scanTimer, &QTimer::timeout, this, &AppController::refreshAvailableConnections);
    m_scanTimer->start(2000);
    refreshAvailableConnections();

    m_workerThread->start();
}

AppController::~AppController() {
    m_workerThread->quit();
    m_workerThread->wait();
    if (m_backend) {
        m_backend->deleteLater();
        m_backend.release();
    }
}

void AppController::connectToDevice(const QString& target) {
    if (!m_backend) {
        return;
    }

    if (m_isDeviceConnected) {
        disconnectFromDevice();
    }

    QMetaObject::invokeMethod(m_backend.get(), [this, target]() {
        m_backend->connectToDevice(target);
    }, Qt::QueuedConnection);
}

void AppController::disconnectFromDevice() {
    if (!m_backend) {
        return;
    }

    QMetaObject::invokeMethod(m_backend.get(), [this]() {
        m_backend->disconnectFromDevice();
    }, Qt::QueuedConnection);
}

void AppController::setupBackend() {  
    connect(m_backend.get(), &IDeviceBackend::manifestReady, this, &AppController::manifestReady);
    connect(m_backend.get(), &IDeviceBackend::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    connect(m_backend.get(), &IDeviceBackend::controlPacketReady, this, &AppController::deviceStateUpdated);
    connect(m_backend.get(), &IDeviceBackend::connectionStatusChanged, this, &AppController::onConnectionStatusChanged);
    connect(m_backend.get(), &IDeviceBackend::errorOccurred, this, &AppController::errorOccurred);
}

void AppController::refreshAvailableConnections() {
    if (!m_backend || m_isDeviceConnected) {
        return;
    }

    QMetaObject::invokeMethod(m_backend.get(), [this]() {
        const std::vector<ConnectionInfo> list = m_backend->availableConnections();
        QMetaObject::invokeMethod(this, [this, list]() {
            emit connectionsUpdated(list);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
}

void AppController::dispatchControlPacket(::Protocol::ControlPacket pkt) {
    if (!m_backend) {
        return;
    }

    pkt.applyCRC();
    QMetaObject::invokeMethod(m_backend.get(), [this, pkt]() {
        m_backend->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
}

void AppController::requestSync() {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::GetState;
    dispatchControlPacket(pkt);
}

void AppController::setParameter(uint8_t slotId, uint8_t paramId, float value) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetParam;
    pkt.slotId = slotId;
    pkt.paramId = paramId;
    pkt.setValue(value);
    dispatchControlPacket(pkt);
}

void AppController::setEffectType(uint8_t slotId, uint8_t effectTypeId) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetEffectType;
    pkt.slotId = slotId;
    pkt.effectTypeId = effectTypeId;
    pkt.setValue(0.0f);
    dispatchControlPacket(pkt);
}

void AppController::setBypass(uint8_t slotId, bool bypass) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::BypassToggle;
    pkt.slotId = slotId;
    pkt.setValue(bypass ? 1.0f : 0.0f);
    dispatchControlPacket(pkt);
}

void AppController::onConnectionStatusChanged(bool connected, const QString& portName) {
    m_isDeviceConnected = connected;
    emit connectionChanged(connected, portName);
    if (!connected) {
        refreshAvailableConnections();
    }
}

} // namespace Host::Core
