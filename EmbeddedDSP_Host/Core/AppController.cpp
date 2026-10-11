#include "AppController.h"
#include "AudioFrameWorker.h"
#include "IDeviceBackend.h"

#include <Protocol/ControlPacket.h>
#include <QThread>
#include <QTimer>
#include <QtGlobal>

using namespace Host::Processing;
using namespace Host::Backend;
using namespace Host::Models;

namespace {

[[nodiscard]] uint8_t toProtocolByte(int value) {
    Q_ASSERT(value >= 0 && value <= 255);
    return static_cast<uint8_t>(value);
}

} // namespace

namespace Host::Core {

AppController::AppController(std::unique_ptr<IDeviceBackend> backend,
                           std::unique_ptr<AudioFrameWorker> audioWorker,
                           QObject* parent)
    : QObject(parent)
{
    m_workerThread = new QThread(this);

    m_backend = backend.release();
    m_audioWorker = audioWorker.release();

    m_backend->moveToThread(m_workerThread);
    m_audioWorker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_backend, &QObject::deleteLater);
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
}

void AppController::connectToDevice(const QString& target) {
    if (!m_backend) return;

    if (m_isDeviceConnected) {
        disconnectFromDevice();
    }

    QMetaObject::invokeMethod(m_backend,&Host::Backend::IDeviceBackend::connectToDevice,
                              Qt::QueuedConnection,target);
}

void AppController::disconnectFromDevice() {
    if (!m_backend) return;

    QMetaObject::invokeMethod(m_backend,&Host::Backend::IDeviceBackend::disconnectFromDevice,
                              Qt::QueuedConnection);
}

void AppController::setupBackend() {  
    connect(m_backend, &IDeviceBackend::manifestReady, this, &AppController::manifestReady);
    connect(m_backend, &IDeviceBackend::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    connect(m_backend, &IDeviceBackend::controlPacketReady, this, &AppController::onControlPacketReady);
    connect(m_backend, &IDeviceBackend::connectionStatusChanged, this, &AppController::onConnectionStatusChanged);
    connect(m_backend, &IDeviceBackend::errorOccurred, this, &AppController::errorOccurred);
    connect(m_backend, &IDeviceBackend::connectionsReady,this, &AppController::connectionsUpdated);
}

void AppController::refreshAvailableConnections() {
    if (!m_backend || m_isDeviceConnected) return;

    QMetaObject::invokeMethod(m_backend,&IDeviceBackend::requestAvailableConnections,Qt::QueuedConnection);
}

void AppController::dispatchControlPacket(::Protocol::ControlPacket pkt) {
    if (!m_backend) return;

    pkt.applyCRC();
    QMetaObject::invokeMethod(m_backend,&Host::Backend::IDeviceBackend::sendControlPacket
                              ,Qt::QueuedConnection,pkt);
}

void AppController::requestSync() {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::GetState;
    dispatchControlPacket(pkt);
}

void AppController::setParameter(int slotIndex, int paramIndex, float value) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetParam;
    pkt.slotId = toProtocolByte(slotIndex);
    pkt.paramId = toProtocolByte(paramIndex);
    pkt.setValue(value);
    dispatchControlPacket(pkt);
}

void AppController::setEffectType(int slotIndex, int effectTypeId) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::SetEffectType;
    pkt.slotId = toProtocolByte(slotIndex);
    pkt.effectTypeId = toProtocolByte(effectTypeId);
    pkt.setValue(0.0f);
    dispatchControlPacket(pkt);
}

void AppController::setBypass(int slotIndex, bool bypass) {
    ::Protocol::ControlPacket pkt;
    pkt.command = ::Protocol::Command::BypassToggle;
    pkt.slotId = toProtocolByte(slotIndex);
    pkt.setValue(bypass ? 1.0f : 0.0f);
    dispatchControlPacket(pkt);
}

void AppController::onControlPacketReady(const Protocol::ControlPacket& pkt) {
    using ::Protocol::Command;

    switch (pkt.command) {
    case Command::SetParam:
        emit slotParameterUpdated(static_cast<int>(pkt.slotId),
                                  static_cast<int>(pkt.paramId),
                                  pkt.getValue());
        break;
    case Command::SetEffectType:
        emit slotEffectTypeUpdated(static_cast<int>(pkt.slotId),
                                   static_cast<int>(pkt.effectTypeId));
        break;
    case Command::BypassToggle:
        emit slotBypassUpdated(static_cast<int>(pkt.slotId), pkt.getValue() > 0.5f);
        break;
    default:
        break;
    }
}

void AppController::onConnectionStatusChanged(bool connected, const QString& portName) {
    m_isDeviceConnected = connected;
    emit connectionChanged(connected, portName);
    if (!connected) {
        refreshAvailableConnections();
    }
}

} // namespace Host::Core
