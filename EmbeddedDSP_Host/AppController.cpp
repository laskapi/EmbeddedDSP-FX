#include "AppController.h"
#include "AudioFrameWorker.h"
#include "SerialBackend.h"
#ifdef HOST_SIMULATOR_ENABLED
#include "SimulatorBackend.h"
#endif
#include "IDeviceBackend.h"

#include <Protocol/AudioFramePacket.h>
#include <Protocol/ControlPacket.h>
#include <QThread>
#include <QDebug>
#include <QSerialPortInfo>

namespace Host {

AppController::AppController() : QObject(nullptr) {
    m_workerThread = new QThread(this);
    m_audioWorker = new AudioFrameWorker();
    m_audioWorker->moveToThread(m_workerThread);
    
    connect(m_workerThread, &QThread::finished, m_audioWorker, &QObject::deleteLater);
    connect(m_audioWorker, &AudioFrameWorker::spectrumReady, this, &AppController::spectrumReady);

#ifdef HOST_SIMULATOR_ENABLED
    setupBackend(new SimulatorBackend());
#else
    setupBackend(new SerialBackend());
#endif

    m_workerThread->start();
}

AppController::~AppController() {
    m_workerThread->quit();
    m_workerThread->wait();
    if (m_backend) {
        m_backend->deleteLater();
    }
}

std::vector<ConnectionInfo> AppController::availableConnections() const {
    return m_backend->enumerateConnections();
}

void AppController::connectToDevice(const QString& target) {
    // If already connected, disconnect first
    if (m_backend->isOpen()) {
        disconnect();
    }

    QMetaObject::invokeMethod(m_backend, [this, target]() {
        m_backend->connect(target);
    }, Qt::QueuedConnection);
}

void AppController::disconnect() {
    if (m_backend) {
        QMetaObject::invokeMethod(m_backend, &IDeviceBackend::disconnect, Qt::BlockingQueuedConnection);
    }
}

void AppController::setupBackend(IDeviceBackend* backend) {
    m_backend = backend;
    m_backend->moveToThread(m_workerThread);

    connect(m_backend, &IDeviceBackend::manifestReady, this, &AppController::manifestReady);
    connect(m_backend, &IDeviceBackend::audioFrameReady, this, &AppController::audioFrameReady);
    connect(m_backend, &IDeviceBackend::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    connect(m_backend, &IDeviceBackend::controlPacketReady, this, &AppController::deviceStateUpdated);
    connect(m_backend, &IDeviceBackend::connectionStatusChanged, this, &AppController::onConnectionStatusChanged);
    connect(m_backend, &IDeviceBackend::errorOccurred, this, &AppController::errorOccurred);
}

void AppController::requestSync() {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::GetState;
    dispatchControlPacket(pkt);
}

void AppController::dispatchControlPacket(::Protocol::ControlPacket pkt) {
    if (!m_backend) return;

    pkt.applyCRC();
    QMetaObject::invokeMethod(m_backend, [this, pkt]() {
        m_backend->sendControlPacket(pkt);
    }, Qt::QueuedConnection);
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
    emit connectionChanged(connected, portName);
}

} // namespace Host
