#include "Core/AppController.h"
#include "Processing/AudioFrameWorker.h"
#include "Device/DeviceSession.h"
#include "Device/IConnectionScanner.h"

#include "Protocol/ControlPacket.h"
#include <QtGlobal>

using namespace Host::Processing;
using namespace Host::Device;
using namespace Host::Models;

namespace {

[[nodiscard]] uint8_t toProtocolByte(int value) {
    Q_ASSERT(value >= 0 && value <= 255);
    return static_cast<uint8_t>(value);
}

} // namespace

namespace Host::Core {

AppController::AppController(std::unique_ptr<IConnectionScanner> connectionScanner,
                             std::unique_ptr<DeviceSession> deviceSession,
                             std::unique_ptr<AudioFrameWorker> audioWorker,
                             QObject* parent)
    : QObject(parent)
{
    m_deviceSession = deviceSession.release();
    m_audioWorker = audioWorker.release();
    m_connectionScanner = connectionScanner.release();

    m_workerThread.adopt(m_deviceSession);
    m_workerThread.adopt(m_audioWorker);
    m_workerThread.adopt(m_connectionScanner);

    connect(m_audioWorker, &AudioFrameWorker::spectrumReady, this, &AppController::spectrumReady);
    connect(m_deviceSession, &DeviceSession::manifestReady, this, &AppController::manifestReady);
    connect(m_deviceSession, &DeviceSession::audioFrameReady, m_audioWorker, &AudioFrameWorker::processFrame);
    connect(m_deviceSession, &DeviceSession::controlPacketReady, this, &AppController::onControlPacketReady);
    connect(m_deviceSession, &DeviceSession::connectionStatusChanged,
            this, &AppController::onConnectionStatusChanged);
    connect(m_deviceSession, &DeviceSession::errorOccurred, this, &AppController::errorOccurred);
    connect(m_connectionScanner, &IConnectionScanner::connectionsUpdated,
            this, &AppController::connectionsUpdated);

    m_workerThread.start();
    QMetaObject::invokeMethod(m_connectionScanner, &IConnectionScanner::start, Qt::QueuedConnection);
}

AppController::~AppController() = default;

void AppController::connectDevice(const QString& connectionId) {
    QMetaObject::invokeMethod(m_deviceSession, &DeviceSession::open, Qt::QueuedConnection, connectionId);
}

void AppController::disconnectDevice() {
    QMetaObject::invokeMethod(m_deviceSession, &DeviceSession::close, Qt::QueuedConnection);
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

void AppController::onConnectionStatusChanged(bool connected, const QString& portName) {
    m_isDeviceConnected = connected;
    emit connectionChanged(connected, portName);

    if (connected) {
        QMetaObject::invokeMethod(m_connectionScanner, &IConnectionScanner::stop, Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(m_connectionScanner, &IConnectionScanner::start, Qt::QueuedConnection);
    }
}

void AppController::onControlPacketReady(const Protocol::ControlPacket& pkt) {
    using ::Protocol::Command;

    switch (pkt.command) {
    case Command::SetParam:
        emit slotParameterUpdated(static_cast<int>(pkt.slotId),
                                  static_cast<int>(pkt.paramId),
                                  pkt.value());
        break;
    case Command::SetEffectType:
        emit slotEffectTypeUpdated(static_cast<int>(pkt.slotId),
                                   static_cast<int>(pkt.effectTypeId));
        break;
    case Command::BypassToggle:
        emit slotBypassUpdated(static_cast<int>(pkt.slotId), pkt.value() > 0.5f);
        break;
    default:
        break;
    }
}

void AppController::dispatchControlPacket(::Protocol::ControlPacket pkt) {
    pkt.applyCRC();
    QMetaObject::invokeMethod(m_deviceSession, &DeviceSession::sendControlPacket, Qt::QueuedConnection, pkt);
}

} // namespace Host::Core
